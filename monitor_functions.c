#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <time.h>
#include <dirent.h>
#include <errno.h>
#include "monitor_functions.h"

#define MAX_PATH 512
#define LOG_MESSAGE_LENGTH 256
#define TREASURE_FILE_NAME "treasures.dat"

void list_hunts() {
    DIR* dir = opendir("."); // opens directory in which monitor_functions.c is located
    if (dir == NULL) {
        perror("opendir");
        return;
    }


    struct dirent* entry;
    struct stat st;


    while ((entry = readdir(dir)) != NULL) {
        if (entry->d_type == DT_DIR &&
            strcmp(entry->d_name, ".") != 0 &&
            strcmp(entry->d_name, "..") != 0) {  // daca nu e folderul curent sau precedent

            char treasure_dat_path[MAX_PATH];
            snprintf(treasure_dat_path, MAX_PATH, "%s/%s", entry->d_name, TREASURE_FILE_NAME);

            if (stat(treasure_dat_path, &st) == 0) {  // daca exista fisierul treasures.dat intra in if
                int fd = open(treasure_dat_path, O_RDONLY);
                if (fd < 0) {
                    continue;
                }

                int count = 0;
                Treasure t;
                while (read(fd, &t, sizeof(Treasure)) == sizeof(Treasure)) {
                    count++;
                }

                close(fd);

                printf("Hunt: %s, Number of Treasures: %d\n", entry->d_name, count);
            }
        }
    }

    printf("\n");
    closedir(dir);
}


void list_treasures(char *hunt_id) {
    char dat_path[MAX_PATH];
    snprintf(dat_path, MAX_PATH, "%s/treasures.dat", hunt_id);

    struct stat st;
    if (stat(dat_path, &st) == -1) {
        perror("file doesn't exist");
        return;
    }

    // Print hunt details
    printf("Hunt: %s\n"
        "File Size: %ld bytes\n"
        "Last Modified: %s", hunt_id, st.st_size, ctime(&st.st_mtime));

    // Read and list treasures
    int fd = open(dat_path, O_RDONLY);
    if (fd == -1) {
        perror("open (treasures.dat)");
        return;
    }

    Treasure t;
    while (read(fd, &t, sizeof(Treasure)) == sizeof(Treasure)) {
        printf("\nTreasure ID: %d\n"
                "Username: %s\n"
                "Coordinates: %.2f, %.2f\n"
                "Clue: %s\n"
                "Value: %d\n", t.treasure_id, t.username, t.latitude, t.longitude, t.clue, t.value);
    }

    close(fd);

    printf("\n");
}


void view_treasure(char *hunt_id, int treasure_id) {
    char file_path[MAX_PATH];
    snprintf(file_path, MAX_PATH, "%s/treasures.dat", hunt_id);

    int fd = open(file_path, O_RDONLY);
    if (fd == -1) {
        perror("open (treasures.dat)");
        return;
    }

    Treasure t;
    while (read(fd, &t, sizeof(Treasure)) == sizeof(Treasure)) {
        if (t.treasure_id == treasure_id) {
            printf("Treasure ID: %d\n"
                "Username: %s\n"
                "Coordinates: %.2f, %.2f\n"
                "Clue: %s\n"
                "Value: %d\n\n", t.treasure_id, t.username, t.latitude, t.longitude, t.clue, t.value);
            close(fd);
            return;
        }
    }

    printf("Treasure with ID %d not found.\n\n", treasure_id);
    close(fd);
}

