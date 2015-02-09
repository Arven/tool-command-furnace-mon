#include <errno.h>
#include <termios.h>
#include <unistd.h>
#include <time.h>
#include <stdio.h>
#include <string.h>
#include <fcntl.h>
#include <termios.h>

FILE* hours;
FILE* minutes;

int
set_interface_attribs (int fd, int speed, int parity)
{
        struct termios tty;
        memset (&tty, 0, sizeof tty);
        if (tcgetattr (fd, &tty) != 0)
        {
                perror("error from tcgetattr");
                return -1;
        }

        cfsetospeed (&tty, speed);
        cfsetispeed (&tty, speed);

        tty.c_cflag = (tty.c_cflag & ~CSIZE) | CS8;     // 8-bit chars
        // disable IGNBRK for mismatched speed tests; otherwise receive break
        // as \000 chars
        tty.c_iflag &= ~IGNBRK;         // disable break processing
        tty.c_lflag = 0;                // no signaling chars, no echo,
                                        // no canonical processing
        tty.c_oflag = 0;                // no remapping, no delays
        tty.c_cc[VMIN]  = 0;            // read doesn't block
        tty.c_cc[VTIME] = 5;            // 0.5 seconds read timeout

        tty.c_iflag &= ~(IXON | IXOFF | IXANY); // shut off xon/xoff ctrl

        tty.c_cflag |= (CLOCAL | CREAD);// ignore modem controls,
                                        // enable reading
        tty.c_cflag &= ~(PARENB | PARODD);      // shut off parity
        tty.c_cflag |= parity;
        tty.c_cflag &= ~CSTOPB;
        tty.c_cflag &= ~CRTSCTS;

        if (tcsetattr (fd, TCSANOW, &tty) != 0)
        {
                perror("error %d from tcsetattr");
                return -1;
        }
        return 0;
}

void
set_blocking (int fd, int should_block)
{
        struct termios tty;
        memset (&tty, 0, sizeof tty);
        if (tcgetattr (fd, &tty) != 0)
        {
                perror("error from tggetattr");
                return;
        }

        tty.c_cc[VMIN]  = should_block ? 1 : 0;
        tty.c_cc[VTIME] = 5;            // 0.5 seconds read timeout

        if (tcsetattr (fd, TCSANOW, &tty) != 0)
                perror("error setting term attributes");
}

void
main(int argc, char** argv) {

char *portname = "/dev/ttyUSB0";
int fd = open (portname, O_RDWR | O_NOCTTY | O_SYNC);
if (fd < 0)
{
        perror("error opening stream");
        return;
}

set_interface_attribs (fd, B115200, 0);  // set speed to 115,200 bps, 8n1 (no parity)
set_blocking (fd, 1);                    // set blocking

time_t t = time(NULL);
struct tm tm = *localtime(&t);

printf("BEGINNING LOGGING\n");
printf("------------------------\n");

char buf;
char strbuf[500];
int current_day = tm.tm_mday;
int current_hour = tm.tm_hour;
int number_minutes = 0;
float minute = 0;
float hour = 0;

snprintf(strbuf, sizeof(strbuf), "data/%04d-%02d-%02d.dat", tm.tm_year + 1900, tm.tm_mon + 1, tm.tm_mday);
hours = fopen(strbuf, "a");
snprintf(strbuf, sizeof(strbuf), "data/%04d-%02d-%02d.minutes.dat", tm.tm_year + 1900, tm.tm_mon + 1, tm.tm_mday);
minutes = fopen(strbuf, "a");

while ( 1 ) {
  int n = read(fd, &buf, 1);
  if (n < 0) {
      perror("stream closed abruptly");
      return;
  }
  time_t t = time(NULL);
  struct tm tm = *localtime(&t);
  if(current_day != tm.tm_mday) {
    printf("--- END LOGGING DAY  ---\n");
    fclose(minutes);
    fclose(hours);
    snprintf(strbuf, sizeof(strbuf), "data/%04d-%02d-%02d.dat", tm.tm_year + 1900, tm.tm_mon + 1, tm.tm_mday);
    hours = fopen(strbuf, "a");
    snprintf(strbuf, sizeof(strbuf), "data/%04d-%02d-%02d.minutes.dat", tm.tm_year + 1900, tm.tm_mon + 1, tm.tm_mday);
    minutes = fopen(strbuf, "a");
    current_day = tm.tm_mday;
  }
  if(current_hour != tm.tm_hour) {
    hour = hour / (float) number_minutes;
    printf("--- END LOGGING HOUR ---\n");
    printf("HOUR TOTAL (MINUTES): %f\n", hour);
    printf("------------------------\n");
    fprintf(minutes, "%d %f\n", tm.tm_hour, hour);
    fflush(minutes);
    hour = 0;
    number_minutes = 0;
    current_hour = tm.tm_hour;
  }
  minute = buf - 'A';
  hour += minute;
  number_minutes++;
  printf("SECONDS THIS MINUTE: %f\n", minute);
  fprintf(minutes, "%d %d %f\n", tm.tm_hour, tm.tm_min, minute);
  fflush(minutes);
}

/* while ( 1 ) {
  start_a_new_day();
  for (int j = 0; j <= 23; j++) {      // hours
    for (int i = 0; i <= 59; i++) {    // minutes
      int n = read(fd, &buf, 1);
      if (n < 0) {
          perror("stream closed abruptly");
          return;
      }
      minute = buf - 'A';
      hour += minute;
      printf("SECONDS THIS MINUTE: %f\n", minute);
      time_t t = time(NULL);
      struct tm tm = *localtime(&t);
      fprintf(minutes, "%d %d %f\n", tm.tm_hour, tm.tm_min, minute);
      fflush(minutes);
      if(is_a_new_day()) {
        start_a_new_day();
        goto dawn;
      }
    }
    hour = hour / (float) 60;
    printf("--- END LOGGING HOUR ---\n");
    printf("HOUR TOTAL (MINUTES): %f\n", hour);
    printf("------------------------\n");
    time_t t = time(NULL);
    struct tm tm = *localtime(&t);
    fprintf(hours, "%d %f\n", tm.tm_hour, hour);
    fflush(hours);
    hour = 0;
  }
  dawn:
  fclose(minutes);
  fclose(hours);
} */

}
