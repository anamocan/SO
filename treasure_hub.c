#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <signal.h>
#include <fcntl.h>
#include <sys/wait.h>
#include "hub.h"

Hub_Monitor_T hub_monitor = { -1, OFF };

void send_command_to_monitor(const char *command) {
    int fd = open(COMMAND_FILE, O_WRONLY | O_CREAT | O_TRUNC, 0644);
    if (fd < 0) {
        perror("open command file");
        return;
    }

    write(fd, command, strlen(command));
    close(fd);

    kill(hub_monitor.monitor_pid, SIGUSR1);
}

int is_monitor_running() {
    return (hub_monitor.monitor_pid > 0 && hub_monitor.monitor_status == RUNNING);
}

void sigchld_handler(int signo) {
    int status;
    waitpid(hub_monitor.monitor_pid, &status, 0);
    hub_monitor.monitor_pid = -1;
    hub_monitor.monitor_status = OFF;
    write(1, "\nMonitor terminated.\n", 22);
}

int start_monitor() {
    if (is_monitor_running()) {
        printf("Monitor already running.\n");
        return 0;
    }

    pid_t pid = fork();
    if (pid < 0) {
        perror("fork");
        return 0;
    }

    if (pid == 0) {
        execl("./treasure_monitor", "treasure_monitor", NULL);
        perror("exec failed");
        exit(1);
    }

    hub_monitor.monitor_pid = pid;
    hub_monitor.monitor_status = RUNNING;
    printf("Monitor started (PID: %d).\n", pid);
    return 1;
}

int main() {
    struct sigaction sa;
    sa.sa_handler = sigchld_handler;
    sa.sa_flags = 0;
    sigemptyset(&sa.sa_mask);
    sigaction(SIGCHLD, &sa, NULL);

    char input[256];
    while (1) {
        printf("> ");
        fflush(stdout);
         fgets(input, sizeof(input), stdin);

        input[strcspn(input, "\n")] = 0; // trim newline

        if (strcmp(input, "exit") == 0) {
            if (is_monitor_running()) {
                printf("Error: Monitor is still running.\n");
            } else {
                break;
            }
        } else if (strcmp(input, "start_monitor") == 0) {
            start_monitor();
        } else if (strcmp(input, "stop_monitor") == 0) {
            if (!is_monitor_running()) {
                printf("Monitor not running.\n");
                continue;
            }
            send_command_to_monitor("stop_monitor");
        } else if (strncmp(input, "list_hunts", 10) == 0) {
            if (is_monitor_running())
                send_command_to_monitor("list_hunts");
            else
                printf("Error: Monitor is not running.\n");
        } else if (strncmp(input, "list_treasures", 14) == 0) {
            if (is_monitor_running())
                send_command_to_monitor(input);
            else
                printf("Error: Monitor is not running.\n");
        } else if (strncmp(input, "view_treasure", 13) == 0) {
            if (is_monitor_running())
                send_command_to_monitor(input);
            else
                printf("Error: Monitor is not running.\n");
        } else {
            printf("Unknown command.\n");
        }
    }

    return 0;
}