/******************************************************************************
*  \file       oled_ssd_1306.c
*
*  \details    Creat Kernel Module communicate oled ssd1306
*
*  \author     PhamToan
*
*******************************************************************************/

#include <linux/module.h>
#include <linux/i2c.h>
#include <linux/delay.h>
#include <linux/of.h>

#define SSD1306_I2C_ADDR 0x3C

#define SSD1306_CMD  0x00
#define SSD1306_DATA 0x40

#define SSD1306_WIDTH   128
#define SSD1306_HEIGHT  64
#define SSD1306_PAGES   (SSD1306_HEIGHT / 8)

/* Minimal init sequence from SSD1306 datasheet */
static const u8 ssd1306_init_seq[] = {
    0xAE, // Display OFF
    0xD5, 0x80, // Set display clock div
    0xA8, 0x3F, // Set multiplex 64
    0xD3, 0x00, // Set display offset
    0x40,       // Set start line
    0x8D, 0x14, // Charge pump enable
    0x20, 0x00, // Memory mode
    0xA1,       // Seg remap
    0xC8,       // COM scan dec
    0xDA, 0x12, // Set COM pins
    0x81, 0xCF, // Set contrast
    0xD9, 0xF1, // Set precharge
    0xDB, 0x40, // Set Vcom detect
    0xA4,       // Display all on resume
    0xA6,       // Normal display
    0xAF        // Display ON
};

struct ssd1306_data {
    struct i2c_client *client;
};

static int ssd1306_write_cmd(struct i2c_client *client, u8 cmd)
{
    u8 buf[2] = {SSD1306_CMD, cmd};
    int ret = i2c_master_send(client, buf, 2);
    if (ret < 0)
        dev_err(&client->dev, "Failed to write command 0x%02x\n", cmd);
    return ret;
}

static int ssd1306_write_data(struct i2c_client *client, const u8 *data, size_t len)
{
    u8 *buf;
    int ret;

    buf = kmalloc(len + 1, GFP_KERNEL);
    if (!buf)
        return -ENOMEM;

    buf[0] = SSD1306_DATA;
    memcpy(&buf[1], data, len);

    ret = i2c_master_send(client, buf, len + 1);
    if (ret < 0)
        dev_err(&client->dev, "Failed to write data\n");

    kfree(buf);
    return ret;
}


static int ssd1306_set_address(struct ssd1306_data *ssd, u8 start_col, u8 end_col, u8 start_page, u8 end_page)
{
    int ret;

    /* Column address range */
    ret = ssd1306_write_cmd(ssd->client, 0x21); // Set Column Address
    if (ret < 0) return ret;
    ret = ssd1306_write_cmd(ssd->client, start_col);
    if (ret < 0) return ret;
    ret = ssd1306_write_cmd(ssd->client, end_col);
    if (ret < 0) return ret;

    /* Page address range */
    ret = ssd1306_write_cmd(ssd->client, 0x22); // Set Page Address
    if (ret < 0) return ret;
    ret = ssd1306_write_cmd(ssd->client, start_page);
    if (ret < 0) return ret;
    ret = ssd1306_write_cmd(ssd->client, end_page);
    return ret;
}


static int ssd1306_update_full(struct ssd1306_data *ssd, const u8 *buffer)
{
    int page, ret;

    /* Set full address range */
    ret = ssd1306_set_address(ssd, 0, SSD1306_WIDTH - 1, 0, SSD1306_PAGES - 1);
    if (ret < 0)
        return ret;

    /* Write each page */
    for (page = 0; page < SSD1306_PAGES; page++) {
        ret = ssd1306_write_data(ssd->client,
                                 &buffer[page * SSD1306_WIDTH],
                                 SSD1306_WIDTH);
        if (ret < 0)
            return ret;
    }
    return 0;
}


static int ssd1306_init_display(struct ssd1306_data *ssd)
{
    int i, ret;

    for (i = 0; i < ARRAY_SIZE(ssd1306_init_seq); i++) {
        ret = ssd1306_write_cmd(ssd->client, ssd1306_init_seq[i]);
        if (ret < 0)
            return ret;
        /* Optional small delay */
        usleep_range(1000, 2000);
    }
    return 0;
}

static int ssd1306_probe(struct i2c_client *client,const struct i2c_device_id *id)
{
    struct ssd1306_data *ssd;
    u8 *test_buf;
    int ret, i;

    dev_info(&client->dev, "SSD1306 OLED probed\n");

    ssd = devm_kzalloc(&client->dev, sizeof(*ssd), GFP_KERNEL);
    if (!ssd)
        return -ENOMEM;

    ssd->client = client;
    i2c_set_clientdata(client, ssd);

    /* Init display */
    ret = ssd1306_init_display(ssd);
    if (ret < 0) {
        dev_err(&client->dev, "Failed to initialize SSD1306\n");
        return ret;
    }

    /* Create test pattern */
    test_buf = devm_kzalloc(&client->dev, SSD1306_WIDTH * SSD1306_PAGES, GFP_KERNEL);
    if (!test_buf)
        return -ENOMEM;

    for (i = 0; i < SSD1306_WIDTH * SSD1306_PAGES; i++) {
        test_buf[i] = (i & 1) ? 0xAA : 0x55;  // Checkerboard pattern
    }

    /* Send test pattern */
    ssd1306_update_full(ssd, test_buf);

    dev_info(&client->dev, "SSD1306 initialized and test pattern sent\n");
    return 0;


}

static int ssd1306_remove(struct i2c_client *client)
{
    dev_info(&client->dev, "SSD1306 removed\n");
    return 0;
}

static const struct i2c_device_id ssd1306_id[] = {
    { "ssd1306", 0 },
    { }
};
MODULE_DEVICE_TABLE(i2c, ssd1306_id);

#ifdef CONFIG_OF
static const struct of_device_id ssd1306_of_match[] = {
    { .compatible = "solomon,ssd1306" },
    { }
};
MODULE_DEVICE_TABLE(of, ssd1306_of_match);
#endif

static struct i2c_driver ssd1306_driver = {
    .driver = {
        .name = "ssd1306",
        .of_match_table = of_match_ptr(ssd1306_of_match),
    },
    .probe = ssd1306_probe,
    .remove = ssd1306_remove,
    .id_table = ssd1306_id,
};

module_i2c_driver(ssd1306_driver);

MODULE_AUTHOR("Your Name");
MODULE_DESCRIPTION("Simple SSD1306 OLED I2C Driver");
MODULE_LICENSE("GPL");

