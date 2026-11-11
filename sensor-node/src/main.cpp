#include <iostream>

#include "generator.hpp"

int main() {
    std::cout << "sensor-node\n";
    Generator gen;
    write_csv_header(std::cout);
    for (int i = 0; i < 20; ++i) write_csv_row(std::cout, gen.next());
}
