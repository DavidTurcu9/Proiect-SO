#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <fcntl.h>
#include <dirent.h>
#include <sys/stat.h>

#define CMD_FILE "monitor_command.txt"
#define MAX_PATH 512
#define MAX_CMD_LEN 256
#define MAX_OUTPUT_LEN 2048
#define MAX_HUNT_NAME_LEN 128
#define TREASURE_FILE_NAME "treasures.dat"

int monitor_running = 0;
int stop_monitor_called_and_not_finished = 0;
pid_t monitor_pid = -1;
int monitor_pipe_fd[2];
int calculate_score_running = 0;

void handle_sigchld(int sig) {
    // if calculate_score ends SIGCHLD gets sent and if this if statement wasn't here
    // that signal would mess up the monitor
    if (calculate_score_running == 0) {
        int status;
        waitpid(monitor_pid, &status, 0); // after this function call the monitor process ends
        monitor_running = 0;
        stop_monitor_called_and_not_finished = 0;
        if (WIFEXITED(status)) {
            int exit_code = WEXITSTATUS(status);
            printf("Monitor process ended. Status: %d\n", exit_code);
        }
    }
}


void start_monitor() {
    if (monitor_running) {
        printf("Monitor already running\n");
        return;
    }

    if (pipe(monitor_pipe_fd) == -1) {
        perror("pipe");
        exit(EXIT_FAILURE);
    }

    monitor_pid = fork();
    if (monitor_pid == -1) { // fork error handling
        perror("fork");
        exit(EXIT_FAILURE);
    }

    if (monitor_pid == 0) { // child process
        // Child: replace stdout with write-end of pipe
        close(monitor_pipe_fd[0]); // close read end
        dup2(monitor_pipe_fd[1], STDOUT_FILENO); // send all printf to pipe
        if (execl("./monitor", "monitor", NULL) < 0) { // must have compiled monitor.c as monitor to work
            perror("execl");
            exit(EXIT_FAILURE);
        }
        close(monitor_pipe_fd[1]);
    }

    if (monitor_pid > 0) {
        close(monitor_pipe_fd[1]);
        monitor_running = 1;
        printf("Monitor started. PID: %d\n", monitor_pid);
    }
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

    // read monitor output
    char buffer[MAX_OUTPUT_LEN];
    ssize_t nbytes = read(monitor_pipe_fd[0], buffer, sizeof(buffer) - 1);
    if (nbytes >= 0) {
        buffer[nbytes] = '\0';
        printf("%s", buffer); // Show monitor's response
    }
}


void stop_monitor() {
    if (!monitor_running) {
        printf("Monitor not running.\n");
        return;
    }

    stop_monitor_called_and_not_finished = 1;
    send_command("stop_monitor");
    close(monitor_pipe_fd[0]);
}


void calculate_score() {
    DIR* dir = opendir("."); // opens directory in which the hunts are located
    if (dir == NULL) {
        perror("opendir");
        return;
    }

    struct dirent* entry;
    struct stat st;
    int treasures_found = 0;

    // goes through all hunt folders and creates calculate_score process for each
    while ((entry = readdir(dir)) != NULL) {
        char hunt_dir_name[MAX_HUNT_NAME_LEN];
        strncpy(hunt_dir_name, entry->d_name, MAX_HUNT_NAME_LEN);

        if (entry->d_type == DT_DIR &&
            strcmp(hunt_dir_name, ".") != 0 &&
            strcmp(hunt_dir_name, "..") != 0) {

            char treasure_dat_path[MAX_PATH];
            snprintf(treasure_dat_path, MAX_PATH, "%s/%s", hunt_dir_name, TREASURE_FILE_NAME);
                
            if (stat(treasure_dat_path, &st) == 0) { // daca exista fisierul treasures.dat intra in if
                printf("%s\n", hunt_dir_name);
                treasures_found = 1;
                int calc_pipe_fd[2];
                if (pipe(calc_pipe_fd) == -1) {
                    perror("pipe");
                    exit(EXIT_FAILURE);
                }

                pid_t calc_pid = fork();
                if (calc_pid < 0) {
                    perror("fork");
                    exit(EXIT_FAILURE);
                }
                
                calculate_score_running = 1;

                if (calc_pid == 0) { // child process
                    close(calc_pipe_fd[0]); // close read
                    dup2(calc_pipe_fd[1], STDOUT_FILENO);
                    if (execl("./calculate_score_exec", "calculate_score", hunt_dir_name, NULL) < 0) {
                        perror("execl");
                        exit(EXIT_FAILURE);
                    }
                    
                    close(calc_pipe_fd[1]); // close write
                }

                if (calc_pid > 0) { // parent(treasure hub)
                    close(calc_pipe_fd[1]); // close write

                    // se citeste raspuns de la child process de genul
                    // user "username" has a score of "calculated_score"
                    // si se afiseaza
                    char buffer[MAX_OUTPUT_LEN];
                    ssize_t nbytes = read(calc_pipe_fd[0], buffer, sizeof(buffer) - 1);
                    if (nbytes > 0) {
                        buffer[nbytes] = '\0';
                        printf("%s", buffer); // Show monitor's response
                    }

                    // end calculate_score process
                    int calc_status;
                    waitpid(calc_pid, &calc_status, 0); // after this function call the calculate_score process ends
                    /*if (WIFEXITED(calc_status)) {
                        int calc_exit_code = WEXITSTATUS(calc_status);
                        //printf("calculate_score process ended. Status: %d\n", calc_exit_code);
                    }*/
                    calculate_score_running = 0;
                    close(calc_pipe_fd[0]); // close read
                }
            }
        }
    }
    if (treasures_found == 0) {
        printf("No treasures found\n");
    }
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
           "calculate_score\n"
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
            continue;
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
        } else if (strncmp(input, "calculate_score", 15) == 0) {
            calculate_score();
        } else {
            printf("Unknown command.\n");
        }

    }

    return 0;
}


// TODO test calcualte_score if no treasures exist