#pragma once
class Lock{
public:
    virtual ~Lock() = default;
    virtual void lock() = 0;
    virtual void unlock() = 0;
};
