#pragma once

#include <new>
#include <atomic>
#include <type_traits>

// Define hardware interference sizes if not available
#ifndef __cpp_lib_hardware_interference_size
    constexpr std::size_t interference_size = sizeof(std::size_t) * 8;
#else
    constexpr std::size_t interference_size = std::hardware_destructive_interference_size;
#endif


// Enumeration for event types
enum class EventType : int { None, Reserved, Committed };

// Concept to restrict types to EventType or std::atomic<EventType>
template<typename T>
concept EventTypeConcept = std::is_same_v<T, EventType>
                           || std::is_same_v<T, std::atomic<EventType>>;

// Concept to check if a type is processable
template<typename E>
concept Processable = requires(E e) {
    {
        e.execute()
    };
};

// Template struct representing the base of an event slot
template<EventTypeConcept T, Processable E>
struct EventSlotBase
{
    // Aligning to avoid false sharing
    alignas(interference_size) T type_{EventType::None};
    alignas(interference_size) E event_;
    alignas(interference_size) std::atomic_uint64_t version_{0};
};

// Alias for an event slot using EventType
template<typename E>
using EventSlot = EventSlotBase<EventType, E>;

// Alias for an event slot using std::atomic<EventType>
template<typename E>
using EventSlotAtomic = EventSlotBase<std::atomic<EventType>, E>;
