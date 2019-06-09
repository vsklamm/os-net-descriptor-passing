#include <cstdlib>
#include <iostream>
#include <string>

#include "client.h"

int main() {
    try {
        client cl;
        cl.connect_to_server();
        cl.work();
    } catch (std::runtime_error &e) {
        // std::cerr << e.what() << std::endl;
    }
}
