#ifndef SERVER_H
#define SERVER_H

#include <sys/un.h>

#include <string>

class server {
public:
    server();

    void wait_clients();
    void work();

private:
    void create_socket();
    void bind_to_address();
    void try_create_pipe(int * pipefd);
    void detach(int fd);

    void print_fatal_error(const std::string &err, bool perr = true);

    int server_fd;
    int new_socket;
    struct sockaddr_un server_address;
    int addrlen = sizeof(server_address);

    const std::string SOCK_NAME = "./sock";
};

#endif  // SERVER_H
