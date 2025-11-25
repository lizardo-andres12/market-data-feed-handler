#ifndef ORDERBOOK_CPP
#define ORDERBOOK_CPP

#include <iomanip>
#include <iostream>

#include "lib/orderbook.hpp"


std::optional<const OrderBookEntry*> OrderBook::getEntry(const __book_key_t key) const {
    auto it = book_.find(key);
    if (it == book_.end()) [[unlikely]] {
	return std::nullopt;
    }
    return &it->second;
}

void OrderBook::upsertEntry(const __book_key_t key, OrderBookEntry& entry) {
    auto it = book_.find(key);
    if (it == book_.end()) {
	book_[key] = std::move(entry);
	return;
    }
    it->second = std::move(entry);
}

void OrderBook::showState() const {
    std::cout << "\n=== Order Books (Final State) ===\n";
    std::cout << std::left << std::setw(12) << "Symbol"
              << std::right << std::setw(12) << "Bid Price"
              << std::setw(12) << "Bid Qty"
              << std::setw(12) << "Ask Price"
              << std::setw(12) << "Ask Qty"
              << std::setw(15) << "Last Update\n";
    std::cout << std::string(73, '-') << '\n';
    
    for (const auto& [symbol, book] : book_) {
        std::cout << std::left << std::setw(12) << symbol
                  << std::right << std::fixed << std::setprecision(2)
                  << std::setw(12) << (book.bidPrice / 100.0)
                  << std::setw(12) << book.bidQuantity
                  << std::setw(12) << (book.askPrice / 100.0)
                  << std::setw(12) << book.askQuantity
                  << std::setw(15) << book.udpatedAt << '\n';
    }
}


#endif

