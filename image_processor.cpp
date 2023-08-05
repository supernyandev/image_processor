#include "application.h"
#include <iostream>
#include <chrono>
#include <iostream>
int main(int argc, char** argv) {
    auto start = std::chrono::system_clock::now();
    try {
        Application app(argc, argv);
        app.Run();
    } catch (std::exception& e) {
        std::cerr << e.what();
        return -1;
    } catch (...) {
        return -2;
    }
    auto end = std::chrono::system_clock::now();
    std::cout<<std::chrono::duration_cast<std::chrono::milliseconds>(end-start).count();
    return 0;
}
