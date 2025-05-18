#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/wait.h>
#include "hub.h"

Hub_Monitor_T hub_monitor = { -1, OFF };
int monitor_pipe_fd[2];  // pipe[0] = read end (hub), pipe[1] = write end (monitor)

void sigchld_handler(int signo) {
    int status;
    waitpid(hub_monitor.monitor_pid, &status, 0);
    hub_monitor.monitor_pid = -1;
    hub_monitor.monitor_status = OFF;
    close(monitor_pipe_fd[0]);
    write(1, "\nMonitor terminated.\n", 22);
}

int is_monitor_running() {
    return (hub_monitor.monitor_pid > 0 && hub_monitor.monitor_status == RUNNING);
}

int start_monitor() {
    if (is_monitor_running()) {
        printf("Monitor already running.\n");
        return 0;
    }

    if (pipe(monitor_pipe_fd) == -1) {
        perror("pipe");
        return 0;
    }

    pid_t pid = fork();
    if (pid < 0) {
        perror("fork");
        return 0;
    }

    if (pid == 0) {
        // Child process: monitor
        close(monitor_pipe_fd[0]);                 // Close read end
        dup2(monitor_pipe_fd[1], STDOUT_FILENO);   // Redirect stdout to write end of pipe
        close(monitor_pipe_fd[1]);                 // Close original write end

        execl("./treasure_monitor", "treasure_monitor", NULL);
        perror("exec failed");
        exit(1);
    }

    // Parent process: hub
    close(monitor_pipe_fd[1]);  // Close write end in parent
    hub_monitor.monitor_pid = pid;
    hub_monitor.monitor_status = RUNNING;
    printf("Monitor started (PID: %d).\n", pid);


    char buffer[1024];
    ssize_t bytes_read = read(monitor_pipe_fd[0], buffer, sizeof(buffer) - 1);
    if (bytes_read > 0) {
        buffer[bytes_read] = '\0';
        printf("%s", buffer);
    }
    return 1;
}

void send_command_to_monitor(const char *command) {
    if (!is_monitor_running()) {
        printf("Error: Monitor is not running.\n");
        return;
    }

    int fd = open(COMMAND_FILE, O_WRONLY | O_CREAT | O_TRUNC, 0644);
    if (fd < 0) {
        perror("open command file");
        return;
    }

    write(fd, command, strlen(command));
    close(fd);

    kill(hub_monitor.monitor_pid, SIGUSR1);
    //printf("alo");
    // Read result from monitor pipe
    char buffer[1024];
    ssize_t bytes_read = read(monitor_pipe_fd[0], buffer, sizeof(buffer) - 1);
    if (bytes_read > 0) {
        buffer[bytes_read] = '\0';
        printf("%s", buffer);
    }
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
        if (!fgets(input, sizeof(input), stdin)) break;

        input[strcspn(input, "\n")] = 0;

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
        } else if (strncmp(input, "list_hunts", 10) == 0 ||
                   strncmp(input, "list_treasures", 14) == 0 ||
                   strncmp(input, "view_treasure", 13) == 0 ||
                   strncmp(input,"calculate_score",15) == 0 ) {
            send_command_to_monitor(input);
        } else {
            printf("Unknown command.\n");
        }
    }

    return 0;
}