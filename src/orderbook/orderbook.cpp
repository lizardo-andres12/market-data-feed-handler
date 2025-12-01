#include <cstring>
#include <iomanip>
#include <iostream>

#include "orderbook/orderbook.hpp"

auto OrderBook::getEntry(const std::uint64_t key) const -> std::optional<const OrderBookEntry*> {
    auto it = book_.find(key);
    if (it == book_.end()) {
        return std::nullopt;
    }
    return &it->second;
}

void OrderBook::upsertEntry(const std::uint64_t key, const QuoteMessage& msg) {
    auto it = book_.find(key);
    if (it == book_.end()) {
        book_[key] = {msg.timestamp, msg.bidPrice, msg.askPrice, msg.bidQuantity, msg.askQuantity};
        return;
    }
    it->second = {msg.timestamp, msg.bidPrice, msg.askPrice, msg.bidQuantity, msg.askQuantity};
}

void OrderBook::showState() const {
    std::cout << "\n=== Order Books (Final State) ===\n";
    std::cout << std::left << std::setw(12) << "Symbol" << std::right << std::setw(12)
              << "Bid Price" << std::setw(12) << "Bid Qty" << std::setw(12) << "Ask Price"
              << std::setw(12) << "Ask Qty" << std::setw(15) << "Last Update\n";
    std::cout << std::string(73, '-') << '\n';

    char symStr[8] = {0};
    for (const auto& [symbol, book] : book_) {
        std::memcpy(symStr, &symbol, sizeof(symbol));
        std::cout << std::left << std::setw(12) << symStr << std::right << std::fixed
                  << std::setprecision(2) << std::setw(12) << (book.bidPrice / 100.0)
                  << std::setw(12) << book.bidQuantity << std::setw(12) << (book.askPrice / 100.0)
                  << std::setw(12) << book.askQuantity << std::setw(15) << book.udpatedAt << '\n';
    }
}
