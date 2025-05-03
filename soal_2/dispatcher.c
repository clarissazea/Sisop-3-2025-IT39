#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <time.h>

#define MAX_ORDERS 100
#define NAME_LEN 64
#define ADDR_LEN 128
#define STATUS_LEN 64

typedef struct {
    char name[NAME_LEN];
    char address[ADDR_LEN];
    char type[16];
    char status[STATUS_LEN];
} Order;

void read_csv_to_shared_memory(Order *orders, int *order_count) {
    FILE *file = fopen("delivery_order.csv", "r");
    if (!file) {
        perror("Failed to open delivery_order.csv");
        exit(1);
    }

    char line[256];
    *order_count = 0;

    while (fgets(line, sizeof(line), file) && *order_count < MAX_ORDERS) {
        char *token = strtok(line, ",");
        if (token) strncpy(orders[*order_count].name, token, NAME_LEN);

        token = strtok(NULL, ",");
        if (token) strncpy(orders[*order_count].address, token, ADDR_LEN);

        token = strtok(NULL, ",\n");
        if (token) strncpy(orders[*order_count].type, token, 16);

        strcpy(orders[*order_count].status, "Pending");

        (*order_count)++;
    }

    fclose(file);
}

int main(int argc, char *argv[]) {
    key_t key = 1234;
    int shmid = shmget(key, sizeof(Order) * MAX_ORDERS + sizeof(int), IPC_CREAT | 0666);
    if (shmid < 0) {
        perror("shmget error");
        exit(1);
    }

    void *shm = shmat(shmid, NULL, 0);
    if (shm == (void *) -1) {
        perror("shmat error");
        exit(1);
    }

    Order *orders = (Order *) shm;
    int *order_count = (int *) ((char *)shm + sizeof(Order) * MAX_ORDERS);

    // If shared memory is empty (order_count == 0), load CSV
    if (*order_count == 0) {
        read_csv_to_shared_memory(orders, order_count);
    }

    if (argc < 2) {
        printf("Usage:\n");
        printf("./dispatcher -deliver [Name]\n");
        printf("./dispatcher -status [Name]\n");
        printf("./dispatcher -list\n");
        return 0;
    }

    if (strcmp(argv[1], "-list") == 0) {
        printf("Order List:\n");
        for (int i = 0; i < *order_count; i++) {
            printf("%s: %s\n", orders[i].name, orders[i].status);
        }
    } else if (strcmp(argv[1], "-status") == 0 && argc == 3) {
        int found = 0;
        for (int i = 0; i < *order_count; i++) {
            if (strcmp(orders[i].name, argv[2]) == 0) {
                printf("Status for %s: %s\n", orders[i].name, orders[i].status);
                found = 1;
                break;
            }
        }
        if (!found) {
            printf("Order not found for %s.\n", argv[2]);
        }
    } else if (strcmp(argv[1], "-deliver") == 0 && argc == 3) {
        int found = 0;
        for (int i = 0; i < *order_count; i++) {
            if (strcmp(orders[i].name, argv[2]) == 0) {
                if (strcmp(orders[i].type, "Reguler") == 0 && strcmp(orders[i].status, "Pending") == 0) {
                    // Set status with agent name clarissazea
                    snprintf(orders[i].status, STATUS_LEN, "Delivered by Agent clarissazea");

                    // Write to log
                    FILE *logfile = fopen("delivery.log", "a");
                    if (logfile) {
                        time_t now = time(NULL);
                        struct tm *t = localtime(&now);
                        fprintf(logfile,
                            "[%02d/%02d/%04d %02d:%02d:%02d] [AGENT clarissazea] Reguler package delivered to %s in %s\n",
                            t->tm_mday, t->tm_mon+1, t->tm_year+1900,
                            t->tm_hour, t->tm_min, t->tm_sec,
                            orders[i].name, orders[i].address);
                        fclose(logfile);
                    } else {
                        perror("Failed to open delivery.log");
                    }

                    printf("Reguler package delivered to %s.\n", orders[i].name);
                } else if (strcmp(orders[i].type, "Express") == 0) {
                    printf("Cannot manually deliver Express package.\n");
                } else {
                    printf("Order already delivered or invalid.\n");
                }
                found = 1;
                break;
            }
        }
        if (!found) {
            printf("Order not found for %s.\n", argv[2]);
        }
    } else {
        printf("Invalid command.\n");
    }

    shmdt(shm);
    return 0;
}
