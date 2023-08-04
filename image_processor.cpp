#include "application.h"
#include <iostream>
int main(int argc, char** argv) {
    try {
        Application app(argc, argv);
        app.Run();
    } catch (std::exception& e) {
        std::cerr << e.what();
        return -1;
    } catch (...) {
        return -2;
    }
    return 0;
}
