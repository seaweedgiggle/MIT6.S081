#include "kernel/types.h"
#include "user/user.h"
#include "kernel/param.h"
// 参数单词最长长度
#define MAXLEN 512

enum state {
    S_WAIT,
    S_ARG,
    S_ARG_END,
    S_ARG_LINE_END,      // 左侧为参数的换行，如 "arg\n"
    S_LINE_END,          // 左侧为空格的换行，如 "arg  \n"
    S_END                // 结束 (其实这个没啥用)
};

enum char_type {
    C_SPACE,
    C_LINE_END,
    C_CHAR
};

enum char_type getCharType(char c) {
    if (c == ' ') {
        return C_SPACE;
    }
    else if (c == '\n') {
        return C_LINE_END;
    }
    else {
        return C_CHAR;
    }
}

// 状态转移函数
// 需要画出 DFA，然后填上去即可。
enum state stateTransition(enum state curr, enum char_type c) {
    switch(curr) {
        case S_WAIT:
            if (c == C_CHAR)        return S_ARG;
            if (c == C_LINE_END)    return S_LINE_END;
            if (c == C_SPACE)       return S_WAIT;
            break;
        case S_ARG:
            if (c == C_CHAR)        return S_ARG;
            if (c == C_LINE_END)    return S_ARG_LINE_END;
            if (c == C_SPACE)       return S_ARG_END;
            break;
        case S_ARG_END:
        case S_ARG_LINE_END:
        case S_LINE_END:
            if (c == C_CHAR)        return S_ARG;
            if (c == C_LINE_END)    return S_LINE_END;
            if (c == C_SPACE)       return S_WAIT;
            break;
        default:
            break;
    }
    return S_END;

}

/**
 * echo hello \n too | xargs echo bye
 *  例如这个命令，实际上执行的是 echo bye hello 与 echo bye too 两条命令。
 *  所以需要在 xargv 数组中保留 echo bye (也就是一开始的 argv)，把后面的全部清零，再读入后面的命令。
 */
void memreset(char* xargv[MAXARG], int args_num) {
    for (int i = 0; i < args_num; i++) {
        xargv[i] = 0;
    }
}



int main(int argc, char* argv[]) {
    char* xargv[MAXARG];
    // 先把原来的参数拷贝进去
    for (int i = 1; i < argc; i++) {
        xargv[i-1] = argv[i];
    }
    int args_id = argc - 1;     // 现在 argv 的索引
    char line[MAXLEN];      // 存储输入串的数组。
    char* cur = line;
    int arg_begin = 0;
    int char_num = 0;
    
    enum state st = S_WAIT;
    while (read(0, cur, sizeof(char)) == sizeof(char)) {
        st = stateTransition(st, getCharType(*cur));
        switch(st) {
            case S_WAIT:
                arg_begin++;
                break;
            case S_ARG:
                char_num++;
                break;
            case S_ARG_END:
                *cur = '\0';
                xargv[args_id++] = line + arg_begin;
                // printf("%s\n", xargv[args_id-1]);
                arg_begin += char_num + 1;
                char_num = 0;
                break;
            case S_ARG_LINE_END:
                *cur = '\0';
                xargv[args_id++] = line + arg_begin;
                // printf("%s\n", xargv[args_id-1]);
                // 这里不用 break
            case S_LINE_END:
                char_num = 0;
                if (fork() == 0) {
                    exec(xargv[0], xargv);
                }
                else {
                    wait(0);
                    // 将指针清空
                    memreset(&xargv[argc], args_id);
                    args_id = argc - 1;
                }
                break;
            default:
                break;
        }
        cur++;
    }    
    exit(0);
}