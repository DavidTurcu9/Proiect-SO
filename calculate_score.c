#include <stdio.h>

int main(int argc, char *argv[]) {
    if (argc != 2) {
        fprintf(stderr, "Usage: %s <hunt_name>\n", argv[0]);
        return 1;
    }

    // TODO: parse files related to the hunt and compute scores
    printf("%s    User1: 100\nUser2: 85\n", argv[1]);
    return 0;
}