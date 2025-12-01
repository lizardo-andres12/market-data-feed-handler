#include <atomic>
#include <chrono>
#include <cstddef>
#include <thread>
#include <gtest/gtest.h>

#include "ringbuffer/spsc_queue.hpp"


class SPSCQueueStressTest : public testing::Test {
protected:
    static constexpr std::size_t QUEUE_SIZE_ = 1024;
    static constexpr std::size_t NUM_MESSAGES_ = 1'000'000;
};

TEST_F(SPSCQueueStressTest, SingleProducerSingleConsumer) {
    SPSCQueue<int, QUEUE_SIZE_> queue;
    std::atomic<bool> producerDone{false};
    std::atomic<std::size_t> messagesProduced{0};
    std::atomic<std::size_t> messagesConsumed{0};
    
    std::thread producer([&]() {
        for (std::size_t i = 0; i < NUM_MESSAGES_; ++i) {
            int value = static_cast<int>(i);
            while (!queue.enqueue(value)) {
                std::this_thread::yield();
            }
            messagesProduced.fetch_add(1, std::memory_order_relaxed);
        }
        producerDone.store(true, std::memory_order_release);
    });
    
    std::thread consumer([&]() {
        int value;
        std::size_t expected = 0;
        
        while (!producerDone.load(std::memory_order_acquire) || !queue.isEmpty()) {
            if (queue.dequeue(value)) {
                EXPECT_EQ(value, static_cast<int>(expected));
                ++expected;
                messagesConsumed.fetch_add(1, std::memory_order_relaxed);
            } else {
                std::this_thread::yield();
            }
        }
    });
    
    producer.join();
    consumer.join();
    
    EXPECT_EQ(messagesProduced.load(), NUM_MESSAGES_);
    EXPECT_EQ(messagesConsumed.load(), NUM_MESSAGES_);
    EXPECT_TRUE(queue.isEmpty());
}

TEST_F(SPSCQueueStressTest, HighContentionBurstLoad) {
    SPSCQueue<int, 64> queue; // smaller queue for higher contention
    std::atomic<bool> producerDone{false};
    std::atomic<std::size_t> failures{0};
    
    constexpr std::size_t BURST_MESSAGES = 100'000;
    
    std::thread producer([&]() {
        for (std::size_t i = 0; i < BURST_MESSAGES; ++i) {
            int value = static_cast<int>(i);
            std::size_t attempts = 0;
            while (!queue.enqueue(value)) {
                ++attempts;
                if (attempts > 1000) {
                    std::this_thread::yield();
                    attempts = 0;
                }
            }
        }
        producerDone.store(true, std::memory_order_release);
    });
    
    std::thread consumer([&]() {
        int value;
        std::size_t expected = 0;
        
        while (!producerDone.load(std::memory_order_acquire) || !queue.isEmpty()) {
            if (queue.dequeue(value)) {
                if (value != static_cast<int>(expected)) {
                    failures.fetch_add(1, std::memory_order_relaxed);
                }
                ++expected;
            }
        }
    });
    
    producer.join();
    consumer.join();
    
    EXPECT_EQ(failures.load(), 0) << "Message ordering violated";
    EXPECT_TRUE(queue.isEmpty());
}

TEST_F(SPSCQueueStressTest, VariableSizeMessages) {
    struct Message {
        std::size_t id;
        std::uint64_t timestamp;
        double value;
    };
    
    SPSCQueue<Message, QUEUE_SIZE_> queue;
    std::atomic<bool> producerDone{false};
    std::atomic<std::size_t> checksum{0};
    
    constexpr std::size_t MSG_COUNT = 500'000;
    
    std::thread producer([&]() {
        for (std::size_t i = 0; i < MSG_COUNT; ++i) {
            Message msg{i, i * 1000, static_cast<double>(i) * 1.5};
            while (!queue.enqueue(msg)) {
                std::this_thread::yield();
            }
            checksum.fetch_add(i, std::memory_order_relaxed);
        }
        producerDone.store(true, std::memory_order_release);
    });
    
    std::thread consumer([&]() {
        Message msg;
        std::size_t localChecksum = 0;
        
        while (!producerDone.load(std::memory_order_acquire) || !queue.isEmpty()) {
            if (queue.dequeue(msg)) {
                EXPECT_EQ(msg.timestamp, msg.id * 1000);
                EXPECT_DOUBLE_EQ(msg.value, static_cast<double>(msg.id) * 1.5);
                localChecksum += msg.id;
            } else {
                std::this_thread::yield();
            }
        }
        
        EXPECT_EQ(localChecksum, checksum.load());
    });
    
    producer.join();
    consumer.join();
}

