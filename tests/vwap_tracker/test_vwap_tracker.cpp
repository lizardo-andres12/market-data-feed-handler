#include <cstdint>
#include <gtest/gtest.h>

#include "messages.hpp"
#include "vwap_tracker/vwap_tracker.hpp"


class VWAPTrackerTest : public testing::Test {
protected:
    void SetUp() override {
	TradeMessage msg = getDefaultMsg();
	tracker_.upsertVWAP(msg.symbol, msg);

	msg.timestamp += 50;
	tracker_.upsertVWAP(msg.symbol, msg);

	msg.timestamp += 50;
	tracker_.upsertVWAP(msg.symbol, msg);
    }

    TradeMessage getDefaultMsg() const {
	TradeMessage msg;
	msg.type = static_cast<MessageType>(1);
	msg.timestamp = std::uint64_t { 1'000'000 };
	msg.symbol = std::uint64_t { 1 };
	msg.price = std::uint64_t { 15'000 };
	msg.quantity = std::uint32_t { 100 };
	return msg;
    }

    VWAPTracker tracker_;
    VWAPTracker emptyTracker_;
};

TEST_F(VWAPTrackerTest, DefaultConstructor) {
    EXPECT_EQ(0u, emptyTracker_.size());
}

TEST_F(VWAPTrackerTest, GetVWAPSuccess) {
    auto vwap = tracker_.getVWAP(std::uint64_t { 1 });
    ASSERT_TRUE(vwap.has_value());

    auto value = *vwap.value();
    EXPECT_EQ(value.updatedAt, std::uint64_t { 1'000'100 });
    EXPECT_EQ(value.totalPriceByQuantity, std::uint64_t { 4'500'000 });
    EXPECT_EQ(value.totalQuantity, std::uint64_t { 300 });
    EXPECT_EQ(value.totalTrades, std::uint32_t { 3 });
}

TEST_F(VWAPTrackerTest, GetVWAPFailure) {
    auto vwap = emptyTracker_.getVWAP(std::uint64_t { 1 });
    EXPECT_FALSE(vwap.has_value());
}

TEST_F(VWAPTrackerTest, Size) {
    EXPECT_EQ(1u, tracker_.size());
}

