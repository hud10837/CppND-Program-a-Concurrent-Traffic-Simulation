#include <iostream>
#include <random>
#include <thread>
#include "TrafficLight.h"

/* Implementation of class "MessageQueue" */

template <typename T>
T MessageQueue<T>::receive()
{
    // FP.5a : The method receive should use std::unique_lock<std::mutex> and _condition.wait() 
    // to wait for and receive new messages and pull them from the queue using move semantics. 
    // The received object should then be returned by the receive function. 
    std::unique_lock<std::mutex> lock(_mtx);
    auto s = _queue.size();
    _cond.wait(lock, [this, s]() { return _queue.size() > s; });
    auto tmp = _queue.back();
    _queue.pop_back();
    return std::move(tmp);
}

template <typename T>
void MessageQueue<T>::send(T &&msg)
{
    // FP.4a : The method send should use the mechanisms std::lock_guard<std::mutex> 
    // as well as _condition.notify_one() to add a new message to the queue and afterwards send a notification.
    std::lock_guard<std::mutex> guard(_mtx);
    _queue.emplace_back(std::move(msg));
    _cond.notify_one();
}


/* Implementation of class "TrafficLight" */

TrafficLight::TrafficLight()
{
    _currentPhase = TrafficLightPhase::red;
}

void TrafficLight::waitForGreen()
{
    // FP.5b : add the implementation of the method waitForGreen, in which an infinite while-loop 
    // runs and repeatedly calls the receive function on the message queue. 
    // Once it receives TrafficLightPhase::green, the method returns.
    while (true) {
       if (_queue.receive() == TrafficLightPhase::green) return;
       else continue;
    }
}

TrafficLightPhase TrafficLight::getCurrentPhase()
{
    return _currentPhase;
}

void TrafficLight::simulate()
{
    // FP.2b : Finally, the private method „cycleThroughPhases“ should be started in a thread when the public method „simulate“ is called. To do this, use the thread queue in the base class. 
    std::thread thread(&TrafficLight::cycleThroughPhases, this);
    threads.emplace_back(std::move(thread));
}

// virtual function which is executed in a thread
void TrafficLight::cycleThroughPhases()
{
    // FP.2a : Implement the function with an infinite loop that measures the time between two loop cycles 
    // and toggles the current phase of the traffic light between red and green and sends an update method 
    // to the message queue using move semantics. The cycle duration should be a random value between 4 and 6 seconds. 
    // Also, the while-loop should use std::this_thread::sleep_for to wait 1ms between two cycles. 
    auto oldTime = std::chrono::system_clock::now();
    double thresh = rand() % 6 + 4;
    while (true) 
    {
        // sleep for 1ms
        std::this_thread::sleep_for(std::chrono::milliseconds(1));
        auto newTime = std::chrono::system_clock::now();
        auto elapsedTime = newTime - oldTime;
        if (std::chrono::duration<double>(elapsedTime).count() >= thresh) {
            // flip the traffic light phase
            if ( this->_currentPhase == TrafficLightPhase::green ) {
                this->_currentPhase = TrafficLightPhase::red;
            }
            else
            {
                this->_currentPhase = TrafficLightPhase::green;
            }

            _queue.send(std::move(_currentPhase));

            thresh = rand() % 6 + 4;
            oldTime = std::chrono::system_clock::now();
        }
    }
}