#include <algorithm>
#include <iostream>
#include <thread>
#include <future>
#include <atomic>

#include "dds/dds.hpp"
#include "shutdownsignal.hpp"
#include "ControlData.hpp"
#include "partitionName.hpp"
#include "TimeStampLogger.h"

int count_recvStrw = 0;

void control_domain_subscriber(int& vehicle, std::string& control_partition_name) {

	//const std::string filename1 = "vehicle_steeringWheel.txt";
	//const std::string filename2 = "vehicle_joystick.txt";

	std::string name = control_partition_name;

	std::cout << "start running subscriber, partition: " << name << std::endl;

    int control_domain = 1;

	dds::domain::DomainParticipant participant(control_domain);

    dds::sub::qos::SubscriberQos sub_qos;

	dds::core::StringSeq partition_name{ name };

	sub_qos << dds::core::policy::Partition(partition_name);

	dds::sub::Subscriber vehicle_subscriber(participant, sub_qos);

	dds::topic::Topic<ControlData::steeringWheel_data> steeringWheel_topic(participant, "steeringWheel_topic");
	dds::topic::Topic<ControlData::joyStick_data> joyStick_topic(participant, "joyStick_topic");

	dds::sub::DataReader<ControlData::steeringWheel_data> steeringWheel_reader(vehicle_subscriber, steeringWheel_topic);
	dds::sub::DataReader<ControlData::joyStick_data> joyStick_reader(vehicle_subscriber, joyStick_topic);

	dds::sub::LoanedSamples<ControlData::steeringWheel_data> sw_samples;
	dds::sub::LoanedSamples<ControlData::joyStick_data> js_samples;

	std::string timestamp;

    //while (!shutdown_requested) {
	for(int i=0 ; i < 205; i++) {

		sw_samples = steeringWheel_reader.take();
		js_samples = joyStick_reader.take();

		if(sw_samples.length() > 0){
			timestamp = TimestampLogger::getTimestamp();
			//TimestampLogger::writeToFile(filename1, timestamp);
			std::cout << "receive steering wheel data at: " << timestamp << std::endl;
		}
		if(js_samples.length() > 0){
			timestamp = TimestampLogger::getTimestamp();
			//TimestampLogger::writeToFile(filename2, timestamp);
			std::cout << "receive joystick data at: " << timestamp << std::endl;
		}

		if (sw_samples.length() > 0) {
			
			dds::sub::LoanedSamples<ControlData::steeringWheel_data>::const_iterator iter;
			for (iter = sw_samples.begin(); iter < sw_samples.end(); ++iter) {

				const ControlData::steeringWheel_data& data = iter->data();
				const dds::sub::SampleInfo& info = iter->info();

				std::cout << "steeringWheel_data: " << data << std::endl;
				
				if (info.valid()) {
					count_recvStrw += 1;
					std::cout << "steeringWheel_data: " << data << std::endl;
				}
			}
		}

		if (js_samples.length() > 0) {

			dds::sub::LoanedSamples<ControlData::joyStick_data>::const_iterator iter;
			for (iter = js_samples.begin(); iter < js_samples.end(); ++iter) {

				const ControlData::joyStick_data& data = iter->data();
				const dds::sub::SampleInfo& info = iter->info();

				if (info.valid()) {
					std::cout << "joyStick_data: " << data <<std::endl;
				}
			}
		}   
	}

	std::cout << "Totally received controller msg from teleop: " << count_recvStrw << std::endl;

}