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
#include <CFXS/IPv4.hpp>

int g_sacn_socket;

// set up UDP socket to send data to 2.255.255.255 on UDP port 5568
void sacn_create_socket() {
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

void sacn_close_socket() {
    auto status = shutdown(g_sacn_socket, SHUT_RDWR);
    if (status == 0) {
        status = close(g_sacn_socket);
    }
}

bool sacn_send_packet(void *data, int len, CFXS::IPv4 destination_address) {
    sockaddr_in addr;
    addr.sin_family      = AF_INET;
    addr.sin_port        = htons(5568);
    addr.sin_addr.s_addr = destination_address.get_value();
    return sendto(g_sacn_socket, data, len, 0, (sockaddr *)&addr, sizeof(addr)) == 0;
}

void sacn_generator() {
    sacn_create_socket();

    uint8_t data[1024];
    int n = 0;
    while (1 < 2) {
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
        for (int i = 0; i < 1024; i++) {
            data[i] = 0;
        }
        sacn_send_packet(data, sizeof(data), "2.255.255.255");
        std::this_thread::sleep_for(std::chrono::milliseconds(10));
        for (int i = 0; i < 1024; i++) {
            data[i] = (i - (0xA8 - 42)) == n ? 0xFF : 0x00;
        }
        n++;
        sacn_send_packet(data, sizeof(data), "2.255.255.255");
        if (n == 24)
            n = 0;
    }

    sacn_close_socket();
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

    sacn_generator();

    return 0;
}
