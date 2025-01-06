#include <cstdlib>
#include <iostream>
#include <chrono>
#include <thread>
#include <cstring>
#include <atomic>

#include "shutdownsignal.hpp"


void command_domain_publisher(int& vehicle, std::atomic<bool>& command_ato);
void command_domain_subscriber(int& vehicle, std::atomic<bool>& command_ato, std::atomic<bool>& control_ato);
void control_domain_publisher(int& vehicle, std::atomic<bool>& control_ato);
void control_domain_subscriber(int& vehicle, std::atomic<bool>& control_ato);



int main(int argc, char* argv[]){
    
    int vehicle = -1;

    if(argc > 2 && strcmp(argv[1], "-id") == 0){
        vehicle = atoi(argv[2]);
    }

    try{

        if(!shutdown_requested){
            std::atomic<bool> command_ato(false);
            std::atomic<bool> control_ato(false);

            std::thread vehicle_command_publisher(command_domain_publisher, std::ref(vehicle), std::ref(command_ato));
            std::thread vehicle_command_subscriber(command_domain_subscriber, std::ref(vehicle), std::ref(command_ato), std::ref(control_ato));

            std::thread vehicle_control_publisher(control_domain_publisher, std::ref(vehicle), std::ref(control_ato));
            std::thread vehicle_control_subscriber(control_domain_subscriber, std::ref(vehicle), std::ref(control_ato));

            vehicle_control_publisher.join();
            vehicle_control_subscriber.join();

            vehicle_command_publisher.join();
            vehicle_command_subscriber.join();
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