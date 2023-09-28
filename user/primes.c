#include "kernel/types.h"
#include "kernel/stat.h"
#include "user/user.h"
#include <sys/types.h>

#define MAX_COUNT 35

void ChildProcess(pid_t r);

void main(void)
{
    pid_t pid;
    int p[2];
    pipe(p);
    for (int i = 2; i <= MAX_COUNT; i++)
    {

        write(p[1], &i, sizeof(i));
    }
    close(p[1]);

    pid = fork();
    if (pid != 0)
    {
        wait(0);
    }
    else
    {
        close(p[1]);
        ChildProcess(p[0]);
    }
}

void ChildProcess(pid_t r)
{
    pid_t new_p[2];
    pipe(new_p);

    int prime;
    int nbytes = read(r, &prime, sizeof(prime));
    if (nbytes == 0)
    {
        exit(0);
    }

    // print first number
    printf("prime %d\n", prime);

    int i;
    while (read(r, &i, sizeof(i)))
    {
        if (i % prime != 0)
        {
            write(new_p[1], &i, sizeof(i));
        }
    }
    close(r);
    close(new_p[1]);
    pid_t pid = fork();
    if (pid == 0)
    {
        ChildProcess(new_p[0]);
    }
    else
    {
        wait(0);
    }
}
