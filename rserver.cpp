#include <cstdlib>
#include <iostream>
#include <string>

#include "server.h"

int main() {
    try {
        server serv;
        serv.wait_clients();
        serv.work();
    } catch (std::runtime_error &e) {
        std::cerr << e.what() << std::endl;
    }
}
