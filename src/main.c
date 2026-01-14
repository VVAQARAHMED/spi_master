#include <zephyr/kernel.h>
#include <zephyr/drivers/spi.h>
#include <zephyr/drivers/gpio.h>
#include <zephyr/devicetree.h>
#include <stdio.h>
#include <string.h>

/* SPI3 device from overlay */
#define SPI_NODE DT_NODELABEL(my_spi_master)
static const struct spi_cs_control cs_ctrl = {
    .gpio = GPIO_DT_SPEC_GET(SPI_NODE, cs_gpios),
    .delay = 10,  // No delay between CS and data
    .cs_is_gpio = true,
};

static struct spi_config spi_cfg = {
    .frequency =  312500, // 500 kHz
    .operation = SPI_WORD_SET(8) | SPI_TRANSFER_MSB  | SPI_OP_MODE_MASTER,
    .slave = 0,
    .cs = cs_ctrl,  // CS control struct
};

void main(void)
{
    const struct device *spi_dev = DEVICE_DT_GET(SPI_NODE);

    if (!device_is_ready(spi_dev)) {
        printf("SPI device not ready!\n");
        return;
    }

    // Don't manually configure CS - let SPI driver handle it
    printf("SPI Master initialized.\n");

    // Debug: Print spi_cfg configuration
    printf("=== SPI Configuration ===\n");
    printf("Frequency: %u Hz\n", spi_cfg.frequency);
    printf("Operation: 0x%08X\n", spi_cfg.operation);
    printf("Slave: %u\n", spi_cfg.slave);
    printf("CS GPIO port: %p\n", spi_cfg.cs.gpio.port);
    printf("CS GPIO pin: %u\n", spi_cfg.cs.gpio.pin);
    printf("CS GPIO flags: 0x%04X\n", spi_cfg.cs.gpio.dt_flags);
    printf("CS delay: %u\n", spi_cfg.cs.delay);
    printf("========================\n\n");

    #define MSG_BUF_SIZE 32
    uint8_t tx_buf[MSG_BUF_SIZE];
    uint8_t rx_buf[MSG_BUF_SIZE];
    uint32_t msg_number = 0;

    struct spi_buf tx = {
        .buf = tx_buf,
        .len = 0  // Will be set per message
    };

    struct spi_buf rx = {
        .buf = rx_buf,
        .len = 0  // Will be set per message
    };

    struct spi_buf_set tx_set = { .buffers = &tx, .count = 1 };
    struct spi_buf_set rx_set = { .buffers = &rx, .count = 1 };

    printf("Starting SPI loop...\n");
    k_sleep(K_SECONDS(2));   // allow SPIS to initialize

    while (1) {
        msg_number++;

       printk("TX HEX (%d bytes): ", MSG_BUF_SIZE);
  for (int i = 0; i < MSG_BUF_SIZE; i++) {  // ← Print all 32 bytes
      printk("%02X ", tx_buf[i]);
  }
  printk("\n");

  //nd make sure you force the transmission length:
  int msg_len = snprintf((char *)tx_buf, MSG_BUF_SIZE,
                         "hello from master %u", msg_number);

  // Pad remaining bytes
  if (msg_len < MSG_BUF_SIZE) {
      memset(tx_buf + msg_len, 0xAA, MSG_BUF_SIZE - msg_len);
  }

  // Print all 32 bytes in hex
  printk("TX HEX (%d bytes): ", MSG_BUF_SIZE);
  for (int i = 0; i < MSG_BUF_SIZE; i++) {
      printk("%02X ", tx_buf[i]);
  }
  printk("\n");

  // IMPORTANT: Force transmission to 32 bytes
  msg_len = MSG_BUF_SIZE;
printk("Prepared message: [%s] (len=%d)\n", tx_buf, msg_len);
        // Set buffer lengths
        tx.len = msg_len;
        rx.len = msg_len;  // Expect same length response

        // Clear RX buffer
        memset(rx_buf, 0, MSG_BUF_SIZE);

        printf("[%lld ms] Sending [%s]... ", k_uptime_get(), tx_buf);

        // Debug: print hex dump of tx_buf
        printf("\nTX HEX: ");
        for (int i = 0; i < msg_len && i < 10; i++) {
            printf("%02X ", tx_buf[i]);
        }
        printf("\n");
        // Driver handles CS automatically via cs_ctrl
       int ret = spi_transceive(spi_dev, &spi_cfg, &tx_set, &rx_set);

        if (ret == 0) {
            // Null-terminate received data for safe printing
            rx_buf[msg_len] = '\0';
            printf("OK, RX = [%s]\n", rx_buf);
        } else {
            printf("SPI ERROR %d\n", ret);
        }

        k_sleep(K_MSEC(1000));
    }
}
