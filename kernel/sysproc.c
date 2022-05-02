#include "types.h"
#include "riscv.h"
#include "defs.h"
#include "date.h"
#include "param.h"
#include "memlayout.h"
#include "spinlock.h"
#include "proc.h"
#include "sysinfo.h"

uint64
sys_exit(void)
{
    int n;
    if (argint(0, &n) < 0)
        return -1;
    exit(n);
    return 0; // not reached
}

uint64
sys_getpid(void)
{
    return myproc()->pid;
}

uint64
sys_fork(void)
{
    return fork();
}

uint64
sys_wait(void)
{
    uint64 p;
    if (argaddr(0, &p) < 0)
        return -1;
    return wait(p);
}

uint64
sys_sbrk(void)
{
    int addr;
    int n;

    if (argint(0, &n) < 0)
        return -1;
    addr = myproc()->sz;
    if (growproc(n) < 0)
        return -1;
    return addr;
}

uint64
sys_sleep(void)
{
    int n;
    uint ticks0;

    if (argint(0, &n) < 0)
        return -1;
    acquire(&tickslock);
    ticks0 = ticks;
    while (ticks - ticks0 < n)
    {
        if (myproc()->killed)
        {
            release(&tickslock);
            return -1;
        }
        sleep(&ticks, &tickslock);
    }
    release(&tickslock);
    return 0;
}

uint64
sys_kill(void)
{
    int pid;

    if (argint(0, &pid) < 0)
        return -1;
    return kill(pid);
}

// return how many clock tick interrupts have occurred
// since start.
uint64
sys_uptime(void)
{
    uint xticks;

    acquire(&tickslock);
    xticks = ticks;
    release(&tickslock);
    return xticks;
}

// lab2 trace
uint64 sys_trace(void) {
    int i;
    // RISC V 约定将系统调用号放置在 a7 寄存器中
    // 通过寄存器 a0 获取参数。
    if (argint(0, &i) < 0) {
        return -1;
    }
    // 将进程的 trace 参数设置好
    myproc()->trace_arg = i;
    return 0;
}

// lab2 exercise2

/**
 *   1. 从寄存器 a0 获取 sysinfo 系统调用中参数：一个指向 struct sysinfo 的指针。
 *   2. 内核填写结果的信息，包括 freemem 和 nproc.
 *   3. 将 sysinfo 的信息从内核中复制回用户空间。（使用 copyout(). ）
 */
uint64 sys_info(void) {
    // 通过寄存器 a0 获取地址
    uint64 addr;
    if (argaddr(0, &addr) < 0) {
        return -1;
    }
    
    // 填写信息
    struct sysinfo info;
    count_free_memory(&info.freemem);
    count_active_proc(&info.nproc);

    // 写入
    struct proc* p = myproc();
    if (copyout(p->pagetable, addr, (char*)&info, sizeof(info))){
        return -1;
    }
    return 0;
}

// 如果是这种实现方式，则会出现 usertrap.

// uint64 sys_info(void) {
//     // 通过寄存器 a0 获取地址
//     uint64 addr;
//     if (argaddr(0, &addr) < 0) {
//         return -1;
//     }
    
//     // 填写信息
//     struct sysinfo* info = (struct sysinfo*)addr;
//     count_free_memory(&info->freemem);
//     count_active_proc(&info->nproc);

//     return 0;
// }
