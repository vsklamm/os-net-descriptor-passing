#ifndef SERVER_H
#define SERVER_H

#include <netinet/in.h>

#include <string>

class server {
public:
    server();

    void wait_client();
    void accept_connection();
    void work();

private:
    void check_ipv4(const std::string &address);
    void create_socket();
    void bind_to_address();
    void try_create_pipe(int * pipefd);
    void detach(int fd);

    void print_fatal_error(const std::string &err);

    int server_fd;
    int new_socket;
    struct sockaddr_in server_address;
    int addrlen = sizeof(server_address);

    const std::string SOCK_NAME = "./sock";
};

#endif  // SERVER_H
