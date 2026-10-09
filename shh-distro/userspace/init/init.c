#include <fcntl.h>
#include <stdio.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <linux/spi/spidev.h>
#include <stdint.h>
#include <dirent.h>
#include <errno.h>
#include <sys/mount.h>

static void list_dir(const char *path)
{
    DIR *dir = opendir(path);
    if (!dir) {
        perror(path);
        return;
    }

    printf("\n[%s]\n", path);
    struct dirent *entry;
    while ((entry = readdir(dir)) != NULL)
        printf("  %s\n", entry->d_name);

    closedir(dir);
}

static int spi_test(uint8_t addr)
{
    int fd = open("/dev/spidev0.0", O_RDWR);
    if (fd < 0)
    {
        perror("open spidev");
        return -1;
    }

    uint8_t mode = SPI_MODE_1;
    uint8_t bits = 8;
    uint32_t hz = 100000;

    if (ioctl(fd, SPI_IOC_WR_MODE, &mode) < 0 ||
        ioctl(fd, SPI_IOC_WR_BITS_PER_WORD, &bits) < 0 ||
        ioctl(fd, SPI_IOC_WR_MAX_SPEED_HZ, &hz) < 0)
    {
        perror("SPI setup");
        close(fd);
        return -1;
    }

    uint8_t header[] = {0x80 + addr, 1};
    uint8_t dummy[4] = {0};
    uint8_t rx = 0;

    struct spi_ioc_transfer tr = {
        .tx_buf = (uintptr_t)header,
        .len = 2,
        .speed_hz = hz,
        .bits_per_word = bits,
    };
    if (ioctl(fd, SPI_IOC_MESSAGE(1), &tr) != 2)
        goto fail;

    usleep(1000);
    tr = (struct spi_ioc_transfer) {
        .tx_buf = (uintptr_t)dummy,
        .rx_buf = (uintptr_t)&rx,
        .len = 1,
        .speed_hz = hz,
        .bits_per_word = bits,
    };
    if (ioctl(fd, SPI_IOC_MESSAGE(1), &tr) != 1)
        goto fail;
    usleep(1000);
    printf("SPI_RX: %02X \n", rx);
    close(fd);
    return 0;

fail:
    perror("SPI Transfer");
    close(fd);
    return -1;
}

int main(void)
{
    if (mount("sysfs", "/sys", "sysfs", 0, NULL) < 0 &&
        errno != EBUSY)
    {
        perror("mount sysfs");
    }
    setvbuf(stdout, NULL, _IONBF, 0);

    printf("\r\nHello, world from SHH init!\r\n");

    //list_dir("/dev");
    //list_dir("/sys/class/spi_master");
    //list_dir("/sys/bus/spi/devices");
    //list_dir("/sys/class/spidev");

    unsigned int count = 0;
    for (;;)
    {
        spi_test((uint8_t)count);
        count ++;
        count = count & 0x0f;
        sleep(1);
        printf("%d\n\r", count);
    }
    return 0;
}
