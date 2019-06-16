#include <arpa/inet.h>
#include <sys/socket.h>
#include <unistd.h>

#include <array>
#include <cstddef>
#include <cstring>
#include <iostream>

#include "client.h"

client::client() {
    client_addr.sun_family = AF_UNIX;
    memcpy(client_addr.sun_path, SOCK_NAME.c_str(), SOCK_NAME.size() + 1);
    create_socket();
}

void client::connect_to_server() {
    if (connect(client_fd, (struct sockaddr *)&client_addr, sizeof(client_addr)) <
            0) {
        print_fatal_error("Connection Failed");
    }
}

void client::work() {
    const std::string msg_to_server = "hello, server\n";
    char buff[1];
    struct iovec io = {.iov_base = buff, .iov_len = sizeof(buff)};
    struct msghdr msg = { 0 };
    union {
        struct cmsghdr align;
        char buf[CMSG_SPACE(sizeof(buff[0]))];
    } u;
    msg.msg_iov = &io;
    msg.msg_iovlen = 1;
    msg.msg_control = u.buf;
    msg.msg_controllen = sizeof(u.buf);
    ssize_t read_bytes = recvmsg(client_fd, &msg, 0);
    if (read_bytes == 0 || read_bytes == -1) {
        perror("client: message sending failed");
        return;
    }
    struct cmsghdr* cmsg = CMSG_FIRSTHDR(&msg);
    int tmfd;
    memcpy(&tmfd, CMSG_DATA(cmsg), sizeof(tmfd));
    write(tmfd, msg_to_server.c_str(), msg_to_server.size());

    detach(tmfd);
    detach(client_fd);
}

void client::detach(int fd) {
    if (close(fd) < 0) {
        perror("Can't close socket");
    }
}

void client::create_socket() {
    if ((client_fd = socket(AF_UNIX, SOCK_STREAM, 0)) < 0) {
        perror("Socket creation error");
    }
}

void client::print_fatal_error(const std::string &err, bool perr) {
    throw std::runtime_error("server error: " + err +
                             (perr ? std::string(": ") + strerror(errno) : ""));
}
