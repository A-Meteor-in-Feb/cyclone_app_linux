#include "dds/dds.hpp"
#include "shutdownsignal.hpp"
#include "ControlData.hpp"

void control_streamdeck(int& vehicle, std::string& control_partition_name){

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

    while(!shutdown_requested){
        ControlData::statistic_data statistic_data(1.1, 2.2, 0);
        statistic_writer.write(statistic_data);
    }

}