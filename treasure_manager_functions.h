#ifndef TREASURE_MANAGER_FUNCTIONS
#define TREASURE_MANAGER_FUNCTIONS

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


void add_treasure(char* hunt_id);
void list_treasures(char* hunt_id);
void view_treasure(char* hunt_id, int treasure_id);
void remove_treasure(const char *hunt_id, int treasure_id);
void remove_hunt(const char *hunt_id);

#endif