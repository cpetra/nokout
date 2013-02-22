/**************************************************************
 Name        : nokout_daemon.c
 Version     : 0.1

 Copyright (C) 2013 Constantin Petra

This program is free software; you can redistribute it and/or
modify it under the terms of the GNU Lesser General Public
License as published by the Free Software Foundation; either
version 2.1 of the License, or (at your option) any later version.

This library is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
Lesser General Public License for more details.
***************************************************************************/

#include <wiringPi.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <time.h>
#include <signal.h>
#include <syslog.h>
#include <sys/sysinfo.h>
#include <sys/types.h>
#include <sys/stat.h>

#include "PCD8544.h"

int _din = 12;  /* MOSI */
int _sclk = 14; /* SCLK */
int _dc = 0;
int _rst = 2;
int _cs = 3;
int _bl = 7;

int contrast = 50;

static void do_init(int led_on);
static void do_stuff(void);


static void quit_handler(int sig)
{
    signal(sig, SIG_IGN);
    digitalWrite(_cs, LOW);
    digitalWrite(_rst, LOW);
    digitalWrite(_bl, LOW);
    syslog(LOG_NOTICE, "daemon stopped\n");
    closelog();
    exit(0);
}

/* Setup this program as daemon */
static void daemonify(const char *progname)
{
    pid_t pid;
    pid_t sid;
    pid = fork();

    if (pid < 0) {
        exit(1); /* couldn't fork */
    }
    else if (pid > 0) {
        exit(0); /* exit if parent, normal */
    }

    umask(0);

    openlog(progname, LOG_NOWAIT|LOG_PID, LOG_USER);

    syslog(LOG_NOTICE, "daemon starting\n"); 
    /* Now change group ID */
    sid = setsid();
    if (sid < 0) {
        syslog(LOG_ERR, "Error creating group\n");
        exit(2);
    }

    /* Change working directory */
    if ((chdir("/")) < 0) {
        exit(EXIT_FAILURE);
    }
    /* No need for STD descriptors */
    close(STDIN_FILENO);
    close(STDOUT_FILENO);
    close(STDERR_FILENO);
    syslog(LOG_NOTICE, "daemon started\n"); 
}

static void do_init(int led_on)
{
    if (wiringPiSetup() == -1) {
        printf("wiringPi-Error\n");
        exit(1);
    }

    if (LCDInit(_sclk, _din, _dc, _cs, _rst, contrast) < 0) {
        printf("LCD Init error\n");
        exit(1);
    }

    signal (SIGTERM, quit_handler);
    signal (SIGHUP, quit_handler);

    LCDclear();
    pinMode(_bl, OUTPUT);    /* hit the backlight */

    digitalWrite(_bl, (led_on == 1)? HIGH : LOW);
}

int main(int argc, char *argv[])
{
    int led_on = 0;
    daemonify(argv[0]);

    /* install SIGTERM handler */
    if (argc > 1 && strcmp(argv[1], "led_on") == 0) {
        led_on = 1;
    }

    do_init(led_on);
    do_stuff();

    exit(0);
}


static void do_stuff(void)
{
    char buf[128];
    struct sysinfo info;
    time_t t;
    struct tm tm = *localtime(&t);
    float fdiv = (float) (1 << SI_LOAD_SHIFT);

    while(1) {
        LCDclear();

        /* Get and display time */
        t  = time(NULL);
        tm = *localtime(&t);


        sprintf(buf, "%02d:%02d:%02d", tm.tm_hour, tm.tm_min, tm.tm_sec);
        LCDdrawstring(0, 0, buf);

        sprintf(buf, "%d/%02d/%02d", tm.tm_year + 1900, tm.tm_mon, tm.tm_mday);
        LCDdrawstring(0, 8, buf);

        /* Display available memory */
        sysinfo(&info);
        sprintf(buf, "RAM: %lu MB", info.freeram / (1024 * 1024));
        LCDdrawstring(0, 24, buf);

        LCDdrawstring(0, 32, "CPU");
        sprintf(buf, "%.2f %.2f %.2f",
                (float)info.loads[0] / fdiv,
                (float)info.loads[1] / fdiv,
                (float)info.loads[2] / fdiv);
        LCDdrawstring(0, 40, buf);

        LCDdisplay();
        delay(1000);
    }
}
