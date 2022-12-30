#include "taco/tacoMain.h"
#include <iostream>

int main(int argc, char **argv) {
    // only node 0 continues afterwards
    taco::init(argc, argv);

    std::cout << "number of TACO nodes: " << taco::nodes() << std::endl;
    std::cout << "this is node " << taco::thisNode() << std::endl;
    return 0;
}
