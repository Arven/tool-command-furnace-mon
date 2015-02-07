#include <errno.h>
#include <termios.h>
#include <unistd.h>
#include <time.h>
#include <stdio.h>
#include <string.h>
#include <fcntl.h>
#include <termios.h>

FILE* hours;

int mday = -1;
int second = 0;
int minute = 0;
int hour = 0;

void
start_a_new_day()
{
    second = minute = hour = 0;
    char strbuf[500];
    time_t t = time(NULL);
    struct tm tm = *localtime(&t);
    mday = tm.tm_mday;
    snprintf(strbuf, sizeof(strbuf), "data/%d-%d-%d.dat", tm.tm_year + 1900, tm.tm_mon, tm.tm_mday);
    hours = fopen(strbuf, "a");
}

int
is_a_new_day()
{
    char strbuf[500];
    time_t t = time(NULL);
    struct tm tm = *localtime(&t);
    return mday != tm.tm_mday;
}

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
set_blocking (fd, 1);                    // set no blocking

char buf;

int n = read (fd, &buf, 1);            // read up to 100 characters if ready to read
while ( 1 ) {
  start_a_new_day();
  for (int k = 0; k <= 24; k++) {        // hours
    for (int j = 0; j <= 60; j++) {      // minutes
      for (int i = 0; i <= 60; i++) {    // seconds
        minute += buf;
        printf("SECOND TOTAL: %d", minute);
      }
      minute = minute / 60;
      hour += minute;
      minute = 0;
      printf("MINUTE TOTAL: %d", hour);
      if(is_a_new_day()) {
          start_a_new_day();
      }
    }
    hour = hour / 60;
    fprintf(hours, "%d\n", hour);
    fflush(hours);
    hour = 0;
  }
  fclose(hours);
}

}
