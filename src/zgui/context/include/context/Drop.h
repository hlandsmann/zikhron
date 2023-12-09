#pragma once
#include <spdlog/spdlog.h>

#include <utility>
namespace context {

template<class DropImpl>
class Drop
{
public:
    Drop() = default;
    virtual ~Drop()
    {
        while (countVars != 0) {
            DropImpl::pop();
            countVars--;
        }
    };

    Drop(const Drop&) = delete;
    Drop(Drop&& other) noexcept
        : countVars{std::exchange(other.countVars, 0)} {}
    auto operator=(const Drop&) -> Drop& = delete;
    auto operator=(Drop&& other) noexcept -> Drop&
    {
        countVars = std::exchange(other.countVars, 0);
        return *this;
    }

protected:
    void incPopCount()
    {
        countVars++;
    }

private:
    int countVars{0};
};
} // namespace context
