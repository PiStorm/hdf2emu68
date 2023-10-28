/*
 * hdf2emu - image converter - (c) Claude Schwarz
 */

#include "fatfs/ff.h"
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#if __MINGW32__
#include <sys/stat.h>
#endif

#define SECTOR_SIZE 512
#define CHUNK_SIZE SECTOR_SIZE * 4
//#define FCHK(a)   { FRESULT res = a; printf("call %s res %d %s\n", #a, res,fatfs_error_to_string(res)); };
#define FCHK(a) a;

PARTITION VolToPart[] = {{0, 0}, {1, 0}, {2, 0}, {3, 0}, {4, 0},
                         {5, 0}, {6, 0}, {7, 0}, {8, 0}, {9, 0}};

static const char *fatfs_error_to_string(FRESULT err) {
  switch (err) {
  case FR_OK:
    return "Succeeded";
  case FR_DISK_ERR:
    return "FATFS: A hard error occurred in the low level disk I/O layer";
  case FR_INT_ERR:
    return "FATFS: Assertion failed (check for corruption)";
  case FR_NOT_READY:
    return "FATFS: The physical drive cannot work";
  case FR_NO_FILE:
    return "FATFS: Could not find the file";
  case FR_NO_PATH:
    return "FATFS: Could not find the path";
  case FR_INVALID_NAME:
    return "FATFS: The path name format is invalid";
  case FR_DENIED:
    return "FATFS: Access denied due to prohibited access or directory full";
  case FR_EXIST:
    return "FATFS: Destination file already exists";
  case FR_INVALID_OBJECT:
    return "FATFS: The file/directory object is invalid";
  case FR_WRITE_PROTECTED:
    return "FATFS: The physical drive is write protected";
  case FR_INVALID_DRIVE:
    return "FATFS: The logical drive number is invalid";
  case FR_NOT_ENABLED:
    return "FATFS: The volume has no work area";
  case FR_NO_FILESYSTEM:
    return "FATFS: There is no valid FAT volume";
  case FR_MKFS_ABORTED:
    return "FATFS: f_mkfs() aborted due to a parameter error. Try adjusting "
           "the partition size.";
  case FR_TIMEOUT:
    return "FATFS: Could not get a grant to access the volume within defined "
           "period";
  case FR_LOCKED:
    return "FATFS: The operation is rejected according to the file sharing "
           "policy";
  case FR_NOT_ENOUGH_CORE:
    return "FATFS: LFN working buffer could not be allocated";
  case FR_TOO_MANY_OPEN_FILES:
    return "FATFS: Number of open files > _FS_SHARE";
  default:
  case FR_INVALID_PARAMETER:
    return "FATFS: Invalid parameter";
  }
}

typedef struct {
  uint8_t bootstrap[446];
  uint8_t part1[16];
  uint8_t part2[16];
  uint8_t part3[16];
  uint8_t part4[16];
  uint16_t signature;
} MBR;

typedef struct {
  uint8_t status;
  uint8_t chs_start[3];
  uint8_t type;
  uint8_t chs_end[3];
  uint32_t lba_start;
  uint32_t sectors;
} PartitionEntry;

void createFAT32(void) {
  FATFS fs;
  FRESULT fr;
  FIL fsrc;
  int w;

  VolToPart[0].pt = 1; // workaround to prevent FatFS rebuilding the MBR
  uint8_t fatfs_buffer[1024 * 4];
  static const MKFS_PARM fmt_opt = {FM_FAT32 | FM_SFD, 0, 0, 1, 0};
  FCHK(f_mkfs("0:", &fmt_opt, fatfs_buffer, sizeof(fatfs_buffer)));
  FCHK(f_mount(&fs, "0:", 0));
  FCHK(f_setlabel("0:EMU68BOOT"));
  FCHK(f_open(&fsrc, "README.TXT", FA_CREATE_ALWAYS | FA_READ | FA_WRITE));
  FCHK(f_write(&fsrc,
               "Copy the EMU68 and Kickstart files into the root directory\n",
               59, &w));
  FCHK(f_sync(&fsrc));
  FCHK(f_close(&fsrc));
  FCHK(f_unmount("0:"));
}

void createPartition(uint8_t *partition, uint8_t status, uint8_t type,
                     uint64_t lba_start, uint64_t sectors) {
  PartitionEntry *part = (PartitionEntry *)partition;
  memset(partition, 0, 32);

  part->status = status;
  part->type = type;
  part->lba_start = lba_start;
  part->sectors = sectors;
}

void createMBR(uint8_t *mbr, uint64_t partition2SizeMB) {
  MBR *mbrData = (MBR *)mbr;
  memset(mbr, 0, SECTOR_SIZE);

  mbrData->signature = 0xAA55;

  // Set up the partition table
  createPartition(mbr + 0x01BE, 0x80, 0x0C, 0x800,
                  (64 * 1024 * 1024) / SECTOR_SIZE);
  createPartition(mbr + 0x01CE, 0x00, 0x76,
                  0x800 + (64 * 1024 * 1024) / SECTOR_SIZE,
                  (partition2SizeMB * 1024 * 1024) / SECTOR_SIZE);
}

void copyFileContent(FILE *source, FILE *destination, long long offset) {
  char buffer[CHUNK_SIZE];
  size_t read_size;
  long long total_written = 0;

  fseek(destination, offset, SEEK_SET);
  printf("Progress: .");
  fflush(stdout);

  while ((read_size = fread(buffer, 1, sizeof(buffer), source)) > 0) {
    fwrite(buffer, 1, read_size, destination);
    total_written += read_size;
    if (total_written % 1000000 == 0 && total_written != 0) {
      printf(".");
      fflush(stdout);
    }
  }
  printf(" ready\n");
}

int main(int argc, char *argv[]) {

  printf("hdf2emu68  -  (c) Claude Schwarz 28.10.2023\n");

  if (argc != 2) {
    printf("Creates a emu68 compatible SD image from an existing Amiga raw "
           "disk image\n");
    printf("\n");
    printf("Usage: %s <source_image>\n", argv[0]);
    return 1;
  }
  char *sourceImage = argv[1];
  char *outputImage = "emu68_converted.img";

  FILE *sourceFile = fopen(sourceImage, "rb");
  if (sourceFile == NULL) {
    perror("Error opening source image");
    return 1;
  }

#if __MINGW32__
  struct __stat64 st;
  _stat64(sourceImage, &st);
  uint64_t partition2SizeMB = (st.st_size / (1024 * 1024) + 1);
#else
  fseek(sourceFile, 0, SEEK_END);
  uint64_t partition2SizeMB = (ftell(sourceFile) / (1024 * 1024) + 1);
  rewind(sourceFile);
#endif

  printf("Source Image Size: %ldMB\n", (long)partition2SizeMB);

  FILE *outputFile = fopen(outputImage, "wb");
  if (outputFile == NULL) {
    perror("Error opening output image");
    return 1;
  }

  uint8_t *diskImage = malloc(SECTOR_SIZE * 512);

  // Create the MBR and partitions
  printf("Create MBR\n");
  createMBR(diskImage, partition2SizeMB);

  // Write MBR
  printf("Write MBR\n");
  fwrite(diskImage, 1, SECTOR_SIZE, outputFile);

  /*
      // Write zeros for the content of both partitions
      printf("Zero the content of both partitions\n");
      memset(diskImage, 0, SECTOR_SIZE);

      uint64_t totalSectors = (64ULL * 1024 * 1024) / SECTOR_SIZE +
     (partition2SizeMB * 1024 * 1024) / SECTOR_SIZE;

      // Write zeros for both partitions
      for (uint64_t i = 0; i < totalSectors; i++) {
          fwrite(diskImage, 1, SECTOR_SIZE, outputFile);
      }
  */
  long startSector = 0x800 + (64 * 1024 * 1024) / SECTOR_SIZE;

  printf("0x76 Partition start sector at: %ld\n", startSector);

  printf("Copy the source image file into the 0x76 type partition, this can "
         "take a while...\n");
  copyFileContent(sourceFile, outputFile, startSector * SECTOR_SIZE);

#if __MINGW32__
  struct __stat64 stout;
  _stat64(outputImage, &stout);
  uint64_t finalSizeMB = (stout.st_size / (1024 * 1024) + 1);
#else
  fseek(outputFile, 0, SEEK_END);
  uint64_t finalSizeMB = (ftell(outputFile) / (1024 * 1024) + 1);
#endif

  free(diskImage);
  fclose(outputFile);
  fclose(sourceFile);

  printf("Format the FAT32 partition\n");
  createFAT32();
  printf("Image creation finished\n");

  printf("\n");
  printf("Final Image size: %ldMB\n", (long)finalSizeMB);
  printf("Make sure that the target SD has sufficent space or shrink the "
         "source HDF if needed.\n");
  printf("\n");
  printf("You can now write the image file to a SD card (with "
         "win32diskimager,etcher,dd..),\n");
  printf("copy the EMU68 files and Kickstart file to the SD after writing the "
         "image to it.\n");

  return 0;
}
