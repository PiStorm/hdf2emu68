# hdf2emu68
Harddisk image converter tool for EMU68

Usage: hdf2emu68 <source_image>

On Windows you can just drag&drop your existing .HDF / disk image to the hdf2emu68.exe.
The resulting EMU68 compatible image will be named "emu68_converted.img"

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
