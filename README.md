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


## Dokumentasi

## Revisi



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
