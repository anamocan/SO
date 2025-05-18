#ifndef HUB_H
#define HUB_H

#include <unistd.h>  // for pid_t

#define COMMAND_FILE "monitor_command.txt"

typedef enum { OFF, RUNNING } MonitorStatus;

typedef struct {
    pid_t monitor_pid;
    MonitorStatus monitor_status;
} Hub_Monitor_T;

extern Hub_Monitor_T hub_monitor;

#endif