#include <arpa/inet.h>
#include <sys/socket.h>
#include <unistd.h>

#include <array>
#include <cstddef>
#include <cstring>
#include <iostream>

#include "client.h"

client::client() {
    create_socket();
}

void client::connect_to_server() {
    if (connect(client_fd, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) <
            0) {
        print_fatal_error("Connection Failed");
    }
}

void client::work() {
    int fd;
    const std::string hello = "hello, server";
    struct msghdr msg = { 0 };
    char buff[1];
    struct iovec io = { .iov_base = buff, .iov_len = sizeof(buff) };
    union {
        char buf[CMSG_SPACE(sizeof(buff[0]))];
        struct cmsghdr align;
    } u;
    msg.msg_iov = &io;
    msg.msg_iovlen = 1;
    msg.msg_control = u.buf;
    msg.msg_controllen = sizeof(u.buf);
    ssize_t read_bytes = recvmsg(client_fd, &msg, 0);
    if (read_bytes == 0) {
        perror("client: message sending failed");
        return;
    }
    std::cout << "message"; // TODO: respond
    detach(client_fd);
    detach(fd);
}

void client::detach(int fd) {
    if (close(fd) < 0) {
        perror("Can't close socket");
    }
}

void client::create_socket() {
    if ((client_fd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        perror("Socket creation error");
    }
}

void client::print_fatal_error(const std::string &err) {
    perror(err.c_str());
    throw std::runtime_error("client: " + err);
}
