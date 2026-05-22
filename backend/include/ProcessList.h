#ifndef PROCESSLIST_H
#define PROCESSLIST_H

#include <vector>
#include <string>

// Структура для хранения информации об одном процессе
struct ProcessInfo {
    int pid;
    std::string name;
    double cpuPercent;   // процент использования CPU (относительно одного ядра)
    double memPercent;   // процент от общей памяти
};

// Класс для получения списка процессов с их загрузкой CPU и памяти
class ProcessList {
public:
    ProcessList() = default;
    ~ProcessList() = default;

    // Возвращает вектор процессов, отсортированный по убыванию cpuPercent
    // В случае ошибок возвращает пустой вектор (не бросает исключений)
    std::vector<ProcessInfo> getProcesses() noexcept;

private:
    unsigned long long prevTotalCpuTime = 0; // не используется, но оставлено для возможного расширения

    // Считывает общее время CPU из /proc/stat (сумма всех полей)
    unsigned long long getTotalCpuTime() const;

    // Читает /proc/<pid>/stat и извлекает utime, stime, starttime (в тактах)
    // Возвращает true при успехе, false при ошибке (процесс мог завершиться)
    bool readProcStat(int pid, unsigned long long& utime,
                      unsigned long long& stime,
                      unsigned long long& starttime) const;

    // Возвращает имя процесса из /proc/<pid>/comm
    std::string getProcessName(int pid) const;

    // Возвращает используемую процессом память в килобайтах (RSS)
    unsigned long long getProcessMemoryKB(int pid) const;
};

#endif