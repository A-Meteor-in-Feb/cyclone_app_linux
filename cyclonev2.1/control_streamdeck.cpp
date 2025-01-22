#include "dds/dds.hpp"
#include "shutdownsignal.hpp"
#include "ControlData.hpp"
#include "TimeStampLogger.h"

int count_sentMsg1 = 100;


void control_streamdeck(int& vehicle, std::string& control_partition_name){

    const std::string filename = "vehicle_streamdeck.txt";


    std::string name = control_partition_name;

    std::cout << "start running publisher for streamdeck, partition: " << name << std::endl;

    std::string vehicle_id = "vehicle" + std::to_string(vehicle);

    int control_domain = 1;

    dds::domain::DomainParticipant participant(control_domain);

    dds::topic::Topic<ControlData::statistic_data> statistic_topic(participant, "statistic_data");

    dds::pub::qos::PublisherQos pub_qos;

	dds::core::StringSeq partition_name{ name };

	pub_qos << dds::core::policy::Partition(partition_name);

    dds::pub::Publisher vehicle_publisher2sd(participant, pub_qos);

    dds::pub::DataWriter<ControlData::statistic_data> statistic_writer(vehicle_publisher2sd, statistic_topic);

    while(!shutdown_requested && count_sentMsg1 > 0){
        ControlData::statistic_data statistic_data(1.1, 2.2, 0);
        statistic_writer.write(statistic_data);

        std::string timestamp = TimestampLogger::getTimestamp();
        TimestampLogger::writeToFile(filename, timestamp);

        count_sentMsg1 -= 1;
    }

}