#ifndef MONITOR_FUNCTIONS
#define MONITOR_FUNCTIONS

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

void list_hunts();
void list_treasures(char *hunt_id);
void view_treasure(char *hunt_id, int treasure_id);

#endif