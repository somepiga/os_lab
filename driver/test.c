#include <fcntl.h>
#include <stdio.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

int main() {
    int fd;
    char input_sentence[100];
    char output_sentence[100];
    fd = open("/dev/globalvar", O_RDWR, S_IRUSR | S_IWUSR);
    if (fd != -1) {
        while (1) {
            int choice;
            printf("chose..\n");
            scanf("%d", &choice);
            char c;
            while ((c = getchar()) != '\n' && c != EOF) {
            }
            switch (choice) {
                case 1:
                    printf("Input...\n");
                    int k = 0;
                    while (1) {
                        char c = getchar();
                        if (c == '\n' || c == '\0') {
                            break;
                        }
                        input_sentence[k++] = c;
                    }
                    input_sentence[k] = '\0';
                    write(fd, &input_sentence, sizeof(char) * k);
                    printf("[user] %s\n", input_sentence);
                    break;
                case 2:
                    read(fd, &output_sentence, 100);
                    printf("%s\n", output_sentence);
                    break;
                case 3:
                    return 0;
                default:
                    return 0;
            }
        }
    } else {
        printf("Device open failure\n");
    }
    return 0;
}