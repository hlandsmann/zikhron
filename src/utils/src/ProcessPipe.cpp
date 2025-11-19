#include "ProcessPipe.h"

#include "format.h"

#include <spdlog/spdlog.h>
#include <sys/poll.h>

#include <array>
#include <cerrno>
#include <csignal>
#include <cstdlib>
#include <cstring>
#include <memory>
#include <stdexcept>
#include <string>
#include <vector>

#include <fcntl.h>
#include <poll.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

namespace utl {
ProcessPipe::ProcessPipe(const std::string& command, const std::vector<std::string>& args)
{
    // Create pipes for parent-to-child (write) and child-to-parent (read)
    if (pipe2(static_cast<int*>(parent_to_child), O_CLOEXEC) == -1 || pipe2(static_cast<int*>(child_to_parent), O_CLOEXEC) == -1) {
        throw std::runtime_error("Failed to create pipes: " + std::string(std::strerror(errno)));
    }

    pid = fork();
    if (pid == -1) {
        closePipes();
        throw std::runtime_error("Failed to fork: " + std::string(std::strerror(errno)));
    }

    if (pid == 0) { // Child process
        // Redirect stdin from parent_to_child[0]
        if (dup2(parent_to_child[0], STDIN_FILENO) == -1) {
            exit(EXIT_FAILURE);
        }
        // Redirect stdout to child_to_parent[1]
        if (dup2(child_to_parent[1], STDOUT_FILENO) == -1) {
            exit(EXIT_FAILURE);
        }

        // Close all pipe ends in child
        close(parent_to_child[0]);
        close(parent_to_child[1]);
        close(child_to_parent[0]);
        close(child_to_parent[1]);

        // Prepare arguments for execvp
        std::vector<char*> c_args;
        c_args.reserve(args.size() + 2);
        c_args.push_back(const_cast<char*>(command.c_str()));
        for (const auto& arg : args) {
            c_args.push_back(const_cast<char*>(arg.c_str()));
        }
        c_args.push_back(nullptr);

        // Execute the command
        execvp(command.c_str(), c_args.data());
        _exit(EXIT_FAILURE); // If execvp fails
    }

    // Parent process: close unused pipe ends
    close(parent_to_child[0]);
    close(child_to_parent[1]);
    spdlog::info("ProcessPipe constructed, {} {}", command, fmt::join(args, " "));
}

// Destructor: Clean up resources
ProcessPipe::~ProcessPipe()
{
    closePipes();
    if (pid > 0) {
        kill(pid, SIGTERM);
        waitpid(pid, nullptr, 0);
    }
}

// Write data to the process's stdin
auto ProcessPipe::write(const std::string& data) -> bool
{
    if (parent_to_child[1] == -1) {
        return false;
    }
    ssize_t bytes_written = ::write(parent_to_child[1], data.data(), data.size());
    return bytes_written == static_cast<ssize_t>(data.size());
}

// Read data from the process's stdout with timeout (milliseconds)
auto ProcessPipe::getChunk(int timeout_ms) -> std::string
{
    if (child_to_parent[0] == -1) {
        return "";
    }

    pollfd pfd = {.fd = child_to_parent[0], .events = POLLIN, .revents = 0};
    int ret = poll(&pfd, 1, timeout_ms);

    if (ret <= 0) {
        return ""; // Timeout or error
    }

    std::string result;
    char nChar = 0;

    while (read(child_to_parent[0], &nChar, 1) == 1) {
        result += nChar;
        if (result.ends_with("EOS\n")) {
            break;
        }
    }
    return result;
}

// Check if the process is still running
auto ProcessPipe::isRunning() const -> bool
{
    int status = 0;
    return pid > 0 && waitpid(pid, &status, WNOHANG) == 0;
}

void ProcessPipe::closePipes()
{
    if (parent_to_child[1] != -1) {
        close(parent_to_child[1]);
        parent_to_child[1] = -1;
    }
    if (child_to_parent[0] != -1) {
        close(child_to_parent[0]);
        child_to_parent[0] = -1;
    }
}
} // namespace utl
