#include "kernel/types.h"
#include "kernel/stat.h"
#include "user/user.h"
#include "kernel/fs.h"

char* fmtname(char *path)
{
    static char buf[DIRSIZ + 1];
    char *p;

    // Find first character after last slash.
    for (p = path + strlen(path); p >= path && *p != '/'; p--)
        ;
    p++;

    // Return blank-padded name.
    if (strlen(p) >= DIRSIZ)
        return p;
    memmove(buf, p, strlen(p));
    memset(buf + strlen(p), ' ', DIRSIZ - strlen(p));
    return buf;
}

/**
 * #define T_DIR 1    // Directory
    #define T_FILE 2   // File
    #define T_DEVICE 3 // Device
    struct stat {
        int dev;     // 文件系统的磁盘设备
        uint ino;    // Inode编号
        short type;  // 文件类型
        short nlink; // 指向文件的链接数
        uint64 size; // 文件字节数
    };
 */

void ls(char *path)
{
    char buf[512], *p;
    int fd;
    struct dirent de;
    struct stat st;

    if ((fd = open(path, 0)) < 0)
    {
        fprintf(2, "ls: cannot open %s\n", path);
        return;
    }

    if (fstat(fd, &st) < 0)
    {
        fprintf(2, "ls: cannot stat %s\n", path);
        close(fd);
        return;
    }

    switch (st.type)
    {
    case T_FILE:
        printf("%s %d %d %l\n", fmtname(path), st.type, st.ino, st.size);
        break;

    case T_DIR:
        if (strlen(path) + 1 + DIRSIZ + 1 > sizeof buf)
        {
            printf("ls: path too long\n");
            break;
        }
        strcpy(buf, path);
        p = buf + strlen(buf);
        *p++ = '/';
        /**
        Directory is a file containing a sequence of dirent structures.
            #define DIRSIZ 14

            struct dirent {
                ushort inum;
                char name[DIRSIZ];
            };

        */

        // 应该是把目录中的内容一个个读出来
        while (read(fd, &de, sizeof(de)) == sizeof(de))
        {
            if (de.inum == 0)
                continue;
            memmove(p, de.name, DIRSIZ);
            p[DIRSIZ] = 0;
            if (stat(buf, &st) < 0)
            {
                printf("ls: cannot stat %s\n", buf);
                continue;
            }
            printf("%s %d %d %d\n", fmtname(buf), st.type, st.ino, st.size);
        }
        break;
    }
    close(fd);
}

int main(int argc, char *argv[])
{
    int i;

    if (argc < 2)
    {
        ls(".");
        exit(0);
    }
    for (i = 1; i < argc; i++)
        ls(argv[i]);
    exit(0);
}
