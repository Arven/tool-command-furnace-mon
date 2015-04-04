#include <stdio.h>
#include <sys/select.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>

#define STDIN 0

int main(int argc, char** argv) {
  fprintf(stderr, "[ xbuffer ] [ running ] : p to pause : ");
  fflush(stderr);
  char buf[256];
  ssize_t sz, filesz;
  int sd = open("stream.pipe", O_RDONLY | O_NONBLOCK);
  FILE * logbuffer = fopen("logbuffer.dat", "w+");
  fd_set fds;
  int maxfd;
  int paused = 0;
  maxfd = ( sd > STDIN ) ? sd : STDIN;
  while(1) {
    FD_ZERO(&fds);
    FD_SET(sd, &fds);
    FD_SET(STDIN, &fds);
    select(maxfd + 1, &fds, NULL, NULL, NULL);
    if(FD_ISSET(sd, &fds)) {
      sz = read(sd, buf, sizeof(buf));
      if(sz > 0) {
        buf[sz] = 0;
        if(paused == 0) {
          fprintf(stdout, "%s", buf);
          if(buf[sz - 1] == '\n') {
            fflush(stdout);
          }
        } else {
          fprintf(logbuffer, "%s", buf);
          if(buf[sz - 1] == '\n') {
            fflush(logbuffer);
          }
        }
      }
    }
    if(FD_ISSET(STDIN, &fds)) {
      sz = read(STDIN, buf, sizeof(buf));
      if(sz > 0) {
        buf[sz] = 0;
        if(buf[0] == 'p' || buf[0] == 'P') {
          fprintf(stderr, "[ xbuffer ] [ stopped ] : r to resume: ");
          fflush(stderr);
          paused = 1;
        } else if(buf[0] == 'r' || buf[0] == 'R') {
          fseek(logbuffer, 0, SEEK_SET);
          while(!feof(logbuffer)) {
            sz = fread(buf, 1, sizeof(buf), logbuffer);
            // fprintf(stderr, "%d\n", sz);
            // fflush(stderr);
            buf[sz] = 0;
            fprintf(stdout, "%s", buf);
            fflush(stdout);
          }
          fclose(logbuffer);
          logbuffer = fopen("logbuffer.dat", "w+");
          fprintf(stderr, "[ xbuffer ] [ running ] : p to pause : ");
          fflush(stderr);
          paused = 0;
        }
      }
    }
  }
  return 0;
}
