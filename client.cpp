#include "client.h"
#include <chrono>
#include <cstdio>
#include <cstring>
#include <iostream>
#include <netinet/in.h>
#include <string>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <sstream>
#include <thread>
#include <unistd.h>

using namespace std;

AppClientCore::AppClientCore(int port): port(port) {
    to_exit = false;
}

const string AppClientCore::getMenu() const {
    string res = \
    "menu                               Get this window.\n"
    "connect [IP addr] [port]           Connect to server socket.\n"
    "close                              Close connection to server.\n"
    "get [time/name/client]             Request current time/server name/client list.\n"
    "sendto [client id] \"[message]\"     Send message to selected client.\n"
    "exit                               Exit the program.\n";
    return res;
}

bool AppClientCore::connect(const string& ip_addr, int port) {

    // in case that the client request to connect again after close,
    // socket should be assigned every time called connect
    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd == -1) {
        cerr << "AppClient: failed to create socket" << endl;
        ::exit(1);
    }

    sockaddr_in server_sock;
    server_sock.sin_family          = AF_INET;
    server_sock.sin_port            = htons(port);
    server_sock.sin_addr.s_addr     = inet_addr(ip_addr.c_str());
    int conn_res = ::connect(sockfd, (struct sockaddr*)&server_sock, sizeof(server_sock));
    if (conn_res == -1) {
        cerr << "AppClient: failed to connect; please check the server" << endl;
        return false;
    }
    cerr << "AppClient: connected to server " << ip_addr << ":" << port << endl;
    
    // connect is successful. start polling by creating a thread
    to_exit = false;
    polling_thread = thread(&AppClientCore::polling, this);
    return true;
}

void AppClientCore::polling() {
    while (1) {
        if (to_exit) break;
        memset(buffer, 0, sizeof(buffer));
        int res = recv(sockfd, buffer, MAX_BUFFER, 0);
        // if some data is received
        if (res > 0) {
            string msg(buffer);
            message_queue.push_back(msg);
            // print();
        }
    }
}

void AppClientCore::print() {
    for (auto& str : message_queue) {
        cout << str << endl;
    }
}

void AppClientCore::close() {

    to_exit = true;
    send(sockfd, "close", 5, 0);
    polling_thread.join();
    ::close(sockfd);
}

void AppClientCore::exit() {
    if (polling_thread.joinable()) close();
    ::exit(0);
}

void AppClientCore::getTime() {
    send(sockfd, "time", 4, 0);
}

void AppClientCore::getName() {
    send(sockfd, "name", 4, 0);
}

void AppClientCore::getClientList() {
    send(sockfd, "list", 4, 0);
}

void AppClientCore::sendMsg(int client_id, const string& content) {
    string send_content = "send " + to_string(client_id) + " " + content;
    cout << "AppClient: " << send_content << endl;
    send(sockfd, send_content.c_str(), send_content.size(), 0);
}


void AppClient::UserLoop() {
    // start the receiver
    to_exit = false;
    receiver = thread(&AppClient::ReceiverLoop, this);

    // looping to get the user command
    while (1) {
        cout << "> ";
        string cmd;
        cin >> cmd;

        // process the command
        if (cmd == "menu") {
            cout << core.getMenu() << endl;
        }
        else if (cmd == "close") {
            core.close();
        }
        else if (cmd == "exit") {
            core.exit();
            break;
        }
        else if (cmd == "connect") {
            cin >> cmd;
            string ip_addr = cmd;
            cin >> cmd;
            int port = stoi(cmd);
            core.connect(ip_addr, port);
        }
        else if (cmd == "get") {
            cin >> cmd;
            if (cmd == "time") {
                core.getTime();
            } else if (cmd == "name") {
                core.getName();
            } else if (cmd == "client") {
                core.getClientList();
            }
        }
        else if (cmd == "sendto") {
            cin >> cmd;
            int client_id = stoi(cmd);
            ostringstream oss;
            char read;
            while (cin.get() != '\"');
            while (cin.get(read) && read != '\"') {
                oss << read;
            }
            getchar();
            string content = oss.str();
            core.sendMsg(client_id, content);
        }
        this_thread::sleep_for(chrono::milliseconds(150));
    }
    to_exit = true;
    receiver.join();
}

void AppClient::ReceiverLoop() {
    while (1) {
        if (to_exit) break;
        // check the message queue and display them
        if (!core.getMsgQueue().empty()) {
            for (auto& msg : core.getMsgQueue()) {
                cout << msg << endl;
            }
            core.getMsgQueue().clear();
        }
    }
}