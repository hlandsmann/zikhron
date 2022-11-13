#pragma once
#include <algorithm>
#include <functional>
#include <list>
#include <memory>

namespace utl {
class PropertyBase;
struct PropertyHandle {
    PropertyBase& propertyBase;
};

class PropertyServer {
    PropertyServer() = default;

public:
    static auto get() -> PropertyServer&;
    void setSignalUpdate(const std::function<void()>& signal_update);
    void updateProperties();

private:
    friend class PropertyBase;
    void emitUpdate();

    std::function<void()> signal_update;
    std::list<std::weak_ptr<PropertyHandle>> handles;
};

class ObserverBase {
public:
    ObserverBase() = default;
    virtual ~ObserverBase() = default;
    auto getHandle() const -> const std::weak_ptr<PropertyHandle>;
    // protected:
    //     virtual void onChanged() = 0;
protected:
    std::weak_ptr<PropertyHandle> handle;
};

template <class Value_t> class Property;
template <class Value_t> class Observer : public ObserverBase {
public:
    Observer(const std::function<void(const Value_t& value)>& _callback) : callback(_callback){};

private:
    friend class Property<Value_t>;
    auto isNewValue(const Value_t& value) { return value != lastValue; }
    void updateValue(const Value_t& value) {
        callback(value);
        lastValue = value;
    }
    // void onChanged(){};
    Value_t lastValue{};
    std::function<void(const Value_t& value)> callback;
};

class ObserverCollection {
public:
    void push(const std::shared_ptr<ObserverBase>& observer);

private:
    std::list<std::shared_ptr<ObserverBase>> observers;
};

class PropertyBase {
public:
    PropertyBase();
    virtual ~PropertyBase() = default;

protected:
    void emitUpdate();
    bool value_changed = false;
    std::shared_ptr<PropertyHandle> handle;

private:
    friend class PropertyServer;
    virtual void updateObservers() = 0;
};

template <class Value_t> class Property : public PropertyBase {
public:
    Property(Value_t _value) : value(_value){};
    Property() = default;
    Property<Value_t>& operator=(const Value_t& _value) {
        if (value != _value) {
            value = _value;
            emitUpdate();
            value_changed = true;
        }
        return *this;
    }

    operator Value_t() const { return value; }

    auto observe(const std::function<void(const Value_t&)>& onChangedCallback)
        -> std::shared_ptr<Observer<Value_t>> {
        auto observer = std::make_shared<Observer<Value_t>>(onChangedCallback);
        observer->handle = handle;
        newObservers.push_back(observer);

        value_changed = true;
        emitUpdate();

        return observer;
    }

private:
    void updateObservers() override {
        if (not value_changed)
            return;
        value_changed = false;
        auto tempObservers = std::move(newObservers);
        newObservers.clear();
        for (std::weak_ptr<Observer<Value_t>>& weakObserver : tempObservers) {
            auto observer = weakObserver.lock();
            if (observer) {
                observer->updateValue(value);
                observers.push_back(observer);
            }
        }
        observers.remove_if([this](const std::weak_ptr<Observer<Value_t>>& weakObserver) {
            auto observer = weakObserver.lock();
            if (not observer)
                return true;
            if (observer->isNewValue(value))
                observer->updateValue(value);
            return false;
        });
    }

    Value_t value{};
    std::list<std::weak_ptr<Observer<Value_t>>> newObservers;
    std::list<std::weak_ptr<Observer<Value_t>>> observers;
};
}  // namespace utl