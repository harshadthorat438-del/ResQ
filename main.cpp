#include <iostream>
#include <csignal>
#include "api/ApiServer.h"

static std::unique_ptr<api::ApiServer> g_server;

void signalHandler(int signum) {
    std::cout << "\n[SIGNAL] Received signal " << signum << ". Shutting down server gracefully...\n";
    if (g_server) {
        g_server->stop();
    }
}

int main(int argc, char* argv[]) {
    int port = 8080;
    if (argc > 1) {
        try {
            port = std::stoi(argv[1]);
        } catch (...) {
            port = 8080;
        }
    }

    std::signal(SIGINT, signalHandler);
    std::signal(SIGTERM, signalHandler);

    try {
        g_server = std::make_unique<api::ApiServer>(port);
        g_server->loadPersistedData();
        g_server->start();
    } catch (const std::exception& e) {
        std::cerr << "[FATAL ERROR] " << e.what() << "\n";
        return 1;
    }

    return 0;
}
