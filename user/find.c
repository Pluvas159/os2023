#include "kernel/types.h"
#include "kernel/stat.h"
#include "user/user.h"
#include "kernel/fs.h"
#include "kernel/fcntl.h"

// char *
// fmtname(char *path)
// {
//     static char buf[DIRSIZ + 1];
//     char *p;

//     // Find first character after last slash.
//     for (p = path + strlen(path); p >= path && *p != '/'; p--)
//         ;
//     p++;

//     // Return blank-padded name.
//     if (strlen(p) >= DIRSIZ)
//         return p;
//     memmove(buf, p, strlen(p));
//     memset(buf + strlen(p), ' ', DIRSIZ - strlen(p));
//     return buf;
// }

void strconcat(char *dest, char *src)
{
    while (*dest)
        dest++;
    while ((*dest++ = *src++))
        ;
}

void find_in_file(char *path, char *what)
{
    struct stat st;
    struct dirent de;
    char buf[512];
    printf("path: %s\n", path);

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

    strcpy(buf, path);
    // p = buf + strlen(buf);
    printf("path i look in: %s\n", buf);
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
            char new_path[512];
            strcpy(new_path, buf);
            char *p = new_path + strlen(new_path);
            *p++ = '/';
            strconcat(new_path, de.name);
            // if p has a trailing slash, remove it

            find_in_file(new_path, what);
        }

        // printf("find: %s file: %d  \n", de.name, st.type);
        if (strcmp(de.name, what) == 0)
        {
            printf("%s%s\n", buf, de.name);
        }
        else
        {
            // printf("find: %s what: %s \n", de.name, what);
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

// if (st.type == T_DIR)
// {
//     find_in_file(start_path, what);
// }

// printf("find: %s\n", start_path);
// printf("find: %d\n", st.type);

// // check if path is a file
// if (st.type == T_DIR)
// {
//     strcpy(buf, start_path);
//     p = buf + strlen(buf);
//     *p++ = '/';
//     while (read(fd, &de, sizeof(de)) == sizeof(de))
//     {
//         if (de.inum == 0)
//             continue;
//         memmove(p, de.name, DIRSIZ);
//         p[DIRSIZ] = 0;
//         if (strcmp(de.name, what) == 0)
//         {
//             printf("%s\n", buf);
//         }
//     }
// }
