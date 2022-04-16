#include <Property.h>
namespace utl {
PropertyServer::PropertyServer(const std::function<void()>& _signal_update)
    : signal_update(_signal_update) {}

void PropertyServer::updateProperties() {}

void PropertyServer::emitUpdate() { signal_update(); }

PropertyBase::PropertyBase(const std::shared_ptr<PropertyServer>& _server) : server(_server) {
    handle = std::make_shared<PropertyHandle>(*this);
    server->handles.push_back(handle);
}

}  // namespace utl