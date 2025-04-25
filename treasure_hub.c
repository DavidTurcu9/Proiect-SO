#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <fcntl.h>

#define CMD_FILE "monitor_command.txt"
#define MAX_CMD_LEN 256

int monitor_running = 0;
pid_t monitor_pid = -1;


void handle_sigchld(int sig) {
    int status;
    waitpid(monitor_pid, &status, 0); // after this function call the monitor process ends
    monitor_running = 0;
    printf("Monitor process ended. Status: %d\n", status);
}


void start_monitor() {
    if (monitor_running) {
        printf("Monitor already running\n");
        return;
    }

    monitor_pid = fork();
    if (monitor_pid == -1) { // fork error handling
        perror("fork");
        exit(EXIT_FAILURE);
    }

    if (monitor_pid == 0) { // child process
        execl("./monitor", "monitor", NULL); // must have compiled monitor.c as monitor to work
        perror("execl");
        exit(EXIT_FAILURE);
    }

    monitor_running = 1;
    printf("Monitor started\n");
}


void send_command(char *command) {
    if (!monitor_running) {
        printf("Monitor not running.\n");
        return;
    }

    // this file is used to give monitor the command
    int fd = open(CMD_FILE, O_RDWR | O_CREAT, 0644);
    if (fd == -1) {
        perror("open");
        exit(EXIT_FAILURE);
    }

    // write command to file, in order to let monitor know what to do next
    if (write(fd, command, strlen(command)) != strlen(command)) {
        perror("write");
        close(fd);
        exit(EXIT_FAILURE);
    }

    close(fd);


    kill(monitor_pid, SIGUSR1); // Notify monitor to process command
}


void stop_monitor() {
    if (!monitor_running) {
        printf("Monitor not running.\n");
        return;
    }

    send_command("stop_monitor");
    // Let SIGCHLD handle the rest
}


int main() {
    // call handle_sigchld once monitor process ended
    struct sigaction sa;
    sa.sa_handler = handle_sigchld;
    sigemptyset(&sa.sa_mask);
    sa.sa_flags = 0;
    sigaction(SIGCHLD, &sa, NULL);

    char input[MAX_CMD_LEN];

    printf("Commands:\n"
           "start_monitor\n"
           "list_hunts\n"
           "list_treasures\n"
           "view_treasure\n"
           "stop_monitor\n"
           "exit\n\n");
    
    // stop_monitor, list_hunts, list_treasures, view_treasure 
    //commands will be sent to monitor process
    while(1) {
        // read command
        printf("Input command: ");
        fflush(stdout);
        if (!fgets(input, sizeof(input), stdin)) {
            break;
        }
        input[strcspn(input, "\n")] = 0; // remove newline

        // handle each possible command
        if (strcmp(input, "start_monitor") == 0) {
            start_monitor();
        } else if (strcmp(input, "stop_monitor") == 0) {
            stop_monitor();
        } else if (strcmp(input, "exit") == 0) {
            if (monitor_running) {
                printf("Monitor still running! Use stop_monitor first.\n");
            } else {
                break; // TODO exit command here
            }
        } else if (strncmp(input, "list_hunts", 10) == 0 ||
                   strncmp(input, "list_treasures", 14) == 0 ||
                   strncmp(input, "view_treasure", 13) == 0) {
            //send_command(input);
        } else {
            printf("Unknown command.\n");
        }
    }

    return 0;
}
