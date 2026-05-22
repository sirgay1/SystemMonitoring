#include <iostream>
#include <cstring>
#include "WebServer.h"

int main(int argc, char* argv[]) {
    std::cout << "Starting web server on http://localhost:8080\n";
    runWebServer();
    return 0;
}