#ifndef TREASURE_MANAGER_H
#define TREASURE_MANAGER_H

#define USERNAME_SIZE 64
#define CLUE_SIZE 256
#define MAX_PATH 256

typedef struct {
    int id;
    char username[USERNAME_SIZE];
    float latitude;
    float longitude;
    char clue[CLUE_SIZE];
    int value;
} Treasure;

void add_treasure(const char *hunt_id);
void list_treasures(const char *hunt_id);
void view_treasure(const char *hunt_id, int tid);
void remove_treasure(const char *hunt_id, int tid);
void remove_hunt(const char *hunt_id);

#endif
