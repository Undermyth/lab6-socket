#ifndef _SERVER_H_
#define _SERVER_H_

#include <netinet/in.h>
#include <string>
#include <sys/socket.h>
#include <thread>
#include <vector>

using namespace std;

#define MAX_CLIENT 10   // the number of clients that server can hold
#define MAX_BUFFER 1024 // maximum buffer size for a coming message

struct ClientSock {
    in_addr ip_addr;        
    int port;
};

/// a table to record current connections from clients
class ClientList {

public:

    ClientList(): next_client_id(0) {}

    void AddClient(struct sockaddr_in* client_addr);

    void RemoveClient(int client_id);

    int getNextClientId() const {return next_client_id; }

    const string getClientList() const;

    const ClientSock& getClientSock(int client_id) const { return list[client_id]; }

private:

    ClientSock list[MAX_CLIENT];
    int next_client_id;
};


class AppServer {

public:

    AppServer(int port);

    const string getTime() const;

    const string getName() const;

    const string getClientList() const;

    const string sendMsg(int sender_id, const string& content) const;

    void sendto(int client_id, const string& content);

    void MainLoop();    // the loop of main thread

private:

    void ClientLoop(int client_id);

    int port;
    int sockfd;

    ClientList clients;
    int client_fd[MAX_CLIENT];      // fd for client socket
    thread client_td[MAX_CLIENT];   // thread that handles the client request

};

#endif