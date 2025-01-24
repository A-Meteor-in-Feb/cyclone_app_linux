#include <string>
#include <iostream>
#include <atomic>
#include <thread>
#include <chrono>

#include "dds/dds.hpp"
#include "shutdownsignal.hpp"
#include "ControlData.hpp"
#include "partitionName.hpp"
#include "TimeStampLogger.h"

void control_domain_publisher(int& vehicle, std::string& contorl_partition_name);
void control_domain_subscriber(int& vehicle, std::string& control_partition_name);
//void control_streamdeck(int& vehicle, std::string& control_partition_name);

int count_ConMsg = 0;

void run_command_domain(int& vehicle){

    //const std::string filename = "vehicle_connection_msg.txt";

    std::string vehicle_id = "vehicle" + std::to_string(vehicle);
    bool online_state = true;
    bool connected_state = false;

    partitionName control_partition_name;

    int command_domain = 0;

    dds::domain::DomainParticipant command_participant(command_domain);

    // ====== PUBLISHER ======
    dds::pub::Publisher command_publisher(command_participant);
    dds::topic::Topic<ControlData::vehicle_status> status_topic(command_participant, "vehicle_status");
    dds::pub::DataWriter<ControlData::vehicle_status> status_writer(command_publisher, status_topic);


    // ====== SUBSCRIBER ======
    dds::sub::qos::SubscriberQos sub_qos;
    dds::core::StringSeq partition_name{ vehicle_id };
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

    bool known = false;
    std::string timestamp;

    while(!shutdown_requested){

        con_samples = con_reader.take();

        if(con_samples.length() > 0){

            timestamp = TimestampLogger::getTimestamp();
            std::cout << "receive connection msg at: " << timestamp << std::endl;
            //TimestampLogger::writeToFile(filename, timestamp);

            dds::sub::LoanedSamples<ControlData::connection_msg>::const_iterator iter;

            for(iter = con_samples.begin(); iter < con_samples.end(); ++iter){

                const ControlData::connection_msg& data = iter->data();
                const dds::sub::SampleInfo& info = iter->info();

                if(info.valid()){

                    count_ConMsg += 1;

                    std::string tele_id = data.tele_id();

                    if(tele_id == "known"){

                        std::cout << "non-matched teleop station for now ..." << std::endl;

                        known = true;

                    } else {

                        std::cout << "match with teleop station: " << tele_id << std::endl;

                        online_state = true;
                        connected_state = true;
                        ControlData::vehicle_status vehicle_status_data(vehicle_id, online_state, connected_state);
                        status_writer.write(vehicle_status_data);

                        std::string name = data.tele_id() + data.vehicle_id();
                        std::cout << "partition name: " << name<< std::endl;
                        std::thread vehicle_control_publisher(control_domain_publisher, std::ref(vehicle), std::ref(name) );
                        std::thread vehicle_control_subscriber(control_domain_subscriber, std::ref(vehicle), std::ref(name) );
                        //std::thread vehicel_control_sreamdeck(control_streamdeck, std::ref(vehicle), std::ref(name) );
                        vehicle_control_publisher.join();
                        vehicle_control_subscriber.join();
                        //vehicel_control_sreamdeck.join();
                    }
                }
            }
        }else if(!known){
            //std::cout << "not receiving any connection info" << std::endl;
            ControlData::vehicle_status vehicle_status_data(vehicle_id, online_state, connected_state);
            status_writer.write(vehicle_status_data);
            timestamp = TimestampLogger::getTimestamp();
            std::cout << "publish status msg at: " << timestamp << std::endl;
            std::this_thread::sleep_for(std::chrono::microseconds(3000));
        }

        /*
        discon_samples = discon_reader.take();

        if(discon_samples.length() > 0){

            dds::sub::LoanedSamples<ControlData::disconnection_msg>::const_iterator iter;

            for(iter = discon_samples.begin(); iter < discon_samples.end(); ++iter){

                const ControlData::disconnection_msg& data = iter->data();
                const dds::sub::SampleInfo& info = iter->info();

                if(info.valid()){

                    std::cout << "Received Disconnection Msg: " << data.msg() << std::endl;

                    ControlData::vehicle_status vehicle_status_data(vehicle_id, false, false);
                    status_writer.write(vehicle_status_data);

                    control_partition_name.setPartitionName("none");
                    
                }
            }
        }*/
    }

    std::cout << "Preparing shutdown ... " << std::endl;
    std::cout << "Totally received connection msg from the command center: " << count_ConMsg << std::endl;
    std::cout << "From vehicle side, totally 200 imu messages are sent." << std::endl;
    //std::cout << "From vehicle side, totallu 50 statistic data for streamdeck are sent." << std::endl;
    
}

int main(int argc, char* argv[]) {

	int vehicle = -1;

	if (argc > 2 && strcmp(argv[1], "-id") == 0) {
		vehicle = atoi(argv[2]);
	}


	try {

		run_command_domain(std::ref(vehicle));

	}
	catch (const std::exception& ex) {
		// This will catch DDS exceptions
		std::cerr << "Exception in run_publisher_application(): " << ex.what()
			<< std::endl;
		return EXIT_FAILURE;
	}
	catch (...) {
		std::cerr << "Unknown Error :(" << std::endl;
		return EXIT_FAILURE;
	}

	return EXIT_SUCCESS;
}