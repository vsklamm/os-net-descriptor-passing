#ifndef CLIENT_H
#define CLIENT_H

#include <sys/un.h>

#include <string>

class client {
public:
    client();

    void connect_to_server();
    void work();

private:
    void create_socket();
    void detach(int fd);

    void print_fatal_error(const std::string &err, bool perr = true);

    int client_fd = 0;
    struct sockaddr_un client_addr;

    const std::string SOCK_NAME = "./sock";
};

#endif  // CLIENT_H
