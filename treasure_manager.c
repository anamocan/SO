#include "treasure_manager.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/stat.h>
#include <time.h>
#include <errno.h>

void log_action(const char *hunt_dir, const char *message) {
    char log_path[MAX_PATH];
    snprintf(log_path, sizeof(log_path), "%s/logged_hunt.txt", hunt_dir);
    int fd = open(log_path, O_WRONLY | O_CREAT | O_APPEND, 0644);
    if (fd == -1) return;

    time_t now = time(NULL);
    char *time_str = ctime(&now);
    char buffer[512];
    snprintf(buffer, sizeof(buffer), "[%.*s] %s\n", (int)(strlen(time_str) - 1), time_str, message);
    write(fd, buffer, strlen(buffer));
    close(fd);
}

void create_symlink(const char *hunt_id) {
    char target[MAX_PATH], link_name[MAX_PATH];
    snprintf(target, sizeof(target), "%s/logged_hunt.txt", hunt_id);
    snprintf(link_name, sizeof(link_name), "logged_hunt-%s", hunt_id);
    symlink(target, link_name);
}

void ensure_directory(const char *hunt_id) {
    struct stat st;
    if (stat(hunt_id, &st) == -1) {
        mkdir(hunt_id, 0755);
    }
    create_symlink(hunt_id);
}

void add_treasure(const char *hunt_id) {
    ensure_directory(hunt_id);

    Treasure t;
    printf("Enter Treasure ID: "); scanf("%d", &t.id); getchar();
    printf("Enter Username: "); fgets(t.username, USERNAME_SIZE, stdin);
    t.username[strcspn(t.username, "\n")] = '\0';
    printf("Enter Latitude: "); scanf("%f", &t.latitude);
    printf("Enter Longitude: "); scanf("%f", &t.longitude); getchar();
    printf("Enter Clue: "); fgets(t.clue, CLUE_SIZE, stdin);
    t.clue[strcspn(t.clue, "\n")] = '\0';
    printf("Enter Value: "); scanf("%d", &t.value);

    char file_path[MAX_PATH];
    snprintf(file_path, sizeof(file_path), "%s/treasures.dat", hunt_id);
    int fd = open(file_path, O_WRONLY | O_CREAT | O_APPEND, 0644);
    if (fd == -1) { perror("open"); return; }

    write(fd, &t, sizeof(Treasure));
    close(fd);

    char log_msg[128];
    snprintf(log_msg, sizeof(log_msg), "Added treasure ID %d", t.id);
    log_action(hunt_id, log_msg);
    printf("Treasure added to hunt %s.\n", hunt_id);
}

void list_treasures(const char *hunt_id) {
    char file_path[MAX_PATH];
    snprintf(file_path, sizeof(file_path), "%s/treasures.dat", hunt_id);

    struct stat st;
    if (stat(file_path, &st) == -1) {
        printf("Hunt '%s' not found or no treasures.\n", hunt_id);
        return;
    }

    printf("Hunt: %s\nSize: %ld bytes\nLast Modified: %s", hunt_id, st.st_size, ctime(&st.st_mtime));

    int fd = open(file_path, O_RDONLY);
    if (fd == -1) { perror("open"); return; }

    Treasure t;
    printf("\n-- Treasures --\n");
    while (read(fd, &t, sizeof(Treasure)) == sizeof(Treasure)) {
        printf("ID: %d | User: %s | GPS: (%.4f, %.4f) | Value: %d\n", t.id, t.username, t.latitude, t.longitude, t.value);
    }

    close(fd);
    log_action(hunt_id, "Listed all treasures.");
}

void view_treasure(const char *hunt_id, int tid) {
    char file_path[MAX_PATH];
    snprintf(file_path, sizeof(file_path), "%s/treasures.dat", hunt_id);

    int fd = open(file_path, O_RDONLY);
    if (fd == -1) { perror("open"); return; }

    Treasure t;
    while (read(fd, &t, sizeof(Treasure)) == sizeof(Treasure)) {
        if (t.id == tid) {
            printf("Treasure ID: %d\nUsername: %s\nGPS: %.4f, %.4f\nClue: %s\nValue: %d\n",
                   t.id, t.username, t.latitude, t.longitude, t.clue, t.value);
            char log_msg[128];
            snprintf(log_msg, sizeof(log_msg), "Viewed treasure ID %d", tid);
            log_action(hunt_id, log_msg);
            close(fd);
            return;
        }
    }

    printf("Treasure ID %d not found.\n", tid);
    close(fd);
}

void remove_treasure(const char *hunt_id, int tid) {
    char file_path[MAX_PATH];
    snprintf(file_path, sizeof(file_path), "%s/treasures.dat", hunt_id);
    char tmp_path[] = "temp.dat";

    int fd_in = open(file_path, O_RDONLY);
    int fd_out = open(tmp_path, O_WRONLY | O_CREAT | O_TRUNC, 0644);
    if (fd_in == -1 || fd_out == -1) {
        perror("open");
        return;
    }

    Treasure t;
    int found = 0;
    while (read(fd_in, &t, sizeof(Treasure)) == sizeof(Treasure)) {
        if (t.id == tid) {
            found = 1;
            continue;
        }
        write(fd_out, &t, sizeof(Treasure));
    }

    close(fd_in);
    close(fd_out);

    if (found) {
        unlink(file_path);
        rename(tmp_path, file_path);
        char log_msg[128];
        snprintf(log_msg, sizeof(log_msg), "Removed treasure ID %d", tid);
        log_action(hunt_id, log_msg);
        printf("Treasure removed.\n");
    } else {
        unlink(tmp_path);
        printf("Treasure ID %d not found.\n", tid);
    }
}

void remove_hunt(const char *hunt_id) {
    char path[MAX_PATH];
    snprintf(path, sizeof(path), "logged_hunt-%s", hunt_id);
    unlink(path);  // Remove symbolic link if exists

    char cmd[MAX_PATH];
    snprintf(cmd, sizeof(cmd), "rm -rf %s", hunt_id);
    int res = system(cmd);
    if (res == 0) {
        printf("Hunt '%s' removed.\n", hunt_id);
    } else {
        printf("Failed to remove hunt.\n");
    }
}
