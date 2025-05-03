#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <time.h>
#include <unistd.h>

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

Order *orders;
int *order_count;

void *agent_thread(void *arg) {
    char *agent_name = (char *)arg;

    while (1) {
        for (int i = 0; i < *order_count; i++) {
            if (strcmp(orders[i].type, "Express") == 0 && strcmp(orders[i].status, "Pending") == 0) {
                snprintf(orders[i].status, STATUS_LEN, "Delivered by Agent %s", agent_name);

                FILE *logfile = fopen("delivery.log", "a");
                if (logfile) {
                    time_t now = time(NULL);
                    struct tm *t = localtime(&now);
                    fprintf(logfile,
                            "[%02d/%02d/%04d %02d:%02d:%02d] [AGENT %s] Express package delivered to %s in %s\n",
                            t->tm_mday, t->tm_mon + 1, t->tm_year + 1900,
                            t->tm_hour, t->tm_min, t->tm_sec,
                            agent_name, orders[i].name, orders[i].address);
                    fclose(logfile);
                } else {
                    perror("Failed to open delivery.log");
                }

                printf("[%s] Delivered Express package to %s\n", agent_name, orders[i].name);

                sleep(1); // simulate delivery delay
            }
        }
        sleep(1);
    }

    return NULL;
}

int main() {
    key_t key = 1234;
    int shmid = shmget(key, sizeof(Order) * MAX_ORDERS + sizeof(int), 0666);
    if (shmid < 0) {
        perror("shmget error");
        exit(1);
    }

    void *shm = shmat(shmid, NULL, 0);
    if (shm == (void *) -1) {
        perror("shmat error");
        exit(1);
    }

    orders = (Order *)shm;
    order_count = (int *)((char *)shm + sizeof(Order) * MAX_ORDERS);

    printf("order_count loaded: %d\n", *order_count); // debug info

    if (*order_count == 0) {
        printf("No orders found in shared memory. Run dispatcher first.\n");
        shmdt(shm);
        return 1;
    }

    pthread_t agentA, agentB, agentC;
    pthread_create(&agentA, NULL, agent_thread, "A");
    pthread_create(&agentB, NULL, agent_thread, "B");
    pthread_create(&agentC, NULL, agent_thread, "C");

    pthread_join(agentA, NULL);
    pthread_join(agentB, NULL);
    pthread_join(agentC, NULL);

    shmdt(shm);
    return 0;
}
