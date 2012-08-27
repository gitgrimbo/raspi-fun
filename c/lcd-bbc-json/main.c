/*
 *    main.c
 *
 *    Written, if that's the word, by Phil Bambridge
 *    with much help from John Honniball.
 *
 *    v1.0.0 - 2012/07/16
 *    v1.0.1 - 2012/07/19 (Changes to signal handling, swapped RS and EN pins for diagram ease)
 *    v1.0.2 - 2012/07/19 (Refactored to move rpi code to rpi.x, lcd code to lcd.x, added timeofday)
 *    v1.0.3 - 2012/07/21 (Rechecks for IP address, allows arbitrary datapins, works on 4 line LCD)
 *    v1.0.4 - 2012/07/22 (Reverted to inline define for CMDxBITS, fixed system load calculation)
 *    v1.0.5 - 2012/07/23 (Added support for multiple LCD modules)
 */


#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <sys/ioctl.h> 
#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>

#include <arpa/inet.h>
#include <net/if.h>

#include <sys/sysinfo.h>
#include <sys/vfs.h>

#include <time.h>

#include <unistd.h>

#include <signal.h>

#include "lcd.h"


// GRIMBO
// JSON includes.
#include "grimbo/bbc.h"
#include <jansson.h>
// GRIMBO


#define SECS_IN_DAY 86400
#define SECS_IN_HOUR 3600
#define SECS_IN_MIN 60

struct lcdmodule module;

void maybe_reload_schedule();
void updateDisplay (struct lcdmodule module);
void sigintHandler(void);
void sigtermHandler(void);
void clearGarbage ();

#define inaddrr(x) (*(struct in_addr *) &ifr->x[sizeof sa.sin_port])
#define IFRSIZE   ((int)(size * sizeof (struct ifreq)))



// GRIMBO
// Globals to keep track of the current data set,
// when we last refreshed the data,
// and which broadcast to show.
int EVENT_REFRESH_PERIOD_S = 60 * 1;
json_t * cur_upcoming_schedule = NULL;
// cur_broadcasts are shortcuts
// DO NOT FREE MEMORY FOR THESE!
json_t * cur_broadcasts = NULL;
time_t last_refresh_time = 0;
int cur_broadcast = 0;
// GRIMBO



int main(int argc, char **argv) {
  struct lcdmodule module2;

  // GRIMBO
  // init vars
  cur_upcoming_schedule = NULL;
  cur_broadcasts = NULL;

  // Here we create and initialise two LCD modules.
  // The arguments are, in order, EN, RS, D4, D5, D6, D7.
  module = lcdInit(17, 18, 22, 23, 24, 25);
  module2 = lcdInit(21, 18, 22, 23, 24, 25);

  prints(module2, "Ti goes with Pi.");

  // Register signal handlers to clear the screen before
  // shutting down. 
  signal(SIGINT, (__sighandler_t)sigintHandler);
  signal(SIGTERM, (__sighandler_t)sigtermHandler);

  while (1) {
    maybe_reload_schedule();
    updateDisplay(module);
    sleep(2);
  }

  clearGarbage();

  return 0;
} // main


void reload_schedule (json_t * schedule) {
    if (cur_upcoming_schedule) {
        json_decref(cur_upcoming_schedule);
    }

    cur_upcoming_schedule = schedule;
    cur_broadcast = 0;

    cur_broadcasts = json_object_get(cur_upcoming_schedule, "broadcasts");
    if (!json_is_array(cur_broadcasts)) {
        fprintf(stderr, "error: broadcasts is not an array\n");
        cur_broadcasts = NULL;
    } else {
        int size = json_array_size(cur_broadcasts);
        printf("Found %d broadcasts\n", size);
    }
}

void maybe_reload_schedule () {
    time_t timenow;
    json_t * schedule = NULL;
    json_error_t error;

    // Work out the time since last refresh
    timenow = time(NULL);
    double diff = difftime(timenow, last_refresh_time);
    int refresh_timeout = diff > EVENT_REFRESH_PERIOD_S;

    // should refresh if schedule is NULL or we're due a refresh.
    int should_refresh = !cur_upcoming_schedule || refresh_timeout;

    printf("now=%d, last=%d, diff=%f, should refresh=%d\n", timenow, last_refresh_time, diff, should_refresh);

    if (!should_refresh) {
        return;
    }

    // we should refresh if there is no current schedule loaded,
    // or if the refresh period has expired
    schedule = grimbo_bbc_get_upcoming_science_and_nature_from_url(&error);
    if (!schedule) {
        // error. print and leave everything as-is.
        fprintf(stderr, "error: on line %d: %s\n", error.line, error.text);
        return;
    }

    // got a local schedule.
    // replace the global schedule and release the previous, if any.
    reload_schedule(schedule);
    last_refresh_time = timenow;
}

void display_broadcast (struct lcdmodule module, json_t * broadcast) {
    if (!json_is_object(broadcast)) {
        fprintf(stderr, "error: broadcast is not an object\n");
        return;
    }

    json_t * programme = json_object_get(broadcast, "programme");
    if (!json_is_object(programme)) {
        fprintf(stderr, "error: programme is not an object\n");
        return;
    }

    json_t * display_titles = json_object_get(programme, "display_titles");
    if (!json_is_object(display_titles)) {
        fprintf(stderr, "error: display_titles is not an object\n");
        return;
    }

    json_t * title = json_object_get(display_titles, "title");
    if (!json_is_string(title)) {
        fprintf(stderr, "error: title is not a string\n");
        return;
    }

    const char * title_str = json_string_value(title);
    prints(module, title_str);
}

void display_cur_broadcast (struct lcdmodule module, json_t * cur_upcoming_schedule, json_t * cur_broadcasts) {
    int size = json_array_size(cur_broadcasts);

    // assign a local broadcast index and increment the global.
    if (cur_broadcast >= size) {
        cur_broadcast = 0;
    }
    int broadcast_idx = cur_broadcast;
    cur_broadcast++;

    json_t * broadcast = json_array_get(cur_broadcasts, broadcast_idx);
    if (!json_is_object(broadcast)) {
        fprintf(stderr, "error: broadcast is not an object\n");
        return;
    }

    display_broadcast(module, broadcast);
}

void updateDisplay (struct lcdmodule module) { 
    // Clear the lines
    gotoXy(module, 0,0);
    prints(module, "                ");
    gotoXy(module, 0,1);
    prints(module, "                ");
    gotoXy(module, 0,0);

    if (cur_broadcasts) {
        display_cur_broadcast(module, cur_upcoming_schedule, cur_broadcasts);
    }
}


void clearGarbage () {
    if (cur_upcoming_schedule) {
        json_decref(cur_upcoming_schedule);
        cur_upcoming_schedule = NULL;
    }
}

void sigintHandler(void) {
    clearLcd(module);
    clearGarbage();
    exit(0);
}

void sigtermHandler(void) {
    clearLcd(module);
    clearGarbage();
    prints(module, "Shutting down...");
    exit(0);
}
