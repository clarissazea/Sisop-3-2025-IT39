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
-Penambahan socket RPC 
-Penambahan error handling 
-Perbaikan kode pada image_client.c & image_server.c


# SOAL 2
Dikerjakan oleh Clarissa Aydin Rahmazea (5027241014)   

## Cara Pengerjaan

Sistem Pengiriman RushGo ini terdiri dari dua program, yaitu:
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

Dokumentasi:
![image](https://github.com/user-attachments/assets/d0c6d9fc-a932-40cc-9d00-2ce28a13a9dc)


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

Sistem manajemen hunter ini menggunakan shared memory untuk menghubungkan admin (system.c) dan hunter (hunter.c), memungkinkan registrasi, battle hunter, dungeon random, serta fitur ban/reset, dengan cleanup otomatis saat shutdown.

### Argumen menjalankan program:
Untuk program system
```bash
./system.c
```

Untuk program hunter
```bash
./hunter.c
```

### a. Membuat 2 file, system.c dan hunter.c

`system.c`: sebagai shared memory utama, hanya bisa dijalankan satu kali. Program ini membuat shared memory untuk data hunter dan dungeon.

`hunter.c`: digunakan untuk registrasi dan interaksi hunter, dijalankan banyak kali oleh user berbeda. Program ini mengakses shared memory tersebut untuk melakukan operasi login, raid, dan lain-lain.

Jadi, alurnya adalah `system.c membuat SHM` → `hunter.c attach` → `Keduanya bisa read/write data yang sama.`

system.c
```bash
init_shared_memory() {
    shm_open(HUNTER_SHM, O_CREAT|O_RDWR, 0666); 
    ftruncate(fd, sizeof(Hunter)*MAX_HUNTERS);  
    mmap(..., MAP_SHARED); 
}
```
- `shm_open(HUNTER_SHM, O_CREAT|O_RDWR, 0666)`: Membuat shared memory dengan permission 0666 (read-write untuk semua).
- ` mmap(..., MAP_SHARED);`: Menggunakan mmap dengan flag MAP_SHARED agar bisa diakses multi-proses.

hunter.c
```bash
int fd = shm_open(HUNTER_SHM, O_RDWR, 0666); // Buka existing SHM
hunters = mmap(..., MAP_SHARED); 
```
- `hunters = mmap(..., MAP_SHARED)` : Attach ke address space
- Error Handling: Jika SHM belum ada, keluar dengan pesan "Run system.c first!".

Dokumentasi:

![image](https://github.com/user-attachments/assets/ae1a015b-fcab-4501-9a82-6c5887140f0c)

![image](https://github.com/user-attachments/assets/fe4eeb64-b23c-48c6-8aab-74974fef1cd1)

Error handling ketika hunter.c dijalankan dahulu (tanpa menjalankan system.c):

![image](https://github.com/user-attachments/assets/b5f06faa-16cd-45e6-b319-f4a82967cec7)



### b. Registrasi dan Login Hunter
Hunter bisa membuat akun (registrasi) dan masuk (login) ke sistem. Setiap akun memiliki stats awal dan disimpan dalam shared memory.

Struktur hunter:
```bash
```bash
typedef struct {
    char name[50];
    int key;
    int level, exp, atk, hp, def;
    int banned;
    int notification;
    int used;
} Hunter;
```
Fungsi Registrasi:
Inisialisasi stats default
```bash
void register_hunter(Hunter *hunters) {
    for (int i = 0; i < MAX_HUNTER; i++) {
        if (!hunters[i].used) {
            printf("Enter name: ");
            scanf("%s", hunters[i].name);
            hunters[i].key = rand() % 1000;
            hunters[i].level = 1;
            hunters[i].exp = 0;
            hunters[i].atk = 10;
            hunters[i].hp = 100;
            hunters[i].def = 5;
            hunters[i].banned = 0;
            hunters[i].notification = 0;
            hunters[i].used = 1;
            printf("Registration successful! Key: %d\n", hunters[i].key);
            break;
        }
    }
}
```
Cek username:
```bash
if (get_hunter_index(name) != -1) { ... }
```
Cek banned status (apabila user di ban)
```bash
if (hunters[idx].banned) printf("You are banned!");
```

Dokumentasi:

![image](https://github.com/user-attachments/assets/154ea3e5-ae2b-4fc8-82a4-2dda64cb3af7)
![image](https://github.com/user-attachments/assets/9d09593b-e389-438a-a7a6-28e1348cf7cd)



### c. Tampilkan Informasi Semua Hunter di system.c
Admin dapat melihat semua hunter yang terdaftar dan statistik mereka.
```bash
void show_all_hunters(Hunter *hunters) {
    for (int i = 0; i < MAX_HUNTER; i++) {
        if (hunters[i].used) {
            printf("%s | Lv:%d | Exp:%d | Atk:%d | HP:%d | Def:%d | Banned:%s\n",
                hunters[i].name, hunters[i].level, hunters[i].exp,
                hunters[i].atk, hunters[i].hp, hunters[i].def,
                hunters[i].banned ? "Yes" : "No");
        }
    }
}
```
Penjelasan:
1. Loop melalui array hunters, Menggunakan perulangan `for` dari indeks `0 hingga MAX_HUNTER - 1`, lalu Memeriksa apakah slot hunter tersebut terisi `(hunters[i].used == 1)`
2. Jika hunter terdaftar (used == 1), akan menampilkan:
  
```bash
<Nama> | Lv:<Level> | Exp:<Exp> | Atk:<Attack> | HP:<HP> | Def:<Defense> | Banned:<Yes/No>
```
3. Status Banned:
- Jika `hunters[i].banned == 1`, maka ditampilkan "Yes".
- Jika `hunters[i].banned == 0`, maka ditampilkan "No".

Dokumentasi:

![image](https://github.com/user-attachments/assets/d6564f68-b62f-4079-9e61-d259b2632247)


### d. Fitur Random Dungeon di system.c
Sistem membuat dungeon dengan reward acak. Menggunakan `rand()` dan simpan di shared memory `Dungeon`.

Struktur Dungeon
```bash
typedef struct {
    char name[100];
    int level_min;
    int atk, hp, def, exp;
    int used;
} Dungeon;
```
Program pembuatan Dungeon
```bash
void create_dungeon(Dungeon *dungeons) {
    for (int i = 0; i < MAX_DUNGEON; i++) {
        if (!dungeons[i].used) {
            sprintf(dungeons[i].name, "Dungeon-%d", rand() % 1000);
            dungeons[i].level_min = rand() % 5 + 1;
            dungeons[i].atk = rand() % 51 + 100;
            dungeons[i].hp = rand() % 51 + 50;
            dungeons[i].def = rand() % 26 + 25;
            dungeons[i].exp = rand() % 151 + 150;
            dungeons[i].used = 1;
            break;
        }
    }
}
```
Penjelasan:
1. Loop melalui array dungeons: Menggunakan perulangan for dari indeks `0` hingga `MAX_DUNGEON - 1` dan mencari slot yang belum terpakai `(dungeons[i].used == 0)`.
2. `(rand() % 5 + 1)` Jika menemukan slot kosong: nama dibuat secara acak, level Minimum random antara 1 sampai 5.
3. Statistik Dungeon:
```bash
- Attack (atk): Random antara 100 sampai 150 (rand() % 51 + 100).
- HP (hp): Random antara 50 sampai 100 (rand() % 51 + 50).
- Defense (def): Random antara 25 sampai 50 (rand() % 26 + 25).
- Hadiah EXP (exp): Random antara 150 sampai 300 (rand() % 151 + 150).
```

Dokumentasi:

![image](https://github.com/user-attachments/assets/6ca06914-c385-42e1-8c45-0db3e4e83ec0)


### e. Tampilkan Semua Dungeon (system.c)
Admin bisa melihat seluruh dungeon aktif beserta status/levelnya. 
```bash
void show_all_dungeons(Dungeon *dungeons) {
    for (int i = 0; i < MAX_DUNGEON; i++) {
        if (dungeons[i].used) {
            printf("%s | LvMin:%d | ATK:%d | HP:%d | DEF:%d | EXP:%d\n",
                dungeons[i].name, dungeons[i].level_min, dungeons[i].atk,
                dungeons[i].hp, dungeons[i].def, dungeons[i].exp);
        }
    }
}
```

Penjelasan:
1. `for (int i = 0; i < MAX_DUNGEON; i++)`: Mengiterasi melalui array dungeons dari indeks 0 hingga MAX_DUNGEON-1
2. `if (dungeons[i].used)` : Memeriksa dungeon yang aktif
3. Menampilkan informasi dungeon
`<Nama> | LvMin:<Level> | ATK:<Attack> | HP:<Health> | DEF:<Defense> | EXP:<Experience>`

Dokumentasi:

![image](https://github.com/user-attachments/assets/b4e740d0-a1f9-4bfc-8373-a904840742e4)


### f. Tampilkan Dungeon yang Sesuai Level Hunter (hunter.c)
Hunter hanya bisa melihat dungeon sesuai level mereka. Filter dungeon berdasarkan `level_min <= hunter.level`

```bash
void show_available_dungeons(Dungeon *dungeons, Hunter *h) {
    for (int i = 0; i < MAX_DUNGEON; i++) {
        if (dungeons[i].used && dungeons[i].level_min <= h->level) {
            printf("%s | LvMin:%d | ATK:%d | HP:%d | DEF:%d | EXP:%d\n",
                dungeons[i].name, dungeons[i].level_min, dungeons[i].atk,
                dungeons[i].hp, dungeons[i].def, dungeons[i].exp);
        }
    }
}
```

Penjelasan: 
1. `if (dungeons[i].used && dungeons[i].level_min <= h->level)` : Filter dungeon yang tersedia:
2. Menampilkan info dungeon:
   `printf("%s | LvMin:%d | ATK:%d | HP:%d | DEF:%d | EXP:%d\n", ...);`

Dokumentasi:

![image](https://github.com/user-attachments/assets/1cf36750-ccec-4248-92dc-fd217ab3734d)



### g. Fitur Menaklukkan Dungeon dan Naik Level (hunter.c)
Hunter bisa memilih dungeon dan jika berhasil, dungeon dihapus dan hunter mendapat stat reward. Hunter naik level setiap meraih 500 exp, dan exp akan kembali default ketika naik level. 

```bash
void raid_dungeon(Dungeon *dungeons, Hunter *h) {
    char dungeon_name[50];
    printf("Enter dungeon name to raid: ");
    scanf("%s", dungeon_name);

    for (int i = 0; i < MAX_DUNGEON; i++) {
        if (dungeons[i].used && strcmp(dungeons[i].name, dungeon_name) == 0) {
            if (h->banned) {
                printf("You are banned from raiding.\n");
                return;
            }
            if (h->level < dungeons[i].level_min) {
                printf("Level too low.\n");
                return;
            }
            h->atk += dungeons[i].atk;
            h->hp += dungeons[i].hp;
            h->def += dungeons[i].def;
            h->exp += dungeons[i].exp;
            if (h->exp >= 500) {
                h->level++;
                h->exp = 0;
            }
            dungeons[i].used = 0;
            printf("Dungeon cleared! Stats updated.\n");
            break;
        }
    }
}
```
Penjelasan:
1. Input nama Dungeon:
```bash
char dungeon_name[50];
printf("Enter dungeon name to raid: ");
scanf("%s", dungeon_name);
```
2. Mencari dungeon dengan nama yang sesuai dan masih aktif (used == 1)
```bash
for (int i = 0; i < MAX_DUNGEON; i++) {
    if (dungeons[i].used && strcmp(dungeons[i].name, dungeon_name) == 0) {
```
3. Menambahkan statistik dungeon ke hunter (ATK, HP, DEF) dan menambahkan EXP dari dungeon ke hunter
```bash
h->atk += dungeons[i].atk;
h->hp += dungeons[i].hp;
h->def += dungeons[i].def;
h->exp += dungeons[i].exp;
```
4. Jika EXP hunter mencapai 500, level naik dan EXP direset ke 0 lalu menampilkan pesan sukses
```bash
if (h->exp >= 500) {
    h->level++;
    h->exp = 0;
}

dungeons[i].used = 0;
printf("Dungeon cleared! Stats updated.\n");
```

Dokumentasi:

![image](https://github.com/user-attachments/assets/338fba48-6422-4480-937f-27261451ae74)


Dokumentasi saaat hunter naik level:

![image](https://github.com/user-attachments/assets/8535ebc6-a75b-4ac3-8256-1b4bc8dfcac6)


![image](https://github.com/user-attachments/assets/1d81e712-d4fa-4062-8976-6ae2b501c3d5)




### h. Fitur Battle antar Hunter (hunter.c)
Hunter bisa menantang hunter lain dalam pertempuran. Pemenang ditentukan berdasarkan perbandingan statistik (membandingkan stats).
Fungsi: `hunter_battle(int hunter_index)`

1. Hanya tampilkan hunter yang tidak banned dan bukan diri sendiri:
```bash
for (int i = 0; i < MAX_HUNTERS; ++i) {
    if (hunters[i].used && i != hunter_index && !hunters[i].banned) {
        printf("%d. %s (Level %d)\n", count, hunters[i].username, hunters[i].level);
        opponents[count - 1] = i; // Simpan index lawan
    }
}

```
2. Menghitung kekuatan. Total stats (ATK + HP + DEF) menentukan kekuatan:
```bash
int my_power = hunters[hunter_index].atk + hunters[hunter_index].hp + hunters[hunter_index].def;
int opp_power = hunters[opp].atk + hunters[opp].hp + hunters[opp].def;
```
3. Hasil battle jika menang:
```bash
hunters[hunter_index].atk += hunters[opp].atk; // Ambil ATK lawan
hunters[hunter_index].hp += hunters[opp].hp;   // Ambil HP lawan
hunters[opp].used = 0; // Hapus lawan dari sistem
```

4. Hasil battle jika kalah:
```bash
hunters[opp].atk += hunters[hunter_index].atk;
hunters[hunter_index].used = 0; // Hapus diri sendiri
exit(0); // Keluar dari program hunter
```

Dokumentasi:

![image](https://github.com/user-attachments/assets/6cc68c3e-44ba-4106-ae12-029a2908c17b)


### i. Fitur Ban Hunter dan Unbanned Hunter (system.c)
Admin bisa membatasi akses hunter untuk melakukan raid dungeon dengan memberi status banned.
Fungsi: `toggle_ban(const char* username, int ban)`
1. Mencari hunter
```bash
for (int i = 0; i < MAX_HUNTERS; i++) {
    if (hunters[i].used && strcmp(hunters[i].username, username) == 0) {
        hunters[i].banned = ban; // Set flag banned
    }
}

```
2. Hunter yang banned tidak bisa login:
```bash
// Di hunter.c (login):
if (hunters[idx].banned) {
    printf("You are banned!");
    return;
}
```
3. Hunter yang banned tidak bisa raid/battle:
```bash
// Di dungeon_raid() dan hunter_battle():
if (hunters[hunter_index].banned) {
    printf("You are banned!");
    return;
}
```
4. Menu admin:
```bash
void ban_menu() {
    printf("1. Ban hunter\n2. Unban hunter\n");
    scanf("%d", &choice);
    toggle_ban(username, choice == 1 ? 1 : 0);
}
```

Dokumentasi (system.c) ---- Banned Hunter

![image](https://github.com/user-attachments/assets/48462a40-7c28-4ff3-9fa1-9fdb03cd3251)

Dokumentasi (hunter.c) ---- Banned Hunter

![image](https://github.com/user-attachments/assets/6066e357-0172-4871-864d-76b70c96a863)

Dokumentasi (system.c) ---- Unbanned Hunter

![image](https://github.com/user-attachments/assets/0ba0979d-3899-480b-9260-a01b0632df6c)

Dokumentasi (hunter.c) ---- Unbanned Hunter

![image](https://github.com/user-attachments/assets/01b84e6f-ef74-47c0-9455-bf1d7672e842)



### j. Fitur Reset Stat Hunter (system.c)
Admin (Sung Jin-Woo) dapat memberikan kesempatan kedua kepada hunter tertentu dengan mereset seluruh statistik mereka ke nilai awal seperti saat pertama kali registrasi.
Fungsi: `reset_stats(const char* username)`

```bash
for (int i = 0; i < MAX_HUNTERS; i++) {
    if (hunters[i].used && strcmp(hunters[i].username, username) == 0) {
        hunters[i].level = 1;
        hunters[i].atk = 10;
        hunters[i].hp = 100;
        hunters[i].def = 5;
        hunters[i].exp = 0;
        // Banned status tetap tidak berubah
    }
}
```
Penjelasan:
1. Stats dikembalikan ke nilai default:
Level=1, ATK=10, HP=100, DEF=5, EXP=0.
2. Banned status tidak direset (tetap seperti sebelumnya).

Dokumentasi:

![image](https://github.com/user-attachments/assets/09643769-6ac9-43ae-92ea-eea2db2be2d1)

![image](https://github.com/user-attachments/assets/e55ba4b0-1665-42e0-8d7b-f3c8df8c2396)


### k. Fitur Notifikasi Dungeon Setiap 3 Detik
```bash
void* notification_thread(void* arg) {
    while (notification_active) {
        system("clear");
        printf("=== HUNTER SYSTEM ===\n");

        int available_count = 0;
        int available_indices[MAX_DUNGEONS];

        for (int j = 0; j < MAX_DUNGEONS; ++j) {
            if (dungeons[j].used && dungeons[j].min_level <= hunters[current_hunter_index].level) {
                available_indices[available_count++] = j;
            }
        }

        if (available_count > 0) {
            int random_idx = available_indices[rand() % available_count];
            printf("\nNew Dungeon Available:\n");
            printf("- %s (Level %d+)\n", dungeons[random_idx].name, dungeons[random_idx].min_level);
        } else {
            printf("\nNo new dungeons available\n");
        }

        // Menu (non-fungsional di thread ini)
        printf("\n1. Dungeon List\n");
        printf("2. Dungeon Raid\n");
        printf("3. Hunters Battle\n");
        printf("4. Toggle Notifications\n");
        printf("5. Exit\n");
        printf("\nChoice: ");
        fflush(stdout);

        sleep(3);
    }
    return NULL;
}
```
```bash
void toggle_notifications(int hunter_index) {
    if (notification_active) {
        notification_active = 0;
        printf("Notifications turned OFF\n");
    } else {
        notification_active = 1;
        current_hunter_index = hunter_index;
        printf("Notifications turned ON\n");
        
        pthread_t thread;
        pthread_create(&thread, NULL, notification_thread, NULL);
        pthread_detach(thread);
    }
    press_enter();
}

```
Penjelasan:
1. `pthread_create()` dan `pthread_detach()` untuk membuat thread baru tanpa blocking.
2. `sleep(3)` untuk delay pergantian dungeon.
3. `rand()` dan `time(NULL)` untuk pemilihan dungeon secara acak dan dinamis.

Dokumentasi:

![image](https://github.com/user-attachments/assets/b1424a10-fc44-4eeb-9b1e-8338f2e3d3c5)



### l. Hapus data shared memory
Fungsi: `cleanup_shared_memory()`

1. Melepaskan mapping dari virtual memory.
```bash
munmap(hunters, sizeof(Hunter) * MAX_HUNTERS);
munmap(dungeons, sizeof(Dungeon) * MAX_DUNGEONS);
```
2. Hapus Shared Memory
```bash
shm_unlink(HUNTER_SHM); // Hapus hunter shared memory
shm_unlink(DUNGEON_SHM); // Hapus dungeon shared memory
```

Penjelasan:
1. Semua data hunter dan dungeon hilang setelah sistem dimatikan.
2. Jika system.c dijalankan ulang, SHM baru akan dibuat kosong.

Dokumentasi:

![image](https://github.com/user-attachments/assets/4c69e7d7-1e1b-4c4e-9646-d89a897f22c5)


