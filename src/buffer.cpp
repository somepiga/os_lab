#include <iostream>
#include <queue>
#include <vector>
#define PAGE_SIZE 4096
#define TASK_SIZE 2048

#define FIFO 1
#define LRU 2

using namespace std;

struct Page {
    int pid;
    int page_id;
    int last_visit_time;
};

class Buffer {
   private:
    int page_num;
    int page_size;

    queue<Page> free_page;

    // For FIFO
    queue<Page> ap_FIFO;

    // For LRU
    vector<Page> ap_LRU;

   public:
    int time = 0;
    queue<pair<int, int>> tasks;

    Buffer(int page_num, int page_size = PAGE_SIZE) {
        this->page_num = page_num;
        this->page_size = page_size;
        for (int i = 0; i < page_num; ++i) {
            free_page.emplace(Page{-1, i, 0});
        }
    }

    void visit(int page_id);
    void set_tasks_randomly(int tasks_size);
    void set_tasks_handly(std::vector<int> task_pid);
    void show_tasks();

    void simu(int algorithm);
    void simu_FIFO();
    void simu_LRU();
};

void Buffer::show_tasks() {
    queue<pair<int, int>> temp = tasks;
    cout << "进程队列： ";
    while (!temp.empty()) {
        cout << temp.front().first << " ";
        temp.pop();
    }
    cout << endl;
}

void Buffer::set_tasks_randomly(int tasks_size) {
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<int> dist(1, tasks_size);
    for (int i = 1; i <= tasks_size; ++i) {
        tasks.emplace(std::make_pair(dist(gen), TASK_SIZE));
    }
}

void Buffer::set_tasks_handly(std::vector<int> task_pid) {
    for (auto pid : task_pid) {
        tasks.emplace(std::make_pair(pid, TASK_SIZE));
    }
}

void Buffer::simu(int algorithm) {
    switch (algorithm) {
        case FIFO:
            simu_FIFO();
            break;
        case LRU:
            simu_LRU();
            break;
        default:
            break;
    }
}

void Buffer::simu_FIFO() {
    show_tasks();
    int task_num = tasks.size();
    float miss = 0;
    while (!tasks.empty()) {
        bool hit = false;
        auto [pid, size] = tasks.front();
        tasks.pop();
        queue<Page> temp = ap_FIFO;
        while (!temp.empty()) {
            auto page = temp.front();
            temp.pop();
            if (page.pid == pid) {
                hit = true;
                cout << "命中进程 " << pid << endl;
                break;
            }
        }
        if (!hit) {
            for (int i = 0; i < size / page_size + 1; ++i) {
                if (!free_page.empty()) {
                    auto& page = free_page.front();
                    page.pid = pid;
                    ap_FIFO.emplace(page);
                    free_page.pop();
                    cout << "把空闲页分配给进程" << pid << endl;
                } else {
                    auto& page = ap_FIFO.front();
                    cout << "进程" << page.pid << "被淘汰" << endl;
                    free_page.emplace(Page{-1, i, 0});
                    ap_FIFO.pop();
                    i -= 1;
                }
            }
            miss++;
        }
    }
    cout << "缺页率:" << miss / task_num * 100 << "%" << endl;
}

void Buffer::visit(int pid) {
    for (long unsigned int i = 0; i < ap_LRU.size(); i++) {
        auto& ap = ap_LRU.at(i);
        if (ap.pid == pid) {
            ap.last_visit_time = time;
            time++;
        }
    }
}

void Buffer::simu_LRU() {
    show_tasks();
    int task_num = tasks.size();
    float miss = 0;
    while (!tasks.empty()) {
        bool hit = false;
        auto [pid, size] = tasks.front();
        tasks.pop();
        for (long unsigned int i = 0; i < ap_LRU.size(); ++i) {
            if (ap_LRU[i].pid == pid) {
                hit = true;
                visit(pid);
                cout << "命中进程 " << pid << endl;
                break;
            }
        }
        if (!hit) {
            for (int i = 0; i < size / page_size + 1; ++i) {
                if (!free_page.empty()) {
                    auto& page = free_page.front();
                    page.pid = pid;
                    ap_LRU.push_back(page);
                    visit(pid);
                    free_page.pop();
                    cout << "把空闲页分配给进程" << pid << endl;
                } else {
                    Page min_page{-1, -1, INT32_MAX};
                    auto it = ap_LRU.begin();
                    for (; it != ap_LRU.end(); it++) {
                        if (it->last_visit_time < min_page.last_visit_time) {
                            min_page = *it;
                        }
                    }
                    cout << "进程" << min_page.pid << "被淘汰" << endl;
                    it = ap_LRU.begin();
                    while (it != ap_LRU.end()) {
                        if (it->pid == min_page.pid) {
                            it = ap_LRU.erase(it);
                            break;
                        } else {
                            ++it;
                        }
                    }
                    free_page.emplace(Page{-1, i, 0});
                    i -= 1;
                }
            }
            miss++;
        }
    }
    cout << "缺页率:" << miss / task_num * 100 << "%" << endl;
}