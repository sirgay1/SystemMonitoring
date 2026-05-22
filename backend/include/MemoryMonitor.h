#ifndef MEMORYMONITOR_H
#define MEMORYMONITOR_H

// Класс для получения информации об использовании оперативной памяти
class MemoryMonitor {
public:
    MemoryMonitor() = default;
    ~MemoryMonitor() = default;

    // Общий объём ОЗУ в килобайтах (берётся из /proc/meminfo)
    unsigned long long getTotalMemory() const noexcept;

    // Доступно (не используется) в килобайтах (MemAvailable)
    unsigned long long getAvailableMemory() const noexcept;

    // Процент использования памяти (0-100)
    double getUsagePercent() const noexcept;

private:
    mutable unsigned long long cachedTotalMem = 0; // кеш для избежания повторного чтения
};

#endif