#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/wait.h>
#include <unistd.h>

int pid1, pid2;
// 定义两个进程变量
int main() {
    int fd[2];
    char InPipe[5000];
    // 定义读缓冲区
    char* c1 = "1";
    char* c2 = "2";
    pipe(fd);
    // 创建管道
    while ((pid1 = fork()) == -1) {
        // 如果进程 1 创建不成功,则空循环
    }

    if (pid1 == 0) {
        // 如果子进程 1 创建成功,pid1 为进程号
        // lockf(fd[1], 1, 0); // 锁定管道
        for (int i = 0; i < 2000; ++i) {
            write(fd[1], c1, strlen(c1));  // 分 2000 次每次向管道写入字符’1’
        }
        sleep(3);  // 等待读进程读出数据
        // lockf(fd[1], 0, 0); // 解除管道的锁定
        exit(0);  // 结束进程 1
    } else {
        while ((pid2 = fork()) == -1) {
            // 若进程 2 创建不成功,则空循环
        }
        if (pid2 == 0) {
            // lockf(fd[1], 1, 0);
            for (int i = 0; i < 2000; ++i) {
                write(fd[1], c2,
                      strlen(c2));  // 分 2000 次每次向管道写入字符’2’
            }
            sleep(3);
            // lockf(fd[1], 0, 0);
            exit(0);
        } else {
            wait(0);  // 等待子进程 1 结束
            wait(0);  // 等待子进程 2 结束
            ssize_t bytes_read = read(
                fd[0], InPipe, sizeof(InPipe) - 1);  // 从管道中读出 4000 个字符
            InPipe[bytes_read] = '\0';               // 加字符串结束符
            printf("%s\n", InPipe);                  // 显示读出的数据
            exit(0);                                 // 父进程结束
        }
    }
}