#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "treasure_manager.h"

typedef struct UserScore {
    char username[USERNAME_SIZE];
    int total_score;
    struct UserScore* next;
} UserScore;

UserScore* find_or_add(UserScore** head, const char* username) {
    UserScore* current = *head;
    while (current) {
        if (strcmp(current->username, username) == 0)
            return current;
        current = current->next;
    }

    UserScore* new_node = malloc(sizeof(UserScore));
    if (!new_node) {
        perror("malloc");
        exit(1);
    }
    strcpy(new_node->username, username);
    new_node->total_score = 0;
    new_node->next = *head;
    *head = new_node;
    return new_node;
}

int main(int argc, char* argv[]) {
    if (argc != 2) {
        fprintf(stderr, "Usage: %s <treasures.dat>\n", argv[0]);
        return 1;
    }
    //printf("%s\n",argv[1]);
    FILE* f = fopen(argv[1], "rb");
    if (!f) {
        perror("fopen");
        return 1;
    }

    UserScore* scores = NULL;
    Treasure t;
    //printf("!!!");
    while (fread(&t, sizeof(Treasure), 1, f) == 1) {
        UserScore* u = find_or_add(&scores, t.username);
        u->total_score += t.value;
        //printf("%s\n", t.username);
    }
    //printf("---");
    fclose(f);

    UserScore* current = scores;
    //printf("aloalo");
    while (current) {
        printf("%s: %d\n", current->username, current->total_score);
        current = current->next;
    }


    while (scores) {
        UserScore* temp = scores;
        scores = scores->next;
        free(temp);
    }

    return 0;
}