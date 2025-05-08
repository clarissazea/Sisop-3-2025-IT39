#ifndef SHOP_C
#define SHOP_C

#include <string.h>

#define MAX_INVENTORY 10

struct weapon {
    int id;
    char name[255];
    int price;
    int dmg;
    char passive[255];
};

struct Player {
    int gold;
    int dmg;
    char weapon[255];
    char passive[255];
    struct weapon inventory[MAX_INVENTORY];
    int inventory_count;
    int kill_count;
    int sock;
};

extern struct Player players[10];
struct weapon weapons[5];

void init_shop() {
    weapons[0].id = 1; strcpy(weapons[0].name, "Magic Straw"); weapons[0].price = 25; weapons[0].dmg = 15; strcpy(weapons[0].passive, "Damage x20%");
    weapons[1].id = 2; strcpy(weapons[1].name, "Knife"); weapons[1].price = 50; weapons[1].dmg = 10; strcpy(weapons[1].passive, "None");
    weapons[2].id = 3; strcpy(weapons[2].name, "Monk's Staff"); weapons[2].price = 120; weapons[2].dmg = 15; strcpy(weapons[2].passive, "Damage x2");
    weapons[3].id = 4; strcpy(weapons[3].name, "Enlightenment Blade"); weapons[3].price = 200; weapons[3].dmg = 50; strcpy(weapons[3].passive, "None");
    weapons[4].id = 5; strcpy(weapons[4].name, "Asian Mom's Slipper"); weapons[4].price = 300; weapons[4].dmg = 100; strcpy(weapons[4].passive, "Instantly Kill");
}

const char* buy(int player_id, int weapon_id) {
    struct Player* p = &players[player_id];

    if (weapon_id < 0 || weapon_id >= 5) return "Invalid weapon ID.\n";

    struct weapon w = weapons[weapon_id];
    if (p->gold < w.price) return "Not enough gold.\n";

    if (p->inventory_count >= MAX_INVENTORY) return "Inventory full.\n";

    p->inventory[p->inventory_count++] = w;
    p->gold -= w.price;
    return "Weapon bought and added to inventory.\n";
}


const char* equip_weapon(int player_id, int index) {
    struct Player* p = &players[player_id];
    if (index < -1 || index >= p->inventory_count) return "Invalid weapon selection.\n";

    if (index == -1) {
        strcpy(p->weapon, "Fists");
        p->dmg = 5;
        strcpy(p->passive, "");
        return "Equipped Fists.\n";
    }

    struct weapon w = p->inventory[index];
    strcpy(p->weapon, w.name);
    p->dmg = w.dmg;
    strcpy(p->passive, w.passive);
    return "Weapon equipped.\n";
}


#endif
