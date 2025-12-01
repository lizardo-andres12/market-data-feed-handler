#include <cstdint>
#include <gtest/gtest.h>

#include "messages.hpp"
#include "orderbook/orderbook.hpp"

class OrderBookTest : public testing::Test {
  protected:
    void SetUp() override {
        QuoteMessage msg = getDefaultMsg();
        book_.upsertEntry(msg.symbol, msg);

        msg.symbol = std::uint64_t{2};
        msg.timestamp += 50;
        book_.upsertEntry(msg.symbol, msg);

        msg.symbol = std::uint64_t{3};
        msg.timestamp += 50;
        book_.upsertEntry(msg.symbol, msg);
    }

    QuoteMessage getDefaultMsg() const {
        QuoteMessage msg;
        msg.type = static_cast<MessageType>(2);
        msg.timestamp = std::uint64_t{1'000'000};
        msg.symbol = std::uint64_t{1};
        msg.bidPrice = std::uint64_t{15'005};
        msg.bidQuantity = std::uint32_t{100};
        msg.askPrice = std::uint64_t{14'995};
        msg.askQuantity = std::uint32_t{90};
        return msg;
    }

    OrderBook book_;
    OrderBook emptyBook_;
};

TEST_F(OrderBookTest, DefaultConstructor) {
    EXPECT_EQ(0u, emptyBook_.size());
}

TEST_F(OrderBookTest, GetEntrySuccess) {
    auto entry = book_.getEntry(std::uint64_t{1});
    ASSERT_TRUE(entry.has_value());

    auto value = *entry.value();
    EXPECT_EQ(value.udpatedAt, std::uint64_t{1'000'000});
    EXPECT_EQ(value.bidPrice, std::uint64_t{15'005});
    EXPECT_EQ(value.bidQuantity, std::uint32_t{100});
    EXPECT_EQ(value.askPrice, std::uint64_t{14'995});
    EXPECT_EQ(value.askQuantity, std::uint32_t{90});

    entry = book_.getEntry(std::uint64_t{2});
    ASSERT_TRUE(entry.has_value());

    value = *entry.value();
    EXPECT_EQ(value.udpatedAt, std::uint64_t{1'000'050});
    EXPECT_EQ(value.bidPrice, std::uint64_t{15'005});
    EXPECT_EQ(value.bidQuantity, std::uint32_t{100});
    EXPECT_EQ(value.askPrice, std::uint64_t{14'995});
    EXPECT_EQ(value.askQuantity, std::uint32_t{90});

    entry = book_.getEntry(std::uint64_t{3});
    ASSERT_TRUE(entry.has_value());

    value = *entry.value();
    EXPECT_EQ(value.udpatedAt, std::uint64_t{1'000'100});
    EXPECT_EQ(value.bidPrice, std::uint64_t{15'005});
    EXPECT_EQ(value.bidQuantity, std::uint32_t{100});
    EXPECT_EQ(value.askPrice, std::uint64_t{14'995});
    EXPECT_EQ(value.askQuantity, std::uint32_t{90});
}

TEST_F(OrderBookTest, GetEntryNull) {
    auto entry = emptyBook_.getEntry(std::uint64_t{1});
    EXPECT_FALSE(entry.has_value());
}

TEST_F(OrderBookTest, Size) {
    EXPECT_EQ(3u, book_.size());
}
