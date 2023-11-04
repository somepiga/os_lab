#include <gtest/gtest.h>
#include <random>
#include "../buffer.cpp"

TEST(BufferTest, FIFO_TEST) {
    Buffer* buffer = new Buffer(5);
    buffer->set_tasks_randomly(20);
    buffer->simu_FIFO();
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