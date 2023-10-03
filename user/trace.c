#include "kernel/types.h"
#include "kernel/stat.h"
#include "user/user.h"

int main(int argc, char *argv[])
{
    if (argc <= 1)
    {
        fprintf(2, "No mask\n");
        exit(1);
    }

    int mask = atoi(argv[1]);
    trace(mask);

    return 0;
}