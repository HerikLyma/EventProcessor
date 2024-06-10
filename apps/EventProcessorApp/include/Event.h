#pragma once

#include <atomic>

// Class representing an event with a value and counters for consumption and production
class Event
{
public:
    // Default constructor
    Event() = default;

    // Constructor with value initialization
    explicit Event(const int value) noexcept
        : value_(value)
    {}

    // Default destructor
    ~Event() = default;

    // Method to increment the consumption counter by 1
    void execute() const noexcept { ++Event::consumed; }

    // Atomic counter for consumed events, initialized to 0
    static inline std::atomic_uint_fast64_t consumed{0};

    // Atomic counter for produced events, initialized to 0
    static inline std::atomic_uint_fast64_t produced{0};

private:
    // Value associated with the event, initialized to 0
    int value_{0};
};
