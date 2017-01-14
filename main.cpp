/**
 * Test software of the robotarmclick firmware.
 *
 * This code is intended to run on the LPC1768 board.
 * pin 9 (SDA) must be connected to RA1 of PIC12LF1552
 * pin 10 (SCL) must be connected to RA3 of PIC12LF1552
 *
 * Test list:
 * 1. write/read register 1-4
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
#define TEST_WRITE_READ_1_4_COUNT           (100)
#define TEST_WRITE_READ_1_4_RANDOM          (1)

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
    for (int i = 0; i < TEST_WRITE_READ_1_4_COUNT; ++i) {
        char reg_address, value;

#ifdef TEST_WRITE_READ_1_4_RANDOM
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
 * @brief Run all tests.
 *
 * @return 0 if all tests are successful, otherwise return the first test number
 * (greater or equal to 1) that failed.
 */
int run_tests(struct test *tests)
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
void flash_all_leds(void)
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
