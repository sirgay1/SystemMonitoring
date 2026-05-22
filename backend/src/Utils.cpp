#include "Utils.h"
#include <fstream>
#include <sstream>
#include <unistd.h>

namespace Utils {

    std::string readFile(const std::string& path) {
        std::ifstream file(path);
        if (!file.is_open()) {
            throw FileReadError(path);
        }
        std::stringstream buffer;
        buffer << file.rdbuf();
        return buffer.str();
    }

    std::vector<std::string> split(const std::string& s, char delim) {
        std::vector<std::string> result;
        std::stringstream ss(s);
        std::string item;
        while (std::getline(ss, item, delim)) {
            if (!item.empty()) result.push_back(item);
        }
        return result;
    }

    long long getSysClockTicks() noexcept {
        return sysconf(_SC_CLK_TCK);
    }

    long long getPageSize() noexcept {
        return sysconf(_SC_PAGESIZE);
    }

    unsigned long long getSystemUptimeTicks() {
        // Читаем /proc/uptime, первое число — аптайм в секундах
        std::string content = readFile("/proc/uptime");
        double uptime_sec;
        std::stringstream(content) >> uptime_sec;
        // Переводим в такты (user-space clock ticks)
        return static_cast<unsigned long long>(uptime_sec * getSysClockTicks());
    }

}