#pragma once

#include <cstdint>

class ITape {
public:
    virtual ~ITape() = default;

    virtual int32_t read() = 0;

    virtual void write(int32_t write_value) = 0;

    virtual bool move_next() = 0;
    virtual bool move_previous() = 0;
    virtual void move_first() = 0;

    virtual bool is_end() const = 0;
};