#include "MemoryMonitor.h"
#include "Utils.h"
#include <sstream>
#include <iostream>

unsigned long long MemoryMonitor::getTotalMemory() const noexcept {
    try {
        std::string content = Utils::readFile("/proc/meminfo");
        std::istringstream ss(content);
        std::string key;
        unsigned long long value;
        while (ss >> key >> value) {
            if (key == "MemTotal:") {
                cachedTotalMem = value;
                return value;
            }
        }
        return 0;
    } catch (...) {
        std::cerr << "MemoryMonitor: failed to read total memory" << std::endl;
        return cachedTotalMem; // возвращаем последнее известное значение
    }
}

unsigned long long MemoryMonitor::getAvailableMemory() const noexcept {
    try {
        std::string content = Utils::readFile("/proc/meminfo");
        std::istringstream ss(content);
        std::string key;
        unsigned long long value;
        while (ss >> key >> value) {
            if (key == "MemAvailable:") return value;
        }
        return 0;
    } catch (...) {
        std::cerr << "MemoryMonitor: failed to read available memory" << std::endl;
        return 0;
    }
}

double MemoryMonitor::getUsagePercent() const noexcept {
    unsigned long long total = getTotalMemory();
    unsigned long long available = getAvailableMemory();
    if (total == 0) return 0.0;
    return 100.0 * (total - available) / total;
}