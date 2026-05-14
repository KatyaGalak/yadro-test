#pragma once

#include <cstdint>

class ITape {
public:
    virtual ~ITape() = default;

    virtual int32_t read() = 0;

    virtual void write(int32_t writeValue) = 0;

    virtual bool moveNext() = 0;
    virtual bool movePrevious() = 0;
    virtual void moveFirst() = 0;

    virtual bool isEnd() const = 0;
};