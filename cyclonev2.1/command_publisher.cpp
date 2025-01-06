#include <string>
#include <iostream>
#include <atomic>

#include "dds/dds.hpp"
#include "shutdownsignal.hpp"
#include "ControlData.hpp"

bool online = true;
bool connected = false;

void update_state(bool online_state, bool connected_state){
    online = online_state;
    connected = connected_state;
}

void command_domain_publisher(int& vehicle, std::atomic<bool>& command_ato){

    std::string vehicle_name = "vehicle" + std::to_string(vehicle);

    bool init = true;

    int command_domain = 0;

    dds::domain::DomainParticipant command_participant(command_domain);

    dds::pub::Publisher command_publisher(command_participant);

    dds::topic::Topic<ControlData::vehicle_status> status_topic(command_participant, "vehicle_status");

    dds::pub::DataWriter<ControlData::vehicle_status> status_writer(command_publisher, status_topic);

    while(!shutdown_requested){

        if(!online && !connected){
            init = false;
            command_ato.store(false);
        }

        if(command_ato.load()){

            if(init){
                init = false;
            }

            ControlData::vehicle_status vehicle_status_data(vehicle_name, online, connected);

            status_writer.write(vehicle_status_data);
        }

        if(init && !command_ato.load()){

            ControlData::vehicle_status vehicle_status_data(vehicle_name, online, connected);

            status_writer.write(vehicle_status_data);
        }
        
    }

}