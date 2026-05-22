#include "ProcessList.h"
#include "Utils.h"
#include "MemoryMonitor.h"
#include <dirent.h>
#include <cstdlib>
#include <algorithm>
#include <sstream>
#include <cmath>
#include <unistd.h>
#include <iostream>

unsigned long long ProcessList::getTotalCpuTime() const {
    // Суммируем все поля первой строки /proc/stat, кроме "cpu"
    std::string stat = Utils::readFile("/proc/stat");
    std::istringstream ss(stat);
    std::string line;
    std::getline(ss, line);
    std::istringstream lineSs(line);
    std::string cpu;
    unsigned long long user, nice, system, idle, iowait, irq, softirq, steal;
    lineSs >> cpu >> user >> nice >> system >> idle >> iowait >> irq >> softirq >> steal;
    return user + nice + system + idle + iowait + irq + softirq + steal;
}

bool ProcessList::readProcStat(int pid, unsigned long long& utime,
                               unsigned long long& stime,
                               unsigned long long& starttime) const {
    try {
        std::string path = "/proc/" + std::to_string(pid) + "/stat";
        std::string content = Utils::readFile(path);
        // Формат: pid (command) state ... utime stime ... starttime
        // Ищем закрывающую скобку после имени команды
        size_t last_close = content.rfind(')');
        if (last_close == std::string::npos) return false;
        std::string after_name = content.substr(last_close + 2); // пропускаем ") "
        std::istringstream iss(after_name);
        std::string tmp;
        // Пропускаем: state, ppid, pgrp, session, tty_nr, tpgid, flags, minflt, cminflt, majflt, cmajflt
        for (int i = 0; i < 11; ++i) iss >> tmp;
        iss >> utime >> stime;                     // время в user и kernel mode
        for (int i = 0; i < 4; ++i) iss >> tmp;    // cutime, cstime, priority, nice
        iss >> starttime;                          // время старта процесса (в тактах)
        return true;
    } catch (const Utils::FileReadError&) {
        return false; // процесс мог завершиться, игнорируем
    }
}

std::string ProcessList::getProcessName(int pid) const {
    try {
        std::string path = "/proc/" + std::to_string(pid) + "/comm";
        std::string name = Utils::readFile(path);
        if (!name.empty() && name.back() == '\n') name.pop_back();
        return name;
    } catch (const Utils::FileReadError&) {
        return "";
    }
}

unsigned long long ProcessList::getProcessMemoryKB(int pid) const {
    try {
        // statm: size resident share text lib data dt
        std::string path = "/proc/" + std::to_string(pid) + "/statm";
        std::string content = Utils::readFile(path);
        std::istringstream iss(content);
        unsigned long long size, resident, share;
        iss >> size >> resident >> share;
        long pageSize = Utils::getPageSize();
        // resident — число страниц в физической памяти (RSS)
        return (resident * pageSize) / 1024; // в KB
    } catch (const Utils::FileReadError&) {
        return 0;
    }
}

std::vector<ProcessInfo> ProcessList::getProcesses() noexcept {
    std::vector<ProcessInfo> processes;
    unsigned long long uptimeTicks;
    try {
        uptimeTicks = Utils::getSystemUptimeTicks();
    } catch (...) {
        return processes; // не можем вычислить время жизни процессов
    }

    DIR* dir = opendir("/proc");
    if (!dir) return processes;

    struct dirent* entry;
    while ((entry = readdir(dir)) != nullptr) {
        if (entry->d_type != DT_DIR) continue;
        int pid = atoi(entry->d_name);
        if (pid == 0) continue; // пропускаем "." и ".."

        unsigned long long utime, stime, starttime;
        if (!readProcStat(pid, utime, stime, starttime)) continue;

        // Расчёт CPU%:
        // CPU% = (utime + stime) / (uptime - starttime) * 100
        // utime+stime — время, которое процесс провёл в CPU (в тактах)
        // uptime - starttime — время жизни процесса (в тактах)
        unsigned long long total_time = utime + stime;
        unsigned long long seconds = uptimeTicks - starttime;
        if (seconds == 0) seconds = 1; // защита от деления на ноль
        double cpuPercent = 100.0 * total_time / seconds;

        // Ограничиваем максимальным значением (100% * число ядер), хотя top показывает по-другому
        long numCpus = sysconf(_SC_NPROCESSORS_ONLN);
        if (cpuPercent > 100.0 * numCpus) cpuPercent = 100.0 * numCpus;

        // Память
        unsigned long long memKB = getProcessMemoryKB(pid);
        MemoryMonitor memMon;
        unsigned long long totalMemKB = memMon.getTotalMemory();
        double memPercent = totalMemKB ? (memKB * 100.0) / totalMemKB : 0.0;

        std::string name = getProcessName(pid);
        if (!name.empty()) {
            processes.push_back({pid, name, cpuPercent, memPercent});
        }
    }
    closedir(dir);

    // Сортировка по убыванию CPU
    std::sort(processes.begin(), processes.end(),
        [](const ProcessInfo& a, const ProcessInfo& b) {
            return a.cpuPercent > b.cpuPercent;
        });
    return processes;
}