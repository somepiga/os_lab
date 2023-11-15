#include <gtest/gtest.h>
#include <random>
#include "../src/mm.cpp"

int remain_free_block_count(MM* mm) {
    int count = 0;
    struct free_block_type* fb_head = mm->free_block;
    while (fb_head != NULL) {
        count++;
        fb_head = fb_head->next;
    }
    return count;
}

void my_test(MM* mm, int mem_size) {
    // 创建100个内存介于100~200的进程
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<int> dist(100, 200);
    int total_size = 0;
    for (int i = 0; i < 100; ++i) {
        int random_size = dist(gen);
        total_size += random_size;
        mm->new_process(random_size);
    }
    EXPECT_EQ(mm->remain_free_block_size(), mem_size - total_size);

    // 进程：隔5个删5个 （删除 1-5, 11-15, 21-25 ...）
    int release_size = 0;
    for (int i = 0; i < 10; ++i) {
        for (int pid = i * 10 + 1; pid <= i * 10 + 5; ++pid) {
            release_size += mm->find_process(pid)->size;
            mm->kill_process(pid);
        }
        EXPECT_EQ(mm->remain_free_block_size(),
                  mem_size - total_size + release_size);
    }
    // 这里在理论上有可能为10,但是服从均匀分布的随机数使得这一可能性为0
    // EXPECT_EQ(remain_free_block_count(mm), 11);

    // 创建10个内存介于100~200的进程
    for (int i = 0; i < 10; ++i) {
        int random_size = dist(gen);
        total_size += random_size;
        mm->new_process(random_size);
    }
    EXPECT_EQ(mm->remain_free_block_size(),
              mem_size - total_size + release_size);

    mm->do_exit();
    EXPECT_EQ(mm->remain_free_block_size(), mem_size);
}

TEST(MMTest, DISABLED_MergeFreeBlock) {
    MM* mm = new MM();
    mm->new_process(100);
    mm->new_process(100);
    mm->new_process(100);
    mm->new_process(100);
    mm->new_process(100);
    EXPECT_EQ(mm->free_block->size, 524);

    mm->kill_process(5);
    mm->kill_process(4);
    mm->kill_process(3);
    EXPECT_EQ(mm->free_block->size, 824);
    mm->kill_process(2);
    mm->kill_process(1);
    EXPECT_EQ(mm->free_block->size, 1024);
}

TEST(MMTest, Hard) {
    {
        int mem_size = 20000;
        MM* mm = new MM(mem_size);
        mm->set_algorithm(MA_FF);
        my_test(mm, mem_size);
        delete (mm);
    }

    {
        int mem_size = 20000;
        MM* mm = new MM(mem_size);
        mm->set_algorithm(MA_BF);
        my_test(mm, mem_size);
        delete (mm);
    }

    {
        int mem_size = 20000;
        MM* mm = new MM(mem_size);
        mm->set_algorithm(MA_WF);
        my_test(mm, mem_size);
        delete (mm);
    }
}

TEST(MMTest, DISABLED_Show) {
    MM* mm = new MM();
    mm->set_algorithm(MA_FF);
    mm->new_process(500);
    mm->new_process(20);
    mm->new_process(500);
    mm->kill_process(2);
    // mm->new_process(450);
    mm->display_mem_usage();
    delete (mm);
}

int main(int argc, char* argv[]) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}