
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>
#include <unistd.h>
#include <dirent.h>
#include <sys/stat.h>
#include <fcntl.h>
#include "treasure_manager.h"

#define COMMAND_FILE "monitor_command.txt"

volatile sig_atomic_t command_ready = 0;

void sigusr1_handler(int signo) {
    command_ready = 1;
}

void monitor_loop() {
    while (1) {
        pause(); // wait for SIGUSR1

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
                    if (entry->d_type == DT_DIR && strncmp(entry->d_name, ".", 1) != 0) {
                        char path[256];
                        snprintf(path, sizeof(path), "%s/treasures.dat", entry->d_name);
                        struct stat st;
                        if (stat(path, &st) == 0) {
                            int fd = open(path, O_RDONLY);
                            if (fd >= 0) {
                                int count = 0;
                                Treasure t;
                                printf("\n[Monitor] Hunt: %s\n", entry->d_name);
                                printf("-- Treasures --\n");
                                while (read(fd, &t, sizeof(Treasure)) == sizeof(Treasure)) {
                                    count++;
                                    printf("ID: %d | User: %s | GPS: (%.4f, %.4f) | Value: %d\n",
                                        t.id, t.username, t.latitude, t.longitude, t.value);
                                }
                                printf("Total treasures: %d\n", count);
                                close(fd);
                            }
                        }
                    }
                }
                closedir(d);
            }
            
            else if (strcmp(token, "list_treasures") == 0) {
                char *hunt_id = strtok(NULL, " \n");
                if (hunt_id)
                    list_treasures(hunt_id);
                else
                    printf("[Monitor] Missing hunt_id.\n");
            }
            else if (strcmp(token, "view_treasure") == 0) {
                char *hunt_id = strtok(NULL, " \n");
                char *id_str = strtok(NULL, " \n");
                if (hunt_id && id_str)
                    view_treasure(hunt_id, atoi(id_str));
                else
                    printf("[Monitor] Missing hunt_id or treasure_id.\n");
            }
            else {
                printf("[Monitor] Unknown command.\n");
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