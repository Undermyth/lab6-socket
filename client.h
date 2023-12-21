#ifndef _CLIENT_H_
#define _CLIENT_H_

#include <sys/socket.h>
#include <unistd.h>
#include <string>
#include <netinet/in.h>
#include <thread>
#include <vector>

#define MAX_BUFFER 1024     // maximum buffer size for a coming message

using namespace std;

class AppClientCore {

public:

    AppClientCore(int port);

    /// print the menu
    const string getMenu() const;

    /// connect to selected server socket
    /// @return whether the connection is successful
    bool connect(const string& ip_addr, int port);

    // terminate the connection
    void close();

    /// terminate the connection and quit app
    void exit();

    /// get time
    void getTime();

    /// get server name
    void getName();

    /// get client list
    void getClientList();

    /// send message to selected client.
    /// client id is the id in client list
    void sendMsg(int client_id, const string& content);

    /// get inbound message
    vector<string>& getMsgQueue() {return message_queue; }

private:

    // function used by the child thread, which 
    // is an infinite loop that receiving messages
    void polling();

    void print();

    int port;
    int sockfd;     // file descriptor of client socket

    // message queue. this queue is exposed to upper app.
    // the child thread will keep receiving data from server,
    // and put them into this queue.
    vector<string> message_queue;
    thread polling_thread;
    bool to_exit;   // used by main thread to tell child thread to exit
    char buffer[MAX_BUFFER];
};

class AppClient {

public:

    AppClient(int port): core(port) {to_exit = false; }
    void UserLoop();

private:

    void ReceiverLoop();    // loop that responsible for displaying inbound messages
    AppClientCore core;
    thread receiver;
    bool to_exit;
};

#endif