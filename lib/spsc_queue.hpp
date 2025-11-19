#ifndef SPSC_QUEUE_HPP
#define SPSC_QUEUE_HPP

#include <atomic>
#include <cstddef>
#include <memory>


template <typename T, std::size_t N>
class SPSCQueue {
    std::unique_ptr<T[]> queue_;
    alignas(64) std::atomic<std::size_t> readIdx_ { 0 };
    alignas(64) std::atomic<std::size_t> writeIdx_ { 0 };
public:
    /**
     * @brief Constructor for SPSPQueue class. Necessary parameters are provided through the template.
     */
    SPSCQueue() : queue_ { std::make_unique<T[]>(N) } {
	static_assert(N >= 2, "SPSC Queue can only be instantiated with size that is greater than or equal to 2");
	static_assert((N & (N - 1)) == 0, "SPSC Queue can only be instantiated with size that is power of 2");
    }

    /**
     * @brief Tries to atomically enqueue `item` into the queue. This operation will std::move `item` into the queue.
     *
     * @param item The item to enqueue.
     * @return `true` on successful enqueue or `false` on failure.
     */
    bool enqueue(T& item);

    /**
     * @brief Tries to atomically dequeue by moving the first element into `item`.
     *
     * @param item The new owner of the dequeued item.
     * @return `true` on successful dequeue or `false` on failure.
     */
    bool dequeue(T& item);

    /**
     * @brief Checks if the SPSC Queue is full.
     *
     * @return `true` if the queue is full or `false` if not.
     */
    bool isFull() const {
	const auto readIdx = readIdx_.load(std::memory_order_relaxed);
	const auto writeIdx = writeIdx_.load(std::memory_order_relaxed);

	const auto nextWriteIdx = (writeIdx + 1) & (N - 1);
	return readIdx == nextWriteIdx;
    }

    /**
     * @brief Checks if the SPSC Queue is empty.
     *
     * @return `true` if the queue is empty or `false` if not.
     */
    bool isEmpty() const {
	const auto readIdx = readIdx_.load(std::memory_order_relaxed);
	const auto writeIdx = writeIdx_.load(std::memory_order_relaxed);
	
	return readIdx == writeIdx;
    }

    /**
     * @brief Gets the size of the SPSC Queue.
     *
     * @return The number of elements currently stored in queue.
     */
    std::size_t size() const {
	const auto readIdx = readIdx_.load(std::memory_order_relaxed);
	const auto writeIdx = writeIdx_.load(std::memory_order_relaxed);

	return (writeIdx - readIdx) & (N - 1); /// handles wrap around
    }

    SPSCQueue(const SPSCQueue& queue) = delete;
    SPSCQueue(SPSCQueue&& queue) = delete;
    void operator=(const SPSCQueue& queue) = delete;
    void operator=(SPSCQueue&& queue) = delete;
};


template <typename T, std::size_t N>
bool SPSCQueue<T, N>::enqueue(T& item) {
    const auto readIdx = readIdx_.load(std::memory_order_acquire);
    const auto writeIdx = writeIdx_.load(std::memory_order_relaxed);

    const auto nextWriteIdx = (writeIdx + 1) & (N - 1);
    if (nextWriteIdx == readIdx) {
	return false;
    }

    queue_[writeIdx] = std::move(item);
    writeIdx_.store(nextWriteIdx, std::memory_order_release);
    return true;
}

template <typename T, std::size_t N>
bool SPSCQueue<T, N>::dequeue(T& item) {
    const auto readIdx = readIdx_.load(std::memory_order_relaxed);
    const auto writeIdx = writeIdx_.load(std::memory_order_acquire);

    if (readIdx == writeIdx) {
	return false;
    }

    item = std::move(queue_[readIdx]);
    const auto nextReadIdx = (readIdx + 1) & (N - 1);
    readIdx_.store(nextReadIdx, std::memory_order_release);
    return true;
}


#endif

