#pragma once
#include <functional>
#include <list>
#include <memory>
namespace utl {
class PropertyBase;
struct PropertyHandle {
    PropertyBase& propertyBase;
};

class PropertyServer {
public:
    PropertyServer(const std::function<void()>& signal_update);
    void updateProperties();

private:
    friend class PropertyBase;
    void emitUpdate();

    std::function<void()> signal_update;
    std::list<std::weak_ptr<PropertyHandle>> handles;
};

class PropertyBase {
public:
    PropertyBase(const std::shared_ptr<PropertyServer>& server);
    virtual ~PropertyBase() = default;

private:
    std::shared_ptr<PropertyHandle> handle;
    std::shared_ptr<PropertyServer> server;
};

template <class Value_t> class Property : public PropertyBase {
public:
    Property(const std::shared_ptr<PropertyServer>& server, Value_t _value);

private:
    Value_t value;
};
}  // namespace utl