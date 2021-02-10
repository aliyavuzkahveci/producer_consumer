
#include <functional>
#include <future>

#include "concurrent_queue.h"
#include "worker.h"

using namespace producer_consumer;

int main(int argc, char **argv) {
    using Functor = std::function<void ()>;

    ConcurrentQueue<Functor> queue;

    // to be able to control threads outside, send exit signal when you (master thread) are done!
    auto numOfProducers = 10;
    std::promise<void> exitSignal;
    std::future<void> futureObj(exitSignal.get_future());

    auto producerLambda = [ & ] {
        uint64_t counter = 0;
        while(futureObj.wait_for(std::chrono::milliseconds(0)) == std::future_status::timeout) { // stop not requested
            auto taskId = counter++;
            queue.push(std::make_shared<Functor>([ = ] { std::cout << "Running task " << taskId << std::endl << std::flush; }));
        }

        std::cout << "Producer Thread: stop requested! Terminating gracefully..." << std::endl << std::flush;
    };

    auto consumerLambda = [ & ] {
        while(futureObj.wait_for(std::chrono::milliseconds(0)) == std::future_status::timeout) { // stop not requested
            auto task = queue.pop();
            if(task) {
                (*task.get())();
            }
        }

        std::cout << "Consumer Thread: stop requested! Terminating gracefully..." << std::endl << std::flush;
    };

    Worker producer(numOfProducers);
    producer.setFunction(producerLambda);

    Worker consumer;
    consumer.setFunction(consumerLambda);

    producer.start();
    consumer.start();

    // sleep for 1 hour
    std::this_thread::sleep_for(std::chrono::hours(1));
    //std::this_thread::sleep_for(std::chrono::seconds(10)); // for test purposes
    
    // if the threads are meant to run forever, then no need to use std::future, at all! 
    // send termination signal to the threads
    exitSignal.set_value();

    return 0;
}
