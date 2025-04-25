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
int stop_monitor_called_and_not_finished = 0;
pid_t monitor_pid = -1;


void handle_sigchld(int sig) {
    int status;
    waitpid(monitor_pid, &status, 0); // after this function call the monitor process ends
    monitor_running = 0;
    stop_monitor_called_and_not_finished = 0;
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
        if (execl("./monitor", "monitor", NULL) < 0) { // must have compiled monitor.c as monitor to work
            perror("execl");
            exit(EXIT_FAILURE);
        }
    }

    monitor_running = 1;
    printf("Monitor started. PID: %d\n", monitor_pid);
}


void send_command(char *command) {
    if (!monitor_running) {
        printf("Monitor not running.\n");
        return;
    }

    // this file is used to give monitor the command
    int fd = open(CMD_FILE, O_WRONLY | O_CREAT, 0644);
    if (fd == -1) {
        perror("open");
        exit(EXIT_FAILURE);
    }

    // truncate needed if you give a command followed by a shorter command
    truncate(CMD_FILE, 0);

    // write command to file, in order to let monitor know what to do next
    if (write(fd, command, strlen(command)) != strlen(command)) {
        perror("write");
        close(fd);
        return;
    }

    close(fd);


    kill(monitor_pid, SIGUSR1); // Notify monitor to process command
}


void stop_monitor() {
    if (!monitor_running) {
        printf("Monitor not running.\n");
        return;
    }

    stop_monitor_called_and_not_finished = 1;
    send_command("stop_monitor");
}


int main() {
    // call handle_sigchld once monitor process ended
    struct sigaction sa;
    sa.sa_handler = handle_sigchld;
    sigemptyset(&sa.sa_mask);
    sa.sa_flags = 0;
    if (sigaction(SIGCHLD, &sa, NULL) < 0) {
        perror("sigaction");
        exit(EXIT_FAILURE);
    }

    char input[MAX_CMD_LEN];

    printf("Commands:\n"
           "start_monitor\n"
           "exit\n\n"

           "Monitor Commands:\n"
           "stop_monitor\n"
           "list_hunts\n"
           "list_treasures <hunt id>\n"
           "view_treasure <hunt id> <treasure id>\n\n");

    printf("Input commands:\n");
    fflush(stdout);

    // stop_monitor, list_hunts, list_treasures, view_treasure 
    //commands will be sent to monitor process

    while(1) {
        // read command
        fgets(input, sizeof(input), stdin);
        input[strcspn(input, "\n")] = 0; // remove newline
        
        if (stop_monitor_called_and_not_finished) {
            fprintf(stderr, "Error: monitor still closing\n");
        }

        // handle each possible command
        if (strcmp(input, "start_monitor") == 0) {
            start_monitor();
        } else if (strcmp(input, "stop_monitor") == 0) {
            stop_monitor();
        } else if (strcmp(input, "exit") == 0) {
            if (monitor_running) {
                printf("Monitor still running! Use stop_monitor first.\n");
            } else {
                return 0;
            }
        } else if (strncmp(input, "list_hunts", 10) == 0 ||
                   strncmp(input, "list_treasures", 14) == 0 ||
                   strncmp(input, "view_treasure", 13) == 0) {
            send_command(input);
        } else {
            printf("Unknown command.\n");
        }
    }

    return 0;
}

// TODO program only stops when exit is inputted (doesn't work if fgets has error handling)

// TODO chestia cu stop_monitor usleep error din cerinta 
// (works but monitor closes right after inputting the command while monitor is closing) idk why