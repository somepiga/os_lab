#include <fcntl.h>
#include <stdio.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
int main() {
    int fd;
    char num[100];
    fd = open("/dev/globalvar", O_RDWR, S_IRUSR | S_IWUSR);
    if (fd != -1) {
        while (1) {
            printf("Please input the globar:\n");
            scanf("%s", num);
            write(fd, num, strlen(num));
            if (strcmp(num, "EXIT") == 0) {
                close(fd);
                break;
            }
        }
    } else {
        printf("device open failure\n");
    }
    return 0;
}