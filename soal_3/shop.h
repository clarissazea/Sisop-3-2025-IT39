#ifndef SHOP_H
#define SHOP_H

struct weapon {
    int id;
    char name[255];
    int price;
    int dmg;
    char passive[255];
};

void init_shop();
const char* buy(int player_id, int weapon_id);

#endif
