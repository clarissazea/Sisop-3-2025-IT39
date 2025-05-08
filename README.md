<div align=center>

  # Sisop-3-2025-IT39
  
</div>

# SOAL 1
Dikerjakan oleh Ahmad Wildan Fawwaz (5027241001)   

Pada soal ini menggunakan 4 direktori folder dan 10 file. Dimana lebih lengkapnya:   
folder: client, secrets, server, database.   
file: image_client, input_1.txt, input_2.txt, input_3.txt, input_4.txt, input_5.txt, image_client.c, image_server.c, image_server, dan server.log.

## Cara Pengerjaan   
a. Mendownload file rahasia dari link   
```bash
wget -O secrets.zip "https://drive.google.com/uc?export=download&id=15mnXpYUimVP1F5Df7qd_Ahbjor3o1cVw" && unzip secrets.zip -d client/ && rm -r secrets.zip
```
Fungsi tersebut digunakan agar image_server berjalan sebagai daemon di background.   

b. Pada image_server.c, program yang dibuat harus berjalan secara daemon di background dan terhubung dengan image_client.c melalui socket RPC.
### Daemonize
```c 
void daemonize() {
    pid_t pid = fork();
    if (pid < 0) exit(EXIT_FAILURE);
    if (pid > 0) exit(EXIT_SUCCESS); // Parent exit

    setsid();                        // Buat session baru
    chdir("/");                      // Ganti direktori kerja
    close(STDIN_FILENO);            // Tutup standard IO
    close(STDOUT_FILENO);
    close(STDERR_FILENO);
}
```
### Socket setup RPC
```c
int sockfd, new_sock;
    struct sockaddr_in servaddr, cliaddr;
    socklen_t addr_size;
    char buffer[BUFSIZE];

    mkdir("server/database", 0777); // Buat folder database jika belum ada

    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    servaddr.sin_family = AF_INET;
    servaddr.sin_port = htons(PORT); // PORT 8080
    servaddr.sin_addr.s_addr = INADDR_ANY;

    bind(sockfd, (struct sockaddr*)&servaddr, sizeof(servaddr));
    listen(sockfd, 5); // Dengarkan hingga 5 koneksi masuk

    while (1) {
        addr_size = sizeof(cliaddr);
        new_sock = accept(sockfd, (struct sockaddr*)&cliaddr, &addr_size);
        ...
        // Handle perintah dari client
    }

    close(sockfd);
```
Setelah menjadi daemon, program membuat socket TCP.  
Kemudian, server akan terus accept() koneksi dari image_client, menjalankan perintah seperti DECRYPT dan DOWNLOAD via RPC.  

c. Program image_client.c harus bisa terhubung dengan image_server.c dan bisa mengirimkan perintah untuk:   
-Decrypt file      
```c
void send_input_file() {
    // Ambil nama file dari user
    char filename[256];
    printf("Enter the file name (e.g. input_1.txt): ");
    scanf("%s", filename);

    // Gabungkan path file input
    char filepath[512];
    snprintf(filepath, sizeof(filepath), "client/secrets/%s", filename);

    // Baca isi file
    FILE *f = fopen(filepath, "r");
    if (!f) {
        printf("Error: File not found!\n");
        return;
    }
    fseek(f, 0, SEEK_END);
    long fsize = ftell(f);
    rewind(f);

    char *buffer = malloc(fsize + 1);
    fread(buffer, 1, fsize, f);
    buffer[fsize] = '\0';
    fclose(f);

    // Buat koneksi ke server
    int sockfd = connect_to_server();
    if (sockfd < 0) {
        free(buffer);
        return;
    }

    // Kirim perintah DECRYPT dan isi text
    char sendbuf[BUFSIZE];
    snprintf(sendbuf, sizeof(sendbuf), "DECRYPT %s", buffer);
    send(sockfd, sendbuf, strlen(sendbuf), 0);

    // Terima respon dari server
    char recvbuf[BUFSIZE] = {0};
    int len = recv(sockfd, recvbuf, BUFSIZE, 0);
    if (len > 0) {
        recvbuf[len] = '\0';
        printf("%s", recvbuf);  // Server: Text decrypted and saved as 17444xxxxx.jpeg
    }

    close(sockfd);
    free(buffer);
}
```
-Request download   
```c
void download_file() {
    // Ambil nama file dari user
    char filename[256];
    printf("Enter the filename to download (e.g. 1744401282.jpeg): ");
    scanf("%s", filename);

    int sockfd = connect_to_server();
    if (sockfd < 0) return;

    // Kirim perintah DOWNLOAD <filename>
    char request[BUFSIZE];
    snprintf(request, sizeof(request), "DOWNLOAD %s", filename);
    send(sockfd, request, strlen(request), 0);

    // Simpan file JPEG hasil unduhan
    char savepath[512];
    snprintf(savepath, sizeof(savepath), "client/%s", filename);
    FILE *f = fopen(savepath, "wb");
    if (!f) {
        printf("Error: Failed to create output file.\n");
        close(sockfd);
        return;
    }

    // Terima isi file dari server
    char buffer[BUFSIZE];
    int bytes, total = 0;
    while ((bytes = recv(sockfd, buffer, BUFSIZE, 0)) > 0) {
        fwrite(buffer, 1, bytes, f);
        total += bytes;
        if (bytes < BUFSIZE) break;
    }
    fclose(f);
    close(sockfd);

    if (total == 0) {
        printf("Error: Server says file not found.\n");
        remove(savepath);
    } else {
        printf("Success! Image saved as %s\n", filename);
    }
}
```

d. Program image_client.c harus disajikan dalam bentuk menu kreatif
```c
void show_menu() {
    printf("==============================\n");
    printf("     | Image Decoder Client |\n");
    printf("==============================\n");
    printf("1. Send input file to server\n");
    printf("2. Download file from server\n");
    printf("3. Exit\n");
    printf("> ");
}

int main() {

    while (1) {
        show_menu();

        int choice;
        scanf("%d", &choice);

        switch (choice) {
            case 1:
                send_input_file();  // Fungsi kirim file untuk didekripsi
                break;
            case 2:
                download_file();    // Fungsi minta file JPEG dari server
                break;
            case 3: {
                // Kirim perintah EXIT ke server
                int sockfd = connect_to_server();
                if (sockfd > 0) {
                    send(sockfd, "EXIT", 4, 0);
                    close(sockfd);
                }
                printf("Exiting...\n");
                return 0;
            }
            default:
                printf("Invalid choice!\n");
        }
    }
}
```
e. Program dianggap berhasil bila pengguna dapat mengirimkan text file dan menerima sebuah file jpeg yang dapat dilihat isinya.   

f. Program image_server.c diharuskan untuk tidak keluar/terminate saat terjadi error dan client akan menerima error message sebagai response
```c
while (1) {
    int client_fd = accept(server_fd, (struct sockaddr*)&client_addr, &addr_len);
    if (client_fd < 0) {
        write_log("Server", "ACCEPT_ERROR", "Failed to accept connection");
        continue;  

    }

    handle_client(client_fd);
}
```

g. Server menyimpan log semua percakapan antara image_server.c dan image_client.c di dalam file server.log.
```c
void write_log(const char *source, const char *action, const char *info) {
    FILE *log = fopen("server/server.log", "a");
    if (!log) return;

    // Ambil waktu saat ini
    time_t now = time(NULL);
    struct tm *t = localtime(&now);
    char timestr[64];
    strftime(timestr, sizeof(timestr), "%Y-%m-%d %H:%M:%S", t);

    // Tulis log ke file
    fprintf(log, "[%s][%s]: [%s] [%s]\n", source, timestr, action, info);
    fclose(log);
}
```
Saat menerima perintah decrypt dari client
```c
write_log("Client", "DECRYPT", "Text data");

// Setelah berhasil disimpan
write_log("Server", "SAVE", filename);
```
Saat menerima perintah download
```c
write_log("Client", "DOWNLOAD", filename);

// Jika file ditemukan dan dikirim:
write_log("Server", "UPLOAD", filename);
```
Saat menerima perintah exit
```c
write_log("Client", "EXIT", "Client requested to exit");
```

## Dokumentasi
![Image](https://github.com/user-attachments/assets/a826b17c-d199-44a0-a1c9-ec188fbfbc81)

![Image](https://github.com/user-attachments/assets/732ea699-cc91-4166-9d01-631abf73a986)

![Image](https://github.com/user-attachments/assets/1ade03cb-bab0-4794-af6e-3d9d49e1905b)

![Image](https://github.com/user-attachments/assets/a25c5a0e-6c12-4b6e-8e08-9b1194d32991)

![Image](https://github.com/user-attachments/assets/9cc71108-c872-4eb3-93a2-ea556e308973)
## Revisi



# SOAL 2
Dikerjakan oleh Clarissa Aydin Rahmazea (5027241014)   

## Cara Pengerjaan

Sistem RushGo ini terdiri dari dua program, yaitu:
1. `delivery_agent.c`: Untuk agen pengantar otomatis pesanan Express.
2. `dispatcher.c`: Untuk mengelola, memantau, dan memproses pesanan reguler (pesanan diantar manual)

File CSV berisi pesanan `delivery_order.csv` dibaca oleh `dispatcher.c`. Kemudian data pesanan dimasukkan ke shared memory agar bisa diakses lintas program. `delivery_agent.c` membaca shared memory dan membuat tiga thread agen (A, B, C) untuk menemukan order Express berstatus Pending dan Mengantarnya otomatis (mengubah status dan mencatat ke log).

User menjalankan dispatcher untuk:
- Menampilkan seluruh pesanan (-list)
- Melihat status pesanan tertentu (-status)
- Mengirim Reguler order secara manual (-deliver [Nama])

### Argumen menjalankan program:
Untuk delivery agent
```bash
./delivery_agent
```

Untuk dispatcher
```bash
./dispatcher -list
./dispatcher -status [Nama]
./dispatcher -deliver [Nama]
```




### a. Mengunduh File Order dan Menyimpannya ke Shared Memory
Fungsi ini bertanggung jawab membaca file CSV delivery_order.csv, yang berisi daftar pesanan. Setiap baris mencakup informasi nama pelanggan, alamat, dan tipe pesanan. Data tersebut kemudian dimasukkan ke shared memory, dan setiap status pesanan di-set ke “Pending” secara default.

Fungsi terkait:

`read_csv_to_shared_memory() – Dibuat di file dispatcher.c`

```bash
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
```
Penjelasan:
- fopen(...): Membuka file CSV.
- strtok(...): Memisahkan nama, alamat, dan tipe dari tiap baris.
- strncpy(...): Menyalin informasi ke struktur Order.
- strcpy(status, "Pending"): Status awal semua pesanan di-set sebagai 'Pending.'
  
Dokumentasi saat membaca `delivery_order.csv`
![image](https://github.com/user-attachments/assets/fc49800a-7671-4ae0-a533-60d29ef637da)
Dokumentasi semua status awal pesanan yang di-set sebagai pending.
![image](https://github.com/user-attachments/assets/2e90f628-d257-4da6-81c0-915185a9c85b)

### b. Pengiriman Bertipe Express
Setiap agen pengiriman (A, B, dan C) berjalan sebagai thread terpisah dan akan secara otomatis mencari pesanan dengan tipe “Express” dan status “Pending”, lalu mengubah statusnya menjadi “Delivered by Agent X” dan mencatat log ke delivery.log.

Fungsi terkait:
`agent_thread() – Dibuat di file delivery_agent.c`

```bash
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
                }

                printf("[%s] Delivered Express package to %s\n", agent_name, orders[i].name);
                sleep(1); // Simulasi delay pengiriman
            }
        }
        sleep(1); // Cek setiap 1 detik
    }

    return NULL;
}
```
Penjelasan:
- Mengecek semua order
- `if (strcmp(orders[i].type, "Express") == 0 && strcmp(orders[i].status, "Pending") == 0)` Mengecek apakah pesanan bertipe Express dan statusnya masih Pending (belum dikirim).
- `snprintf(orders[i].status, STATUS_LEN, "Delivered by Agent %s", agent_name);` Jika cocok, status pesanan diubah menjadi “Delivered by Agent A/B/C” sesuai nama agen thread.
- `sleep(1);` Menunggu 1 detik untuk menyimulasikan proses pengiriman barang (agar tidak terlalu cepat dalam loop).


### c. Pengiriman Bertipe Reguler
Jika user menjalankan `./dispatcher -deliver [Nama]`, maka program mencari order reguler sesuai nama. Jika ditemukan dan statusnya masih "Pending", maka status diubah menjadi `"Delivered by Agent clarissazea"` dan log ditulis ke file `delivery.log.` Jika order tipe Express, maka user tidak diizinkan mengantar.

Fungsi terkait:
`Bagian dari main() dispatcher.c saat memproses -deliver`

```bash
} else if (strcmp(argv[1], "-deliver") == 0 && argc == 3) {
    int found = 0;
    for (int i = 0; i < *order_count; i++) {
        if (strcmp(orders[i].name, argv[2]) == 0) {
            if (strcmp(orders[i].type, "Reguler") == 0 && strcmp(orders[i].status, "Pending") == 0) {
                snprintf(orders[i].status, STATUS_LEN, "Delivered by Agent clarissazea");

                FILE *logfile = fopen("delivery.log", "a");
                if (logfile) {
                    time_t now = time(NULL);
                    struct tm *t = localtime(&now);
                    fprintf(logfile,
                            "[%02d/%02d/%04d %02d:%02d:%02d] [AGENT clarissazea] Reguler package delivered to %s in %s\n",
                            t->tm_mday, t->tm_mon + 1, t->tm_year + 1900,
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
```
Penjelasan:
- Saat pengguna menjalankan program dengan argumen `-deliver [nama]`, program akan mencari order berdasarkan nama. Jika ditemukan dan order bertipe Reguler serta statusnya masih Pending, maka status order diubah menjadi `"Delivered by Agent clarissazea"`
- `if (strcmp(orders[i].type, "Reguler") == 0 && strcmp(orders[i].status, "Pending") == 0)` Memastikan bahwa order tersebut bertipe Reguler dan statusnya masih Pending.
-  `snprintf(orders[i].status, STATUS_LEN, "Delivered by Agent clarissazea");` Mengubah status pesanan menjadi terkirim oleh agen bernama clarissazea.
-  `fprintf(logfile, ...)` Menulis log pengiriman dengan format waktu, nama agen, nama pemesan, dan alamat.
-  `printf("Reguler package delivered to %s.\n", orders[i].name);` Menampilkan pesan bahwa pengiriman berhasil.

   Dokumentasi:
   Jika orderan reguler
  ![image](https://github.com/user-attachments/assets/dfc622e2-8033-4fc5-8c62-a2ee89812faf)
   Jika orderan express namun diantar manual (dengan dispatcher)
  ![image](https://github.com/user-attachments/assets/a618c169-0d71-46df-9a7a-21f759bd3a01)



  
### d. Mengecek Status Pesanan
Dengan perintah `./dispatcher -status [Nama]`, user bisa melihat status order sesuai nama. Jika ditemukan, status dicetak ke layar. Jika tidak, muncul pesan bahwa order tidak ditemukan.

```bash
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
```

Penjelasan:
- Saat pengguna menjalankan program dengan argumen `-status [nama]`, program akan menampilkan status orderan dari deliver
- `if (strcmp(orders[i].name, argv[2]) == 0)` Mencocokkan nama pemesan dari argumen dengan nama yang ada di order.
- `printf("Status for %s: %s\n", orders[i].name, orders[i].status);` Jika ditemukan, akan mencetak status pesanan milik nama tersebut.
- `if (!found)` Jika tidak ditemukan, mencetak pesan bahwa order tidak ditemukan sesuai nama yang diberikan.

  Dokumentasi:
  ![image](https://github.com/user-attachments/assets/fd035ac1-8f2f-4d80-9ff3-2322be3c99e8)


### e. Melihat Daftar Semua Pesanan
Dengan menjalankan `./dispatcher -list`, program akan mencetak semua nama pemesan (baik express maupun reguler) beserta status pengirimannya. Hal ini berguna untuk memonitoring seluruh orderan.

```bash
if (strcmp(argv[1], "-list") == 0) {
    printf("Order List:\n");
    for (int i = 0; i < *order_count; i++) {
        printf("%s: %s\n", orders[i].name, orders[i].status);
    }
}
```

Penjelasan:
- Saat pengguna menjalankan program dengan argumen `-list`, program akan menampilkan seluruh pesanan baik express atau reguler.
- `for (int i = 0; i < *order_count; i++)` Melakukan iterasi untuk setiap order yang ada dalam shared memory berdasarkan jumlah order yang ada (dari order_count).
- `printf("%s: %s\n", orders[i].name, orders[i].status);` Mencetak nama pemesan dan status pesanan dari setiap order dalam daftar.

Dokumentasi:
![image](https://github.com/user-attachments/assets/d6a0fcd2-5d92-4b41-8978-6036174075d3)
![image](https://github.com/user-attachments/assets/b6b7b99f-3260-426f-af3f-8260fc2abcb1)



# SOAL 3
Dikerjakan oleh Muhammad Rafi' Adly (5027241082)   

## Cara Pengerjaan

## Dokumentasi

## Revisi


# SOAL 4
Dikerjakan oleh Clarissa Aydin Rahmazea (5027241014)   

## Cara Pengerjaan

## Dokumentasi

## Revisi
