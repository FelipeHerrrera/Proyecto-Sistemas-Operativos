#pragma once
#include <mutex>

class tenedor {
public:
    tenedor() = default;
    // No copiar ni mover tenedors
    tenedor(const tenedor&) = delete;
    tenedor& operator=(const tenedor&) = delete;

    std::mutex& getMutex() { return mtx_; }

private:
    std::mutex mtx_;
};
