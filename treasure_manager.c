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

#define USERNAME_MAX 32
#define CLUE_MAX 256
#define MAX_PATH 512

typedef struct {
    int treasure_id;
    char username[USERNAME_MAX];
    float latitude;
    float longitude;
    char clue[CLUE_MAX];
    int value;
} Treasure;

// Function declarations
void add_treasure(const char *hunt_id);
void list_treasures(const char *hunt_id);
void view_treasure(const char *hunt_id, int treasure_id);
void remove_treasure(const char *hunt_id, int treasure_id);
void remove_hunt(const char *hunt_id);
void log_operation(const char *hunt_id, const char *message);
void create_symlink(const char *hunt_id);

int main(int argc, char *argv[]) {
    if (argc < 3) {
        fprintf(stderr, "Usage:\n"
                        "--add <hunt_id>\n"
                        "--list <hunt_id>\n"
                        "--view <hunt_id><treasure_id>\n"
                        "--remove_treasure <hunt_id> <treasure_id>\n"
                        "--remove_hunt <hunt_id>\n");
        exit(EXIT_FAILURE);
    }

    if (strcmp(argv[1], "--add") == 0) {
        add_treasure(argv[2]);
    } else if (strcmp(argv[1], "--list") == 0) {
        //list_treasures(argv[2]);
    } else if (strcmp(argv[1], "--view") == 0) {
        if (argc < 4) {
            fprintf(stderr, "Missing treasure ID\n");
            exit(EXIT_FAILURE);
        }
        view_treasure(argv[2], atoi(argv[3]));
    } else if (strcmp(argv[1], "--remove_treasure") == 0) {
        if (argc < 4) {
            fprintf(stderr, "Missing treasure ID\n");
            exit(EXIT_FAILURE);
        }
        //remove_treasure(argv[2], atoi(argv[3]));
    } else if (strcmp(argv[1], "--remove_hunt") == 0) {
        //remove_hunt(argv[2]);
    } else {
        fprintf(stderr, "Unknown command\n");
        exit(EXIT_FAILURE);
    }

    return 0;
}


void log_operation(const char *hunt_id, const char *message) {
    char log_path[MAX_PATH];
    snprintf(log_path, sizeof(log_path), "%s/logged_hunt", hunt_id);

    int fd = open(log_path, O_WRONLY | O_CREAT | O_APPEND, 0644);
    if (fd == -1) {
        perror("open (logged_hunt)");
        return;
    }

    write(fd, message, strlen(message));
    close(fd);
}

void create_symlink(const char *hunt_id) {
    char target[MAX_PATH];
    char link_name[MAX_PATH];

    snprintf(target, sizeof(target), "%s/logged_hunt", hunt_id);
    snprintf(link_name, sizeof(link_name), "logged_hunt-%s", hunt_id);

    // Remove if exists
    unlink(link_name);

    if (symlink(target, link_name) == -1) {
        perror("symlink");
    }
}

void add_treasure(const char* hunt_id) {
    char dir_path[MAX_PATH];
    char file_path[MAX_PATH];
    char log_path[MAX_PATH];
    Treasure t;

    snprintf(dir_path, sizeof(dir_path), "%s", hunt_id);
    snprintf(file_path, sizeof(file_path) + 14, "%s/treasures.dat", dir_path);
    snprintf(log_path, sizeof(log_path) + 12, "%s/logged_hunt", dir_path);

    if (mkdir(dir_path, 0755) == -1 && errno != EEXIST) {
        perror("mkdir");
        exit(EXIT_FAILURE);
    }


    // Get treasure data from user
    printf("Enter Treasure ID: ");
    scanf("%d", &t.treasure_id);
    getchar(); // consume newline

    printf("Enter Username: ");
    fgets(t.username, USERNAME_MAX, stdin);
    t.username[strcspn(t.username, "\n")] = 0;
    /*strcspn(t.username, "\n") finds the index of the newline character \n (if it exists).
    That character is replaced with a null terminator (\0) to remove the trailing 
    newline that fgets typically captures when the user hits Enter.*/

    printf("Enter Latitude: ");
    scanf("%f", &t.latitude);

    printf("Enter Longitude: ");
    scanf("%f", &t.longitude);
    getchar(); // consume newline

    printf("Enter Clue: ");
    fgets(t.clue, CLUE_MAX, stdin);
    t.clue[strcspn(t.clue, "\n")] = 0;

    printf("Enter Value: ");
    scanf("%d", &t.value);

    // Write to binary file
    int fd = open(file_path, O_WRONLY | O_CREAT | O_APPEND, 0644);
    if (fd == -1) {
        perror("open (treasures.dat)");
        exit(EXIT_FAILURE);
    }

    if (write(fd, &t, sizeof(Treasure)) != sizeof(Treasure)) {
        perror("write");
        close(fd);
        exit(EXIT_FAILURE);
    }

    close(fd);


    // Log the operation
    char log_msg[256];
    snprintf(log_msg, sizeof(log_msg), "Added treasure ID %d by user %s\n", t.treasure_id, t.username);
    log_operation(hunt_id, log_msg);

    // Create symlink to logged_hunt
    create_symlink(hunt_id);

    printf("Treasure added successfully.\n");
}

void view_treasure(const char *hunt_id, int treasure_id) {
    char file_path[MAX_PATH];
    snprintf(file_path, sizeof(file_path), "%s/treasures.dat", hunt_id);

    int fd = open(file_path, O_RDONLY);
    if (fd == -1) {
        perror("open (treasures.dat)");
        return;
    }

    Treasure t;
    while (read(fd, &t, sizeof(Treasure)) == sizeof(Treasure)) {
        if (t.treasure_id == treasure_id) {
            printf("Treasure ID: %d\n", t.treasure_id);
            printf("Username: %s\n", t.username);
            printf("Coordinates: %.2f, %.2f\n", t.latitude, t.longitude);
            printf("Clue: %s\n", t.clue);
            printf("Value: %d\n", t.value);
            close(fd);
            return;
        }
    }

    printf("Treasure with ID %d not found.\n", treasure_id);
    close(fd);
}