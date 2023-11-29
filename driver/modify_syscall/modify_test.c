#include <stdio.h>
#include <sys/syscall.h>
#include <unistd.h>

#define MY_SYSCALL_NUMBER 178

int main() {
    int result = syscall(MY_SYSCALL_NUMBER);

    printf("Result from modified syscall: %d\n", result);

    return 0;
}
