#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <string.h>

#define PORT 8080

int main() {
    struct sockaddr_in address;
    int sock = 0, valread;
    char buffer[2048] = {0};
    int pilihan;

    if ((sock = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        printf("Socket creation error\n");
        return -1;
    }

    address.sin_family = AF_INET;
    address.sin_port = htons(PORT);
    inet_pton(AF_INET, "127.0.0.1", &address.sin_addr);

    if (connect(sock, (struct sockaddr *)&address, sizeof(address)) < 0) {
        printf("Connection failed\n");
        return -1;
    }

    while (1) {
        printf("\nMenu: \n");
        printf("1. Player Stats\n");
        printf("2. Shop\n");
        printf("3. Inventory\n");
        printf("4. Battle\n");
        printf("5. Exit\n");
        printf("Choose option: ");
        scanf("%d", &pilihan);

        sprintf(buffer, "%d", pilihan);
        send(sock, buffer, strlen(buffer), 0);

        memset(buffer, 0, sizeof(buffer));
        valread = read(sock, buffer, 2048);
        printf("%s\n", buffer);

        if (pilihan == 2) {
            int weapon_id;
            printf("Enter weapon ID to buy (0 to cancel): ");
            scanf("%d", &weapon_id);
            sprintf(buffer, "%d", weapon_id);
            send(sock, buffer, strlen(buffer), 0);

            memset(buffer, 0, sizeof(buffer));
            valread = read(sock, buffer, 2048);
            printf("%s\n", buffer);
        } else if (pilihan == 3) {
            valread = read(sock, buffer, 1024);
            buffer[valread] = '\0';
            printf("%s", buffer);
        
            int idx;
            scanf("%d", &idx);
            sprintf(buffer, "%d", idx);
            send(sock, buffer, strlen(buffer), 0);
        
            valread = read(sock, buffer, 1024);
            buffer[valread] = '\0';
            printf("%s\n", buffer);
        }        

        if (pilihan == 5) break;
    }

    close(sock);
    return 0;
}

