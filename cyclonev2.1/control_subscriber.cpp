#include <algorithm>
#include <iostream>
#include <thread>
#include <future>
#include <atomic>

#include "dds/dds.hpp"
#include "shutdownsignal.hpp"
#include "ControlData.hpp"


std::string subscriber_control_partition_name = "none";

void set_control_subscriber_partition(std::string& partition_name){
    subscriber_control_partition_name = partition_name;
}


void control_domain_subscriber(int& vehicle) {

    while(!shutdown_requested){

        if(subscriber_control_partition_name != "none"){

            std::cout << "\n\n\n" << subscriber_control_partition_name << std::endl;

            int control_domain = 1;

	        dds::domain::DomainParticipant participant(control_domain);

            dds::sub::qos::SubscriberQos sub_qos;

		    dds::core::StringSeq partition_name{ subscriber_control_partition_name };

		    sub_qos << dds::core::policy::Partition(partition_name);

	        dds::sub::Subscriber vehicle_subscriber(participant, sub_qos);

	        dds::topic::Topic<ControlData::steeringWheel_data> steeringWheel_topic(participant, "steeringWheel_topic");
	        dds::topic::Topic<ControlData::joyStick_data> joyStick_topic(participant, "joyStick_topic");

	        dds::sub::DataReader<ControlData::steeringWheel_data> steeringWheel_reader(vehicle_subscriber, steeringWheel_topic);
	        dds::sub::DataReader<ControlData::joyStick_data> joyStick_reader(vehicle_subscriber, joyStick_topic);

	        dds::sub::LoanedSamples<ControlData::steeringWheel_data> sw_samples;
	        dds::sub::LoanedSamples<ControlData::joyStick_data> js_samples;

            while (!shutdown_requested && subscriber_control_partition_name != "none") {

		        sw_samples = steeringWheel_reader.take();

		        if (sw_samples.length() > 0) {

			        dds::sub::LoanedSamples<ControlData::steeringWheel_data>::const_iterator iter;
			        for (iter = sw_samples.begin(); iter < sw_samples.end(); ++iter) {

				        const ControlData::steeringWheel_data& data = iter->data();
				        const dds::sub::SampleInfo& info = iter->info();

				        std::cout << "steeringWheel_data: " << data << std::endl;
				
				        if (info.valid()) {
					        std::cout << "steeringWheel_data: " << data << std::endl;
				        }
			        }
		        }

		        js_samples = joyStick_reader.take();

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

        }
    }
}