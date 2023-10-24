# hdf2emu68
Harddisk image converter tool for emu68

Usage: hdf2emu68 <source_image> <output_image>

A example :
```
./hdf2emu68 my_existing_amiga_harddisk.img output.img
hdf2emu68  -  (c) Claude Schwarz 24.10.2023
Source Image Size: 28673MB
Create MBR
Write MBR
Zero the content of both partitions
0x76 Partition start sector at: 133120
Copy the source image file into the 0x76 type partition, this can take a while...
Disk image created successfully.

Final Disk image size: 28738MB
Make sure that the target SD has sufficent space or shrink the source HDF if needed.

You can now write the image file to a SD card, don't forget to format the 64MB FAT32 partition.
Copy the emu68 files and kickstart files to this FAT32 partition after formating.
```

Compiles for Windows 64bit and Linux 64bit

As I'm too lazy for writing a Makefile, a example how to compile for Windows on Linux :
```
x86_64-w64-mingw32-gcc hdf2emu68.c -O3 -o hdf2emu68.exe
```

For Linux :
```
gcc hdf2emu68.c -O3 -o hdf2emu68.exe
```
