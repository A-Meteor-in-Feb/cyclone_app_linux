#include <cstdlib>
#include <iostream>
#include <chrono>
#include <thread>
#include <cstring>
#include <string>
#include <locale>
#include <codecvt>
#include <bitset>

#include "shutdownsignal.hpp"


int run_subscriber_application(int vehicle_id);


int main(int argc, char* argv[]) {

    int vehicle_id = -1;

    if (argc > 2 && strcmp(argv[1], "-id") == 0) {
        vehicle_id = atoi(argv[2]);
    }


    try {
        std::thread vehicle_subscriber(run_subscriber_application, vehicle_id);
        vehicle_subscriber.join();
    }
    catch (const std::exception& ex) {
        // This will catch DDS exceptions
        std::cerr << "Exception in run_subscriber_application(): " << ex.what()
            << std::endl;
        return EXIT_FAILURE;
    }
    catch (...) {
        std::cerr << "Unknown Error :(" << std::endl;
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}