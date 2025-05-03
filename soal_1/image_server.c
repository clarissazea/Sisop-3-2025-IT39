#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <time.h>
#include <ctype.h>

#define PORT 8080
#define BUFFER_SIZE 4096

void log_entry(const char* source, const char* action, const char* info) {
    FILE *log = fopen("server/server.log", "a");
    if (!log) return;

    time_t t = time(NULL);
    struct tm tm = *localtime(&t);
    fprintf(log, "[%s][%04d-%02d-%02d %02d:%02d:%02d]: [%s] [%s]\n",
        source,
        tm.tm_year + 1900, tm.tm_mon + 1, tm.tm_mday,
        tm.tm_hour, tm.tm_min, tm.tm_sec,
        action, info);
    fclose(log);
}

// Decode hex string into binary data
int hex_decode(const char *hex, unsigned char *out) {
    int len = strlen(hex);
    if (len % 2 != 0) return -1;
    for (int i = 0; i < len; i += 2) {
        sscanf(hex + i, "%2hhx", &out[i / 2]);
    }
    return len / 2;
}

void handle_client(int client_sock) {
    char buffer[BUFFER_SIZE] = {0};
    read(client_sock, buffer, sizeof(buffer));

    if (strncmp(buffer, "DECRYPT ", 8) == 0) {
        char filename[256];
        sscanf(buffer + 8, "%s", filename);

        char filepath[512];
        snprintf(filepath, sizeof(filepath), "/home/amdfz/soal_1/client/secrets/%s", filename);

        FILE *fp = fopen(filepath, "r");
        if (!fp) {
            char *error_msg = "ERROR: Cannot decrypt file";
            write(client_sock, error_msg, strlen(error_msg));
            log_entry("Server", "ERROR", filename);
            close(client_sock);
            return;
        }

        char hex_data[BUFFER_SIZE];
        fread(hex_data, 1, sizeof(hex_data), fp);
        fclose(fp);

        log_entry("Client", "DECRYPT", "Text data");

        // Reverse string
        int len = strlen(hex_data);
        for (int i = 0; i < len / 2; ++i) {
            char tmp = hex_data[i];
            hex_data[i] = hex_data[len - i - 1];
            hex_data[len - i - 1] = tmp;
        }

        unsigned char image_data[BUFFER_SIZE];
        int img_len = hex_decode(hex_data, image_data);

        if (img_len <= 0) {
            char *error_msg = "ERROR: Decryption failed";
            write(client_sock, error_msg, strlen(error_msg));
            log_entry("Server", "ERROR", "Hex decode failed");
            close(client_sock);
            return;
        }

        time_t timestamp = time(NULL);
        char save_path[512];
        snprintf(save_path, sizeof(save_path), "server/database/%ld.jpeg", timestamp);
        FILE *img = fopen(save_path, "wb");
        fwrite(image_data, 1, img_len, img);
        fclose(img);

        char response[256];
        snprintf(response, sizeof(response), "Server: Text decrypted and saved as %ld.jpeg", timestamp);
        write(client_sock, response, strlen(response));

        char log_info[64];
        snprintf(log_info, sizeof(log_info), "%ld.jpeg", timestamp);
        log_entry("Server", "SAVE", log_info);
    }
    else if (strncmp(buffer, "DOWNLOAD ", 9) == 0) {
        char filename[256];
        sscanf(buffer + 9, "%s", filename);

        log_entry("Client", "DOWNLOAD", filename);

        char filepath[512];
        snprintf(filepath, sizeof(filepath), "server/database/%s", filename);

        FILE *fp = fopen(filepath, "rb");
        if (!fp) {
            char *error_msg = "ERROR: File not found on server";
            write(client_sock, error_msg, strlen(error_msg));
            log_entry("Server", "ERROR", filename);
            close(client_sock);
            return;
        }

        unsigned char buffer[BUFFER_SIZE];
        size_t n;
        while ((n = fread(buffer, 1, sizeof(buffer), fp)) > 0) {
            write(client_sock, buffer, n);
        }
        fclose(fp);
        log_entry("Server", "UPLOAD", filename);
    } else {
        char *msg = "ERROR: Invalid command";
        write(client_sock, msg, strlen(msg));
    }

    close(client_sock);
}

int main() {
    pid_t pid = fork();
    if (pid < 0) exit(EXIT_FAILURE);
    if (pid > 0) exit(EXIT_SUCCESS);

    setsid();
    chdir("/");
    close(STDIN_FILENO);
    close(STDOUT_FILENO);
    close(STDERR_FILENO);

    int server_fd, client_sock;
    struct sockaddr_in server_addr, client_addr;
    socklen_t addr_len = sizeof(client_addr);

    server_fd = socket(AF_INET, SOCK_STREAM, 0);
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = INADDR_ANY;
    server_addr.sin_port = htons(PORT);

    bind(server_fd, (struct sockaddr *)&server_addr, sizeof(server_addr));
    listen(server_fd, 10);

    while (1) {
        client_sock = accept(server_fd, (struct sockaddr *)&client_addr, &addr_len);
        if (client_sock >= 0) {
            if (fork() == 0) {
                handle_client(client_sock);
                exit(0);
            }
        }
    }

    close(server_fd);
    return 0;
}
