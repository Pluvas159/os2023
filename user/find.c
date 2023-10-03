#include "kernel/types.h"
#include "kernel/stat.h"
#include "user/user.h"
#include "kernel/fs.h"
#include "kernel/fcntl.h"

void find_in_file(char *path, char *what)
{
    struct stat st;
    struct dirent de;

    // open path
    int fd = open(path, 0);

    if (fd < 0)
    {
        fprintf(2, "find: cannot open %s\n", path);
        exit(1);
    }

    // get file info
    if (fstat(fd, &st) < 0)
    {
        fprintf(2, "find: cannot stat %s\n", path);
        close(fd);
        exit(1);
    }
    while (read(fd, &de, sizeof(de)) == sizeof(de))
    {
        stat(de.name, &st);
        if (de.inum == 0)
            continue;
        if (strcmp(de.name, ".") == 0 || strcmp(de.name, "..") == 0)
        {
            continue;
        }

        if (st.type == T_FILE)
        {
        }
        else if (st.type == T_DIR)
        {
            char new_buf[100];
            strcpy(new_buf, path);
            char *new_p = new_buf + strlen(new_buf);
            *new_p++ = '/';
            memmove(new_p, de.name, DIRSIZ);
            new_p[DIRSIZ] = 0;
            find_in_file(new_buf, what);
        }

        if (strcmp(de.name, what) == 0)
        {
            printf("%s/%s\n", path, de.name);
        }
    }
}

int main(int argc, char *argv[])
{

    if (argc != 3)
    {
        fprintf(2, "Usage: find <path> <what>\n");
        exit(1);
    }

    char *start_path = argv[1];
    char *what = argv[2];

    find_in_file(start_path, what);

    exit(0);
}
