#include "WebServer.h"
#include "CpuMonitor.h"
#include "MemoryMonitor.h"
#include "ProcessList.h"
#include "httplib.h"
#include "json.hpp"
#include <fstream>
#include <sstream>
#include <iostream>

using json = nlohmann::json;

static CpuMonitor cpuMon;
static MemoryMonitor memMon;
static ProcessList procList;

static std::string getStatsJson() {
    double cpu = cpuMon.getUsage();
    std::cout << "CPU usage: " << cpu << std::endl;  // отладка
    json j;
    j["cpu_usage"] = cpuMon.getUsage();
    j["memory"]["total_mb"] = memMon.getTotalMemory() / 1024.0;
    j["memory"]["available_mb"] = memMon.getAvailableMemory() / 1024.0;
    j["memory"]["percent"] = memMon.getUsagePercent();

    auto processes = procList.getProcesses();
    json procsArray = json::array();
    for (const auto& p : processes) {
        json proc;
        proc["pid"] = p.pid;
        proc["name"] = p.name;
        proc["cpu"] = p.cpuPercent;
        proc["mem"] = p.memPercent;
        procsArray.push_back(proc);
    }
    j["processes"] = procsArray;
    return j.dump();
}

// Чтение статических файлов из папки ../frontend/dist
static std::string readStaticFile(const std::string& relativePath) {
    std::string path = "../frontend/dist/" + relativePath;
    std::ifstream file(path);
    if (!file.is_open()) {
        // fallback: если запуск из другой директории
        path = "frontend/dist/" + relativePath;
        file.open(path);
        if (!file.is_open()) return "";
    }
    std::stringstream buffer;
    buffer << file.rdbuf();
    return buffer.str();
}

void runWebServer() {
    httplib::Server svr;

    svr.Get("/", [](const httplib::Request&, httplib::Response& res) {
        std::string html = readStaticFile("index.html");
        if (html.empty()) {
            res.status = 404;
            res.set_content("index.html not found", "text/plain");
        } else {
            res.set_content(html, "text/html");
        }
    });

    svr.Get("/style.css", [](const httplib::Request&, httplib::Response& res) {
        std::string css = readStaticFile("style.css");
        if (css.empty()) res.status = 404;
        else res.set_content(css, "text/css");
    });

    svr.Get("/main.js", [](const httplib::Request&, httplib::Response& res) {
        std::string js = readStaticFile("main.js");
        if (js.empty()) res.status = 404;
        else res.set_content(js, "application/javascript");
    });

    svr.Get("/api/stats", [](const httplib::Request&, httplib::Response& res) {
        std::string json = getStatsJson();
        res.set_content(json, "application/json");
    });

    std::cout << "Server listening on http://localhost:8080\n";
    svr.listen("0.0.0.0", 8080);
}