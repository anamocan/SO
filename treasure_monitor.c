#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>
#include <unistd.h>
#include <fcntl.h>
#include <dirent.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include "treasure_manager.h"

#define COMMAND_FILE "monitor_command.txt"

volatile sig_atomic_t command_ready = 0;

void sigusr1_handler(int signo) {
    command_ready = 1;
}

void monitor_loop() {
    while (1) {
        pause();

        if (command_ready) {
            command_ready = 0;

            int fd = open(COMMAND_FILE, O_RDONLY);
            if (fd < 0) {
                perror("monitor: open command file");
                continue;
            }

            char command[256] = {0};
            read(fd, command, sizeof(command) - 1);
            close(fd);
            unlink(COMMAND_FILE);

            char *token = strtok(command, " \n");
            if (!token) continue;

            if (strcmp(token, "stop_monitor") == 0) {
                printf("[Monitor] Stopping...\n");
                fflush(stdout);
                usleep(2000000);
                exit(0);
            }

            else if (strcmp(token, "list_hunts") == 0) {
                DIR *d = opendir(".");
                if (!d) {
                    perror("opendir");
                    continue;
                }

                struct dirent *entry;
                while ((entry = readdir(d)) != NULL) {
                    struct stat st;
                    if (lstat(entry->d_name, &st) == 0 &&
                        S_ISDIR(st.st_mode) &&
                        strncmp(entry->d_name, "hunt:", 5) == 0) {

                        char path[256];
                        snprintf(path, sizeof(path), "%s/treasures.dat", entry->d_name);
                        if (stat(path, &st) == 0) {
                            int fd = open(path, O_RDONLY);
                            if (fd >= 0) {
                                int count = 0;
                                Treasure t;
                                printf("[Monitor] Hunt: %s\n", entry->d_name);
                                printf("-- Treasures --\n");
                                while (read(fd, &t, sizeof(Treasure)) == sizeof(Treasure)) {
                                    count++;
                                    printf("ID: %d | User: %s | GPS: (%.4f, %.4f) | Value: %d\n",
                                           t.id, t.username, t.latitude, t.longitude, t.value);
                                }
                                printf("Total treasures: %d\n\n", count);
                                close(fd);
                            }
                        }
                    }
                }
                closedir(d);
                fflush(stdout);
            }

            else if (strcmp(token, "list_treasures") == 0) {
                char *hunt_id = strtok(NULL, " \n");
                if (hunt_id)
                    list_treasures(hunt_id);
                else
                    printf("[Monitor] Missing hunt_id.\n");
                fflush(stdout);
            }

            else if (strcmp(token, "view_treasure") == 0) {
                char *hunt_id = strtok(NULL, " \n");
                char *id_str = strtok(NULL, " \n");
                if (hunt_id && id_str)
                    view_treasure(hunt_id, atoi(id_str));
                else
                    printf("[Monitor] Missing hunt_id or treasure_id.\n");
                fflush(stdout);
            }

            else if (strcmp(token, "calculate_score") == 0) {
                DIR* d = opendir(".");

                if (!d) {
                    perror("opendir");
                    continue;
                }

                struct dirent* entry;
                while ((entry = readdir(d)) != NULL) {
                    struct stat st;
                    //printf("%s\n",entry->d_name);
                    if (lstat(entry->d_name, &st) == 0 &&
                        S_ISDIR(st.st_mode) ) {

                        char treasures_path[256];
                        snprintf(treasures_path, sizeof(treasures_path), "%s/treasures.dat", entry->d_name);

                        if (stat(treasures_path, &st) != 0) continue;

                        int pipefd[2];
                        if (pipe(pipefd) == -1) {
                            perror("pipe");
                            continue;
                        }

                        pid_t pid = fork();
                        if (pid == 0) {
                            // child
                            close(pipefd[0]);
                            dup2(pipefd[1], STDOUT_FILENO);
                            close(pipefd[1]);
                            execl("./calculate_score", "calculate_score", treasures_path, NULL);
                            perror("exec calculate_score");
                            exit(1);
                        } else if (pid > 0) {
                            // parent
                            close(pipefd[1]);
                            char buffer[512];
                            ssize_t bytes_read;
                            printf("[Monitor] Scores for %s:\n", entry->d_name);
                            while ((bytes_read = read(pipefd[0], buffer, sizeof(buffer)-1)) > 0) {
                                buffer[bytes_read] = '\0';
                                printf("%s", buffer);
                            }
                            close(pipefd[0]);
                            waitpid(pid, NULL, 0);
                            printf("\n");
                        }
                    }
                }
                closedir(d);
                fflush(stdout);
            }

            else {
                printf("[Monitor] Unknown command.\n");
                fflush(stdout);
            }
        }
    }
}

int main() {
    struct sigaction sa;
    sa.sa_handler = sigusr1_handler;
    sigemptyset(&sa.sa_mask);
    sa.sa_flags = 0;
    sigaction(SIGUSR1, &sa, NULL);

    printf("[Monitor] Ready and waiting...\n");
    fflush(stdout);

    monitor_loop();
    return 0;
}