#ifndef UTILS_H
#define UTILS_H

#include <string>
#include <vector>
#include <stdexcept>

namespace Utils {
    // Исключение, которое бросается при невозможности прочитать системный файл
    class FileReadError : public std::runtime_error {
    public:
        explicit FileReadError(const std::string& path)
            : std::runtime_error("Failed to read file: " + path) {}
    };

    // Читает весь файл в строку. Бросает FileReadError при ошибке.
    std::string readFile(const std::string& path);

    // Разбивает строку на части по разделителю (убирает пустые)
    std::vector<std::string> split(const std::string& s, char delim);

    // Возвращает количество тактов ядра в секунду (sysconf(_SC_CLK_TCK))
    long long getSysClockTicks() noexcept;

    // Возвращает размер страницы памяти в байтах
    long long getPageSize() noexcept;

    // Возвращает время работы системы в тактах (с момента загрузки)
    // Необходимо для вычисления CPU% процессов
    unsigned long long getSystemUptimeTicks();
}

#endif