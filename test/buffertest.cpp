#include <gtest/gtest.h>
#include <random>
#include "../src/buffer.cpp"

TEST(BufferTest, FIFO_TEST) {
    Buffer* buffer = new Buffer(5);
    buffer->set_tasks_randomly(20);
    buffer->simu_FIFO();
}

TEST(BufferTest, BLEADY_TEST) {
    {
        Buffer* buffer = new Buffer(3);
        buffer->set_tasks_handly(
            std::vector<int>{1, 2, 3, 4, 1, 2, 5, 1, 2, 3, 4, 5});
        buffer->simu_FIFO();
    }
    {
        Buffer* buffer = new Buffer(4);
        buffer->set_tasks_handly(
            std::vector<int>{1, 2, 3, 4, 1, 2, 5, 1, 2, 3, 4, 5});
        buffer->simu_FIFO();
    }
}

TEST(BufferTest, LRU_TEST) {
    Buffer* buffer = new Buffer(5);
    buffer->set_tasks_randomly(10);
    buffer->simu_LRU();

    for (int i = 1; i <= 5; i += 2) {
        for (int j = 0; j < 20; ++j) {
            buffer->visit(i);
        }
    }
    buffer->set_tasks_randomly(10);
    buffer->simu_LRU();
}