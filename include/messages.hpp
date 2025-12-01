#pragma once

#include <cstdint>

/**
 * @brief The enumeration of exchange message types. This is needed to correctly interpret message
 * size and contents.
 */
enum class MessageType : std::uint8_t { Trade = 1, Quote };

/**
 * @brief The struct representation of a `TradeMessage` as sent by the exchange. This struct is
 * packed to match the exact binary layout of a `TradeMessage`.
 */
struct __attribute__((packed)) TradeMessage {
    MessageType type;
    std::uint64_t timestamp;
    std::uint64_t symbol;
    std::uint64_t price;
    std::uint32_t quantity;
    std::uint8_t padding[3];
};

/**
 * @brief The struct representation of a `QuoteMessage` as sent by the exchange. This struct is
 * packed to match the exact binary layout of a `QuoteMessage`.
 */
struct __attribute__((packed)) QuoteMessage {
    MessageType type;
    std::uint64_t timestamp;
    std::uint64_t symbol;
    std::uint64_t bidPrice;
    std::uint32_t bidQuantity;
    std::uint64_t askPrice;
    std::uint32_t askQuantity;
    std::uint8_t padding[3];
};

/**
 * @brief Helper union to allow for reinterpretation of messages based on `type` field.
 */
union MarketDataMessage {
    MessageType type;
    TradeMessage trade;
    QuoteMessage quote;
};
