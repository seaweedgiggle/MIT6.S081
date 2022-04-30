#include "kernel/types.h"
#include "kernel/stat.h"
#include "user/user.h"

int main(int argc, char *argv[]) {
    // p[0] for read, p[1] for write
    int p[2];
    pipe(p);
    char byte[1];
    if (fork() == 0) {
        int n = read(p[0], byte, 1);
        if (n == 0) {
            fprintf(1, "Error in receiving from child process.\n");
            exit(1);
        }
        fprintf(1, "%d: received ping\n", getpid());
        // 向父进程发送信息
        if (write(p[1], byte, 1) != 1) {
            fprintf(1, "Error in sending from child process.\n");
            exit(1);
        }
        close(p[0]);
        close(p[1]);
        exit(0);
    }
    else {
        byte[0] = 'a';
        if (write(p[1], byte, 1) != 1) {
            fprintf(1, "Error in sending from father process.\n");
            exit(1);
        }
        // 如果没有这行 wait(0)，将会缺少 received ping. (Why?)
        // 我猜是因为父进程跑太快了，比子进程先结束了。
        wait(0);
        int n = read(p[0], byte, 1);
        if (n == 0) {
            fprintf(1, "Error in receiving from child process.\n");
            exit(1);
        }
        fprintf(1, "%d: received pong\n", getpid());
        close(p[0]);
        close(p[1]);
        exit(0);
    }
}