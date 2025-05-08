#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <time.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>

#define PORT 8080
#define BUFFER_SIZE 4096
#define LOG_FILE "server/server_log"
#define DATABASE_DIR "server/database/"

// Function to write log
void write_log(const char *source, const char *action, const char *info) {
    FILE *log = fopen(LOG_FILE, "a");
    if (!log) return;
    time_t now = time(NULL);
    struct tm *t = localtime(&now);
    fprintf(log, "[%s][%04d-%02d-%02d %02d:%02d:%02d]: [%s] [%s]\n", 
            source, t->tm_year + 1900, t->tm_mon + 1, t->tm_mday, 
            t->tm_hour, t->tm_min, t->tm_sec, action, info);
    fclose(log);
}

// Reverse and decode hex to raw binary
typedef unsigned char byte;
void reverse_and_decode(const char *input_file, char *output_file) {
    FILE *fin = fopen(input_file, "r");
    if (!fin) return;

    // Prepare output filename in server database
    sprintf(output_file, "%s%ld.jpeg", DATABASE_DIR, time(NULL));
    FILE *fout = fopen(output_file, "wb");
    if (!fout) {
        fclose(fin);
        return;
    }

    // Read, reverse and decode
    fseek(fin, 0, SEEK_END);
    long file_size = ftell(fin);
    fseek(fin, 0, SEEK_SET);
    byte *buffer = (byte*)malloc(file_size);
    fread(buffer, 1, file_size, fin);
    fclose(fin);

    // Reverse and decode hex
    for (long i = file_size - 1; i >= 0; i -= 2) {
        if (i == 0) break;  // Skip incomplete hex pairs
        char hex[3] = {buffer[i-1], buffer[i], '\0'};
        byte decoded = (byte)strtol(hex, NULL, 16);
        fwrite(&decoded, 1, 1, fout);
    }

    free(buffer);
    fclose(fout);
}

int main() {
    // Daemonize the process
    if (fork() != 0) return 0;
    setsid();
    chdir("server");
    umask(0);

    // Setup server socket
    int server_fd, new_socket;
    struct sockaddr_in address;
    int addrlen = sizeof(address);

    server_fd = socket(AF_INET, SOCK_STREAM, 0);
    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(PORT);
    bind(server_fd, (struct sockaddr *)&address, sizeof(address));
    listen(server_fd, 3);

    // Server main loop
    while (1) {
        new_socket = accept(server_fd, (struct sockaddr *)&address, (socklen_t*)&addrlen);
        if (new_socket < 0) continue;

        char buffer[BUFFER_SIZE] = {0};
        read(new_socket, buffer, BUFFER_SIZE);

        // Decrypt command
        if (strncmp(buffer, "DECRYPT ", 8) == 0) {
            char input_file[256], output_file[256];
            sscanf(buffer + 8, "%255s", input_file);
            reverse_and_decode(input_file, output_file);
            char *filename = output_file + strlen(DATABASE_DIR);
            write_log("Server", "SAVE", filename);
            send(new_socket, filename, strlen(filename), 0);
        }

        // Close connection
        close(new_socket);
    }

    return 0;
}
