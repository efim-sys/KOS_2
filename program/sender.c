#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <termios.h>
#include <sys/stat.h>

// Таблица и функция CRC32
uint32_t rc_crc32(uint32_t crc, const uint8_t *buf, size_t len) {
    crc = ~crc;
    while (len--) {
        crc ^= *buf++;
        for (int i = 0; i < 8; i++)
            crc = (crc >> 1) ^ (0xEDB88320 & (-(crc & 1)));
    }
    return ~crc;
}

int setup_serial(const char* portname, speed_t speed) {
    int fd = open(portname, O_RDWR | O_NOCTTY | O_NONBLOCK);
    if (fd < 0) return -1;
    struct termios tty;
    tcgetattr(fd, &tty);
    cfsetospeed(&tty, speed);
    cfsetispeed(&tty, speed);
    tty.c_cflag = (tty.c_cflag & ~CSIZE) | CS8 | CLOCAL | CREAD;
    tty.c_lflag = tty.c_oflag = tty.c_iflag = 0;
    tcsetattr(fd, TCSANOW, &tty);
    return fd;
}

int main(int argc, char *argv[]) {
    if (argc < 2) return 1;
    int fd = setup_serial("/dev/ttyACM0", B9600);
    FILE *file = fopen(argv[1], "rb");
    
    struct stat st;
    stat(argv[1], &st);
    uint32_t filesize = (uint32_t)st.st_size;

    // Считаем CRC32 файла
    uint8_t *file_data = malloc(filesize);
    fread(file_data, 1, filesize, file);
    uint32_t crc_val = rc_crc32(0, file_data, filesize);
    rewind(file);

    printf("Файл: %u байт, CRC32: 0x%08X\n", filesize, crc_val);

    // Отправка заголовка: [SIZE (4b)] + [CRC32 (4b)]
    write(fd, &filesize, 4);
    write(fd, &crc_val, 4);
    usleep(200000);

    uint8_t byte;
    for (uint32_t i = 0; i < filesize; i++) {
        write(fd, &file_data[i], 1);
        // if (i % 100 == 0) {
        char rx;
        if (read(fd, &rx, 1) > 0) printf("%c", rx);
            // printf("Отправка: %u%%", (i * 100) / filesize);
        fflush(stdout);
        // }
        usleep(100); 
    }
    
    printf("\nЗавершено. Слушаем ESP32...\n");
    while(1) {
        char rx;
        if (read(fd, &rx, 1) > 0) putchar(rx);
    }
    return 0;
}
