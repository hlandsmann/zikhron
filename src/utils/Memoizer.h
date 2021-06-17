#pragma once

namespace utl {

/* Single_memoinize is a memoinizer - but for single values. This means, that if you calculate a value
 * from the same key multiple times right after each other, the value is calculated only once. Once you
 * choose a different key, the value is lost
 *
 * This can be useful, if you process a few variables (in a loop), and may access them multiple times or
 * never. You can ommit the test, whether the value was calculated. Save some lines of code! */
template <class Key, class Value> class Single_memoinize {
    Value (*fun)(Key);
    Key key;
    Value value;

public:
    Single_memoinize(Value(_fun)(Key), Key _key = Key{}, Value _value = Value{})
        : fun(_fun), key(_key), value(_value) {}
    auto evaluate(Key _key) {
        if (key == _key)
            return value;
        value = fun(_key);
        key = _key;
        return value;
    }
};

}  // namespace utl
