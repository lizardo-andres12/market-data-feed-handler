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

#include "lib/messages.hpp"
#include "lib/orderbook.hpp"
#include "lib/spsc_queue.hpp"
#include "lib/vwap_tracker.hpp"


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

int main() {
    SPSCQueue<MarketDataMessage, 4096> queue;
    MarketDataMessage msg;
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
	MarketDataMessage currentMsg;
	std::uint8_t type;

	std::uint64_t processedCount { 0 };

	for (std::uint64_t i = 0; i < numExpectedMessages; ++i) {
	    MarketDataMessage msg;
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
	OrderBookEntry newEntry;

	std::uint64_t processedCount { 0 };

	while (processedCount < numExpectedMessages) {
	    if (queue.dequeue(currentMsg)) {
		if (currentMsg.type == MessageType::Trade) {
		    const TradeMessage& msg = currentMsg.trade;
		    vwapTracker.upsertVWAP(msg.symbol, msg.price, msg.quantity);
		} else {
		    const QuoteMessage& msg = currentMsg.quote;
		    newEntry = {
			msg.timestamp, msg.bidPrice, msg.askPrice, msg.bidQuantity, msg.askQuantity
		    };

		    book.upsertEntry(msg.symbol, newEntry);
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

    return 0;
}

