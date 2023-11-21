#include <fcntl.h>
#include <stdio.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
int main(void) {
    int fd, i;
    char num[101];
    fd = open("/dev/globalvar", O_RDWR, S_IRUSR | S_IWUSR);
    if (fd != -1) {
        while (1) {
            for (i = 0; i < 101; i++) {
                num[i] = '\0';
            }
            read(fd, num, 100);
            printf("%s\n", num);
            if (strcmp(num, "EXIT") == 0) {
                close(fd);
                break;
            }
        }
    } else {
        printf("device open failure,%d\n", fd);
    }
    return 0;
}