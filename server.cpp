#include <arpa/inet.h>
#include <sys/socket.h>
#include <unistd.h>

#include <array>
#include <cstring>
#include <iostream>

#include "server.h"

server::server() {
    server_address.sun_family = AF_UNIX;
    memcpy(server_address.sun_path, SOCK_NAME.c_str(), SOCK_NAME.size() + 1);
    unlink(SOCK_NAME.c_str());

    create_socket();
    bind_to_address();
}

void server::wait_clients() {
    if (listen(server_fd, 10) == -1) {
        detach(server_fd);
        print_fatal_error("listen");
    }
}

void server::work() {
    for (;;) {
        if ((new_socket = accept(server_fd, nullptr, nullptr)) < 0) {
            print_fatal_error("accept");
        }
        int pipefd[2];
        try_create_pipe(pipefd);
        char iobuf[1];
        struct iovec io = {.iov_base = iobuf, .iov_len = sizeof(iobuf)};

        struct msghdr msg = {0};
        struct cmsghdr *cmsg;
        union {
            struct cmsghdr align;
            char buf[CMSG_SPACE(sizeof(pipefd[1]))];
        } u;

        msg.msg_iov = &io;
        msg.msg_iovlen = 1;
        msg.msg_control = u.buf;
        msg.msg_controllen = sizeof(u.buf);

        cmsg = CMSG_FIRSTHDR(&msg);
        cmsg->cmsg_len = CMSG_LEN(sizeof(pipefd[1]));
        cmsg->cmsg_level = SOL_SOCKET;
        cmsg->cmsg_type = SCM_RIGHTS;
        memcpy(CMSG_DATA(cmsg), &pipefd[1], sizeof(pipefd[1]));

        std::array<char, CMSG_SPACE(sizeof pipefd[1])> buffer;
        int res_sent = sendmsg(new_socket, &msg, 0);
        if (res_sent == -1) {
            perror("send message failed");
        }
        std::cout << "server: ";
        while (read(pipefd[0], &buffer, 1) > 0) {
            write(STDOUT_FILENO, &buffer, 1);
        }

        detach(pipefd[1]);
        detach(pipefd[0]);
        detach(new_socket);
    }
}

void server::create_socket() {
    if ((server_fd = socket(AF_UNIX, SOCK_STREAM, 0)) == -1) {
        print_fatal_error("socket failed");
    }
}

void server::bind_to_address() {
    if (bind(server_fd, (struct sockaddr *)&server_address,
             sizeof(server_address)) < 0) {
        detach(server_fd);
        print_fatal_error("bind failed");
    }
}

void server::try_create_pipe(int *pipefd) {
    int pipest = pipe(pipefd);
    if (pipest == -1) {
        print_fatal_error("pipe failed", false);
    }
}

void server::detach(int fd) {
    if (close(fd) < 0) {
        perror("Can't close socket");
    }
}

void server::print_fatal_error(const std::string &err, bool perr) {
    throw std::runtime_error("server error: " + err +
                             (perr ? std::string(": ") + strerror(errno) : ""));
}
