#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <unistd.h>
#include <time.h>
#include <pthread.h>

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
int notification_active = 0;
int current_hunter_index = -1;

void press_enter() {
    printf("\nPress ENTER to continue...");
    while (getchar() != '\n');
}

int get_hunter_index(const char* name) {
    for (int i = 0; i < MAX_HUNTERS; ++i) {
        if (hunters[i].used && strcmp(hunters[i].username, name) == 0) {
            return i;
        }
    }
    return -1;
}

void* notification_thread(void* arg) {
    while (notification_active) {
        system("clear");
        printf("=== HUNTER SYSTEM ===\n");


        int available_count = 0;
        int available_indices[MAX_DUNGEONS];

        for (int j = 0; j < MAX_DUNGEONS; ++j) {
            if (dungeons[j].used && dungeons[j].min_level <= hunters[current_hunter_index].level) {
                available_indices[available_count++] = j;
            }
        }

        if (available_count > 0) {
            int random_idx = available_indices[rand() % available_count];
            printf("\nNew Dungeon Available:\n");
            printf("- %s (Level %d+)\n", dungeons[random_idx].name, dungeons[random_idx].min_level);
        } else {
            printf("\nNo new dungeons available\n");
        }

        // Display menu
        printf("\n1. Dungeon List\n");
        printf("2. Dungeon Raid\n");
        printf("3. Hunters Battle\n");
        printf("4. Toggle Notifications\n");
        printf("5. Exit\n");
        printf("\nChoice: ");
        fflush(stdout);
        
        sleep(3);
    }
    return NULL;
}

void toggle_notifications(int hunter_index) {
    if (notification_active) {
        notification_active = 0;
        printf("Notifications turned OFF\n");
    } else {
        notification_active = 1;
        current_hunter_index = hunter_index;
        printf("Notifications turned ON\n");
        
        pthread_t thread;
        pthread_create(&thread, NULL, notification_thread, NULL);
        pthread_detach(thread);
    }
    press_enter();
}

void show_dungeon_list(int hunter_index) {
    printf("\n=== AVAILABLE DUNGEONS ===\n");
    int count = 1;
    for (int i = 0; i < MAX_DUNGEONS; ++i) {
        if (dungeons[i].used && dungeons[i].min_level <= hunters[hunter_index].level) {
            printf("%d. %s (Level %d+)\n", count++, dungeons[i].name, dungeons[i].min_level);
        }
    }
    if (count == 1) {
        printf("No available dungeons for your level.\n");
    }
    press_enter();
}

void dungeon_raid(int hunter_index) {
    if (hunters[hunter_index].banned) {
        printf("\nYou are banned and cannot raid dungeons!\n");
        press_enter();
        return;
    }

    printf("\n=== DUNGEON RAID ===\n");
    int available[MAX_DUNGEONS];
    int count = 0;
    for (int i = 0; i < MAX_DUNGEONS; ++i) {
        if (dungeons[i].used && dungeons[i].min_level <= hunters[hunter_index].level) {
            printf("%d. %s (Level %d+)\n", count + 1, dungeons[i].name, dungeons[i].min_level);
            available[count++] = i;
        }
    }
    if (count == 0) {
        printf("No available dungeons to raid.\n");
        press_enter();
        return;
    }
    printf("Choose a dungeon to raid: ");
    int choice;
    scanf("%d", &choice);
    getchar();
    if (choice < 1 || choice > count) {
        printf("Invalid choice.\n");
        press_enter();
        return;
    }
    int idx = available[choice - 1];
    hunters[hunter_index].atk += dungeons[idx].atk_reward;
    hunters[hunter_index].hp += dungeons[idx].hp_reward;
    hunters[hunter_index].def += dungeons[idx].def_reward;
    hunters[hunter_index].exp += dungeons[idx].exp_reward;

    if (hunters[hunter_index].exp >= 500) {
        hunters[hunter_index].exp = 0;
        hunters[hunter_index].level++;
        printf("\nLevel up! You are now level %d\n", hunters[hunter_index].level);
    }

    dungeons[idx].used = 0;

    printf("\nRaid success! Gained:\n");
    printf("ATK: %d\n", dungeons[idx].atk_reward);
    printf("HP: %d\n", dungeons[idx].hp_reward);
    printf("DEF: %d\n", dungeons[idx].def_reward);
    printf("EXP: %d\n", dungeons[idx].exp_reward);

    press_enter();
}

void hunter_battle(int hunter_index) {
    if (hunters[hunter_index].banned) {
        printf("\nYou are banned and cannot battle other hunters!\n");
        press_enter();
        return;
    }

    printf("\n=== HUNTERS BATTLE ===\n");
    int count = 1;
    int opponents[MAX_HUNTERS];
    for (int i = 0; i < MAX_HUNTERS; ++i) {
        if (hunters[i].used && i != hunter_index && !hunters[i].banned) {
            printf("%d. %s (Level %d)\n", count, hunters[i].username, hunters[i].level);
            opponents[count - 1] = i;
            count++;
        }
    }
    if (count == 1) {
        printf("No available opponents.\n");
        press_enter();
        return;
    }
    printf("Choose opponent: ");
    int choice;
    scanf("%d", &choice);
    getchar();
    if (choice < 1 || choice >= count) {
        printf("Invalid choice.\n");
        press_enter();
        return;
    }
    int opp = opponents[choice - 1];
    int my_power = hunters[hunter_index].atk + hunters[hunter_index].hp + hunters[hunter_index].def;
    int opp_power = hunters[opp].atk + hunters[opp].hp + hunters[opp].def;

    printf("\nBattle between %s (Power: %d) vs %s (Power: %d)\n", 
           hunters[hunter_index].username, my_power,
           hunters[opp].username, opp_power);

    if (my_power >= opp_power) {
        hunters[hunter_index].atk += hunters[opp].atk;
        hunters[hunter_index].hp += hunters[opp].hp;
        hunters[hunter_index].def += hunters[opp].def;
        hunters[opp].used = 0;
        printf("\nYou won! Gained all opponent stats.\n");
    } else {
        hunters[opp].atk += hunters[hunter_index].atk;
        hunters[opp].hp += hunters[hunter_index].hp;
        hunters[opp].def += hunters[hunter_index].def;
        hunters[hunter_index].used = 0;
        printf("\nYou lost! Opponent gained your stats.\n");
        press_enter();
        exit(0);
    }
    press_enter();
}

void hunter_menu(int index) {
    while (1) {
        if (!notification_active) {
            system("clear");
            printf("=== HUNTER SYSTEM ===\n");
            printf("1. Dungeon List\n");
            printf("2. Dungeon Raid\n");
            printf("3. Hunters Battle\n");
            printf("4. Toggle Notifications\n");
            printf("5. Exit\n");
            printf("Choice: ");
        }

        int choice;
        scanf("%d", &choice);
        getchar();

        switch (choice) {
            case 1: 
                notification_active = 0;
                show_dungeon_list(index); 
                break;
            case 2: 
                notification_active = 0;
                dungeon_raid(index); 
                break;
            case 3: 
                notification_active = 0;
                hunter_battle(index); 
                break;
            case 4: 
                toggle_notifications(index);
                break;
            case 5: 
                notification_active = 0;
                exit(0);
            default: 
                printf("Invalid choice.\n"); 
                press_enter(); 
                break;
        }
    }
}

int main() {
    srand(time(NULL));
    
    // Try to access existing shared memory
    int h_fd = shm_open(HUNTER_SHM, O_RDWR, 0666);
    if (h_fd == -1) {
        printf("Error: System is not running. Please run system.c first!\n");
        exit(1);
    }
    hunters = mmap(NULL, sizeof(Hunter) * MAX_HUNTERS, PROT_READ | PROT_WRITE, MAP_SHARED, h_fd, 0);
    
    int d_fd = shm_open(DUNGEON_SHM, O_RDWR, 0666);
    if (d_fd == -1) {
        printf("Error: System is not running. Please run system.c first!\n");
        exit(1);
    }
    dungeons = mmap(NULL, sizeof(Dungeon) * MAX_DUNGEONS, PROT_READ | PROT_WRITE, MAP_SHARED, d_fd, 0);

    while (1) {
        system("clear");
        printf("=== HUNTER SYSTEM ===\n");
        printf("1. Register\n");
        printf("2. Login\n");
        printf("3. Exit\n");
        printf("Choice: ");

        int choice;
        scanf("%d", &choice);
        getchar();

        if (choice == 1) {
            char name[NAME_LEN];
            printf("Enter new hunter name: ");
            fgets(name, NAME_LEN, stdin);
            name[strcspn(name, "\n")] = 0;
            if (get_hunter_index(name) != -1) {
                printf("Hunter already exists.\n");
                press_enter();
                continue;
            }
            for (int i = 0; i < MAX_HUNTERS; ++i) {
                if (!hunters[i].used) {
                    strcpy(hunters[i].username, name);
                    hunters[i].level = 1;
                    hunters[i].atk = 10;
                    hunters[i].hp = 100;
                    hunters[i].def = 5;
                    hunters[i].exp = 0;
                    hunters[i].used = 1;
                    hunters[i].banned = 0;
                    hunters[i].notify = 0;
                    printf("Hunter registered successfully!\n");
                    press_enter();
                    break;
                }
            }
        } else if (choice == 2) {
            char name[NAME_LEN];
            printf("Enter your hunter name: ");
            fgets(name, NAME_LEN, stdin);
            name[strcspn(name, "\n")] = 0;
            int idx = get_hunter_index(name);
            if (idx == -1) {
                printf("Hunter not found.\n");
                press_enter();
                continue;
            }
            if (hunters[idx].banned) {
                printf("This hunter is banned and cannot login!\n");
                press_enter();
                continue;
            }
            hunter_menu(idx);
        } else if (choice == 3) {
            exit(0);
        } else {
            printf("Invalid choice.\n");
            press_enter();
        }
    }
    return 0;
}
