#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
#include <iostream>

#define PROCESS_NAME_LEN 32   /*进程名长度*/
#define MIN_SLICE 10          /*最小碎片的大小*/
#define DEFAULT_MEM_SIZE 1024 /*内存大小*/
#define DEFAULT_MEM_START 0   /*起始位置*/
/* 内存分配算法 */
#define MA_FF 1
#define MA_BF 2
#define MA_WF 3

/*描述每一个空闲块的数据结构*/
struct free_block_type {
    int size;
    int start_addr;
    struct free_block_type* next;
};

/*每个进程分配到的内存块的描述*/
struct allocated_block {
    int pid;
    int size;
    int start_addr;
    char process_name[PROCESS_NAME_LEN];
    struct allocated_block* next;
};

class MM {
   public:
    int mem_size = DEFAULT_MEM_SIZE; /*内存大小*/
    int ma_algorithm = MA_FF;        /*当前分配算法*/
    int pid = 0;                     /*初始 pid*/
    int flag = 0;                    /*设置内存大小标志*/

   public:
    /*指向内存中空闲块链表的首指针*/
    struct free_block_type* free_block;

    /*进程分配内存块链表的首指针*/
    struct allocated_block* allocated_block_head = NULL;

    MM();
    MM(int size);
    ~MM();
    struct free_block_type* init_free_block(int mem_size);
    struct free_block_type* init_free_block(int mem_size, int start_addr);
    int remain_free_block_size();
    void display_menu();
    int set_mem_size();
    void set_algorithm(int algorithm);
    void rearrange(int algorithm);
    void rearrange_FF();
    void rearrange_BF();
    void rearrange_WF();
    int new_process(int size);
    int allocate_mem(struct allocated_block* ab);
    struct allocated_block* find_process(int pid);
    void kill_process(int pid);
    void do_exit();
    int free_mem(struct allocated_block* ab);
    int dispose(struct allocated_block* free_ab);
    int dispose(struct free_block_type* free_ab);
    int display_mem_usage();
    struct free_block_type* sort_free_block();
    struct free_block_type* merge_free_block();
};

MM::MM() {
    free_block = init_free_block(mem_size);
}

MM::MM(int size) {
    free_block = init_free_block(size);
}

MM::~MM() {
    do_exit();
}

/*初始化空闲块，默认为一块，可以指定大小及起始地址*/
struct free_block_type* MM::init_free_block(int mem_size) {
    struct free_block_type* fb;
    fb = (struct free_block_type*)malloc(sizeof(struct free_block_type));
    if (fb == NULL) {
        printf("No mem\n");
        return NULL;
    }
    fb->size = mem_size;
    fb->start_addr = DEFAULT_MEM_START;
    fb->next = NULL;
    return fb;
}

struct free_block_type* MM::init_free_block(int mem_size, int start_addr) {
    struct free_block_type* fb;
    fb = (struct free_block_type*)malloc(sizeof(struct free_block_type));
    if (fb == NULL) {
        printf("No mem\n");
        return NULL;
    }
    fb->size = mem_size;
    fb->start_addr = start_addr;
    fb->next = NULL;
    return fb;
}

/*显示菜单*/
void MM::display_menu() {
    printf("\n");
    printf("1 - Set memory size (default = %d)\n", DEFAULT_MEM_SIZE);
    printf("2 - Select memory allocation algorithm\n");
    printf("3 - New process \n");
    printf("4 - Terminate a process \n");
    printf("5 - Display memory usage \n");
    printf("6 - Exit\n");
}

/*设置内存的大小*/
int MM::set_mem_size() {
    int size;
    if (flag != 0) {
        // 防止重复设置
        printf("Cannot set memory size again\n");
        return 0;
    }
    printf("Total memory size =");
    scanf("%d", &size);
    if (size > 0) {
        mem_size = size;
        free_block->size = mem_size;
    }
    flag = 1;
    return 1;
}

/* 设置当前的分配算法 */
void MM::set_algorithm(int algorithm) {
    // int algorithm;
    // printf("\t1 - First Fit\n");
    // printf("\t2 - Best Fit \n");
    // printf("\t3 - Worst Fit \n");
    // scanf("%d", &algorithm);
    if (algorithm >= 1 && algorithm <= 3) {
        ma_algorithm = algorithm;
    }
    // 按指定算法重新排列空闲区链表
    rearrange(ma_algorithm);
}

/*按指定的算法整理内存空闲块链表*/
void MM::rearrange(int algorithm) {
    switch (algorithm) {
        case MA_FF:
            rearrange_FF();
            break;
        case MA_BF:
            rearrange_BF();
            break;
        case MA_WF:
            rearrange_WF();
            break;
    }
}

/*按 FF 算法重新整理内存空闲块链表*/
void MM::rearrange_FF() {
    if (free_block == NULL)
        return;
    struct free_block_type* fb_head;
    struct free_block_type dummy;
    struct free_block_type* tail = &dummy;

    struct free_block_type* i = free_block;
    int count = 0;
    while (i != NULL) {
        count++;
        i = i->next;
    }

    while (count) {
        fb_head = free_block;
        struct free_block_type* min_addr = fb_head;
        while (fb_head != NULL) {
            if (fb_head->start_addr < min_addr->start_addr) {
                min_addr = fb_head;
            }
            fb_head = fb_head->next;
        }
        tail->next = init_free_block(min_addr->size, min_addr->start_addr);
        tail = tail->next;
        dispose(min_addr);

        count--;
    }

    free_block = dummy.next;
}

/*按 BF 算法重新整理内存空闲块链表*/
void MM::rearrange_BF() {
    struct free_block_type* fb_head;
    struct free_block_type dummy;
    struct free_block_type* tail = &dummy;

    struct free_block_type* i = free_block;
    int count = 0;
    while (i != NULL) {
        count++;
        i = i->next;
    }

    while (count) {
        fb_head = free_block;
        struct free_block_type* min_size = fb_head;
        while (fb_head != NULL) {
            if (fb_head->size < min_size->size) {
                min_size = fb_head;
            }
            fb_head = fb_head->next;
        }
        tail->next = init_free_block(min_size->size, min_size->size);
        tail = tail->next;
        dispose(min_size);

        count--;
    }

    free_block = dummy.next;
}

/*按 WF 算法重新整理内存空闲块链表*/
void MM::rearrange_WF() {
    struct free_block_type* fb_head;
    struct free_block_type dummy;
    struct free_block_type* tail = &dummy;

    struct free_block_type* i = free_block;
    int count = 0;
    while (i != NULL) {
        count++;
        i = i->next;
    }

    while (count) {
        fb_head = free_block;
        struct free_block_type* max_size = fb_head;
        while (fb_head != NULL) {
            if (fb_head->size > max_size->size) {
                max_size = fb_head;
            }
            fb_head = fb_head->next;
        }
        tail->next = init_free_block(max_size->size, max_size->size);
        tail = tail->next;
        dispose(max_size);

        count--;
    }

    free_block = dummy.next;
}

/*创建新的进程，主要是获取内存的申请数量*/
int MM::new_process(int size) {
    struct allocated_block* ab;
    // int size;
    int ret;
    ab = (struct allocated_block*)malloc(sizeof(struct allocated_block));
    if (!ab) {
        exit(-5);
    }
    ab->next = NULL;
    pid++;
    sprintf(ab->process_name, "PROCESS-%02d", pid);
    ab->pid = pid;
    // printf("Memory for %s:", ab->process_name);
    // scanf("%d", &size);
    if (size > 0)
        ab->size = size;
    ret = allocate_mem(ab);
    /* 从空闲区分配内存，ret==1 表示分配 ok*/
    /* 如果此时 allocated_block_head 尚未赋值，则赋值*/
    if ((ret == 1) && (allocated_block_head == NULL)) {
        allocated_block_head = ab;
        return 1;
    }
    /*分配成功，将该已分配块的描述插入已分配链表*/
    else if (ret == 1) {
        ab->next = allocated_block_head;
        allocated_block_head = ab;
        return 2;
    } else if (ret == -1) {
        /*分配不成功*/
        printf("Allocation fail\n");
        free(ab);
        return -1;
    }
    return 3;
}

/*分配内存模块*/
int MM::allocate_mem(struct allocated_block* ab) {
    int request_size = ab->size;
    // 根据当前算法在空闲分区链表中搜索合适空闲分区进行分配，分配时注意以下情况：
    // 1. 找到可满足空闲分区且分配后剩余空间足够大，则分割
    // 2. ? 找到可满足空闲分区且但分配后剩余空间比较小，则一起分配 ?
    // 3. 找不可满足需要的空闲分区但空闲分区之和能满足需要，则采用内存紧缩技术
    //    进行空闲分区的合并，然后再分配
    // 4. 在成功分配内存后，应保持空闲分区按照相应算法有序
    // 5. 分配成功则返回 1，否则返回-1

    if (remain_free_block_size() >= request_size) {
        struct free_block_type* fb_head = free_block;
        while (fb_head != NULL) {
            if (fb_head->size >= request_size) {
                ab->start_addr = fb_head->start_addr;
                fb_head->size -= request_size;
                fb_head->start_addr += request_size;
                rearrange(ma_algorithm);
                return 1;
            }
            fb_head = fb_head->next;
        }

        // 内存紧缩
        sort_free_block();
        merge_free_block();

        fb_head = free_block;
        while (fb_head != NULL) {
            if (fb_head->size >= request_size) {
                ab->start_addr = fb_head->start_addr;
                fb_head->size -= request_size;
                fb_head->start_addr += request_size;
                rearrange(ma_algorithm);
                return 1;
            }
            fb_head = fb_head->next;
        }

        // 满足要求第2条，但是没有写测试用例
        // int min_frag = 50;
        // struct free_block_type* fb_head = free_block;
        // while (fb_head != NULL) {
        //     if (fb_head->size >= request_size + min_frag) {
        //         ab->start_addr = fb_head->start_addr;
        //         fb_head->size -= request_size;
        //         fb_head->start_addr += request_size;
        //         rearrange(ma_algorithm);
        //         return 1;
        //     } else if (fb_head->size >= request_size) {
        //         ab->start_addr = fb_head->start_addr;
        //         dispose(fb_head);
        //         rearrange(ma_algorithm);
        //         return 1;
        //     }
        //     fb_head = fb_head->next;
        // }
    }

    return -1;
}

struct allocated_block* MM::find_process(int pid) {
    struct allocated_block* ab = allocated_block_head;
    while (ab != NULL) {
        if (ab->pid == pid) {
            return ab;
        }
        ab = ab->next;
    }
    return NULL;
}

/*删除进程，归还分配的存储空间，并删除描述该进程内存分配的节点*/
void MM::kill_process(int pid) {
    struct allocated_block* ab;
    // int pid;
    // printf("Kill Process, pid=");
    // scanf("%d", &pid);
    ab = find_process(pid);
    if (ab != NULL) {
        free_mem(ab); /*释放 ab 所表示的分配区*/
        dispose(ab);  /*释放 ab 数据结构节点*/
    }
}

void MM::do_exit() {
    for (int i = 1; i <= pid; ++i) {
        kill_process(i);
    }
}

// 按地址升序排列空闲页
struct free_block_type* MM::sort_free_block() {
    struct free_block_type* fb_head;
    struct free_block_type dummy;
    struct free_block_type* tail = &dummy;
    if (free_block == NULL || free_block->next == NULL) {
        return free_block;
    }

    struct free_block_type* i = free_block;
    int count = 0;
    while (i != NULL) {
        count++;
        i = i->next;
    }

    while (count) {
        fb_head = free_block;
        struct free_block_type* min_addr = fb_head;
        while (fb_head != NULL) {
            if (fb_head->start_addr < min_addr->start_addr) {
                min_addr = fb_head;
            }
            fb_head = fb_head->next;
        }
        tail->next = init_free_block(min_addr->size, min_addr->start_addr);
        tail = tail->next;
        dispose(min_addr);

        count--;
    }
    return dummy.next;
}

struct free_block_type* MM::merge_free_block() {
    if (free_block == NULL || free_block->next == NULL) {
        return free_block;
    }

    free_block_type dummy;
    free_block_type* tail = &dummy;
    free_block_type* fb_head = free_block;

    while (fb_head != NULL && fb_head->next != NULL) {
        int cur_start_addr = fb_head->start_addr;
        int cur_size = fb_head->size;
        int total_size = cur_size;

        struct free_block_type* next;
        while (fb_head->next != NULL && fb_head->start_addr + fb_head->size ==
                                            fb_head->next->start_addr) {
            next = fb_head->next;
            total_size += fb_head->next->size;
            dispose(fb_head);
            fb_head = next;
        }
        next = fb_head->next;
        if (total_size != cur_size) {
            tail->next = init_free_block(total_size, cur_start_addr);
        } else {
            tail->next = init_free_block(fb_head->size, fb_head->start_addr);
        }
        tail = tail->next;

        dispose(fb_head);
        fb_head = next;
    }
    if (fb_head != NULL) {
        tail->next = init_free_block(fb_head->size, fb_head->start_addr);
        dispose(fb_head);
    }
    return dummy.next;
}

/*将 ab 所表示的已分配区归还，并进行可能的合并*/
int MM::free_mem(struct allocated_block* ab) {
    int algorithm = ma_algorithm;
    struct free_block_type *fbt, *fb_head;
    fb_head = free_block;
    fbt = init_free_block(ab->size, ab->start_addr);
    if (!fbt)
        return -1;
    // 进行可能的合并，基本策略如下
    // 1. 将新释放的结点插入到空闲分区队列末尾
    if (fb_head == NULL) {
        free_block = fbt;
    } else {
        while (fb_head->next != NULL) {
            fb_head = fb_head->next;
        }
        fb_head->next = fbt;
    }

    // 2. 对空闲链表按照地址有序排列
    free_block = sort_free_block();

    // 3. 检查并合并相邻的空闲分区
    free_block = merge_free_block();

    // 4. 将空闲链表重新按照当前算法排序
    rearrange(algorithm);

    return 1;
}

/*释放 fb 数据结构节点*/
int MM::dispose(struct free_block_type* free_ab) {
    struct free_block_type *pre, *fb;
    if (free_ab == free_block) {
        /*如果要释放第一个节点*/
        free_block = free_block->next;
        free(free_ab);
        return 1;
    }
    pre = free_block;
    fb = free_block->next;
    while (fb != free_ab) {
        pre = fb;
        fb = fb->next;
    }
    pre->next = fb->next;
    free(fb);
    return 2;
}

/*释放 ab 数据结构节点*/
int MM::dispose(struct allocated_block* free_ab) {
    struct allocated_block *pre, *ab;
    if (free_ab == allocated_block_head) {
        /*如果要释放第一个节点*/
        allocated_block_head = allocated_block_head->next;
        free(free_ab);
        return 1;
    }
    pre = allocated_block_head;
    ab = allocated_block_head->next;
    while (ab != free_ab) {
        pre = ab;
        ab = ab->next;
    }
    pre->next = ab->next;
    free(ab);
    return 2;
}

/* 显示当前内存的使用情况，包括空闲区的情况和已经分配的情况 */
int MM::display_mem_usage() {
    struct free_block_type* fbt = free_block;
    struct allocated_block* ab = allocated_block_head;
    /* 显示空闲区 */
    printf("----------------------------------------------------------\n");
    printf("Free Memory:\n");
    printf("%20s %20s\n", "start_addr", "size");
    if (fbt == NULL) {
        printf("%20s %20s\n", "NULL", "NULL");
    }
    while (fbt != NULL) {
        printf("%20d %20d\n", fbt->start_addr, fbt->size);
        fbt = fbt->next;
    }

    /* 显示已分配区 */
    printf("\nUsed Memory:\n");
    printf("%10s %20s %10s %10s\n", "PID", "ProcessName", "start_addr",
           " size");
    while (ab != NULL) {
        printf("%10d %20s %10d %10d\n ", ab->pid, ab->process_name,
               ab->start_addr, ab->size);
        ab = ab->next;
    }
    printf("----------------------------------------------------------\n");
    return 0;
}

int MM::remain_free_block_size() {
    int size = 0;
    struct free_block_type* fb_head = free_block;
    while (fb_head != NULL) {
        size += fb_head->size;
        fb_head = fb_head->next;
    }
    return size;
}

// int main() {
//     int choice;
//     pid = 0;
//     free_block = init_free_block(mem_size);  // 初始化空闲区
//     while (1) {
//         display_menu();        // 显示菜单
//         scanf("%d", &choice);  // 获取用户输入
//         switch (choice) {
//             case 1:
//                 set_mem_size();
//                 break;  // 设置内存大小
//             case 2:
//                 set_algorithm();
//                 flag = 1;
//                 break;  // 设置算法
//             case 3:
//                 new_process();
//                 flag = 1;
//                 break;  // 创建新进程
//             case 4:
//                 kill_process();
//                 flag = 1;
//                 break;  // 删除进程
//             case 5:
//                 display_mem_usage();
//                 flag = 1;
//                 break;  // 显示内存使用
//             case 6:
//                 do_exit();
//                 exit(0);
//             default:
//                 break;
//         }
//     }
// }