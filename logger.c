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
char* AM = "AM";
char* PM = "PM";

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

float
minutes_v(float seconds) {
  return seconds / (float) 60;
}

void
slide_window4(float* array) {
  for(int i = 1; i < 4; i++) {
    array[i - 1] = array[i];
  }
}

float
window4_avg(float* array) {
  float total = 0;
  int number = 0;
  for(int i = 0; i < 3; i++) {
    if (array[i] >= 0) {
      total += array[i];
      number++;
    }
  }
  total /= (float) 60;
  total /= (float) number;
  return total;
}

float
window4_total(float* array) {
  float total = 0;
  for(int i = 0; i < 3; i++) {
    if (array[i] > 0) {
      total += array[i];
    }
  }
  total /= (float) 60;
  return total;
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

char buf;
char strbuf[500];
int num_day_minutes = 0;
int num_hour_minutes = 0;
float day_seconds = 0;
float hour_seconds = 0;
float window4[4];

window4[0] = (float) -1;
window4[1] = (float) -1;
window4[2] = (float) -1;
window4[3] = (float) -1;

snprintf(strbuf, sizeof(strbuf), "data/%04d-%02d-%02d.minutes.dat", tm.tm_year + 1900, tm.tm_mon + 1, tm.tm_mday);

if (access(strbuf, F_OK) >= 0) {
  printf("READING OLD LOGS FOR TODAY\n");
  printf("------------------------\n");
  minutes = fopen(strbuf, "r");
  int hour, minute, current_hour = -1;
  float secs, hour_secs_for_window = 0;
  while (!feof(minutes)) {
    int v = fscanf(minutes, "%d %d %f\n", &hour, &minute, &secs);
    if(v < 0) { perror("scanf"); }
    // printf("GOT LINE: %d %d %f\n", hour, minute, secs);
    num_day_minutes++;
    day_seconds += secs;
    hour_secs_for_window += secs;
    if(tm.tm_hour == hour) {
      num_hour_minutes++;
      hour_seconds += secs;
    }
    if(current_hour != hour) {
      slide_window4(window4);
      window4[3] = hour_secs_for_window;
      current_hour = hour;
      hour_secs_for_window = 0;
    }
  }
  slide_window4(window4);
  window4[3] = hour_secs_for_window;
  printf("MINUTES LOGGED THIS HOUR: %f running of %d total\n", hour_seconds / (float) 60, num_hour_minutes);
  printf("MINUTES LOGGED THIS DAY : %f running of %d total\n", day_seconds / (float) 60, num_day_minutes);
  printf("MINUTES LOGGED IN PAST 4 HOURS: %f\n", window4_total(window4));
  printf("AVG MINUTES LOGGED IN PAST 4 HOURS: %f\n", window4_avg(window4));
  fclose(minutes);
  printf("------------------------\n");
}

printf("BEGINNING LOGGING\n");
printf("------------------------\n");
fflush(stdout);

int current_day = tm.tm_mday;
int current_hour = tm.tm_hour;
float minute_seconds = 0;

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
    printf("----NEW LOGGING DAY  ---\n");
    printf("DATE: %04d-%02d-%02d\n", tm.tm_year + 1900, tm.tm_mon + 1, tm.tm_mday );
    fflush(stdout);
    current_day = tm.tm_mday;
  }
  if(current_day == tm.tm_mday && tm.tm_hour == 23 && tm.tm_min == 59) {
    current_hour = 0;
  }
  if(current_hour != tm.tm_hour) {
    printf("--- END LOGGING HOUR ---\n");
    printf("HOUR TOTAL (MINUTES): %f\n", minutes_v(hour_seconds));
    printf("MINUTES LOGGED IN PAST 4 HOURS: %f\n", window4_total(window4));
    printf("AVG MINUTES LOGGED IN PAST 4 HOURS: %f\n", window4_avg(window4));
    printf("----NEW LOGGING HOUR ---\n");
    fflush(stdout);

    if(tm.tm_hour == 0) {
      printf("HOUR: 12 MIDNIGHT\n");
    } else if (tm.tm_hour == 12) {
      printf("HOUR: 12 NOON\n");
    } else if (tm.tm_hour > 0 && tm.tm_hour < 12) {
      printf("HOUR: %d AM\n", tm.tm_hour);
    } else if (tm.tm_hour > 12) {
      printf("HOUR: %d PM\n", tm.tm_hour - 12);
    }
    fflush(stdout);

    fprintf(minutes, "%d %f\n", tm.tm_hour, minutes_v(hour_seconds));
    fflush(minutes);
    slide_window4(window4);
    window4[3] = hour_seconds;

    day_seconds += hour_seconds;
    hour_seconds = 0;
    num_hour_minutes = 0;
    current_hour = tm.tm_hour;
  }
  if(current_day == tm.tm_mday && tm.tm_hour == 23 && tm.tm_min == 59) {
    printf("--- END LOGGING DAY  ---\n");
    printf("DAY TOTAL (MINUTES): %f\n", minutes_v(day_seconds));
    fflush(stdout);
    fclose(minutes);
    fclose(hours);
    snprintf(strbuf, sizeof(strbuf), "data/%04d-%02d-%02d.dat", tm.tm_year + 1900, tm.tm_mon + 1, tm.tm_mday);
    hours = fopen(strbuf, "a");
    snprintf(strbuf, sizeof(strbuf), "data/%04d-%02d-%02d.minutes.dat", tm.tm_year + 1900, tm.tm_mon + 1, tm.tm_mday);
    minutes = fopen(strbuf, "a");
    day_seconds = 0;
    num_day_minutes = 0;
    current_hour = 0;
  }
  minute_seconds = buf - 'A';
  hour_seconds += minute_seconds;
  num_hour_minutes++;
  num_day_minutes++;
  printf("MINUTE %d (SECONDS): %f\n", tm.tm_min, minute_seconds);
  fflush(stdout);
  fprintf(minutes, "%d %d %f\n", tm.tm_hour, tm.tm_min, minute_seconds);
  fflush(minutes);
}

}
