# EventProcessor

The EventProcessor shares similarities with a ThreadPool but has some key differences:

| Feature          | Event Processor                                     | Thread Pool                                      |
|------------------|-----------------------------------------------------|--------------------------------------------------|
| Purpose          | Handles asynchronous events                         | Executes tasks concurrently                      |
| Execution Model  | Event-driven                                        | Thread-based                                     |
| Concurrency      | Single/multi-threaded                               | Multi-threaded                                   |
| Latency          | Low, especially for short events                    | Can vary based on thread count and load          |
| Resource Usage   | Efficient in CPU and memory usage                   | More CPU and memory intensive                    |
| Ordering         | Can process events in order using queue/ring buffer | Tasks can have priorities using priority queue   |


## Results

Both solutions, mutex-based and lock-free, were successfully implemented. The results demonstrated high performance and Ultra Low Latency in both approaches:

- **Mutex Version**: Latency under 756 nanoseconds, even when generating 100 million events with 3 producers and one consumer.
- **Lock-Free Version**: Latency of approximately 86 nanoseconds under the same test conditions.


