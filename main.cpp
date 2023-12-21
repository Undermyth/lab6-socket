#include "client.h"
#include "server.h"
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <iostream>
#include <csignal>

enum Mode {
    CLIENT,
    SERVER,
    INVALID
};

// to exit the server main loop
void SigHandler(int sig) {
    exit(0);
}

int main(int argc, char* argv[]) {

    // cmd line option processing
    Mode mode = INVALID;
    int port = 6335;
    for (int cur = 1; cur < argc; /* nop */) {
        if (!strcmp(argv[1], "--help") or !strcmp(argv[1], "-h")) {
            cout << \
            "Options: " << endl <<
            "--help, -h                     show this help message" << endl <<
            "--mode, -m [client/server]     act as client or server" << endl <<
            "--port, -p [port]              set the port number" << endl <<
            "In the client mode, you can use command `menu` to get help." << endl <<
            "port option is ignored in client mode." << endl <<
            "In the server mode, you can use CTRL+D to exit" << endl;
            exit(0);
        }
        else if (!strcmp(argv[cur], "--mode") or !strcmp(argv[cur], "-m")) {
            cur++;
            if (!strcmp(argv[cur], "client")) {
                mode = CLIENT;
            }
            else if (!strcmp(argv[cur], "server")) {
                mode = SERVER;
            }
            else {
                cout << "Unrecognized client mode. Use --help to get help." << endl;
                exit(1);
            }
            cur++;
        } 
        else if (!strcmp(argv[cur], "--port") or !strcmp(argv[cur], "-p")) {
            cur++;
            port = atoi(argv[cur]);
            cur++;
        }
        else {
            cout << "Unrecognized option. Use --help to get help." << endl;
            exit(1);
        }
    }
    if (mode == INVALID) {
        cout << "Mode is not specified. Use --help to get help." << endl;
        exit(1);
    }
    if (mode == SERVER and (port > 65535 or port < 0)) {
        cout << "Illegal port number." << endl;
        exit(1);
    }
    else if (mode == SERVER and port <= 1024) {
        cout << "Warning: the port number is too small (<1024)." << endl;
    }

    // register exit signal for server
    signal(SIGINT, SigHandler);

    // start client or server
    if (mode == CLIENT) {
        AppClient client(port);
        client.UserLoop();
    }
    else if (mode == SERVER) {
        AppServer server(port);
        server.MainLoop();
    }
}