#include <stdio.h>
#include <string.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <unistd.h>

#define MAX_HUNT_NAME_LEN 128
#define MAX_PATH 512
#define TREASURE_FILE_NAME "treasures.dat"
#define MAX_USERS 64

#define USERNAME_MAX 32
#define CLUE_MAX 256

typedef struct Treasure{
    int treasure_id;
    char username[USERNAME_MAX];
    float latitude;
    float longitude;
    char clue[CLUE_MAX];
    int value;
} Treasure;

int main(int argc, char **argv) {
    if (argc != 2) {
        fprintf(stderr, "Usage: %s <hunt_name>\n", argv[0]);
        return 1;
    }

    // TODO: parse files related to the hunt and compute scores
    char hunt_dir_name[MAX_HUNT_NAME_LEN];
    strncpy(hunt_dir_name, argv[1], MAX_HUNT_NAME_LEN);

    char treasure_dat_path[MAX_PATH];
    snprintf(treasure_dat_path, MAX_PATH, "%s/%s", hunt_dir_name, TREASURE_FILE_NAME);

    struct stat st;
    if (stat(treasure_dat_path, &st) == -1) {
        perror("file doesn't exist");
        return 0;
    }

    int fd = open(treasure_dat_path, O_RDONLY);
    if (fd == -1) {
        perror("open (treasures.dat)");
        return 0;
    }

    char users[MAX_USERS][USERNAME_MAX];
    int user_score[MAX_USERS] = { 0 }; // initialize all scores with 0
    int total_users = 0, user_found = 0;
    Treasure t;
    while (read(fd, &t, sizeof(Treasure)) == sizeof(Treasure)) { // go through all treasures and add the scores
        /*printf("\nTreasure ID: %d\n"
                "Username: %s\n"
                "Coordinates: %.2f, %.2f\n"
                "Clue: %s\n"
                "Value: %d\n", t.treasure_id, t.username, t.latitude, t.longitude, t.clue, t.value);*/
        user_found = 0;
        for (int i = 0; i < total_users; i++) {
            if (strcmp(t.username, users[i]) == 0) {
                user_score[i] += t.value;
                user_found = 1;
            }
        }
        if (user_found == 0) {
            strncpy(users[total_users], t.username, USERNAME_MAX);
            user_score[total_users] += t.value;
            total_users++;
        }
    }

    // print the score of each user
    printf("Hunt: %s\n", hunt_dir_name);
    for (int i = 0; i < total_users; i++) {
        printf("\tUser: %s, Score: %d\n", users[i], user_score[i]);
    }
    
    fflush(stdout);

    return 0;
}