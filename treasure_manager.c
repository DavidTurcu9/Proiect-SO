#include <stdio.h>
#include <unistd.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/types.h>

typedef struct GPSCoords_t {
    float x, y;
} GPSCoords_t;

typedef struct treasure_t {
    int ID;
    char userName[50];
    GPSCoords_t coords;
    char clueText[50];
    int value;
} treasure_t;



int main(int argc, char** argv) {

    //de pus verificari la argumente

    struct stat st = {0};
    //char currentDirectory[50] = "/home/student/david";

    if (stat("/home/student/david/treasure_manager", &st) == -1) {
        mkdir("/home/student/david/treasure_manager", 0700);
    }
    return 0;
}