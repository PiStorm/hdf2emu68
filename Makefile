all:
	cc hdf2emu68.c fatfs/diskio.c  fatfs/ff.c -I fatfs -O3 -o hdf2emu68
