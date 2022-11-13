#include <Property.h>
#include <spdlog/spdlog.h>
namespace utl {

std::unique_ptr<PropertyServer> propertyServer;

auto PropertyServer::get() -> PropertyServer& {
    if (!propertyServer)
        propertyServer = std::unique_ptr<PropertyServer>(new PropertyServer());
    return *propertyServer;
}

void PropertyServer::setSignalUpdate(const std::function<void()>& _signal_update) {
    signal_update = _signal_update;
}

void PropertyServer::updateProperties() {
    handles.remove_if([](const std::weak_ptr<PropertyHandle>& handle) {
        std::shared_ptr<PropertyHandle> propertyHandle = handle.lock();
        if (not propertyHandle) {
            spdlog::warn("Remove Property ------------------");
            return true;
        }
        auto& property = propertyHandle->propertyBase;
        property.updateObservers();
        return false;
    });
}

void PropertyServer::emitUpdate() {
    if (signal_update)
        signal_update();
}

auto ObserverBase::getHandle() const -> const std::weak_ptr<PropertyHandle> { return handle; }

void ObserverCollection::push(const std::shared_ptr<ObserverBase>& observer) {
    observers.remove_if([](const std::shared_ptr<ObserverBase> _observer) {
        if (_observer->getHandle().lock() == nullptr)
            spdlog::warn("Remove Observer ------------------");
        return _observer->getHandle().lock() == nullptr;
    });
    observers.push_back(observer);
}

PropertyBase::PropertyBase() {
    handle = std::make_shared<PropertyHandle>(*this);
    PropertyServer::get().handles.push_back(handle);
}

void PropertyBase::emitUpdate() { PropertyServer::get().emitUpdate(); }

}  // namespace utl