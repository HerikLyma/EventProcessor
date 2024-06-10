#include <thread>
#include <cstdint>
#include <iostream>

#include <Event.h>
#include <EventProcessorMutex.h>
#include <EventProcessorLockFree.h>

// Enumeration to specify the type of event processor
enum class EventProcessorTypes : uint8_t { Mutex, LockFree };

// Define the selected event processor type
constexpr auto eventProcessorType = EventProcessorTypes::Mutex;

// Concept to restrict to specific EventProcessor types
template<typename T>
concept EventProcessorConcept = std::is_same_v<T, EventProcessorLockFree<typename T::EvtType>>
                             || std::is_same_v<T, EventProcessorMutex<typename T::EvtType>>;

// Function template to run event processing
template<typename EventProcessorType> requires EventProcessorConcept<EventProcessorType>
void runEventProcessing(const size_t producersCount = 3, const uint64_t maxEvents = 100'000'000)
{
    // Output line for visual separation
    static std::string line(60, '-');
    std::cout << line << std::endl;

    // Output the name of the EventProcessorType being used
    std::cout << typeid(EventProcessorType).name() << std::endl;

    // Output information about the event processing configuration
    std::cout << line << "\nProducers: " << producersCount << std::endl;
    std::cout << "Consumer: 1" << std::endl;
    std::cout << "Max Events: " << maxEvents << std::endl;

    // Create an instance of the specified EventProcessorType
    EventProcessorType event_processor(1024);
    std::chrono::high_resolution_clock::time_point beg;

    {
        // Atomic flag to synchronize thread start
        std::atomic_bool start{false};

        // Atomic counter for produced events
        std::atomic_int produced{0};

        // Create a vector of producer threads
        std::vector<std::jthread> producers(producersCount);
        for (auto &producer : producers) {
            producer = std::jthread([&] {
                // Wait until the start flag is set
                while (!start.load())
                    std::this_thread::yield();

                // Produce events until the maximum number is reached
                while (Event::produced.load() < maxEvents) {
                    // Reserve an event of type Event using the event_processor
                    event_processor.template reserve<Event>(produced++);
                }
            });
        }

        // Create a consumer thread
        std::jthread consumer([&] {
            // Wait until the start flag is set
            while (!start.load())
                std::this_thread::yield();

            // Variable to track the consumed events
            uint64_t consumed = 0;

            // Consume events until the maximum number is reached
            while ((consumed = Event::consumed.load()) < maxEvents) {
                // Commit the event with index consumed using the event_processor
                event_processor.commit(consumed++);
            }
        });

        // Set the start flag to true to start the threads
        start.store(true);

        // Record the start time
        beg = std::chrono::high_resolution_clock::now();
    }

    // Record the end time
    const auto end = std::chrono::high_resolution_clock::now();

    // Calculate the average latency per event
    const auto diff = std::chrono::duration_cast<std::chrono::nanoseconds>(end - beg)
                          .count() / maxEvents;

    // Output the average latency per event
    std::cout << "AVG per event: " << diff << "ns" << std::endl;
}

// Main function
int main()
{
    // Run event processing based on the selected event processor type
    if constexpr (eventProcessorType == EventProcessorTypes::LockFree) {
        // Run the event processing with EventProcessorLockFree
        runEventProcessing<EventProcessorLockFree<Event>>();
    } else if (eventProcessorType == EventProcessorTypes::Mutex) {
        // Run the event processing with EventProcessorMutex
        runEventProcessing<EventProcessorMutex<Event>>();
    }

    return 0;
}
