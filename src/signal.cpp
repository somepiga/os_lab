#include <semaphore.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/wait.h>
#include <unistd.h>
pid_t pid1 = -1, pid2 = -1;

void do_nothing(int signum) {}

void printer(int signum) {
    printf("捕获到信号 %d\n", signum);
}

void my_kill() {
    kill(pid1, 16);
    kill(pid2, 17);
}

void inter_handler(int signum) {
    printf("捕获到信号 %d\n", signum);
    my_kill();
}

void timeout_handler(int signum) {
    my_kill();
}

void waiting() {
    sleep(5);
}

int main() {
    sem_t mutex;
    sem_init(&mutex, 0, 1);
    signal(SIGINT, printer);
    signal(SIGQUIT, printer);
    // TODO: 五秒之后或接收到两个信号
    while (pid1 == -1) {
        pid1 = fork();
    }
    if (pid1 > 0) {
        while (pid2 == -1) {
            pid2 = fork();
        }
        if (pid2 > 0) {
            while (pid1 == -1 || pid2 == -1) {
            }
            signal(SIGALRM, inter_handler);
            alarm(5);

            // sleep(5);
            // my_kill();

            // TODO: 父进程
            int status1, status2;
            waitpid(pid1, &status1, 0);
            waitpid(pid2, &status2, 0);
            if (WIFEXITED(status1)) {
                printf("子进程1已经正常退出，退出码为 %d\n",
                       WEXITSTATUS(status1));
            } else {
                printf("子进程1没有正常退出\n");
            }
            if (WIFEXITED(status2)) {
                printf("子进程2已经正常退出，退出码为 %d\n",
                       WEXITSTATUS(status2));
            } else {
                printf("子进程2没有正常退出\n");
            }
            printf("Parent process is killed!!\n");
        } else {
            // TODO: 子进程 2
            signal(17, printer);
            pause();
            printf("Child process2 is killed by parent!!\n");
            return 0;
        }
    } else {
        // TODO：子进程 1
        signal(16, printer);
        pause();
        printf("Child process1 is killed by parent!!\n");
        return 0;
    }
    sem_destroy(&mutex);
    return 0;
}
