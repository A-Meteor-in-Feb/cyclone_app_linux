#include <cstdlib>
#include <iostream>
#include <chrono>
#include <thread>
#include <cstring>
#include <atomic>

#include "shutdownsignal.hpp"


void run_command_domain(int& vehicle, std::atomic<bool>& run_controlPart);
void control_domain_publisher(int& vehicle, std::atomic<bool>& run_controlPart);
void control_domain_subscriber(int& vehicle, std::atomic<bool>& run_controlPart);


int main(int argc, char* argv[]){
    
    int vehicle = -1;

    if(argc > 2 && strcmp(argv[1], "-id") == 0){
        vehicle = atoi(argv[2]);
    }

    try{

        if(!shutdown_requested){

            std::atomic<bool> run_controlPart = false;

            std::thread vehicle_command_domain(run_command_domain, std::ref(vehicle), std::ref(run_controlPart));
            std::thread vehicle_control_publisher(control_domain_publisher, std::ref(vehicle), std::ref(run_controlPart));
            std::thread vehicle_control_subscriber(control_domain_subscriber, std::ref(vehicle), std::ref(run_controlPart));

            vehicle_command_domain.join();
            vehicle_control_publisher.join();
            vehicle_control_subscriber.join();
        }

    } catch (const std::exception& ex){
        std::cerr << ex.what() << std::endl;
        return EXIT_FAILURE;
    } catch (...){
        std::cerr << "Unknown Error :( " << std::endl;
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}