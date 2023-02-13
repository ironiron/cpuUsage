# About

This program reads /proc/stat file and calculate cpu usage in percentage for each of the cores accessible for this process.

## Building
To build and run the aplication type
```sh
 ./run.sh
```
Other options - for running unit tests type:
```sh
./run_valgrind.sh
```
for memory leak analysis:
```sh
./run_valgrind.sh
```
**NOTE** Clang has problems with Valgrind (tested on 14.0.0) It is strongly advised to compile with gcc.

**NOTE 2** Tested on Ubuntu 22.04 and Debian 11

### Author
Rafał Mazurkiewicz