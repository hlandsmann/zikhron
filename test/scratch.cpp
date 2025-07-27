// #include <spdlog/spdlog.h>
//
// #include <cstring>
// #include <string>
//
// #include <errno.h>
// #include <stdio.h>
// #include <stdlib.h>
// #include <unistd.h>
//
// #define PIPE_READ 0
// #define PIPE_WRITE 1
//
// int createChild(const char* szCommand, char* const aArguments[], char* const aEnvironment[], const char* szMessage)
// {
//     int aStdinPipe[2];
//     int aStdoutPipe[2];
//     int nChild;
//     char nChar;
//     int nResult;
//
//     if (pipe(aStdinPipe) < 0) {
//         perror("allocating pipe for child input redirect");
//         return -1;
//     }
//     if (pipe(aStdoutPipe) < 0) {
//         close(aStdinPipe[PIPE_READ]);
//         close(aStdinPipe[PIPE_WRITE]);
//         perror("allocating pipe for child output redirect");
//         return -1;
//     }
//
//     nChild = fork();
//     if (0 == nChild) {
//         // child continues here
//
//         // redirect stdin
//         if (dup2(aStdinPipe[PIPE_READ], STDIN_FILENO) == -1) {
//             exit(errno);
//         }
//
//         // redirect stdout
//         if (dup2(aStdoutPipe[PIPE_WRITE], STDOUT_FILENO) == -1) {
//             exit(errno);
//         }
//
//         // redirect stderr
//         if (dup2(aStdoutPipe[PIPE_WRITE], STDERR_FILENO) == -1) {
//             exit(errno);
//         }
//
//         // all these are for use by parent only
//         close(aStdinPipe[PIPE_READ]);
//         close(aStdinPipe[PIPE_WRITE]);
//         close(aStdoutPipe[PIPE_READ]);
//         close(aStdoutPipe[PIPE_WRITE]);
//
//         chdir("/home/harmen/zikhron/sudachi-0.7.5/");
//
//         // run child process image
//         // replace this with any exec* function find easier to use ("man exec")
//         nResult = execve(szCommand, aArguments, aEnvironment);
//         spdlog::info("exec {}", szCommand);
//
//         // if we get here at all, an error occurred, but we are in the child
//         // process, so just exit
//         exit(nResult);
//         spdlog::info("error {}", szCommand);
//     } else if (nChild > 0) {
//         // parent continues here
//
//         // close unused file descriptors, these are for child only
//         close(aStdinPipe[PIPE_READ]);
//         close(aStdoutPipe[PIPE_WRITE]);
//         // sleep(1);
//
//         // Include error check here
//         if (NULL != szMessage) {
//             spdlog::info("Hello World: {}", szMessage);
//             write(aStdinPipe[PIPE_WRITE], szMessage, strlen(szMessage));
//         }
//         sleep(1);
//         spdlog::info("slept");
//
//         std::string response;
//         // Just a char by char read here, you can change it accordingly
//         while (read(aStdoutPipe[PIPE_READ], &nChar, 1) == 1) {
//             write(STDOUT_FILENO, &nChar, 1);
//             response += nChar;
//         }
//         spdlog::info("done, msg: {}", response);
//
//         // done with these in this example program, you would normally keep these
//         // open of course as long as you want to talk to the child
//         close(aStdinPipe[PIPE_WRITE]);
//         close(aStdoutPipe[PIPE_READ]);
//     } else {
//         // failed to create child
//         close(aStdinPipe[PIPE_READ]);
//         close(aStdinPipe[PIPE_WRITE]);
//         close(aStdoutPipe[PIPE_READ]);
//         close(aStdoutPipe[PIPE_WRITE]);
//     }
//     return nChild;
// }
//
// int main(int argc, char* argv_[])
// {
//     spdlog::info("Hello {:03d}", 3);
//     char* const* aEnv = environ; //{nullptr};
//     // char* argv[] = {"-jar", "/home/harmen/zikhron/sudachi-0.7.5/sudachi-0.7.5.jar", nullptr};
//     char* argv[] = {"", "-jar", "sudachi-0.7.5.jar", nullptr};
//     // char* argv[] = {"sudachi-0.7.5.jar", nullptr};
//     // /usr/bin/java --jar /home/harmen/zikhron/sudachi-0.7.4/sudachi-0.7.4.jar
//     std::string text1 = "投降してほしけりゃてめえをあと百万体呼んで来るんだな\n";
//     // std::string text2 = "あとでジュース おごってあげるね - 暑そうだから";
//     // return createChild("/usr/bin/mecab", argv_, aEnv,text1.c_str());
//     return createChild("/usr/bin/java", argv, aEnv, text1.c_str());
//     // return createChild("/home/harmen/src/zikhron/build/test/sr_treewalker", argv_, aEnv, "hello world");
//     // return createChild("/home/harmen/src/zikhron/build/test/callee", argv, aEnv, "hello world");
//     // return createChild("/usr/bin/ls", argv, aEnv, "hello world");
//     // return createChild("/usr/bin/printenv", argv_, aEnv, "");
// }
//
// // auto main() -> int
// // {
// //     std::string text1 = "投降してほしけりゃてめえをあと百万体呼んで来るんだな";
// //     createChild("mecab", nullptr, nullptr, text1.c_str());
// //
// //     return 0;
// #pragma once
#include <array>
#include <cstring>
#include <iostream>
#include <memory>
#include <stdexcept>
#include <string>
#include <vector>

#include <fcntl.h>
#include <poll.h>
#include <sys/wait.h>
#include <unistd.h>

class ProcessPipe
{
public:
    // Constructor: Start the external process with given command and arguments
    ProcessPipe(const std::string& command, const std::vector<std::string>& args)
    {
        // Create pipes for parent-to-child (write) and child-to-parent (read)
        if (pipe2(parent_to_child, O_CLOEXEC) == -1 || pipe2(child_to_parent, O_CLOEXEC) == -1) {
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
                _exit(EXIT_FAILURE);
            }
            // Redirect stdout to child_to_parent[1]
            if (dup2(child_to_parent[1], STDOUT_FILENO) == -1) {
                _exit(EXIT_FAILURE);
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
    }

    // Destructor: Clean up resources
    ~ProcessPipe()
    {
        closePipes();
        if (pid > 0) {
            kill(pid, SIGTERM);
            waitpid(pid, nullptr, 0);
        }
    }

    // Write data to the process's stdin
    bool write(const std::string& data)
    {
        if (parent_to_child[1] == -1) {
            return false;
        }
        ssize_t bytes_written = ::write(parent_to_child[1], data.data(), data.size());
        return bytes_written == static_cast<ssize_t>(data.size());
    }

    // Read data from the process's stdout with timeout (milliseconds)
    std::string read(int timeout_ms = 1000)
    {
        if (child_to_parent[0] == -1) {
            return "";
        }

        pollfd pfd = {child_to_parent[0], POLLIN, 0};
        int ret = poll(&pfd, 1, timeout_ms);

        if (ret <= 0) {
            return ""; // Timeout or error
        }

        std::string result;
        char buffer[4096];
        ssize_t bytes_read = ::read(child_to_parent[0], buffer, sizeof(buffer) - 1);
        if (bytes_read > 0) {
            buffer[bytes_read] = '\0';
            result = buffer;
        }
        return result;
    }

    // Check if the process is still running
    bool isRunning() const
    {
        if (pid <= 0)
            return false;
        int status;
        pid_t result = waitpid(pid, &status, WNOHANG);
        return result == 0;
    }

    // Delete copy constructor and assignment operator
    ProcessPipe(const ProcessPipe&) = delete;
    ProcessPipe& operator=(const ProcessPipe&) = delete;

    // Move constructor
    ProcessPipe(ProcessPipe&& other) noexcept
        : pid(other.pid), parent_to_child{other.parent_to_child[0], other.parent_to_child[1]}, child_to_parent{other.child_to_parent[0], other.child_to_parent[1]}
    {
        other.pid = -1;
        other.parent_to_child[0] = other.parent_to_child[1] = -1;
        other.child_to_parent[0] = other.child_to_parent[1] = -1;
    }

    // Move assignment operator
    ProcessPipe& operator=(ProcessPipe&& other) noexcept
    {
        if (this != &other) {
            closePipes();
            if (pid > 0) {
                kill(pid, SIGTERM);
                waitpid(pid, nullptr, 0);
            }
            pid = other.pid;
            parent_to_child[0] = other.parent_to_child[0];
            parent_to_child[1] = other.parent_to_child[1];
            child_to_parent[0] = other.child_to_parent[0];
            child_to_parent[1] = other.child_to_parent[1];
            other.pid = -1;
            other.parent_to_child[0] = other.parent_to_child[1] = -1;
            other.child_to_parent[0] = other.child_to_parent[1] = -1;
        }
        return *this;
    }

private:
    pid_t pid = -1;
    int parent_to_child[2] = {-1, -1}; // Pipe for writing to child
    int child_to_parent[2] = {-1, -1}; // Pipe for reading from child

    void closePipes()
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
};

// Example usage:
int main()
{
    try {
        // Example: Start a process running 'bc' (basic calculator)
        ProcessPipe proc("bc", {});

        // Write a calculation
        proc.write("2 + 2\n");

        // Read the result
        std::string result = proc.read();
        std::cout << "Result: " << result << std::endl;

        proc.write("3 + 2\n");
        result = proc.read();
        std::cout << "Result: " << result << std::endl;

    std::string text1 = "投降してほしけりゃてめえをあと百万体呼んで来るんだな\n";
    // std::string text2 = "あとでジュース おごってあげるね - 暑そうだから";
//     // return createChild("/usr/bin/mecab", argv_, aEnv,text1.c_str());
        // Create another instance for a different process
        // ProcessPipe proc2("grep", {"-i", "test"});
        ProcessPipe proc2("mecab", {});
        proc2.write(text1);
        std::string result2 = proc2.read();
        std::cout << "Grep result: " << result2 << std::endl;
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return 1;
    }
    return 0;
}

// // }
