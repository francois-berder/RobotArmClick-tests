/**
 * Test software of the robotarmclick firmware.
 *
 * This code is intended to run on the LPC1768 board.
 * pin 9 (SDA) must be connected to RA1 of PIC12LF1552
 * pin 10 (SCL) must be connected to RA3 of PIC12LF1552
 *
 * Test list:
 * 1. write/read register 1-4
 * 2. write/read register 0
 * 3. write to register 0-4 and read all the other.
 * 4. write register 5-255 and read registers 0-4
 * 5. write register 5-255 and i2c read
 */

#include "mbed.h"
#include <stdio.h>

#define SLAVE_ADDRESS       (0x3A)

I2C i2c(p9, p10);
DigitalOut led1(LED1);
DigitalOut led2(LED2);
DigitalOut led3(LED3);
DigitalOut led4(LED4);

struct test {
    const char *name;
    bool (*f)(void);
};

/** Tests configuration */
#define TEST_WRITE_READ_REG_1_4_COUNT           (100)
#define TEST_WRITE_READ_REG_1_4_RANDOM          (1)
#define TEST_WRITE_READ_REG_0_COUNT             (10)
#define TEST_WRITE_READ_REG_0_RANDOM            (1)
#define TEST_WRITE_REG_READ_ALL_COUNT           (500)
#define TEST_WRITE_INVALID_REG_READ_ALL_RANDOM  (1)
#define TEST_WRITE_INVALID_REG_READ_ALL_COUNT   (500)
#define TEST_WRITE_INVALID_REG_READ_ZERO_COUNT  (500)
#define TEST_WRITE_INVALID_REG_READ_ZERO_RANDOM (1)

/**
 * @brief Show a number in binary form using the 4 LED's present on the board.
 *
 * This function is used to display which test failed without having to look
 * at the UART output.
 * Notice that it only displays the lower half of the number.
 *
 * @param[in] c number to show
 */
static void led_show_number(char c)
{
    if (c & 0x1)
        led1 = 1;
    if (c & 0x2)
        led2 = 1;
    if (c & 0x4)
        led3 = 1;
    if (c & 0x8)
        led4 = 1;
}

static bool write_register(char addr, char val)
{
    char data[2] = {addr, val};

    return i2c.write(SLAVE_ADDRESS, data, sizeof(data)) == 0;
}

static bool read_register(char addr, char *val)
{
    return i2c.write(SLAVE_ADDRESS, &addr, 1) == 0
        && i2c.read(SLAVE_ADDRESS, val, 1) == 0;
}

static bool check_all_register(char *expected_values)
{
    for (int i = 0; i < 5; ++i) {
        char value = 0;
        if (!read_register(i, &value))
            return false;

        if (i == 0) {
            if ((value & 0x0F) != (expected_values[i] & 0x0F))
                return false;
        } else {
            if (value != expected_values[i])
                return false;
        }
    }

    return true;
}

/**
 * @brief Write and read values to register 1-4
 *
 * It writes values to a register (always in range 1-4).
 * It then reads back from the same register and compares the result. If the
 * value read is different from the value written, then the test is not
 * successful. Also, all i2c operations must be successful.
 *
 * @return True if successful, false otherwise
 */
static bool test_write_read_reg_1_4(void)
{
    for (int i = 0; i < TEST_WRITE_READ_REG_1_4_COUNT; ++i) {
        char reg_address, value;

#if TEST_WRITE_READ_REG_1_4_RANDOM
        reg_address = rand() % 4;
        value = rand();
#else
        reg_address = i % 4;
        value = i;
#endif
        char value_received = 0;
        if (!write_register(reg_address, value)
        ||  !read_register(reg_address, &value_received))
            return false;

        if (value != value_received) {
            fprintf(stderr, "Wrote %02X to register %d, but read %02X\n", value, reg_address, value_received);
            return false;
        }
    }

    return true;
}

/**
 * @brief Write and read values to register 0
 *
 * Only the lower half of register 0 can be written. This means that the value
 * written can be different from the value read.
 *
 * @return True if successful, false otherwise
 */
static bool test_write_read_reg_0(void)
{
    for (int i = 0; i < TEST_WRITE_READ_REG_0_COUNT; ++i) {
        char value;
#if TEST_WRITE_READ_REG_0_RANDOM
        value = rand();
#else
        value = i;
#endif
        char value_received = 0;
        if (!write_register(0, value)
        ||  !read_register(0, &value_received))
            return false;

        if ((value & 0x0F) != (value_received & 0x0F)) {
            fprintf(stderr, "Wrote %02X to register 0, but read %02X\n", value & 0x0F, value_received & 0x0F);
            return false;
        }
    }

    return true;
}

/**
 * @brief Check that a write to one register does not affect other registers
 *
 * @return True if successful, false otherwise
 */
static bool test_write_reg_read_all(void)
{
    char regs[5] = {0, 0, 0, 0, 0};

    /* Ensure all registers are set to 0 at the beginning */
    for (int i = 0; i < 5; ++i)
        if (!write_register(i, 0))
            return false;

    for (int i = 0; i < TEST_WRITE_REG_READ_ALL_COUNT; ++i) {
        int reg_address;

        reg_address = rand() % 5;
        regs[reg_address] = rand();

        if (!write_register(reg_address, regs[reg_address]))
            return false;

        if (!check_all_register(regs))
            return false;
    }

    return true;
}

/**
 * @brief Write to an invalid register (5-255) and read register 0-4
 *
 * Writing to an invalid register should not change the values of register 0-4.
 *
 * @return True if successful, false otherwise
 */
static bool test_write_invalid_reg_read_all(void)
{
    char regs[5];

    /* Write values to register 0-4 */
    for (int i = 0; i < 5; ++i) {
#if TEST_WRITE_INVALID_REG_READ_ALL_RANDOM
        regs[i] = rand();
#else
        regs[i] = i;
#endif
        if (!write_register(i, regs[i]))
            return false;
    }

    for (int i = 0; i < TEST_WRITE_INVALID_REG_READ_ALL_COUNT; ++i) {
        char reg_address, value;

#if TEST_WRITE_INVALID_REG_READ_ALL_RANDOM
        reg_address = (rand() % 250) + 5;
        value = rand();
#else
        reg_address = (i % 250) + 5;
        value = i;
#endif

        if (!write_register(reg_address, value))
            return false;

        if (!check_all_register(regs))
            return false;
    }

    return true;
}

/**
 * @Brief Write to an invalid register and perform read on I2C
 *
 * When writing to an invalid register is executed, any following read on the
 * I2C bus must return zeros.
 *
 * @return True if successful, false otherwise
 */
static bool test_write_invalid_reg_read_zero(void)
{
    for (int i = 0; i < TEST_WRITE_INVALID_REG_READ_ZERO_COUNT; ++i) {
        char reg_address, value;

#if TEST_WRITE_INVALID_REG_READ_ZERO_RANDOM
        reg_address = (rand() % 250) + 5;
        value = rand();
#else
        reg_address = (i % 250) + 5;
        value = i;
#endif

        if (!write_register(reg_address, value))
            return false;

        char value_received = 0xFF;
        if (i2c.read(SLAVE_ADDRESS, &value_received, sizeof(value_received)) != 0)
            return false;

        if (value_received != 0)
            return false;
    }

    return true;
}

/**
 * @brief Run all tests.
 *
 * @return 0 if all tests are successful, otherwise return the first test number
 * (greater or equal to 1) that failed.
 */
static int run_tests(struct test *tests)
{
    int n = 0;

    if (tests == NULL)
        return 0;

    while (tests[n].name != NULL && tests[n].f != NULL) {
        printf("test %d: %s: ", n + 1, tests[n].name);
        if (!tests[n].f()) {
            printf("FAIL\n");
            return n+1;
        }

        printf("PASS\n");
        ++n;
    }

    return 0;
}

/**
 * @brief Flash all LED's present on the board.
 *
 * This function is used to indicate that all tests were successfull without
 * having to look at the UART output.
 */
static void flash_all_leds(void)
{
    while (1) {
        led1 = led2 = led3 = led4 = 1;
        wait_ms(100);
        led1 = led2 = led3 = led4 = 0;
        wait_ms(100);
    }
}

int main()
{
    srand(time(NULL));
    i2c.frequency(400000);

    led1 = 0;
    led2 = 0;
    led3 = 0;
    led4 = 0;

    struct test tests[] = {
        {"write/read registers 1-4", test_write_read_reg_1_4},
        {"write/read register 0", test_write_read_reg_0},
        {"write reg/read all", test_write_reg_read_all},
        {"write invalid reg/read all", test_write_invalid_reg_read_all},
        {"write invalid reg/read zero", test_write_invalid_reg_read_zero},
        {NULL, NULL}
    };

    int ret = run_tests(tests);
    if (ret == 0) {
        printf("All tests passed.\n");
        flash_all_leds();
    } else {
        led_show_number(ret);
    }

    return 0;
}

