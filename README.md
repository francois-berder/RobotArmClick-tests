## RobotArmClick - tests

This project is intended to be run on a LPC1768 board. 
The following pins must be connected:

| function | LCP1768 | PIC12LF1552 |
|:--------:|:-------:|:-----------:|
| SDA | pin 9 | RA3 |
| SCL | pin 10 | RA1|

Pull-up resistors must be added.

### Compiling & flashing

First, install the toolchain available [here](https://launchpad.net/gcc-arm-embedded/).
Then:
```
$ make
```

Copy robotarmclick-tests.bin located in ```.build``` directory (show hidden folder if you do not see this folder) to your board.

### Running the tests

Once the board is flashed, press the reset button. If all tests are successful,
all 4 LED's on the board will flash. If one the test fails, its number will be
displayed in binary form using the 4 LED's. For instance, if test 3 fails, the
two leftmost LED's are switched on.

You can also check the serial output to find out which test failed.
