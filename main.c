#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "treasure_manager.h"

int main(int argc, char *argv[]) {
    if (argc < 3) {
        printf("Usage:\n");
        printf("  %s --add <hunt_id>\n", argv[0]);
        printf("  %s --list <hunt_id>\n", argv[0]);
        printf("  %s --view <hunt_id> <treasure_id>\n", argv[0]);
        printf("  %s --remove <hunt_id> <treasure_id>\n", argv[0]);
        printf("  %s --remove-hunt <hunt_id>\n", argv[0]);
        return 1;
    }

    if (strcmp(argv[1], "--add") == 0) {
        add_treasure(argv[2]);
    } else if (strcmp(argv[1], "--list") == 0) {
        list_treasures(argv[2]);
    } else if (strcmp(argv[1], "--view") == 0 && argc == 4) {
        view_treasure(argv[2], atoi(argv[3]));
    } else if (strcmp(argv[1], "--remove") == 0 && argc == 4) {
        remove_treasure(argv[2], atoi(argv[3]));
    } else if (strcmp(argv[1], "--remove-hunt") == 0) {
        remove_hunt(argv[2]);
    } else {
        fprintf(stderr, "Unknown command.\n");
    }

    return 0;
}
