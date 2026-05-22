#include "CpuMonitor.h"
#include "Utils.h"
#include <sstream>
#include <iostream>

void CpuMonitor::readCpuTimes(unsigned long long& idle, unsigned long long& total) {
    // Читаем первую строку /proc/stat: "cpu  user nice system idle iowait irq softirq steal"
    std::string stat;
    try {
        stat = Utils::readFile("/proc/stat");
    } catch (const Utils::FileReadError& e) {
        std::cerr << "CpuMonitor: " << e.what() << std::endl;
        idle = total = 0;
        throw; // пробрасываем, чтобы getUsage обработал
    }
    std::istringstream ss(stat);
    std::string line;
    std::getline(ss, line);
    std::istringstream lineSs(line);
    std::string cpu;
    unsigned long long user, nice, system, idle_, iowait, irq, softirq, steal;
    lineSs >> cpu >> user >> nice >> system >> idle_ >> iowait >> irq >> softirq >> steal;
    // idle = время, когда CPU не выполнял полезную работу (включая iowait)
    idle = idle_ + iowait;
    // total = всё время CPU (сумма всех полей)
    total = user + nice + system + idle_ + iowait + irq + softirq + steal;
}

double CpuMonitor::getUsage() noexcept {
    try {
        unsigned long long idle, total;
        readCpuTimes(idle, total);
        if (prevTotal == 0) {
            // Первый вызов — сохраняем базовые значения, возвращаем 0
            prevIdle = idle;
            prevTotal = total;
            return 0.0;
        }
        unsigned long long diffIdle = idle - prevIdle;
        unsigned long long diffTotal = total - prevTotal;
        double usage = (diffTotal - diffIdle) * 100.0 / diffTotal;
        prevIdle = idle;
        prevTotal = total;
        return usage;
    } catch (...) {
        // Любая ошибка (например, нечитаемый /proc/stat) не должна останавливать программу
        return 0.0;
    }
}
