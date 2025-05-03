// image_client.c
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <time.h>

#define PORT 8080
#define SERVER_IP "127.0.0.1"
#define BUFFER_SIZE 5242880 // 5MB max buffer

void decrypt_file() {
    char filename[256];
    printf("Enter the file name: ");
    scanf("%s", filename);

    int sock = socket(AF_INET, SOCK_STREAM, 0);
    if (sock < 0) {
        perror("Socket");
        return;
    }

    struct sockaddr_in serv_addr = {
        .sin_family = AF_INET,
        .sin_port = htons(PORT)
    };
    inet_pton(AF_INET, SERVER_IP, &serv_addr.sin_addr);

    if (connect(sock, (struct sockaddr*)&serv_addr, sizeof(serv_addr)) < 0) {
        perror("Connect");
        close(sock);
        return;
    }

    char message[300];
    snprintf(message, sizeof(message), "DECRYPT %s", filename);
    send(sock, message, strlen(message), 0);

    char response[256] = {0};
    read(sock, response, sizeof(response));
    printf("Server: %s\n", response);

    close(sock);
}

void download_file() {
    char filename[256];
    printf("Enter the file name: ");
    scanf("%s", filename);

    int sock = socket(AF_INET, SOCK_STREAM, 0);
    if (sock < 0) {
        perror("Socket");
        return;
    }

    struct sockaddr_in serv_addr = {
        .sin_family = AF_INET,
        .sin_port = htons(PORT)
    };
    inet_pton(AF_INET, SERVER_IP, &serv_addr.sin_addr);

    if (connect(sock, (struct sockaddr*)&serv_addr, sizeof(serv_addr)) < 0) {
        perror("Connect");
        close(sock);
        return;
    }

    char message[300];
    snprintf(message, sizeof(message), "DOWNLOAD %s", filename);
    send(sock, message, strlen(message), 0);

    char *buffer = malloc(BUFFER_SIZE);
    if (!buffer) {
        perror("Malloc");
        close(sock);
        return;
    }

    int bytes = read(sock, buffer, BUFFER_SIZE);
    if (bytes > 0 && strncmp(buffer, "ERROR", 5) != 0) {
        FILE *fp = fopen(filename, "wb");
        if (fp) {
            fwrite(buffer, 1, bytes, fp);
            fclose(fp);
            printf("Success! Image saved as %s\n", filename);
        } else {
            printf("ERROR: Cannot save image to disk\n");
        }
    } else {
        printf("%s\n", buffer);
    }

    free(buffer);
    close(sock);
}

int main() {
    int choice;

    while (1) {
        printf("\n| Image Decoder Client |\n");
        printf("1. Send input file to server\n");
        printf("2. Download file from server\n");
        printf("3. Exit\n>> ");
        scanf("%d", &choice);

        switch (choice) {
            case 1:
                decrypt_file();
                break;
            case 2:
                download_file();
                break;
            case 3:
                printf("Exiting client...\n");
                return 0;
            default:
                printf("Invalid choice. Try again.\n");
        }
    }

    return 0;
}
