#include <cstddef>
#include <gtest/gtest.h>

#include "ringbuffer/spsc_queue.hpp"


class SPSCQueueTest : public testing::Test {
protected:
    void SetUp() override {
	for (int i = 1; i <= QUEUE_SIZE_; ++i) {
	    queue_.enqueue(i);
	}
    }

    static constexpr int QUEUE_SIZE_ = 8;
    SPSCQueue<int, 2> empty_;
    SPSCQueue<int, 16> queue_;
};

TEST_F(SPSCQueueTest, DefaultConstructor) {
    EXPECT_EQ(0u, empty_.size());
}

TEST_F(SPSCQueueTest, Enqueue) {
    SPSCQueue<int, 2> queue;
    int toInsert = 1;

    EXPECT_TRUE(queue.enqueue(toInsert)); /// queue gets full here
    
    toInsert++;
    EXPECT_FALSE(queue.enqueue(toInsert));
}

TEST_F(SPSCQueueTest, Dequeue) {
    SPSCQueue<int, QUEUE_SIZE_> queue;
    for (int i = 1; i < QUEUE_SIZE_; ++i) {
	queue.enqueue(i);
    }
    int res;
    int expected = 1;

    EXPECT_TRUE(queue.dequeue(res));
    EXPECT_EQ(res, expected);

    expected++;
    EXPECT_TRUE(queue.dequeue(res));
    EXPECT_EQ(res, expected);

    EXPECT_FALSE(empty_.dequeue(res));
    EXPECT_EQ(res, expected); /// failed dequeue should not reassign existing result parameter
}

TEST_F(SPSCQueueTest, IsFull) {
    SPSCQueue<int, QUEUE_SIZE_> queue;
    for (int i = 1; i < QUEUE_SIZE_; ++i) {
	EXPECT_TRUE(queue.enqueue(i));
    }

    EXPECT_TRUE(queue.isFull());
    EXPECT_FALSE(empty_.isFull());
}

TEST_F(SPSCQueueTest, IsEmpty) {
    EXPECT_FALSE(queue_.isEmpty());
    EXPECT_TRUE(empty_.isEmpty());
}

