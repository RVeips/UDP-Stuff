#include "Log.hpp"
#include <argparse/argparse.hpp>
#include <chrono>
#include <thread>
#ifdef _WIN32
/* See http://stackoverflow.com/questions/12765743/getaddrinfo-on-win32 */
#ifndef _WIN32_WINNT
#define _WIN32_WINNT 0x0501 /* Windows XP. */
#endif
#include <winsock2.h>
#include <Ws2tcpip.h>
#else
/* Assume that any non-Windows platform uses POSIX-style sockets instead. */
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netdb.h>  /* Needed for getaddrinfo() and freeaddrinfo() */
#include <unistd.h> /* Needed for close() */
#include <pthread.h>
#endif

int g_sacn_socket;

// set up UDP socket to send data to 2.255.255.255 on UDP port 5568
void sACN_CreateSocket() {
    // Create a UDP socket
    g_sacn_socket = socket(AF_INET, SOCK_DGRAM, 0);
    if (g_sacn_socket < 0) {
        Log.error("Failed to create socket");
        return;
    }

    int broadcastEnable = 1;
    int ret             = setsockopt(g_sacn_socket, SOL_SOCKET, SO_BROADCAST, &broadcastEnable, sizeof(broadcastEnable));
    if (ret < 0) {
        Log.error("Failed to set socket to broadcast mode");
        return;
    }
}

void sACN_CloseSocket() {
    auto status = shutdown(g_sacn_socket, SHUT_RDWR);
    if (status == 0) {
        status = close(g_sacn_socket);
    }
}

bool sACN_SendPacket(void *data, int len, uint32_t destination) {
    struct sockaddr_in addr;
    addr.sin_family      = AF_INET;
    addr.sin_port        = htons(5568);
    addr.sin_addr.s_addr = htonl(destination);
    return sendto(g_sacn_socket, data, len, 0, (struct sockaddr *)&addr, sizeof(addr)) == 0;
}

void sACN_Generator() {
    sACN_CreateSocket();

    uint8_t data[1024];
    while (1 < 2) {
        std::this_thread::sleep_for(std::chrono::milliseconds(25));
        for (int i = 0; i < 1024; i++) {
            data[i] = 0;
        }
        sACN_SendPacket(data, sizeof(data), 0x02FFFFFF);
        std::this_thread::sleep_for(std::chrono::milliseconds(1000));
        for (int i = 0; i < 1024; i++) {
            data[i] = 0xFF;
        }
        sACN_SendPacket(data, sizeof(data), 0x02FFFFFF);
    }

    sACN_CloseSocket();
}

int main(int argc, char **argv) {
    const auto version_string = std::to_string(CFXS_BUILD_VERSION_MAJOR) + "." + std::to_string(CFXS_BUILD_VERSION_MINOR);
    argparse::ArgumentParser args("UDP Stuff", version_string);

    try {
        args.parse_args(argc, argv);
    } catch (const std::runtime_error &err) {
        Log.error("{}", err.what());
        std::cout << args.usage() << std::endl;
        return 1;
    }

    // try {
    //     if (args["-t"] == true) {
    //         s_log_trace = true;
    //     }
    // } catch (const std::exception &e) {
    // }
    initialize_logging();

    Log.info("UDP Stuff v{}", version_string);

// set this thread to high prio
#ifdef _WIN32
#else
    // set scheduler to FIFO and set max prio
    sched_param sch_params;
    sch_params.sched_priority = sched_get_priority_max(SCHED_FIFO);
    int policy                = SCHED_FIFO;
    auto ret                  = pthread_setschedparam(pthread_self(), policy, &sch_params);
    if (ret != 0) {
        Log.error("Failed to set RT priority");
    }
#endif

    sACN_Generator();

    return 0;
}
