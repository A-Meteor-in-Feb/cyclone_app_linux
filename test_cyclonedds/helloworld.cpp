#include <cstdlib>
#include <iostream>
#include <chrono>
#include <thread>

#include "dds/dds.hpp"
#include "shutdownsignal.hpp"
#include "HelloWorldData.hpp"

using namespace org::eclipse::cyclonedds;

int main() {
    try {
        
        dds::domain::DomainParticipant participant(0);
        dds::topic::Topic<HelloWorldData::Msg> topic(participant, "HelloWorldData_Msg");
        dds::sub::Subscriber subscriber(participant);
        dds::sub::DataReader<HelloWorldData::Msg> reader(subscriber, topic);

        
        while (!shutdown_requested) {
            
            dds::sub::LoanedSamples<HelloWorldData::Msg> samples;
            samples = reader.take();

            if (samples.length() > 0) {

                dds::sub::LoanedSamples<HelloWorldData::Msg>::const_iterator sample_iter;
                for (sample_iter = samples.begin(); sample_iter < samples.end(); ++sample_iter) {
                   
                    const HelloWorldData::Msg& msg = sample_iter->data();
                    const dds::sub::SampleInfo& info = sample_iter->info();

                    if (info.valid()) {
                        std::cout << "=== [Subscriber] Message received: " << msg << std::endl;
                    }
                }

            } else {
                std::this_thread::sleep_for(std::chrono::milliseconds(10));
            }
        }
    } catch (const dds::core::Exception& e) {
        std::cerr << "=== [Subscriber] DDS exception: " << e.what() << std::endl;
        return EXIT_FAILURE;
    } catch (const std::exception& e) {
        std::cerr << "=== [Subscriber] C++ exception: " << e.what() << std::endl;
        return EXIT_FAILURE;
    }

    std::cout << "=== [Subscriber] Done." << std::endl;

    return EXIT_SUCCESS;
}
