#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <pthread.h>
#include "shop.c"

#define PORT 8080
#define MAX_CLIENTS 10

struct Player players[MAX_CLIENTS];

void* handle_client(void* arg);
void battle(int player_id, char* output);
const char* equip_weapon(int player_id, int index);

struct ThreadArgs {
    int sock;
    int player_id;
};

void* handle_client(void* arg) {
    struct ThreadArgs* args = (struct ThreadArgs*)arg;
    int sock = args->sock;
    int id = args->player_id;
    free(arg);
    players[id].sock = sock;
    char buffer[1024] = {0};
    int valread;

    while (1) {
        memset(buffer, 0, sizeof(buffer));
        valread = read(sock, buffer, 1024);
        if (valread <= 0) break;

        int pilihan = atoi(buffer);
        if (pilihan == 5) {
            send(sock, "Goodbye!\n", strlen("Goodbye!\n"), 0);
            break;
        }

        if (pilihan == 1) {
            char stats[1024];
            sprintf(stats, "Gold: %d\nWeapon: %s\nDamage: %d (%s)\nKills: %d\n",
                players[id].gold, players[id].weapon, players[id].dmg,
                strlen(players[id].passive) ? players[id].passive : "No passive",
                players[id].kill_count);
            send(sock, stats, strlen(stats), 0);
        } else if (pilihan == 2) {
            char shopinfo[1024] = "==== SHOP ====\n";
            for (int i = 0; i < 5; i++) {
                char line[256];
                sprintf(line, "%d. %s (Price: %d, DMG: %d, Passive: %s)\n",
                        weapons[i].id, weapons[i].name, weapons[i].price,
                        weapons[i].dmg, weapons[i].passive[0] ? weapons[i].passive : "None");
                strcat(shopinfo, line);
            }
            send(sock, shopinfo, strlen(shopinfo), 0);

            memset(buffer, 0, sizeof(buffer));
            valread = read(sock, buffer, 1024);
            int weapon_pilihan = atoi(buffer);

            const char* result = buy(id, weapon_pilihan);
            send(sock, result, strlen(result), 0);
        } else if (pilihan == 3) {
            struct Player* p = &players[id];
            char inv[1024];
            strcpy(inv, "Inventory:\n");
            strcat(inv, "0. Fists (DMG: 5, Passive: None)\n");

            for (int i = 0; i < p->inventory_count; i++) {
                char line[256];
                sprintf(line, "%d. %s (DMG: %d, Passive: %s)\n", i + 1,
                        p->inventory[i].name, p->inventory[i].dmg,
                        strlen(p->inventory[i].passive) ? p->inventory[i].passive : "None");
                strcat(inv, line);
            }

            strcat(inv, "Choose item to equip (number): ");
            send(sock, inv, strlen(inv), 0);

            int index;
            read(sock, buffer, 1024);
            sscanf(buffer, "%d", &index);
            const char* msg = equip_weapon(id, index - 1);
            send(sock, msg, strlen(msg), 0);
        } else if (pilihan == 4) {
            struct Player* p = &players[id];
            char result[2048];
            battle(0, result);
            send(sock, result, strlen(result), 0);
        } else {
            send(sock, "Feature not implemented.\n", 26, 0);
        }
    }

    close(sock);
    pthread_exit(NULL);
}

void battle(int player_id, char* output) {
    struct Player* p = &players[player_id];
    int enemy_hp = (p->kill_count == 0) ? (rand() % 151 + 50) : (rand() % 901 + 100);  // HP acak
    int enemy_max_hp = enemy_hp;
    int hit = p->dmg;

    if (strcmp(p->passive, "Damage x2") == 0) hit *= 2;
    else if (strcmp(p->passive, "Damage x20%%") == 0) hit *= 0.2;
    else if (strcmp(p->passive, "Instantly Kill") == 0) {
        sprintf(output, "You used %s. Enemy instantly defeated!\n", p->weapon);
        p->kill_count++;
        int reward_gold = enemy_hp / 4; 
        p->gold += reward_gold; 
        sprintf(output + strlen(output), "You earned %d gold!\n", reward_gold);
        return;
    }

    char battle_status[2048];
    while (enemy_hp > 0) {
        memset(battle_status, 0, sizeof(battle_status));

        int bar_length = 50;
        int health_bar = (int)((float)enemy_hp / enemy_max_hp * bar_length);

        sprintf(battle_status, "\n--- Battle Mode ---\n");
        sprintf(battle_status + strlen(battle_status), "Enemy HP: %d/%d\n", enemy_hp, enemy_max_hp);
        sprintf(battle_status + strlen(battle_status), "Health Bar: [");
        for (int i = 0; i < health_bar; i++) {
            strcat(battle_status, "#");
        }
        for (int i = health_bar; i < bar_length; i++) {
            strcat(battle_status, "-");
        }
        strcat(battle_status, "]\n");

        strcat(battle_status, "\nChoose action: \n1. Attack\n2. Exit\n");
        send(p->sock, battle_status, strlen(battle_status), 0);

        char buffer[1024];
        int valread = read(p->sock, buffer, sizeof(buffer));
        buffer[valread] = '\0';
        
        if (atoi(buffer) == 2) {
            send(p->sock, "Exiting Battle Mode...\n", strlen("Exiting Battle Mode...\n"), 0);
            return;
        }

        enemy_hp -= hit;
        sprintf(battle_status, "You dealt %d damage!\n", hit);
        send(p->sock, battle_status, strlen(battle_status), 0);

        if (enemy_hp <= 0) {
            p->kill_count++;
            int reward_gold = enemy_max_hp / 4; 
            p->gold += reward_gold;

            sprintf(battle_status, "You defeated the enemy!\nYou earned %d gold!\n", reward_gold);
            send(p->sock, battle_status, strlen(battle_status), 0);
            break;
        }
    }
}

int main() {
    init_shop();

    int server_fd, new_socket;
    struct sockaddr_in address;
    int opt = 1;
    int addrlen = sizeof(address);
    pthread_t thread_id[MAX_CLIENTS];
    int client_count = 0;

    server_fd = socket(AF_INET, SOCK_STREAM, 0);
    setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT, &opt, sizeof(opt));

    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(PORT);

    bind(server_fd, (struct sockaddr*)&address, sizeof(address));
    listen(server_fd, MAX_CLIENTS);
    printf("Server listening on port %d...\n", PORT);

    while (1) {
        new_socket = accept(server_fd, (struct sockaddr*)&address, (socklen_t*)&addrlen);
        if (new_socket < 0) continue;

        if (client_count >= MAX_CLIENTS) {
            char* msg = "Server full. Try again later.\n";
            send(new_socket, msg, strlen(msg), 0);
            close(new_socket);
            continue;
        }

        players[client_count].gold = 200;
        players[client_count].dmg = 5;
        strcpy(players[client_count].weapon, "Fists");
        strcpy(players[client_count].passive, "No passive");
        players[client_count].kill_count = 0;
        players[client_count].inventory_count = 0;

        struct ThreadArgs* args = malloc(sizeof(struct ThreadArgs));
        args->sock = new_socket;
        args->player_id = client_count;

        pthread_create(&thread_id[client_count], NULL, handle_client, args);
        client_count++;
    }

    return 0;
}

