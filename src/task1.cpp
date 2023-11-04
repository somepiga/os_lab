#include <sys/types.h>
#include <sys/wait.h>
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <iostream>
#include <pthread.h>
#include <sys/syscall.h>
#include <thread>
#include <mutex>

int value = 0;

void has_wait()
{
    pid_t pid, pid1;
    pid = fork();

    if (pid < 0)
    {
        fprintf(stderr, "Fork Failed");
        return;
    }
    else if (pid == 0)
    {
        pid1 = getpid();
        printf("child:pid = %d\n", pid);
        printf("child:pid1 = %d\n", pid1);
    }
    else
    {
        pid1 = getpid();
        printf("parent:pid = %d\n", pid);
        printf("parent:pid1 = %d\n", pid1);
        wait(NULL);
        printf("parent exit!\n");
    }
}

void no_wait()
{
    pid_t pid, pid1;
    pid = fork();

    if (pid < 0)
    {
        fprintf(stderr, "Fork Failed");
        return;
    }
    else if (pid == 0)
    {
        pid1 = getpid();
        printf("child:pid = %d\n", pid);
        printf("child:pid1 = %d\n", pid1);
    }
    else
    {
        pid1 = getpid();
        printf("parent:pid = %d\n", pid);
        printf("parent:pid1 = %d\n", pid1);
        printf("parent exit!\n");
    }
}

void extra()
{
    pid_t pid;
    pid = fork();
    if (pid < 0)
    {
        fprintf(stderr, "Fork Failed");
        return;
    }
    else if (pid == 0)
    {
        value++;
        printf("child:value = %d\n", value);
        printf("child:value address = %p\n", &value);
    }
    else
    {
        printf("parent:value = %d\n", value);
        printf("parent:value address = %p\n", &value);
        wait(NULL);
    }
}

void before_return()
{
    pid_t pid;
    pid = fork();
    if (pid < 0)
    {
        fprintf(stderr, "Fork Failed");
        return;
    }
    else if (pid == 0)
    {
        value++;
        printf("child:value = %d\n", value);
        printf("child:value address = %p\n", &value);
    }
    else
    {
        printf("parent:value = %d\n", value);
        printf("parent:value address = %p\n", &value);
        wait(NULL);
    }
    value--;
    printf("before return:value = %d\n", value);
}

void system_call()
{
    pid_t pid, pid1;
    pid = fork();
    if (pid < 0)
    {
        fprintf(stderr, "Fork Failed");
        return;
    }
    else if (pid == 0)
    {
        pid1 = getpid();
        printf("child:pid1 = %d\n", pid1);
        system("./system_call");
    }
    else
    {
        pid1 = getpid();
        printf("parent:pid1 = %d\n", pid1);
        system("./system_call");
    }
}

void exec()
{
    pid_t pid, pid1;
    pid = fork();
    if (pid < 0)
    {
        fprintf(stderr, "Fork Failed");
        return;
    }
    else if (pid == 0)
    {
        pid1 = getpid();
        printf("child:pid1 = %d\n", pid1);
        char *program = "system_call";
        char *args[] = {program, "-l", NULL};
        execv(program, args);
    }
    else
    {
        pid1 = getpid();
        printf("parent:pid1 = %d\n", pid1);
        char *program = "system_call";
        char *args[] = {program, "-l", NULL};
        execv(program, args);
    }
    value++;
}

void threadFunction1()
{
    for (int i = 0; i < 50000; ++i)
    {
        value++;
        printf("value = %d\n", value);
    }
}

void threadFunction2()
{
    for (int i = 0; i < 50000; ++i)
    {
        value--;
        printf("value = %d\n", value);
    }
}

void my_thread()
{
    std::thread t1(threadFunction1);
    std::thread t2(threadFunction2);
    t1.join();
    t2.join();
}

std::mutex mtx;
void threadFunction1_mutex()
{
    for (int i = 0; i < 50000; ++i)
    {
        mtx.lock();
        value++;
        printf("value = %d\n", value);
        mtx.unlock();
    }
}

void threadFunction2_mutex()
{
    for (int i = 0; i < 50000; ++i)
    {
        mtx.lock();
        value--;
        printf("value = %d\n", value);
        mtx.unlock();
    }
}

void my_thread_mutex()
{
    std::thread t1(threadFunction1_mutex);
    std::thread t2(threadFunction2_mutex);
    t1.join();
    t2.join();
}

void threadFunction1_system()
{
    printf("thread1 tid: %ld, pid: %d\n", syscall(SYS_gettid), getpid());
    system("./my_gettid");
}

void threadFunction2_system()
{
    printf("thread2 tid: %ld, pid: %d\n", syscall(SYS_gettid), getpid());
    system("./my_gettid");
}

void my_thread_system()
{
    std::thread t1(threadFunction1_system);
    std::thread t2(threadFunction2_system);
    t1.join();
    t2.join();
}

void threadFunction1_exec()
{
    printf("tid: %ld, pid: %d\n", syscall(SYS_gettid), getpid());
    char *program = "my_gettid";
    char *args[] = {program, "-l", NULL};
    execv(program, args);
}

void threadFunction2_exec()
{
    printf("tid: %ld, pid: %d\n", syscall(SYS_gettid), getpid());
    char *program = "my_gettid";
    char *args[] = {program, "-l", NULL};
    execv(program, args);
}

void my_thread_exec()
{
    std::thread t1(threadFunction1_exec);
    std::thread t2(threadFunction2_exec);
    t1.join();
    t2.join();
}

/////////////////////////////////
void fork_in_thread()
{
    pid_t pid;
    printf("[before fork]tid: %ld, pid: %d\n", syscall(SYS_gettid), getpid());
    pid = fork();
    if (pid == 0)
    {
        printf("[after fork Child]tid: %ld, pid: %d\n", syscall(SYS_gettid), getpid());
    }
    if (pid > 0)
    {
        printf("[after fork Parent]tid: %ld, pid: %d\n", syscall(SYS_gettid), getpid());
    }
}

void my_test_func()
{
    std::thread t(fork_in_thread);
    t.join();
}

int main()
{
    int chose;
    printf("---------------------------\n");
    printf("---------- chose  ---------\n");
    printf("-------1. has wait---------\n");
    printf("-------2. no wait ---------\n");
    printf("-------3. extra -----------\n");
    printf("-------4. before_return----\n");
    printf("-------5. system_call------\n");
    printf("-------6. exec      -------\n");
    printf("-------7. thread     ------\n");
    printf("-------8. thread mutex ----\n");
    printf("-------9. thread system ---\n");
    printf("-------10. thread exec ----\n");
    printf("---------------------------\n");
    printf("-------99. my test func-----\n");
    scanf("%d", &chose);
    system("clear");

    switch (chose)
    {
    case 1:
        has_wait();
        break;
    case 2:
        no_wait();
        break;
    case 3:
        extra();
        break;
    case 4:
        before_return();
        break;
    case 5:
        system_call();
        break;
    case 6:
        exec();
        break;
    case 7:
        my_thread();
        break;
    case 8:
        my_thread_mutex();
        break;
    case 9:
        my_thread_system();
        break;
    case 10:
        my_thread_exec();
        break;
    case 99:
        my_test_func();
        break;
    default:
        break;
    }
}