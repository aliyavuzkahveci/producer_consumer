
#ifndef _PRODUCER_H_
#define _WORKER_H_

#include <thread>
#include <iostream>
#include <future>
#include <memory>
#include <vector>

namespace producer_consumer {

class Worker {

public:
    Worker(size_t numOfWorkers = 1) : m_numOfWorkers(numOfWorkers), m_state(State::IDLE) {
        m_workerList.reserve(m_numOfWorkers);
    }
    
    virtual ~Worker() {
        if(m_state == State::RUNNING) { // if threads are still running
            for(auto & worker : m_workerList) {
                if(worker.joinable()) {
                    worker.join(); // waiting for the threads to complete their execution cycle
                }
            }
        }

        m_workerList.clear();
    }

    void setFunction(std::function<void ()>&& lambda) {
        std::cout << "Setting worker lambda function..." << std::endl;
        m_lambda = std::move(lambda);
    }

    void start() {
        if(m_state == State::RUNNING) {
            std::cout << "Workers are already working..." << std::endl;
            return;
        }

        m_state = State::RUNNING;
        m_workerList.clear();
        for(size_t i=0; i<m_numOfWorkers; ++i) {
            m_workerList.push_back(std::thread(m_lambda));
        }

    }

private:
    enum class State {
        IDLE,
        RUNNING,
        STOPPED
    };

    size_t m_numOfWorkers;
    std::vector<std::thread> m_workerList;
    State m_state;

    std::function<void ()> m_lambda;

};

}

#endif // _Worker_H_
