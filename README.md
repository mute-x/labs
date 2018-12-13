# ucu-pok

STM32F4 project for UCU POK course labs.

Can be opened as project for Qt Creator.

To build, pass `COMPILER_DIR` to `make`, set to path to arm-none-eabi-gcc, like this:
```sh
$ make COMPILER_DIR=~/opt/gcc-arm-none-eabi-7-2017-q4-major/bin
```
Supported `make` targets:
- `build`
- `debug` (same as `build`, but with debug flags)
- `clean`
- `flash`

![picture alt](https://github.com/poc-team/labs/blob/master/longdistznce-2.png "Title is optional")
