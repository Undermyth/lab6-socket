#include "server.h"
#include <arpa/inet.h>
#include <cstring>
#include <iomanip>
#include <ios>
#include <iostream>
#include <netdb.h>
#include <netinet/in.h>
#include <sstream>
#include <string>
#include <sys/socket.h>
#include <chrono>
#include <ctime>
#include <unistd.h>

void ClientList::AddClient(struct sockaddr_in* client_addr) {
    list[next_client_id].ip_addr = client_addr->sin_addr;
    list[next_client_id].port    = ntohs(client_addr->sin_port);
    next_client_id++;
}

void ClientList::RemoveClient(int client_id) {
    for (int i = client_id; i < next_client_id - 1; i++) {
        list[i] = list[i + 1];
    }
    next_client_id--;
}

const string ClientList::getClientList() const {
    ostringstream oss;
    oss << left << setw(12) << "Number" << setw(20) << "IP addr" << setw(20) << "Port" << endl;
    for (int i = 0; i < next_client_id; i++) {
        oss << left << setw(12) << i << setw(20) << inet_ntoa(list[i].ip_addr) 
            << setw(20) << list[i].port << endl;
    }
    return oss.str();
}

std::string getIPAddress() {
    char hostName[256];
    char ipAddress[INET_ADDRSTRLEN];
    struct hostent *hostEntry;
    int hostnameSuccess = gethostname(hostName, sizeof(hostName));
    if (hostnameSuccess != 0) {
        // 处理获取主机名失败的情况
        return "";
    }
    hostEntry = gethostbyname(hostName);
    if (hostEntry == nullptr) {
        // 处理获取主机信息失败的情况
        return "";
    }
    char *ipAddressPtr = inet_ntoa(*((struct in_addr *)hostEntry->h_addr_list[0]));
    if (ipAddressPtr == nullptr) {
        // 处理获取IP地址失败的情况
        return "";
    }
    std::strcpy(ipAddress, ipAddressPtr);
    return std::string(ipAddress);
}

AppServer::AppServer(int port): port(port) {
    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd == -1) {
        cerr << "Server: failed to create socket" << endl;
        exit(1);
    }

    // bind and listen
    sockaddr_in server_sock;
    server_sock.sin_family          = AF_INET;
    server_sock.sin_port            = htons(port);
    server_sock.sin_addr.s_addr     = inet_addr(getIPAddress().c_str());
    int bindres = ::bind(sockfd, (struct sockaddr*)&server_sock, sizeof(server_sock));
    if (bindres == -1) {
        cerr << "Server: failed to bind to port " << port << ". Please check if the port is usable." << endl;
        exit(1);
    }
    listen(sockfd, MAX_CLIENT);

    // entering main loop, waiting for connections
    // MainLoop();
}

void AppServer::MainLoop() {
    while (1) {
        struct sockaddr_in client_addr;
        socklen_t addr_len;
        int accept_fd = accept(sockfd, (struct sockaddr*)&client_addr, &addr_len);
        if (accept_fd == -1) {
            cerr << "Server: error occurred during accept a connection." << endl;
            exit(1);
        }

        // a client is connected. Add it to list and create a thread for it
        cerr << "Server: client " << inet_ntoa(client_addr.sin_addr) << ":" 
            << ntohs(client_addr.sin_port) << " is connected." << endl;
        int this_id = clients.getNextClientId();
        clients.AddClient(&client_addr);
        client_fd[this_id] = accept_fd;
        client_td[this_id] = thread(&AppServer::ClientLoop, this, this_id);
    }
}

void AppServer::ClientLoop(int client_id) {
    // keep receiving
    char buffer[MAX_BUFFER];
    while (1) {
        memset(buffer, 0, sizeof(buffer));
        int res = recv(client_fd[client_id], buffer, MAX_BUFFER, 0);
        if (res > 0) {
            // some data is received, process it
            cerr << "Server: received a message from client " << client_id << endl;
            string msg(buffer);
            if (msg == "time") {
                sendto(client_id, getTime());
            }
            if (msg == "name") {
                sendto(client_id, getName());
            }
            if (msg == "list") {
                sendto(client_id, getClientList());
            }
            if (msg == "close") {
                sendto(client_id, "goodbye");     // a fake segment, just to close the client thread
                close(client_fd[client_id]);
                clients.RemoveClient(client_id);
                break;
            }
            if (msg.substr(0, 4) == "send") {
                istringstream iss(msg);
                string dst_id_str, content;
                iss >> dst_id_str;      // abandon the part "send"
                iss >> dst_id_str;
                content = msg.substr(dst_id_str.length() + 6);
                sendto(stoi(dst_id_str), sendMsg(client_id, content));
            }
        }
    }
}

const string AppServer::getTime() const {
    // Get the current system time
    std::chrono::system_clock::time_point now = std::chrono::system_clock::now();

    // Convert the system time to a time_t value
    std::time_t currentTime = std::chrono::system_clock::to_time_t(now);

    // Convert the time_t value to a string representation
    std::string timeStr = std::ctime(&currentTime);

    // strip extra newline character
    timeStr[timeStr.length() - 1] = '\0';

    // Output the current time
    return "Current time: " + timeStr;
}

const string AppServer::getName() const {
    char hostname[256];
    gethostname(hostname, sizeof(hostname));
    return "Server name: " + string(hostname);
}

const string AppServer::getClientList() const {
    return "Client list: \n" + clients.getClientList();
}

const string AppServer::sendMsg(int sender_id,const string& content) const {
    ostringstream oss;
    oss << "\033[32m" << "Message from " << inet_ntoa(clients.getClientSock(sender_id).ip_addr) << "\033[0m" << endl;
    oss << content << endl;
    cout << "Server: message = " << content << endl;
    return oss.str();
}

void AppServer::sendto(int client_id, const string& content) {
    send(client_fd[client_id], content.c_str(), content.size(), 0);
}