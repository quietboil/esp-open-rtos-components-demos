#include <ff.h>
#include <diskio.h>
#include <hspi.h>
#include <esp/uart.h>
#include <FreeRTOS.h>
#include <task.h>
#include <malloc.h>
#include <string.h>
#include <stdio.h>

static const char * const ERR[] = {
    "OK", "DISK_ERR", "INT_ERR", "NOT_READY", "NO_FILE", "NO_PATH", "INVALID_NAME",
    "DENIED", "EXIST", "INVALID_OBJECT", "WRITE_PROTECTED", "INVALID_DRIVE",
    "NOT_ENABLED", "NO_FILE_SYSTEM", "MKFS_ABORTED", "TIMEOUT", "LOCKED",
    "NOT_ENOUGH_CORE", "TOO_MANY_OPEN_FILES", "INVALID_PARAMETER"
};

static FRESULT mount() {
    static const char * const fst[] = {"", "FAT12", "FAT16", "FAT32", "exFAT"};
    static FATFS sdfs;

    FRESULT err = f_mount(&sdfs, "/", 1);
    if (err) {
        printf("ERROR: mount=%s\n", ERR[err]);
    } else {
        DWORD free_clusters;
        FATFS * fs;
        err = f_getfree("/", &free_clusters, &fs);
        if (err) {
            printf("ERROR: getfree=%s\n", ERR[err]);
        } else {
            DWORD total_sectors = (fs->n_fatent - 2) * fs->csize;
            DWORD free_sectors  = free_clusters * fs->csize;
            printf(
                "FAT type = %s\n"                   "Bytes/Cluster = %u\n"
                "Number of FATs = %u\n"             "Root DIR entries = %u\n"
                "Sectors/FAT = %u\n"                "Number of clusters = %u\n"
                "Volume start (lba) = %u\n"         "FAT start (lba) = %u\n"
                "DIR start (lba,cluster) = %u\n"    "Data start (lba) = %u\n"
                "Disk space = %u KB\n"              "Available  = %u KB\n",
                fst[fs->fs_type],                   (DWORD)fs->csize * 512,
                fs->n_fats,                         fs->n_rootdir,
                fs->fsize,                          fs->n_fatent - 2,
                fs->volbase,                        fs->fatbase,
                fs->dirbase,                        fs->database,
                total_sectors / 2,                  free_sectors / 2
            );
        }
        char label[36];
        DWORD sn;
        err = f_getlabel("/", label, &sn);
        if (err) {
            printf("ERROR: getlabel=%s\n", ERR[err]);
        } else {
            printf("Label = %s\nSerial No = %08x\n", label, sn);
        }
    }
    return err;
}

static void display_card_size()
{
    uint32_t count;
    DRESULT err = disk_ioctl(0, GET_SECTOR_COUNT, &count);
    if (err) {
        printf("ERROR: ioctl(GET_SECTOR_COUNT)=%d\n", err);
    } else {
        printf("Size: %d sect / %d KB\n", count, count / 2);
    }
}

static FRESULT dir_make(const char * name)
{
    FRESULT err = f_mkdir(name);
    if (err) {
        printf("!! Cannot make %s (%s)\n", name, ERR[err]);
    } else {
        printf("Created directory %s\n", name);
    }
    return err;
}

static void unlink(const char * name)
{
    FRESULT err = f_unlink(name);
    if (err) {
        printf("!! Cannot remove %s (%s)\n", name, ERR[err]);
    }
}

static inline void dir_remove(const char * name)
{
    unlink(name);
}

static FRESULT dir_change(const char * name)
{
    FRESULT err = f_chdir(name);
    if (err) {
        printf("!! Cannot change current dir to %s (%s)\n", name, ERR[err]);
    }
    return err;
}

static FRESULT change_cwd(DIR * dir, const char * name)
{
    FRESULT err = f_opendir(dir, name);
    if (err) {
        printf("!! Cannot open %s (%s)\n", name, ERR[err]);
    } else {
        err = dir_change(name);
    }
    return err;
}

#define MAX_TREE_HEIGHT 16

static void dir_tree(const char * root)
{
    DIR * path = calloc(MAX_TREE_HEIGHT, sizeof(DIR));
    if (path) {
        int i = 0;
        FRESULT err = change_cwd(&path[i], root);
        if (!err) {
            FILINFO file_info;
            while (i >= 0) {
                err = f_readdir(&path[i], &file_info);
                while (!err && file_info.fname[0]) {
                    if (file_info.fattrib & AM_DIR) {
                        printf("%*s%s\n", i*2, "", file_info.fname);
                        if (i < MAX_TREE_HEIGHT - 1) {
                            ++i;
                            err = change_cwd(&path[i], file_info.fname);
                            if (err){
                                --i;
                            }
                        }
                    } else {
                        printf("%*s%s %10u\n", i*2, "", file_info.fname, file_info.fsize);
                    }
                    err = f_readdir(&path[i], &file_info);
                }
                err = f_closedir(&path[i]);
                if (err) {
                    printf("!! Cannot close cwd (%s)\n", ERR[err]);
                }
                if (i > 0 || strcmp(root, ".") != 0) {
                    err = f_chdir("..");
                    if (err) {
                        printf("!! Cannot change cwd to .. (%s)\n", ERR[err]);
                    }
                }
                --i;
            }
        }
        free(path);
    }
}

static void dir_list(const char * name)
{
    DIR dir;
    FRESULT err = f_opendir(&dir, name);
    if (err) {
        printf("!! Cannot open %s (%s)\n", name, ERR[err]);
    } else {
        FILINFO file_info;
        err = f_readdir(&dir, &file_info);
        while (!err && file_info.fname[0]) {
            if (file_info.fattrib & AM_DIR) {
                printf("[%-12s]\n", file_info.fname);
            } else {
                printf("%-12s %10u\n", file_info.fname, file_info.fsize);
            }
            err = f_readdir(&dir, &file_info);
        }
        err = f_closedir(&dir);
        if (err) {
            printf("!! Cannot close %s (%s)\n", name, ERR[err]);
        }
    }
}

static void file_write(const char * name)
{
    FIL file;
    FRESULT err = f_open(&file, name, FA_CREATE_ALWAYS | FA_WRITE);
    if (err) {
        printf("!! Cannot open %s (%s)\n", name, ERR[err]);
    } else {
        static char buf[128];
        static char * end = buf + sizeof(buf);
        for (;;) {
            char * wp = buf;
            while (wp < end) {
                int c = getchar();
                if (c <= 4)
                    break;
                *wp++ = c;
                putchar(c);
            }
            if (wp > buf && !err) {
                UINT num_written;
                err = f_write(&file, buf, wp - buf, &num_written);
                if (err) {
                    printf("!! Cannot write to %s (%s)\n", name, ERR[err]);
                } else if (num_written < wp - buf) {
                    printf("!! Card is full\n");
                    err = FR_DENIED;
                }
            }
            if (wp < end) break;
        }
        err = f_close(&file);
        if (err) {
            printf("!! Cannot close %s (%s)\n", name, ERR[err]);
        }
    }
}

static void file_read(const char * name)
{
    FIL file;
    FRESULT err = f_open(&file, name, FA_READ);
    if (err) {
        printf("!! Cannot open %s (%s)\n", name, ERR[err]);
    } else {
        static char buf[128];
        UINT num_read;
        do {
            err = f_read(&file, buf, sizeof(buf), &num_read);
            if (err) {
                printf("!! Cannot read %s (%s)\n", name, ERR[err]);
                break;
            }
            const char * ptr = buf;
            const char * end = ptr + num_read;
            while (ptr < end) {
                putchar(*ptr++);
            }
        } while (num_read == sizeof(buf));

        err = f_close(&file);
        if (err) {
            printf("!! Cannot close %s (%s)\n", name, ERR[err]);
        }
    }
}

static inline void file_delete(const char * name)
{
    unlink(name);
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

static void main_task(void * name)
{
    static char cmd[32];
    static char cwd[256];
    cwd[0] = 0;

    for (;;) {
        printf("%s> ", cwd); fflush(stdout);
        if (!getline(sizeof(cmd), cmd)) continue;

        const char * arg = cmd + 2;
        while (*arg == ' ') ++arg;

        switch (cmd[0]) {
            case 'c': {
                // card commands
                switch (cmd[1]) {
                    case 'm': {
                        FRESULT err = mount();
                        if (!err) {
                            strcpy(cwd, "/");
                        }
                        break;
                    }
                    case 's': {
                        display_card_size();
                        break;
                    }
                }
                break;
            }
            case 'd': {
                // directories
                if (!*arg) {
                    arg = ".";
                }
                switch (cmd[1]) {
                    case 't': {
                        dir_tree(arg);
                        break;
                    }
                    case 'l': {
                        dir_list(arg);
                        break;
                    }
                    case 'c': {
                        FRESULT err = dir_change(arg);
                        if (!err) {
                            f_getcwd(cwd, sizeof(cwd));
                        }
                        break;
                    }
                    case 'm': {
                        dir_make(arg);
                        break;
                    }
                    case 'r': {
                        dir_remove(arg);
                        break;
                    }
                }
                break;
            }
            case 'f': {
                // files
                switch (cmd[1]) {
                    case 'w': {
                        file_write(arg);
                        break;
                    }
                    case 'r': {
                        file_read(arg);
                        break;
                    }
                    case 'd': {
                        file_delete(arg);
                        break;
                    }
                }
                break;
            }
        }
        // printf("# %u\n", (uint32_t)uxTaskGetStackHighWaterMark(NULL));
    }
}

#define TASK_STACK_SIZE 512

void user_init(void)
{
    uart_set_baud(0, 115200);
    hspi_init();
    xTaskCreate(main_task, "main", TASK_STACK_SIZE, NULL, tskIDLE_PRIORITY, NULL);
}