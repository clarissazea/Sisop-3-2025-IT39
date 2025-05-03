#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <unistd.h>
#include <time.h>

#define MAX_HUNTERS 100
#define MAX_DUNGEONS 100
#define HUNTER_SHM "/hunter_shm"
#define DUNGEON_SHM "/dungeon_shm"
#define NAME_LEN 50
#define DUNGEON_NAME_LEN 100

typedef struct {
    char username[NAME_LEN];
    int level;
    int exp;
    int atk;
    int hp;
    int def;
    int banned;
    int notify;
    int used;
} Hunter;

typedef struct {
    char name[DUNGEON_NAME_LEN];
    int min_level;
    int exp_reward;
    int atk_reward;
    int hp_reward;
    int def_reward;
    int used;
} Dungeon;

Hunter* hunters;
Dungeon* dungeons;

void cleanup_shared_memory() {
    munmap(hunters, sizeof(Hunter) * MAX_HUNTERS);
    munmap(dungeons, sizeof(Dungeon) * MAX_DUNGEONS);
    shm_unlink(HUNTER_SHM);
    shm_unlink(DUNGEON_SHM);
}

void init_shared_memory() {
    int h_fd = shm_open(HUNTER_SHM, O_CREAT | O_RDWR, 0666);
    if (h_fd == -1) {
        perror("shm_open hunters failed");
        exit(1);
    }
    ftruncate(h_fd, sizeof(Hunter) * MAX_HUNTERS);
    hunters = mmap(NULL, sizeof(Hunter) * MAX_HUNTERS, PROT_READ | PROT_WRITE, MAP_SHARED, h_fd, 0);
    if (hunters == MAP_FAILED) {
        perror("mmap hunters failed");
        exit(1);
    }

    int d_fd = shm_open(DUNGEON_SHM, O_CREAT | O_RDWR, 0666);
    if (d_fd == -1) {
        perror("shm_open dungeons failed");
        exit(1);
    }
    ftruncate(d_fd, sizeof(Dungeon) * MAX_DUNGEONS);
    dungeons = mmap(NULL, sizeof(Dungeon) * MAX_DUNGEONS, PROT_READ | PROT_WRITE, MAP_SHARED, d_fd, 0);
    if (dungeons == MAP_FAILED) {
        perror("mmap dungeons failed");
        exit(1);
    }

    // Initialize all hunters and dungeons as unused
    for (int i = 0; i < MAX_HUNTERS; i++) {
        hunters[i].used = 0;
    }
    for (int i = 0; i < MAX_DUNGEONS; i++) {
        dungeons[i].used = 0;
    }
}

void show_all_hunters() {
    printf("\n=== Hunter Information ===\n");
    printf("%-20s %-6s %-6s %-6s %-6s %-6s %-8s\n", 
           "Name", "Level", "EXP", "ATK", "HP", "DEF", "Status");
    for (int i = 0; i < MAX_HUNTERS; i++) {
        if (hunters[i].used) {
            printf("%-20s %-6d %-6d %-6d %-6d %-6d %-8s\n",
                   hunters[i].username, 
                   hunters[i].level, 
                   hunters[i].exp,
                   hunters[i].atk,
                   hunters[i].hp,
                   hunters[i].def,
                   hunters[i].banned ? "Banned" : "Active");
        }
    }
}

void show_all_dungeons() {
    printf("\n=== Dungeon Information ===\n");
    printf("%-30s %-10s %-6s %-6s %-6s %-6s\n", 
           "Name", "Min Level", "EXP", "ATK", "HP", "DEF");
    for (int i = 0; i < MAX_DUNGEONS; i++) {
        if (dungeons[i].used) {
            printf("%-30s %-10d %-6d %-6d %-6d %-6d\n",
                   dungeons[i].name,
                   dungeons[i].min_level,
                   dungeons[i].exp_reward,
                   dungeons[i].atk_reward,
                   dungeons[i].hp_reward,
                   dungeons[i].def_reward);
        }
    }
}

void reset_stats(const char* username) {
    for (int i = 0; i < MAX_HUNTERS; i++) {
        if (hunters[i].used && strcmp(hunters[i].username, username) == 0) {
            hunters[i].level = 1;
            hunters[i].exp = 0;
            hunters[i].atk = 10;
            hunters[i].hp = 100;
            hunters[i].def = 5;
            printf("Hunter %s stats reset to default.\n", username);
            return;
        }
    }
    printf("Hunter %s not found.\n", username);
}

void toggle_ban(const char* username, int ban) {
    for (int i = 0; i < MAX_HUNTERS; i++) {
        if (hunters[i].used && strcmp(hunters[i].username, username) == 0) {
            hunters[i].banned = ban;
            printf("Hunter %s has been %s.\n", username, ban ? "banned" : "unbanned");
            return;
        }
    }
    printf("Hunter %s not found.\n", username);
}

const char* dungeon_names[] = {
    "Double Dungeon",
    "Demon Castle",
    "Pyramid Dungeon",
    "Red Gate Dungeon",
    "Hunters Guild Dungeon",
    "Busan A-Rank Dungeon",
    "Insects Dungeon",
    "Goblins Dungeon",
    "D-Rank Dungeon",
    "Gwanak Mountain Dungeon",
    "Hapjeong Subway Station Dungeon"
};

void generate_dungeon() {
    for (int i = 0; i < MAX_DUNGEONS; i++) {
        if (!dungeons[i].used) {
            // 60% chance for level 1 dungeon, 40% for higher levels
            int min_level = (rand() % 10 < 6) ? 1 : ((rand() % 4) + 2);
            
            strcpy(dungeons[i].name, dungeon_names[rand() % (sizeof(dungeon_names)/sizeof(char*))]);
            dungeons[i].min_level = min_level;
            dungeons[i].atk_reward = (rand() % 51) + 100;  // 100-150
            dungeons[i].hp_reward = (rand() % 51) + 50;     // 50-100
            dungeons[i].def_reward = (rand() % 26) + 25;    // 25-50
            dungeons[i].exp_reward = (rand() % 151) + 150;  // 150-300
            dungeons[i].used = 1;
            
            printf("Generated dungeon: %s (Level %d+)\n", dungeons[i].name, dungeons[i].min_level);
            return;
        }
    }
    printf("Cannot generate more dungeons - maximum limit reached.\n");
}

void ban_menu() {
    char username[NAME_LEN];
    int choice;
    
    printf("Enter hunter name: ");
    scanf("%49s", username);
    getchar();
    
    printf("\nBan Options for %s:\n", username);
    printf("1. Ban hunter\n");
    printf("2. Unban hunter\n");
    printf("Choice: ");
    scanf("%d", &choice);
    getchar();
    
    if (choice == 1) {
        toggle_ban(username, 1);
    } else if (choice == 2) {
        toggle_ban(username, 0);
    } else {
        printf("Invalid choice.\n");
    }
}

int main() {
    srand(time(NULL));
    init_shared_memory();
    atexit(cleanup_shared_memory);

    int choice;
    
    printf("=== SYSTEM ADMINISTRATOR ===\n");
    printf("Initializing shared memory...\n");
    printf("System is now ready.\n\n");

    do {
        printf("\n=== SYSTEM MENU ===\n");
        printf("1. Hunter Info\n");
        printf("2. Dungeon Info\n");
        printf("3. Generate Dungeon\n");
        printf("4. Ban Hunter\n");
        printf("5. Reset Hunter\n");
        printf("6. Exit\n");
        printf("Choice: ");
        scanf("%d", &choice);
        getchar();

        switch (choice) {
            case 1:
                show_all_hunters();
                break;
            case 2:
                show_all_dungeons();
                break;
            case 3:
                generate_dungeon();
                break;
            case 4:
                ban_menu();
                break;
            case 5:
                {
                    char username[NAME_LEN];
                    printf("Enter hunter name to reset: ");
                    scanf("%49s", username);
                    getchar();
                    reset_stats(username);
                }
                break;
            case 6:
                printf("Shutting down system...\n");
                break;
            default:
                printf("Invalid choice. Please try again.\n");
        }
    } while (choice != 6);

    return 0;
}
