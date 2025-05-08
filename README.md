<div align=center>

  # Sisop-3-2025-IT39
  
</div>

# SOAL 1
Dikerjakan oleh Ahmad Wildan Fawwaz (5027241001)   

## Cara Pengerjaan

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
