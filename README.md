# I2C_BBB
1) Communicate with Oled SSD1306, sensor SHT30.
2) I2C Function.
In the Linux kernel, the I2C subsystem is split into main layers:
+ I2C core function (framework provided by the kernel)
+ Adapter/driver function (implemented by hardware driver writers)

3) I2C Core

| Function                                                                 | Purpose                                                        |
| ------------------------------------------------------------------------ | -------------------------------------------------------------- |
| `i2c_transfer(struct i2c_adapter *adap, struct i2c_msg *msgs, int num)`  | Low-level generic transfer for multiple messages (read/write). |
| `i2c_master_send(struct i2c_client *client, const char *buf, int count)` | Send data to device (write only).                              |
| `i2c_master_recv(struct i2c_client *client, char *buf, int count)`       | Receive data from device (read only).                          |

  
4) SMBus protocol helper functions

| Function                                                                                      | Purpose                                  |
| --------------------------------------------------------------------------------------------- | ---------------------------------------- |
| `i2c_smbus_read_byte(struct i2c_client *client)`                                              | Read a single byte.                      |
| `i2c_smbus_write_byte(struct i2c_client *client, u8 value)`                                   | Write a single byte.                     |
| `i2c_smbus_read_byte_data(struct i2c_client *client, u8 reg)`                                 | Read a byte from a register.             |
| `i2c_smbus_write_byte_data(struct i2c_client *client, u8 reg, u8 value)`                      | Write a byte to a register.              |
| `i2c_smbus_read_word_data(struct i2c_client *client, u8 reg)`                                 | Read a 16-bit word from a register.      |
| `i2c_smbus_write_word_data(struct i2c_client *client, u8 reg, u16 value)`                     | Write a 16-bit word to a register.       |
| `i2c_smbus_read_i2c_block_data(struct i2c_client *client, u8 reg, u8 len, u8 *values)`        | Read block data starting at a register.  |
| `i2c_smbus_write_i2c_block_data(struct i2c_client *client, u8 reg, u8 len, const u8 *values)` | Write block data starting at a register. |


5) Device Management functions

| Function                                                                             | Purpose                                                      |
| ------------------------------------------------------------------------------------ | ------------------------------------------------------------ |
| `i2c_get_clientdata(struct i2c_client *client)`                                      | Retrieve driver-specific data pointer from the `i2c_client`. |
| `i2c_set_clientdata(struct i2c_client *client, void *data)`                          | Associate driver-specific data with the `i2c_client`.        |
| `i2c_new_client_device(struct i2c_adapter *adap, struct i2c_board_info const *info)` | Create/register a new I²C device dynamically.                |
| `i2c_unregister_device(struct i2c_client *client)`                                   | Remove/unregister an I²C device.                             |

6) Probe/Remove Hooks in I2C Client driver.
