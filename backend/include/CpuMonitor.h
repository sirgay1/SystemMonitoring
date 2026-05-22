#ifndef CPUMONITOR_H
#define CPUMONITOR_H

// Класс для мониторинга загрузки CPU (агрегированной по всем ядрам)
class CpuMonitor {
public:
    CpuMonitor() = default;
    ~CpuMonitor() = default;

    // Возвращает процент загрузки CPU с момента последнего вызова.
    // При ошибках возвращает 0.0 и не бросает исключений.
    double getUsage() noexcept;

private:
    unsigned long long prevIdle = 0;   // предыдущее значение idle (включая iowait)
    unsigned long long prevTotal = 0;  // предыдущее общее время CPU

    // Считывает из /proc/stat текущие значения idle и total
    void readCpuTimes(unsigned long long& idle, unsigned long long& total);
};

#endif
