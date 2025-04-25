#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <signal.h>
#include <fcntl.h>
#include <unistd.h>

#define CMD_FILE "monitor_command.txt"
#define MAX_CMD_LEN 256

int got_command = 0;


void monitor_loop() {
    char command[MAX_CMD_LEN];

    while (1) {
        // wait for command
        if (got_command) {
            got_command = 0;

            int fd = open(CMD_FILE, O_RDONLY, 0644);
            if (fd == -1) {
                perror("open");
                exit(EXIT_FAILURE);
            }

            // for clearing old commands
            memset(command, 0, MAX_CMD_LEN);

            if (read(fd, command, MAX_CMD_LEN) < 0) {
                perror("read");
                exit(EXIT_FAILURE);
            }

            close(fd);

            if (strcmp(command, "stop_monitor") == 0) {
                printf("Monitor: Stopping with delay\n");
                usleep(5000000); // simulate delay: 5 seconds
                printf("Monitor: Stopped\n");
                exit(EXIT_SUCCESS);
            } else if (strcmp(command, "list_hunts") == 0) {
                printf("Monitor: Listing hunts (placeholder).\n");
                // TODO: Implement actual hunt listing using treasure_manager code
            } else if (strncmp(command, "list_treasures", 14) == 0) {
                printf("Monitor: Listing treasures in hunt (placeholder).\n");
            } else if (strncmp(command, "view_treasure", 13) == 0) {
                printf("Monitor: Viewing treasure (placeholder).\n");
            } else {
                printf("Monitor: Unknown command: %s\n", command);
            }
        }
        pause();
    }
}


void handle_sigusr1(int sig) {
    got_command = 1;
}

int main() {
    /*  When treasure_hub gives a command to the monitor,
        SIGUSR1 gets activated and monitor reads the command
        from the monitor_command.txt file, then processes
        the command written in that file  */
    struct sigaction sa;
    sa.sa_handler = handle_sigusr1;
    sigemptyset(&sa.sa_mask);
    sa.sa_flags = 0;
    if (sigaction(SIGUSR1, &sa, NULL) == -1) {
        perror("sigaction");
        exit(EXIT_FAILURE);
    }

    printf("Monitor running (PID: %d). Waiting for commands...\n", getpid());
    monitor_loop();

    return 0;
}