#include "kernel/types.h"
#include "kernel/stat.h"
#include "user/user.h"

#define RD 0
#define WR 1
const int INT_SIZE = sizeof(int);
/**
 * 伪代码如下。
 * 
 * p = get a number from left neighbor
    print p
    loop:
        n = get a number from left neighbor
        if (p does not divide n)
            send n to right neighbor

    每个进程要做的事情：
     1. 从左边进程管道读取一个素数 prime，如果有就打印，如果没有就结束。
        - 需要一个左边到它的管道的读入端

     2. 继续从左边进程管道读取数据，将其中不能被 prime 整除的数据发送给右管道。
        - 需要一个左边到它的管道的读入端，一个它到右边的管道的写入端

     3. 创建右边的进程，读取右管道中的数据。
        - 需要一个
 */

/**
 * @brief read a prime from left pipe.
 * 
 * @param leftpipe leftpipe
 * @return int 1 represents successed, 0 failed.
 */
int read_prime(int* leftpipe) {
    close(leftpipe[WR]);
    int prime;
    if (read(leftpipe[RD], &prime, INT_SIZE) != 0) {
        fprintf(1, "prime %d\n", prime);
        return prime;
    }
    return 0;
}

void read_left_send_right(int* leftpipe, int* rightpipe, int prime) {
    // 多了这一行就不行了。为啥啊。。。 关闭两次会发生什么错误？
    // close(leftpipe[WR]);
    int num;
    while (read(leftpipe[RD], &num, INT_SIZE) != 0) {
        if (num % prime != 0) {
            write(rightpipe[WR], &num, INT_SIZE);
        }
    }
    close(leftpipe[RD]);
    close(rightpipe[WR]);
}

void f(int* leftpipe) {
    int prime = read_prime(&leftpipe[RD]);
    if (prime != 0) {
        int rightpipe[2];
        pipe(rightpipe);
        read_left_send_right(leftpipe, rightpipe, prime);
        if (fork() == 0) {
            f(rightpipe);
        }
        else {
            close(rightpipe[RD]);
            wait(0);
        }
    }
    exit(0);
}



int main(int argc, char *argv[]) {
    // p[0] for read, p[1] for write.
    int p[2];
    pipe(p);
    if (fork() == 0) {
        f(p);
    }
    else {
        for (int i = 2; i <= 35; i++) {
            write(p[WR], &i, INT_SIZE);
        }
        close(p[0]);
        close(p[1]);
        wait(0);
    }
    exit(0);
}