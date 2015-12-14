#include <errno.h>
#include <termios.h>
#include <unistd.h>
#include <time.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <termios.h>

FILE* hours;
FILE* minutes;
char* AM = "AM";
char* PM = "PM";
int just_started = 1;
int retro_year;
int retro_month;
int retro_day;
int retro_num_days;
int retro_year_end;
int retro_month_end;
int retro_day_end;

float
minutes_v(float seconds) {
  return seconds / (float) 60;
}

void
slide_window24(float* array) {
  for(int i = 1; i < 24; i++) {
    array[i - 1] = array[i];
  }
}

float
window24_avg(float* array, int j) {
  float total = 0;
  int number = 0;
  for(int i = 24 - j; i < 24; i++) {
    if (array[i] >= 0) {
      total += array[i];
      number++;
    }
  }
  total /= (float) 60;
  total /= (float) number;
  return total;
}

int
window24_wsize(float* array) {
  int number = 0;
  for(int i = 0; i < 24; i++) {
    if (array[i] >= 0) {
      number++;
    }
  }
  return number;
}

float
window24_total(float* array, int j) {
  float total = 0;
  for(int i = 24 - j; i < 24; i++) {
    if (array[i] > 0) {
      total += array[i];
    }
  }
  total /= (float) 60;
  return total;
}

void
main(int argc, char** argv) {

if(argc != 2 && argc != 3) {
  printf("USAGE: retrolog <start> [<end>]\n");
  return;
}

int dte = atoi(argv[1]);
retro_year = dte / 10000;
retro_month = ( dte / 100 ) % 100;
retro_day = dte % 100;

time_t t = time(NULL);
struct tm tm = *localtime(&t);
tm.tm_year = retro_year - 1900;
tm.tm_mon = retro_month - 1;
tm.tm_mday = retro_day;

struct tm tm_end = *localtime(&t);
if(argc == 3) {
  int dte2 = atoi(argv[2]);
  retro_year_end = dte2 / 10000;
  retro_month_end = ( dte2 / 100 ) % 100;
  retro_day_end = dte2 % 100;
  tm_end.tm_year = retro_year_end - 1900;
  tm_end.tm_mon = retro_month_end - 1;
  tm_end.tm_mday = retro_day_end + 1;
} else {
  tm_end.tm_year = tm.tm_mon;
  tm_end.tm_mon = tm.tm_mon;
  tm_end.tm_mday = tm.tm_mday + 1;
}

mktime(&tm_end);

char buf;
char strbuf[500];
int num_day_minutes = 0;
int num_hour_minutes = 0;
float day_seconds = 0;
float hour_seconds = 0;
float window24[24];

for(int i = 0; i < 24; i++) {
  window24[i] = (float) -1;
}

time_t t_yesterday = time(NULL);
struct tm tm_yesterday = *localtime(&t);
tm_yesterday.tm_year = retro_year - 1900;
tm_yesterday.tm_mon = retro_month - 1;
tm_yesterday.tm_mday = retro_day;
tm_yesterday.tm_mday--;
mktime(&tm_yesterday);
snprintf(strbuf, sizeof(strbuf), "data/%04d-%02d-%02d.minutes.dat", tm_yesterday.tm_year + 1900, tm_yesterday.tm_mon + 1, tm_yesterday.tm_mday);

if (access(strbuf, F_OK) >= 0) {
  //printf("READING OLD LOGS FROM YESTERDAY\n");
  //printf("-------------------------------\n");
  minutes = fopen(strbuf, "r");
  int hour, minute, past_day_minutes = 0, past_day_seconds = 0, current_hour = -1;
  float secs, hour_secs_for_window = 0;
  while (!feof(minutes)) {
    int v = fscanf(minutes, "%d %d %f\n", &hour, &minute, &secs);
    if(v < 0) { perror("scanf"); }
    // printf("GOT LINE: %d %d %f\n", hour, minute, secs);
    hour_secs_for_window += secs;
    past_day_seconds += secs;
    past_day_minutes++;
    if(current_hour != hour) {
      slide_window24(window24);
      window24[23] = hour_secs_for_window;
      current_hour = hour;
      hour_secs_for_window = 0;
    }
  }
  slide_window24(window24);
  window24[23] = hour_secs_for_window;
  //printf("MINUTES LOGGED YESTERDAY : %f running of %d total\n", past_day_seconds / (float) 60, past_day_minutes);
  fclose(minutes);
  //printf("------------------------\n");
}

snprintf(strbuf, sizeof(strbuf), "data/%04d-%02d-%02d.minutes.dat", tm.tm_year + 1900, tm.tm_mon + 1, tm.tm_mday);

if (access(strbuf, F_OK) >= 0) {
  //printf("READING OLD LOGS FOR TODAY\n");
  //printf("------------------------\n");
  minutes = fopen(strbuf, "r");
  int hour, minute, current_hour = -1;
  float secs, hour_secs_for_window = 0;
  while (!feof(minutes)) {
    int v = fscanf(minutes, "%d %d %f\n", &hour, &minute, &secs);
    if(v < 0) { perror("scanf"); }
    num_day_minutes++;
    day_seconds += secs;
    hour_secs_for_window += secs;
    if(tm.tm_hour == hour) {
      num_hour_minutes++;
      hour_seconds += secs;
    }
    if(current_hour != hour) {
      slide_window24(window24);
      window24[23] = hour_secs_for_window;
      current_hour = hour;
      hour_secs_for_window = 0;
    }
  }
  slide_window24(window24);
  window24[23] = hour_secs_for_window;
  //printf("MINUTES LOGGED THIS HOUR: %f running of %d total\n", hour_seconds / (float) 60, num_hour_minutes);
  //printf("MINUTES LOGGED THIS DAY : %f running of %d total\n", day_seconds / (float) 60, num_day_minutes);
  if( window24_wsize(window24) >= 4 ) {
  //  printf("MINUTES LOGGED IN PAST 4 HOURS: %f\n", window24_total(window24, 4));
  //  printf("AVG MINUTES LOGGED IN PAST 4 HOURS: %f\n", window24_avg(window24, 4));
  } else {
  //  printf("NO DATA FOR PAST 4 HOURS\n");
  }
  if( window24_wsize(window24) >= 6 ) {
  //  printf("MINUTES LOGGED IN PAST 6 HOURS: %f\n", window24_total(window24, 6));
  //  printf("AVG MINUTES LOGGED IN PAST 6 HOURS: %f\n", window24_avg(window24, 6));
  } else {
  //  printf("NO DATA FOR PAST 6 HOURS\n");
  }
  fclose(minutes);
  //printf("------------------------\n");
}

//printf("BEGINNING LOGGING\n");
t = time(NULL);
tm = *localtime(&t);
tm.tm_year = retro_year - 1900;
tm.tm_mon = retro_month - 1;
tm.tm_mday = retro_day;

struct tm tm_p = *localtime(&t);
tm_p.tm_year = retro_year - 1900;
tm_p.tm_mon = retro_month - 1;
tm_p.tm_mday = retro_day;
tm_p.tm_hour = 0;
tm_p.tm_min = 0;

struct tm tm_g = *localtime(&t);
tm_g.tm_year = 0;
tm_g.tm_mon = 0;
tm_g.tm_mday = 0;
tm_g.tm_hour = 0;
tm_g.tm_min = 0;

struct tm tm_o = *localtime(&t);
tm_g.tm_year = 0;
tm_g.tm_mon = 0;
tm_g.tm_mday = 0;
tm_g.tm_hour = 0;
tm_g.tm_min = 0;

mktime(&tm_end);
mktime(&tm);

int current_day = tm.tm_mday;
int current_hour = tm.tm_hour;
float minute_seconds = 0;

while (tm.tm_yday < tm_end.tm_yday) {
printf("--- BEGINNING RETROACTIVE LOGGING FOR %d/%d/%d ---\n", tm.tm_mon + 1, tm.tm_mday, tm.tm_year + 1900);
fflush(stdout);

snprintf(strbuf, sizeof(strbuf), "data/%04d-%02d-%02d.minutes.dat", tm.tm_year + 1900, tm.tm_mon + 1, tm.tm_mday);
if (access(strbuf, F_OK) < 0) {
  printf("Error loading logs...\n");
  return;
}

minutes = fopen(strbuf, "r");
int flushed = 0;
int gap = 0;
float gap_minute_seconds;
while (!feof(minutes) || flushed == 0) {

  if(feof(minutes)) {
    flushed = 1;
    tm.tm_min ++;
    mktime(&tm);
  } else {
    int h, m;
    if(!gap) {
    int v = fscanf(minutes, "%d %d %f\n", &h, &m, &minute_seconds);
    if (v < 0) { perror("fscanf"); }
    tm.tm_hour = h;
    tm.tm_min = m;
    time_t diff = difftime(mktime(&tm), mktime(&tm_p));
    if(diff > 60) {
      if(diff % 60 == 0) {
        tm_g.tm_year = tm.tm_year;
        tm_g.tm_mon = tm.tm_mon;
        tm_g.tm_mday = tm.tm_mday;
        tm_g.tm_hour = tm.tm_hour;
        tm_g.tm_min = tm.tm_min;
        gap_minute_seconds = minute_seconds;
        tm.tm_year = tm_p.tm_year;
        tm.tm_mon = tm_p.tm_mon;
        tm.tm_mday = tm_p.tm_mday;
        tm.tm_hour = tm_p.tm_hour;
        tm.tm_min = tm_p.tm_min;
        mktime(&tm);
        gap = 1;
        minute_seconds = 0;
      }
      //printf("TIME ABERRATION FOUND %d!\n", diff);
    }
    }
    if (gap) {
      time_t diff = difftime(mktime(&tm_g), mktime(&tm_p));
      //printf("Timedifference... %d %d\n", diff, tm_g.tm_min);
      if(diff == 60) {
        gap = 0;
        minute_seconds = gap_minute_seconds;
        tm.tm_year = tm_g.tm_year;
        tm.tm_mon = tm_g.tm_mon;
        tm.tm_mday = tm_g.tm_mday;
        tm.tm_hour = tm_g.tm_hour;
        tm.tm_min = tm_g.tm_min;
        mktime(&tm);
      } else {
        tm.tm_min++;
        mktime(&tm);
        minute_seconds = 0;
        //minute_seconds=tm.tm_min;
      }
    }
  }

  if(current_day != tm.tm_mday) {
    slide_window24(window24);
    window24[23] = hour_seconds;
    day_seconds += hour_seconds;

    printf("---TOTAL:[%2.2f] 4H:[TOTAL %3.2f / AVG %2.2f] 6H:[TOTAL %3.2f / AVG %2.2f]---\n", 
        minutes_v(hour_seconds),
        window24_total(window24, 4), window24_avg(window24, 4),
        window24_total(window24, 6), window24_avg(window24, 6)
    );

    fflush(stdout);

    if(flushed) {
      tm_o.tm_mon = tm.tm_mon;
      tm_o.tm_mday = tm.tm_mday - 1;
      tm_o.tm_year = tm.tm_year;
      mktime(&tm_o);
      printf("-- DAY TOTAL %02d-%02d-%04d: %4.2f AVG: %2.2f ---\n", tm_o.tm_mon + 1, tm_o.tm_mday, tm_o.tm_year + 1900, minutes_v(day_seconds), minutes_v(day_seconds) / (float) 24);
      goto end_log;
    }

    if(tm.tm_hour == 0) {
      printf("--- HOUR: 12 AM ---\n");
    } else if (tm.tm_hour == 12) {
      printf("--- HOUR: 12 PM ---\n");
    } else if (tm.tm_hour > 0 && tm.tm_hour < 12) {
      printf("--- HOUR: %2d AM ---\n", tm.tm_hour);
    } else if (tm.tm_hour > 12) {
      printf("--- HOUR: %2d PM ---\n", tm.tm_hour - 12);
    }
    fflush(stdout);

    current_day = tm.tm_mday;
    current_hour = tm.tm_hour;
    day_seconds = 0;
    num_day_minutes = 0;
    hour_seconds = 0;
    num_hour_minutes = 0;
  } else if (current_hour != tm.tm_hour) {
    slide_window24(window24);
    window24[23] = hour_seconds;

    printf("---TOTAL:[%2.2f] 4H:[TOTAL %3.2f / AVG %2.2f] 6H:[TOTAL %3.2f / AVG %2.2f]---\n", 
        minutes_v(hour_seconds),
        window24_total(window24, 4), window24_avg(window24, 4),
        window24_total(window24, 6), window24_avg(window24, 6)
    );

    fflush(stdout);

    if(tm.tm_hour == 0) {
      printf("--- HOUR: 12 AM ---\n");
    } else if (tm.tm_hour == 12) {
      printf("--- HOUR: 12 PM ---\n");
    } else if (tm.tm_hour > 0 && tm.tm_hour < 12) {
      printf("--- HOUR: %2d AM ---\n", tm.tm_hour);
    } else if (tm.tm_hour > 12) {
      printf("--- HOUR: %2d PM ---\n", tm.tm_hour - 12);
    }
    fflush(stdout);

    day_seconds += hour_seconds;
    hour_seconds = 0;
    num_hour_minutes = 0;
    current_hour = tm.tm_hour;
  }

  hour_seconds += minute_seconds;
  num_hour_minutes++;
  num_day_minutes++;

  if(tm.tm_min % 20 == 0) {
    printf("MIN %02d: ", tm.tm_min);
  } else if(just_started == 1) {
    printf("MIN %02d: ", (tm.tm_min / 20) * 20);
    int m = tm.tm_min % 20;
    while(m > 0) {
      printf("-- ");
      m--;
    }
  }

  if(!gap) {
    printf("%02d ", (int) minute_seconds);
  } else {
    printf("-- ");
  }

  if(tm.tm_min % 20 == 19) {
    printf("\n");
  }
  fflush(stdout);
  just_started = 0;

  tm_p.tm_year = tm.tm_year;
  tm_p.tm_mon = tm.tm_mon;
  tm_p.tm_mday = tm.tm_mday;
  tm_p.tm_hour = tm.tm_hour;
  tm_p.tm_min = tm.tm_min;

}

printf("\n");
end_log:
if(tm.tm_hour != 0 || tm.tm_min != 0) {   // Add a whole day if the file is done without hitting the
  //printf("Incomplete previous day. %d %d %d\n", tm.tm_hour, tm.tm_min);
  tm.tm_hour = 0;// end of the day. Reset all the time counters to 0
  tm.tm_min = 0;
  tm.tm_sec = 0;
  tm.tm_mday ++;
  mktime(&tm);
}

tm_o.tm_mon = tm.tm_mon;
tm_o.tm_mday = tm.tm_mday - 1;
tm_o.tm_year = tm.tm_year;
mktime(&tm_o);
printf("--- ENDING RETROACTIVE LOGGING %d/%d/%d ---\n", tm_o.tm_mon + 1, tm_o.tm_mday, tm_o.tm_year + 1900);

}

}
