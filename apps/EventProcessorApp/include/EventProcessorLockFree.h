#pragma once

#include <atomic>
#include <vector>

// For SSE intrinsics
#include <emmintrin.h>

// Include necessary headers
#include <Event.h>
#include <EventSlot.h>
#include <MathUtils.h>

// Class template for a lock-free event processor
template<typename E>
class EventProcessorLockFree
{
public:
    using EvtType = E;
    // Constructor accepting the capacity of the event processor
    explicit EventProcessorLockFree(const size_t capacity) noexcept
        : m_capacity(is_power_of_two(capacity) ? (capacity - 1) : (next_power_of_two(capacity) - 1))
        , m_events(m_capacity + 1)
    {
    }

    // Function template to reserve space for an event
    template<typename T, typename... Args>
    const E* reserve(Args&&... args) noexcept
    {
        size_t index = 0;

        while (true) {
            //Gets the next index position
            index = m_nextSequenceNumber++ & m_capacity;
            auto& event = m_events[index];

            // If an event is not Reserved or Committed, returns it, otherwise goes to the next one
            auto expectedType = EventType::None;
            auto expectedVersion = event.version_.load(std::memory_order_acquire);

            // Atomically check and set the event's type to Reserved
            if (event.type_.compare_exchange_strong(expectedType, EventType::Reserved, std::memory_order_acq_rel)) [[likely]] {
                // Increment the version number of the event to mitigate the ABA problem in concurrent access scenarios.
                // The ABA problem occurs when a value changes from A to B and then back to A, potentially leading to unexpected results
                // if a thread incorrectly assumes the value hasn't changed. By incrementing the version number atomically upon each modification,
                // even if the value returns to a previous state, the version will have been updated, avoiding false matches.
                // Using 'memory_order_release' ensures that the increment operation completes before any subsequently stored operations,
                // maintaining data consistency.
                event.version_.store(expectedVersion + 1, std::memory_order_release);
                ++Event::produced;
                // Construct the event in place
                new (&event.event_) E(std::forward<Args>(args)...);
                return &event.event_;
            }
            // Pause instruction for spin-wait
            _mm_pause();
        }

        return nullptr;
    }

    // Method to commit an event by its sequence number
    void commit(const size_t sequence_number) noexcept { commit(sequence_number, 1); }

    // Method to commit multiple events starting from a sequence number
    void commit(const size_t sequence_number, const size_t count) noexcept
    {
        const auto last = (sequence_number + count - 1) & m_capacity;

        size_t startAt = 0;
        size_t index = 0;

        while (true) {
            index = (sequence_number + startAt) & m_capacity;

            auto& event = m_events[index];

            auto expectedType = EventType::Reserved;
            auto expectedVersion = event.version_.load(std::memory_order_acquire);

            // Atomically check and set the event's type to Committed
            if (event.type_.compare_exchange_strong(expectedType, EventType::Committed, std::memory_order_acq_rel)) [[likely]] {
                // Execute the event
                event.event_.execute();
                // Reset the event's type and increment its version
                event.type_.store(EventType::None, std::memory_order_release);
                // Increment the version number of the event to mitigate the ABA problem in concurrent access scenarios.
                // The ABA problem occurs when a value changes from A to B and then back to A, potentially leading to unexpected results
                // if a thread incorrectly assumes the value hasn't changed. By incrementing the version number atomically upon each modification,
                // even if the value returns to a previous state, the version will have been updated, avoiding false matches.
                // Using 'memory_order_release' ensures that the increment operation completes before any subsequently stored operations,
                // maintaining data consistency.
                event.version_.store(expectedVersion + 1, std::memory_order_release);
                ++startAt;
                if (startAt == count) {
                    return;
                }
                continue;
            }

            // Pause instruction for spin-wait
            _mm_pause();
        }
    }

private:
    const size_t m_capacity; // Capacity of the event processor
    std::vector<EventSlotAtomic<E>> m_events; // Vector of event slots
    std::atomic_size_t m_nextSequenceNumber{ 0 }; // Atomic counter for the next sequence number
};
