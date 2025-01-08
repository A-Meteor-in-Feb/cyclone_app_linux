#include <string>
#include <iostream>
#include <atomic>

#include "dds/dds.hpp"
#include "shutdownsignal.hpp"
#include "ControlData.hpp"


//void set_control_publisher_partition(std::string& partition_name);
void set_control_subscriber_partition(std::string& partition_name);


void run_command_domain(int& vehicle){

    std::string vehicle_name = "vehicle" + std::to_string(vehicle);
    bool online_state = true;
    bool connected_state = false;

    int command_domain = 0;

    dds::domain::DomainParticipant command_participant(command_domain);

    // ====== PUBLISHER ======
    dds::pub::Publisher command_publisher(command_participant);
    dds::topic::Topic<ControlData::vehicle_status> status_topic(command_participant, "vehicle_status");
    dds::pub::DataWriter<ControlData::vehicle_status> status_writer(command_publisher, status_topic);


    // ====== SUBSCRIBER ======
    dds::sub::qos::SubscriberQos sub_qos;
    dds::core::StringSeq partition_name{ vehicle_name };
    sub_qos << dds::core::policy::Partition(partition_name);

    dds::sub::Subscriber command_subscriber(command_participant, sub_qos);

    dds::topic::Topic<ControlData::connection_msg> con_topic(command_participant, "connection_msg");
    dds::topic::Topic<ControlData::disconnection_msg> discon_topic(command_participant, "disconnection_msg");

    dds::core::QosProvider provider("ReliableQos.xml");
    auto reader_qos = provider.datareader_qos("myqos::reliable_reader");

    dds::sub::DataReader<ControlData::connection_msg> con_reader(command_subscriber, con_topic, reader_qos);
    dds::sub::DataReader<ControlData::disconnection_msg> discon_reader(command_subscriber, discon_topic, reader_qos);

    dds::sub::LoanedSamples<ControlData::connection_msg> con_samples;
    dds::sub::LoanedSamples<ControlData::disconnection_msg> discon_samples;

    while(!shutdown_requested){

        con_samples = con_reader.take();

        if(con_samples.length() > 0){

            dds::sub::LoanedSamples<ControlData::connection_msg>::const_iterator iter;

            for(iter = con_samples.begin(); iter < con_samples.end(); ++iter){

                const ControlData::connection_msg& data = iter->data();
                const dds::sub::SampleInfo& info = iter->info();

                if(info.valid()){

                    std::string tele_id = data.tele_id();

                    if(tele_id == "non-matched"){

                        std::cout << "non-matched teleop station for now ..." << std::endl;

                        online_state = true;
                        connected_state = false;
                        ControlData::vehicle_status vehicle_status_data(vehicle_name, online_state, connected_state);
                        status_writer.write(vehicle_status_data);

                    } else {

                        std::cout << "match with teleop station: " << tele_id << std::endl;

                        online_state = true;
                        connected_state = true;
                        ControlData::vehicle_status vehicle_status_data(vehicle_name, online_state, connected_state);
                        status_writer.write(vehicle_status_data);

                        std::string control_partition_name = data.tele_id() + data.vehicle_id();
                        //set_control_publisher_partition(control_partition_name);
                        set_control_subscriber_partition(control_partition_name);

                    }
                }
            }
        }else{
            //std::cout << "not receiving any connection info" << std::endl;
            ControlData::vehicle_status vehicle_status_data(vehicle_name, online_state, connected_state);
            status_writer.write(vehicle_status_data);
        }

        discon_samples = discon_reader.take();

        if(discon_samples.length() > 0){

            dds::sub::LoanedSamples<ControlData::disconnection_msg>::const_iterator iter;

            for(iter = discon_samples.begin(); iter < discon_samples.end(); ++iter){

                const ControlData::disconnection_msg& data = iter->data();
                const dds::sub::SampleInfo& info = iter->info();

                if(info.valid()){

                    std::cout << "Received Disconnection Msg: " << data.msg() << std::endl;

                    ControlData::vehicle_status vehicle_status_data(vehicle_name, false, false);
                    status_writer.write(vehicle_status_data);

                    std::string control_partition_name = "none";

                    //set_control_publisher_partition(control_partition_name);
                    set_control_subscriber_partition(control_partition_name);
                }
            }
        }
    }
}