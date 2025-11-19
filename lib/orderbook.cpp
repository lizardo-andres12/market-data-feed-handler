#ifndef ORDERBOOK_CXX
#define ORDERBOOK_CXX

#include <utility>

#include "lib/orderbook.hpp"


const OrderBookEntry* OrderBook::getEntry(const __book_key_t key) const {
    auto it = book.find(key);
    if (it == book.end()) [[unlikely]] {
	return nullptr;
    }
    return &it->second;
}

bool OrderBook::insertEntry(const __book_key_t key, OrderBookEntry& entry) {
    auto it = book.find(key);
    if (it != book.end()) [[unlikely]] { 
	return false;
    }
    book[key] = std::move(entry);
    return true;
}

bool OrderBook::updateEntry(const __book_key_t key, OrderBookEntry& entry) {
    auto it = book.find(key);
    if (it == book.end()) [[unlikely]] {
	return false;
    }
    it->second = std::move(entry);
    return true;
}


#endif

