#pragma once
#include <mutex>

class tenedor {
public:
    tenedor() = default;
    // No copiable ni movible
    tenedor(const tenedor&) = delete;
    tenedor& operator=(const tenedor&) = delete;
    tenedor(tenedor&&) = delete;
    tenedor& operator=(tenedor&&) = delete;

    std::mutex& getMutex() { return mtx_; }

private:
    std::mutex mtx_;
};
