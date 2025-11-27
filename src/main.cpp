#include <cassert>
#include <chrono>
#include <cstddef>
#include <cstdint>
#include <cstdio>
#include <cstring>
#include <iomanip>
#include <iostream>
#include <thread>

#include <sys/stat.h>
#include <sys/mman.h>
#include <unistd.h>

#include "messages.hpp"
#include "orderbook/orderbook.hpp"
#include "ringbuffer/spsc_queue.hpp"
#include "vwap_tracker/vwap_tracker.hpp"


/**
 * @brief Helper function to hold all end-of-execution output logic.
 *
 * @param book The orderbook used in execution.
 * @param vwap The VWAP tracker used in execution.
 * @param totalMessages The total count of messages processed.
 * @param elapsedMs The time from start of execution to end of execution.
 */
void printResults(const OrderBook& book, const VWAPTracker& vwap, std::uint64_t totalMessages, double elapsedMs) {
    book.showState();
    vwap.showStats();

    std::cout << "\n=== Performance Metrics ===\n";
    std::cout << "Total messages processed: " << totalMessages << '\n';
    std::cout << "Processing time: " << std::fixed << std::setprecision(2) 
	<< elapsedMs << " ms\n";

    double throughput = totalMessages / (elapsedMs / 1000.0);
    std::cout << "Throughput: " << std::fixed << std::setprecision(0) 
	<< throughput << " msgs/sec\n";

    double latency_us = (elapsedMs * 1000.0) / totalMessages;
    std::cout << "Average latency per message: " << std::fixed << std::setprecision(2)
	<< latency_us << " μs\n";
}

/**
 * @brief The entry point of the program. This program simulates reading a stream market exchange data and processing that data by maintaining an in-memory copy of the Orderbook and relevant statistics used in trading strategies. Data is processed in little-endian format and inserted into a lock-free queue by a single producer thread, and a single consumer thread dequeues the messages and updates the in-memory data structures accordingly. 
 */
int main() {
    SPSCQueue<MarketDataMessage, 8192> queue;
    OrderBook book;
    VWAPTracker vwapTracker;
    std::uint8_t* bufferPtr;

    if (freopen(nullptr, "rb", stdin) == NULL) {
	perror("freopen");
	return 1;
    }

    const int STDIN_FD = fileno(stdin);
    if (STDIN_FD == -1) {
	perror("fileno");
	return 1;
    }

    struct stat st;
    if (fstat(STDIN_FD, &st) == -1) {
	perror("fstat");
	return 1;
    }

    void* mappedData = mmap(nullptr, st.st_size, PROT_READ, MAP_PRIVATE, STDIN_FD, 0);
    if (mappedData == MAP_FAILED) {
        perror("mmap");
        return 1;
    }

    bufferPtr = static_cast<std::uint8_t*>(mappedData);
    const std::uint64_t numExpectedMessages { *reinterpret_cast<std::uint64_t*>(bufferPtr) };
    bufferPtr += sizeof(std::uint64_t);

    const auto producerFunctor = [&queue, &bufferPtr](const std::uint64_t numExpectedMessages) {
	MarketDataMessage msg;
	std::uint8_t type;

	for (std::uint64_t i = 0; i < numExpectedMessages; ++i) {
	    if (i + 1 < numExpectedMessages) {
		__builtin_prefetch(bufferPtr + 64, 0, 3);
	    }
	    type = *bufferPtr;

	    if (static_cast<MessageType>(type) == MessageType::Trade) {
		std::memcpy(&msg.trade, bufferPtr, sizeof(TradeMessage));
		bufferPtr += sizeof(TradeMessage);
	    } else {
		std::memcpy(&msg.quote, bufferPtr, sizeof(QuoteMessage));
		bufferPtr += sizeof(QuoteMessage);
	    }
	    while (!queue.enqueue(msg)) {
		std::this_thread::yield();
	    }
	}
    };

    const auto consumerFunctor = [&queue, &book, &vwapTracker](const std::uint64_t numExpectedMessages) {
	MarketDataMessage currentMsg;
	std::uint64_t processedCount { 0 };

	while (processedCount < numExpectedMessages) {
	    if (queue.dequeue(currentMsg)) {
		if (currentMsg.type == MessageType::Trade) {
		    vwapTracker.upsertVWAP(currentMsg.trade.symbol, currentMsg.trade);
		} else {
		    book.upsertEntry(currentMsg.quote.symbol, currentMsg.quote);
		}
		++processedCount;
	    } else { /// SPSCQueue::dequeue is empty, wait for producer to enqueue
		std::this_thread::yield();
	    }
	}
	return;
    };

    auto start = std::chrono::high_resolution_clock::now();

    std::thread producer(producerFunctor, numExpectedMessages);
    std::thread consumer(consumerFunctor, numExpectedMessages);

    producer.join();
    consumer.join();

    auto end = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double, std::milli> duration = end - start;
    printResults(book, vwapTracker, numExpectedMessages, duration.count());

    if (munmap(mappedData, st.st_size)) {
        perror("munmap");
        return 1;
    }

    return 0;
}

