#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>

#define PORT 8080
#define BUFFER_SIZE 4096

void send_decrypt_command(int sock) {
    char filename[256];
    printf("Enter the file name: ");
    scanf("%255s", filename);

    // Prepare command
    char command[BUFFER_SIZE];
    snprintf(command, sizeof(command), "DECRYPT %s", filename);

    // Send command to server
    send(sock, command, strlen(command), 0);

    // Receive response from server
    char response[BUFFER_SIZE] = {0};
    read(sock, response, sizeof(response));
    printf("Server: Text decrypted and saved as %s\n", response);
}

void send_download_command(int sock) {
    char filename[256];
    printf("Enter the JPEG filename to download: ");
    scanf("%255s", filename);

    // Prepare command
    char command[BUFFER_SIZE];
    snprintf(command, sizeof(command), "DOWNLOAD %s", filename);

    // Send command to server
    send(sock, command, strlen(command), 0);

    // Receive file data from server
    FILE *file = fopen(filename, "wb");
    if (!file) {
        printf("Failed to create file %s\n", filename);
        return;
    }

    int bytes_received;
    char buffer[BUFFER_SIZE];
    while ((bytes_received = read(sock, buffer, sizeof(buffer))) > 0) {
        fwrite(buffer, 1, bytes_received, file);
        if (bytes_received < BUFFER_SIZE) break;
    }

    fclose(file);
    printf("File %s downloaded successfully.\n", filename);
}

int main() {
    int sock = socket(AF_INET, SOCK_STREAM, 0);
    struct sockaddr_in server_addr;

    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(PORT);
    server_addr.sin_addr.s_addr = INADDR_ANY;

    if (connect(sock, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0) {
        printf("Failed to connect to server.\n");
        return 1;
    }

    int choice;
    while (1) {
        printf("\n| Image Decoder Client |\n");
        printf("1. Send input file to server\n");
        printf("2. Download file from server\n");
        printf("3. Exit\n");
        printf(">> ");
        scanf("%d", &choice);

        switch (choice) {
            case 1:
                send_decrypt_command(sock);
                break;
            case 2:
                send_download_command(sock);
                break;
            case 3:
                close(sock);
                printf("Client exiting...\n");
                return 0;
            default:
                printf("Invalid choice, please try again.\n");
        }
    }

    return 0;
}
