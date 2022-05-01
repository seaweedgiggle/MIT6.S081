#include "kernel/types.h"
#include "kernel/stat.h"
#include "user/user.h"
#include "kernel/fs.h"


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

void find(char *path, char* str)
{
    char buf[512], *p;
    int fd;
    struct dirent de;
    struct stat st;

    if ((fd = open(path, 0)) < 0)
    {
        fprintf(2, "find: cannot open %s\n", path);
        return;
    }

    if (fstat(fd, &st) < 0)
    {
        fprintf(2, "find: cannot stat %s\n", path);
        close(fd);
        return;
    }

    if (st.type == T_FILE) {
        fprintf(2, "find: argv[1] must be a directory.\n");
        exit(1);
    }
        
    if (strlen(path) + 1 + DIRSIZ + 1 > sizeof buf)
    {
        printf("find: path too long\n");
        exit(1);
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
    // 把目录中的内容一个个读出来，存到 p 里边（长度是 DIRSIZ）
    while (read(fd, &de, sizeof(de)) == sizeof(de))
    {
        if (de.inum == 0)
            continue;
        memmove(p, de.name, DIRSIZ);
        p[DIRSIZ] = 0;
        if (stat(buf, &st) < 0)
        {
            printf("find: cannot stat %s\n", buf);
            continue;
        }
        if (st.type == T_FILE && strcmp(p, str) == 0) {
            printf("%s\n", buf);
        }
        else if (st.type == T_DIR && strcmp(p, ".") != 0 && strcmp(p, "..") != 0) {
            find(buf, str);
        }
    }    
    close(fd);
}

int main(int argc, char *argv[])
{
    if (argc != 3)
    {
        printf("Usage: <directory> <str>.\n");
        exit(0);
    }
    find(argv[1], argv[2]);
        
    exit(0);
}
