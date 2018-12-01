#include <sdcard.h>
#include <hspi.h>
#include <esp/uart.h>
#include <FreeRTOS.h>
#include <task.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>

static void print_sector(uint32_t sector, const uint8_t * data)
{
    printf("# %d:\n", sector);
    for (int r = 0; r < 512 / 16; r++, data += 16) {
        printf("%03x:", r * 16);
        const uint8_t * b = data;
        for (int g = 0; g < 4; g++) {
            for (int o = 0; o < 4; o++) {
                printf(" %02x", *b++);
            }
            printf(" ");
        }
        printf(" | ");
        b = data;
        for (int o = 0; o < 16; o++) {
            int c = *b++;
            if (c < ' ' || 0x7f <= c) {
                c = '.';
            }
            putchar(c);
        }
        printf("\n");
    }
}

static int getline(int capacity, char * line)
{
    int n = 0;
    --capacity; // leave space for the '\0' terminator
    for (int c = getchar(); n < capacity && c != '\n'; c = getchar()) {
        if (c >= ' ') {
            line[n++] = c;
            putchar(c); fflush(stdout);
        }
    }
    line[n] = '\0';
    putchar('\n');
    return n;
}

static void main_task(void * arg)
{
    static sdcard_t card;
    static uint32_t addr = 0;
    static char cmd[12];
    static uint8_t data[512];

    for (;;) {
        printf("> "); fflush(stdout);
        if (!getline(sizeof(cmd), cmd)) continue;

        switch (cmd[0]) {
            case 'i': {
                sdcard_result_t err = sdcard_init(&card);
                if (err) {
                    printf("!init error: %d\n", err);
                }
                break;
            }
            case 'c': {
                card.slow = true;
                break;
            }
            case 'C': {
                card.slow = false;
                break;
            }
            case 'a': {
                addr = strtoul(cmd + 1, NULL, 10);
                break;
            }
            case 'r': {
                sdcard_result_t err = sdcard_read(card, addr, 1, data);
                if (err) {
                    printf("!read error: %d\n", err);
                } else {
                    print_sector(addr, data);
                }
                break;
            }
            case 'w': {
                if (addr) {
                    for (int i = 0; i < 512; i++) {
                        data[i] = (uint8_t) i;
                    }
                    sdcard_result_t err = sdcard_write(card, addr, 1, data);
                    if (err) {
                        printf("!write error: %d\n", err);
                    }
                }
                break;
            }
            case 'e': {
                if (addr) {
                    sdcard_result_t err = sdcard_erase(card, addr, 1);
                    if (err) {
                        printf("!erase error: %d\n", err);
                    }
                }
                break;
            }
        }
        // printf("# %u\n", (uint32_t)uxTaskGetStackHighWaterMark(NULL));
    }
}

#define TASK_STACK_SIZE 192

void user_init(void)
{
    uart_set_baud(0, 115200);
    hspi_init();
    xTaskCreate(main_task, "main", TASK_STACK_SIZE, NULL, tskIDLE_PRIORITY, NULL);
}