#ifndef CLIENT_H
#define CLIENT_H

#include <netinet/in.h>

#include <string>

class client {
public:
    client();

    void connect_to_server();
    void work();

private:
    void create_socket();
    void detach(int fd);

    void print_fatal_error(const std::string &err);

    int client_fd = 0;
    struct sockaddr_in serv_addr;
};

#endif  // CLIENT_H
