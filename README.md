# hdf2emu68
Harddisk image converter tool for EMU68

Usage: hdf2emu68 <source_image> [options]

-fatsize x  x is in MB, higher than 32 (default: 64)

-fatdir x   x is the path of files to write to fat partition

-output x   x is the filename (with path) of output file
   
On Windows you can just drag&drop your existing .HDF / disk image to the hdf2emu68.exe.

The resulting EMU68 compatible image will be named "emu68_converted.img" unless you specify -output option

Optionally you can specify fat32 partition size (default is 64Mb). Minimum is 33Mb since lower than that does not work (will not boot on rPi).

Also optionally you can fill the fat32 partition with files and directories from -fatdir path.

Don't forget to copy your EMU68 and Kickstart file to the FAT32 partition after you 
have written the final image to your SD card. Suitable tools for writing the image to
a SD are for example Win32diskimager, Etcher or dd


Compiles for Windows 64bit and Linux 64bit

A example how to compile for Windows on Linux :
```
make -f Makefile.win
```

For Linux :
```
make
```
