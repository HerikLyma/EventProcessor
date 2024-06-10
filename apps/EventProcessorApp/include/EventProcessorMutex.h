#pragma once

#include <mutex>
#include <vector>
#include <cstdlib>
#include <condition_variable>

// Include necessary headers
#include <Event.h>
#include <EventSlot.h>
#include <MathUtils.h>

// Class template for an event processor using mutex
template<typename E>
class EventProcessorMutex
{
public:
    using EvtType = E;
    // Constructor accepting the capacity of the event processor
    explicit EventProcessorMutex(const size_t capacity) noexcept
        : m_capacity(is_power_of_two(capacity) ? (capacity - 1) : (next_power_of_two(capacity) - 1))
        , m_events(m_capacity + 1)
    {}

    // Function template to reserve space for an event
    template<typename T, typename... Args>
    const E* reserve(Args &&...args) noexcept
    {
        size_t index = 0;

        while (true) {
            std::unique_lock lock(m_mutex);
            index = (m_nextSequenceNumber++) & m_capacity;
            auto &event = m_events[index];
            auto type = event.type_;
            // If the event slot is empty, reserve it
            if (type == EventType::None) [[likely]] {
                event.type_ = EventType::Reserved;
                // Construct the event in place
                new (&event.event_) E(std::forward<Args>(args)...);
                lock.unlock();

                // Notify waiting threads
                m_cv.notify_one();
                ++Event::produced;
                return &event.event_;
            }
        }

        return nullptr;
    }

    // Method to commit an event by its sequence number
    void commit(const size_t sequence_number) noexcept { commit(sequence_number, 1); }

    // Method to commit multiple events starting from a sequence number
    void commit(const size_t sequence_number, const size_t count) noexcept
    {
        const size_t last = (sequence_number + count - 1) & m_capacity;

        std::unique_lock lock(m_mutex);

        size_t begin = 0;
        // Wait until all events are reserved
        m_cv.wait(lock, [this, sequence_number, count, &begin] {
            for (size_t i = begin; i < count; ++i) {
                const auto &event = m_events[(sequence_number + i) & m_capacity];
                if (event.type_ != EventType::Reserved) {
                    return false;
                }
                ++begin;
            }
            return true;
        });

        // Commit all reserved events
        for (size_t i = 0; i < count; ++i) {
            const auto element = (sequence_number + i) & m_capacity;
            auto &eventSlot = m_events[element];
            auto &tmp = eventSlot.event_;
            eventSlot.type_ = EventType::Committed;

            lock.unlock();

            // Execute the event
            tmp.execute();

            lock.lock();
            // Reset the event type
            m_events[element].type_ = EventType::None;
        }
    }

private:
    const size_t m_capacity; // Capacity of the event processor
    std::vector<EventSlot<E>> m_events; // Vector of event slots
    size_t m_nextSequenceNumber{0}; // Next sequence number
    std::mutex m_mutex; // Mutex for thread synchronization
    std::condition_variable m_cv; // Condition variable for thread waiting
};
