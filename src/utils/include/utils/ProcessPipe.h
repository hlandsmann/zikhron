#pragma once

#include <string>
#include <vector>

#include <sys/types.h>

namespace utl {
class ProcessPipe
{
public:
    ProcessPipe(const std::string& command, const std::vector<std::string>& args);
    ProcessPipe(const ProcessPipe&) = delete;
    ~ProcessPipe();
    auto operator=(ProcessPipe&& other) noexcept -> ProcessPipe& = delete;
    auto operator=(const ProcessPipe&) -> ProcessPipe& = delete;

    ProcessPipe(ProcessPipe&& other) noexcept
        : pid(other.pid), parent_to_child{other.parent_to_child[0], other.parent_to_child[1]}, child_to_parent{other.child_to_parent[0], other.child_to_parent[1]}
    {
        other.pid = -1;
        other.parent_to_child[0] = other.parent_to_child[1] = -1;
        other.child_to_parent[0] = other.child_to_parent[1] = -1;
    }

    auto write(const std::string& data) -> bool;
    auto getChunk(int timeout_ms = 100) -> std::string;
    [[nodiscard]] auto isRunning() const -> bool;

private:
    void closePipes();
    pid_t pid = -1;
    int parent_to_child[2] = {-1, -1}; // Pipe for writing to child
    int child_to_parent[2] = {-1, -1}; // Pipe for reading from child
};

} // namespace utl
