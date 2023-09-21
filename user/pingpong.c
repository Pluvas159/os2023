#include "kernel/types.h"
#include "kernel/stat.h"
#include "user/user.h"



int main(int argc, char *argv[]) {
  int p[2];
  int pid = fork();
  pipe(p);
  char recv_buf[5];

  if (pid == 0){
    write(p[1],"ping", 5);
    close(p[1]);
    read(p[0],recv_buf, 5);
    fprintf(1, "3: received: %s\n", recv_buf);
  } else {
    write(p[1],"pong", 5);
    close(p[1]);
    wait(0);
    read(p[0],recv_buf, 5);
    fprintf(1, "4: received: %s\n", recv_buf);
  }

  exit(0);

}
