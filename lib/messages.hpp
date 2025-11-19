#ifndef MESSAGES_HPP
#define MESSAGES_HPP

#include <cstdint>


struct __attribute__((packed)) TradeMessage {
    std::uint8_t type;
    std::uint64_t timestamp;
    std::uint64_t symbol;
    std::uint64_t price;
    std::uint32_t quantity;
    std::uint8_t padding[3];
};

struct __attribute__((packed)) QuoteMessage {
    std::uint8_t type;
    std::uint64_t timestamp;
    std::uint64_t symbol;
    std::uint64_t bidPrice;
    std::uint32_t bidQuantity;
    std::uint64_t askPrice;
    std::uint32_t askQuantity;
    std::uint8_t padding[3];
};

union MarketDataMessage {
    std::uint8_t type;
    TradeMessage trade;
    QuoteMessage quote;
};


#endif

