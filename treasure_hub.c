#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int main(int argc, char** argv) {
    if (argc != 2) {
        fprintf(stderr, "Usage:\n"
                        "--start_monitor\n"
                        "--list_hunts\n"
                        "--list_treasures\n"
                        "--view_treasure\n"
                        "--stop_monitor\n"
                        "--exit\n");
        exit(EXIT_FAILURE);
    }

    if (strcmp(argv[1], "--start_monitor") == 0) {
        //add_treasure(argv[2]);
    } else if (strcmp(argv[1], "--list_hunts") == 0) {
        //list_treasures(argv[2]);
    } else if (strcmp(argv[1], "--list_treasures") == 0) {
        //view_treasure(argv[2], atoi(argv[3]));
    } else if (strcmp(argv[1], "--view_treasure") == 0) {
        //remove_treasure(argv[2], atoi(argv[3]));
    } else if (strcmp(argv[1], "--stop_monitor") == 0) {
        //remove_hunt(argv[2]);
    } else if (strcmp(argv[1], "--exit") == 0) {

    } else {
        fprintf(stderr, "Unknown command\n");
        exit(EXIT_FAILURE);
    }

    return 0;
}
