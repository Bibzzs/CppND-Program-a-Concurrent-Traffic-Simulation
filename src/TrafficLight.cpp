#include <iostream>
#include <random>
#include "TrafficLight.h"

/* Implementation of class "MessageQueue" */

template <typename T>
T MessageQueue<T>::Receive()
{
    // FP.5a : The method receive should use std::unique_lock<std::mutex> and _condition.wait() 
    // to wait for and receive new messages and pull them from the queue using move semantics. 
    // The received object should then be returned by the receive function. 
    std::unique_lock<std::mutex> uLock(_mutex);

    _cond.wait(uLock, [this] {return !_queue.empty();}); // pass unique lock to condiation variable
    T msg = std::move(_queue.back()); // copy move the last message
    _queue.pop_back(); // remove last message from the queue

    return msg;
}

template <typename T>
void MessageQueue<T>::Send(T&& msg)
{
    // FP.4a : The method send should use the mechanisms std::lock_guard<std::mutex> 
    // as well as _condition.notify_one() to add a new message to the queue and afterwards send a notification.
    std::lock_guard<std::mutex> uLock(_mutex);
    _queue.push_back(std::move(msg));
    _cond.notify_one();

}


/* Implementation of class "TrafficLight" */


TrafficLight::TrafficLight()
{
    
    _currentPhase = TrafficLightPhase::red;
    _trafficLightID = getID();
    //_messageQueue = std::make_shared(new MessageQueue<TrafficLightPhase>);
}

TrafficLight::~TrafficLight(){}
/*
TrafficLight::TrafficLight(int ID)
{

    _currentPhase = TrafficLightPhase::red;
    _trafficLightID = getID();
    //_messageQueue = std::make_shared(new MessageQueue<TrafficLightPhase>);
} */

void TrafficLight::waitForGreen()
{
    // FP.5b : add the implementation of the method waitForGreen, in which an infinite while-loop 
    // runs and repeatedly calls the receive function on the message queue. 
    // Once it receives TrafficLightPhase::green, the method returns.
    while (true)
    {
        if (_messageQueue.Receive() == TrafficLightPhase::green)
        {
            return;
        }
    }
}

void TrafficLight::simulate()
{
    // FP.2b : Finally, the private method „cycleThroughPhases“ should be started in a thread when the public method „simulate“ is called. To do this, use the thread queue in the base class. 
    // besoin de créer un thread ici.
    cycleThroughPhases();
}


TrafficLightPhase TrafficLight::getCurrentPhase()
{
    return _currentPhase;
}

void TrafficLight::setCurrentPhase(TrafficLightPhase phase)
{
    _currentPhase = phase;
}

int randomInRange(const int min, const int max)
{
    // Create a random number generator engine
    std::random_device rd;  // Seed the random number generator
    std::mt19937 gen(rd()); // Mersenne Twister engine
    std::uniform_int_distribution<int> distribution(min, max); // Uniform distribution between a and b

    // Generate a random number within the specified range
    return distribution(gen);
}

// virtual function which is executed in a thread
void TrafficLight::cycleThroughPhases()
{
    // FP.2a : Implement the function with an infinite loop that measures the time between two loop cycles 
    // and toggles the current phase of the traffic light between red and green and sends an update method 
    // to the message queue using move semantics. The cycle duration should be a random value between 4 and 6 seconds. 
    // Also, the while-loop should use std::this_thread::sleep_for to wait 1ms between two cycles. 
    int min = 4000;
    int max = 6000;
    double cycleDuration = randomInRange(  min,   max);

    std::chrono::time_point<std::chrono::system_clock> lastTLUpdate;
    lastTLUpdate = std::chrono::system_clock::now();
    //_trafficLightID = 99;
    while (true)
    {
        // sleep at every iteration to reduce CPU usage
        std::this_thread::sleep_for(std::chrono::milliseconds(1));

        long timeSinceLastTLUpdate = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now() - lastTLUpdate).count();
        if ( timeSinceLastTLUpdate > cycleDuration)
        { 
            if (getCurrentPhase() == TrafficLightPhase::green)
            {
                setCurrentPhase(TrafficLightPhase::red);
            }
            else { 
                setCurrentPhase(TrafficLightPhase::green); 
            }
            //send update
            _messageQueue.Send(std::move(this->getCurrentPhase()));
            lastTLUpdate = std::chrono::system_clock::now();
            cycleDuration = cycleDuration = randomInRange(min, max);
        }

    }
    

}
