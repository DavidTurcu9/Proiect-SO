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
#include "treasure_manager_functions.h"


#define MAX_PATH 512
#define LOG_MESSAGE_LENGTH 256


void add_treasure(char* hunt_id) {
    char hunt_dir_path[MAX_PATH];
    snprintf(hunt_dir_path, MAX_PATH, "%s", hunt_id);
    
    char log_path[MAX_PATH];
    snprintf(log_path, MAX_PATH, "%s/logged_hunt", hunt_id);
    
    char dat_path[MAX_PATH];
    snprintf(dat_path, MAX_PATH, "%s/treasures.dat", hunt_id);

    Treasure t;
    
    //check if hunt directory exists already, if not, create it
    struct stat st;
    if (stat(hunt_dir_path, &st) == -1) {
        mkdir(hunt_dir_path, 0755);
    }


    //get treasure details from stdin
    printf("Enter Treasure ID (int): ");
    scanf("%d", &t.treasure_id);
    getchar(); // consume newline

    printf("Enter Username (string): ");
    fgets(t.username, USERNAME_MAX, stdin);
    t.username[strcspn(t.username, "\n")] = 0;
    /*strcspn(t.username, "\n") finds the index of the newline character \n (if it exists).
    That character is replaced with a null terminator (\0) to remove the trailing 
    newline that fgets typically captures when the user hits Enter.*/

    printf("Enter Latitude (float): ");
    scanf("%f", &t.latitude);

    printf("Enter Longitude (float): ");
    scanf("%f", &t.longitude);
    getchar(); // consume newline

    printf("Enter Clue (string): ");
    fgets(t.clue, CLUE_MAX, stdin);
    t.clue[strcspn(t.clue, "\n")] = 0;

    printf("Enter Value (int): ");
    scanf("%d", &t.value);


    //create treasures.dat
    int fd = open(dat_path, O_WRONLY | O_CREAT | O_APPEND, 0644);
    if (fd == -1) {
        perror("open (treasures.dat)");
        exit(EXIT_FAILURE);
    }

    //write treasure to treasures.dat
    //write returns sizeof whatever it wrote
    if (write(fd, &t, sizeof(Treasure)) != sizeof(Treasure)) {
        perror("write");
        close(fd);
        exit(EXIT_FAILURE);
    }

    close(fd);

    //log treasure
    char log_message[LOG_MESSAGE_LENGTH];
    snprintf(log_message, LOG_MESSAGE_LENGTH, "Added treasure ID %d by user %s\n", t.treasure_id, t.username);
    fd = open(log_path, O_WRONLY | O_CREAT | O_APPEND, 0644);
    if (fd == -1) {
        perror("open (logged_hunt)");
        exit(EXIT_FAILURE);
    }
    
    if (write(fd, log_message, strlen(log_message)) != strlen(log_message)) {
        perror("write");
        close(fd);
        exit(EXIT_FAILURE);
    }

    close(fd);
 
    //create symlink
    char target[MAX_PATH];
    char link_name[MAX_PATH];

    snprintf(target, MAX_PATH, "%s/logged_hunt", hunt_id);
    snprintf(link_name, MAX_PATH, "logged_hunt-%s", hunt_id);

    // Remove symlink if exists
    unlink(link_name);

    if (symlink(target, link_name) == -1) {
        perror("symlink");
    }

    printf("Treasure added successfully.\n");
}


void list_treasures(char* hunt_id) {
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
}

void view_treasure(char* hunt_id, int treasure_id) {
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
                "Value: %d\n", t.treasure_id, t.username, t.latitude, t.longitude, t.clue, t.value);
            close(fd);
            return;
        }
    }

    printf("Treasure with ID %d not found.\n", treasure_id);
    close(fd);
}

void remove_treasure(const char *hunt_id, int treasure_id) {
    char file_path[MAX_PATH];
    snprintf(file_path, MAX_PATH, "%s/treasures.dat", hunt_id);
    int fd = open(file_path, O_RDWR);
    if (fd == -1) {
        perror("open (treasures.dat)");
        return;
    }

    // Temporary file with all treasures except the one that needs to be removed
    char temp_path[MAX_PATH];
    snprintf(temp_path, MAX_PATH, "%s/temp_treasures.dat", hunt_id);
    int temp_fd = open(temp_path, O_WRONLY | O_CREAT, 0644);
    if (temp_fd == -1) {
        perror("open (temp file)");
        close(fd);
        return;
    }

    Treasure t;
    int treasure_found = 0;
    while (read(fd, &t, sizeof(Treasure)) == sizeof(Treasure)) {
        if (t.treasure_id == treasure_id) {
            treasure_found = 1;
            continue;
        }
        if (write(temp_fd, &t, sizeof(Treasure)) != sizeof(Treasure)) {
            perror("write");
            close(fd);
            exit(EXIT_FAILURE);
        }
    }

    if (treasure_found) {
        printf("Treasure with ID %d removed successfully.\n", treasure_id);
        // Replace the old file with the new file
        remove(file_path);
        rename(temp_path, file_path);
    }
    else {
        printf("Treasure with ID %d not found.\n", treasure_id);
    }

    close(fd);
    close(temp_fd);
}

void remove_hunt(const char *hunt_id) {
    char hunt_dir_path[MAX_PATH];
    snprintf(hunt_dir_path, MAX_PATH, "%s", hunt_id);

    char log_path[MAX_PATH];
    snprintf(log_path, MAX_PATH, "%s/logged_hunt", hunt_id);
    
    char dat_path[MAX_PATH];
    snprintf(dat_path, MAX_PATH, "%s/treasures.dat", hunt_id);

    char log_symlink_path[MAX_PATH];
    snprintf(log_symlink_path, MAX_PATH, "logged_hunt-%s", hunt_id);

    if (unlink(dat_path) == -1) {
        perror("unlink");
        return;
    }
    if (unlink(log_path) == -1) {
        perror("unlink");
        return;
    }
    if (unlink(log_symlink_path) == -1) {
        perror("unlink");
        return;
    }

    //dir must be empty so I need to remove the files inside first
    if (rmdir(hunt_dir_path) == -1) {
        perror("rmdir");
        return;
    }
}