#include "chatroom.h"

volatile int read_listen = 1;

void signal_handler(int signum) {
    if (signum == SIGUSR1) {
        read_listen = 0;
    }
}

int main(void) {
    int fd, i;
    char input[100];
    char output[101];
    char my_id;
    fd = open("/dev/globalvar", O_RDWR, S_IRUSR | S_IWUSR);
    if (fd != -1) {
        signal(SIGUSR1, signal_handler);
        printf("输入账号\n");
        scanf("%c", &my_id);
        if (fork() == 0) {
            while (1) {
                message msg = {TRANS, 0, {0}, {0}};
                msg.type = TRANS;
                msg.from = my_id;

                scanf("%s", msg.data);

                if (strcmp(msg.data, "EXIT") == 0) {
                    kill(getppid(), SIGUSR1);  // 向父进程发送一个终止信号，
                                               // 示意其可以退出读取循环
                    close(fd);
                    break;
                }
                scanf("%s", msg.to);

                toString(msg, input);
                write(fd, input, 100);
            }
        } else {
            while (read_listen) {
                for (i = 0; i < 101; i++) {
                    output[i] = '\0';
                }
                read(fd, output, 100);
                message msg = parse(output);
                if (msg.type == TRANS) {
                    for (i = 0; i < 10; ++i) {
                        if (my_id == msg.to[i] ||
                            (msg.to[i] == '?' && msg.from != my_id)) {
                            if (strcmp(msg.data, "EXIT") != 0) {
                                printf("new message: %s\n", msg.data);
                            }
                            break;
                        }
                    }
                }
            }
            wait(NULL);
            close(fd);
        }
    } else {
        printf("device open failure,%d\n", fd);
    }
    return 0;
}
