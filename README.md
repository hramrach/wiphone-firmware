### Cloning

```sh
git clone http://repo.mzjtechnology.com/wiphone/wiphone-firmware WiPhone
cd WiPhone
git submodule init
git submodule update
```

### Compiling

Open the file `WiPhone.ino` in the source folder with Arduino IDE and compile.

### Make a New Release

1. update CHANGELOG.txt
2. increment FIRMWARE_VERSION appropriately
3. define WIPHONE_PRODUCTION
4. turn off verbose debugging
5. record the commit version of the Arduino SDK builder library that was used
6. save the sdkconfig
7. compile
8. save the entire build directory (including the .elf file so we can do stack traces if needed)
9. update the ini file and put it and the binary file on the server, with the .ini file at a testing URL
10. do whatever testing we need
11. move the ini file to the main URL
