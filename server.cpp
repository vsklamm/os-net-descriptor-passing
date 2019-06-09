#include <arpa/inet.h>
#include <sys/socket.h>
#include <unistd.h>

#include <array>
#include <cstring>
#include <iostream>

#include "server.h"

server::server() {
    unlink(SOCK_NAME.c_str());
    create_socket();
    bind_to_address();
}

void server::wait_client() {
    if (listen(server_fd, 10) < 0) {
        detach(server_fd);
        print_fatal_error("listen");
    }
}

void server::accept_connection() {
    for (;;) {
        if ((new_socket = accept(server_fd, nullptr, nullptr)) < 0) {
            print_fatal_error("accept");
        }
        int pipefd[2];
        try_create_pipe(pipefd);
        struct msghdr msg = { 0 };
        struct cmsghdr * cmsg;
        char iobuf[1];
        struct iovec io = { .iov_base = iobuf, .iov_len = sizeof(iobuf) };
        union {
            char buf[CMSG_SPACE(sizeof(pipefd[1]))];
            struct cmsghdr align;
        } u;
        msg.msg_iov = &io;
        msg.msg_iovlen = 1;
        msg.msg_control = u.buf;
        msg.msg_controllen = sizeof(u.buf);
        cmsg = CMSG_FIRSTHDR(&msg);
        cmsg->cmsg_type = SCM_RIGHTS;
        cmsg->cmsg_level = SOL_SOCKET;
        cmsg->cmsg_len = CMSG_LEN(sizeof(pipefd[1]));
        memcpy(CMSG_DATA(cmsg), &pipefd[1], sizeof(pipefd[1]));

        std::array<char, CMSG_SPACE(sizeof pipefd[1])> buffer;
        int res_sent = sendmsg(new_socket, &msg, 0);
        if (res_sent == -1) {
            perror("Send message failed");
        }
        detach(pipefd[1]);
        std::cout << buffer.data() << std::endl;
        while (read(pipefd[0], &buffer, 1) > 0) {
            write(STDOUT_FILENO, &buffer, 1);
        }
        detach(pipefd[0]);
        detach(new_socket);
    }
}

void server::work() {
    const std::string hello = "hello, client";
    std::array<char, 1024> buffer;
    read(new_socket, buffer.data(), buffer.size());
    std::cout << buffer.data() << std::endl;

    const int flags = 0;
    send(new_socket, hello.c_str(), hello.size(), flags);
    std::cout << "server: sent" << std::endl;
}

void server::check_ipv4(const std::string &address) {
    struct sockaddr_in sa;
    if (inet_pton(AF_INET, address.c_str(), &(sa.sin_addr)) == 0) {
        std::cerr << "server: invalid ipv4 address" << std::endl;
        throw std::runtime_error("invalid ipv4 address");
    }
}

void server::create_socket() {
    if ((server_fd = socket(AF_INET, SOCK_STREAM, 0)) == -1) {
        perror("socket failed");
    }
    std::cout << "socket: " << server_fd << std::endl;
}

void server::bind_to_address() {
    if (bind(server_fd, (struct sockaddr *)&server_address,
             sizeof(server_address)) < 0) {
        detach(server_fd);
        print_fatal_error("bind failed");
    }
}

void server::try_create_pipe(int * pipefd)
{
    int pipest = pipe(pipefd);
    if (pipest == -1) {
        std::cerr << "oops" << std::endl;
        exit(EXIT_FAILURE);
    }
}

void server::detach(int fd) {
    if (close(fd) < 0) {
        perror("Can't close socket");
    }
}

void server::print_fatal_error(const std::string &err) {
    perror(err.c_str());
    throw std::runtime_error("server: " + err);
}
