/* hdf2emu - image converter - (c) Claude Schwarz
*/
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#define SECTOR_SIZE 512
#define CHUNK_SIZE 1024

#if __MINGW32__
#include <sys/stat.h>
#endif

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

void createPartition(uint8_t* partition, uint8_t status, uint8_t type, uint64_t lba_start, uint64_t sectors) {
    PartitionEntry* part = (PartitionEntry*)partition;
    memset(partition, 0, 32);

    part->status = status;
    part->type = type;
    part->lba_start = lba_start;
    part->sectors = sectors;
}

void createMBR(uint8_t* mbr, uint64_t partition2SizeMB) {
    MBR* mbrData = (MBR*)mbr;
    memset(mbr, 0, SECTOR_SIZE);

    mbrData->signature = 0xAA55;

    // Set up the partition table
    createPartition(mbr + 0x01BE, 0x80, 0x0C, 0x800, (64 * 1024 * 1024) / SECTOR_SIZE);
    createPartition(mbr + 0x01CE, 0x00, 0x76, 0x800 + (64 * 1024 * 1024) / SECTOR_SIZE, (partition2SizeMB * 1024 * 1024) / SECTOR_SIZE);
}

void copyFileContent(FILE *source, FILE *destination, long long offset) {
    char buffer[CHUNK_SIZE];
    size_t read_size;
    long long total_written = 0;

    fseek(destination, offset, SEEK_SET);

    while ((read_size = fread(buffer, 1, sizeof(buffer), source)) > 0) {
        fwrite(buffer, 1, read_size, destination);
        total_written += read_size;
    }
}

int main(int argc, char* argv[]) {
    
    printf("hdf2emu68  -  (c) Claude Schwarz 24.10.2023\n");

    if (argc != 3) {
        printf("Creates a emu68 compatible SD image from an existing Amiga raw disk image\n");
        printf("\n");
        printf("Usage: %s <source_image> <output_image> \n", argv[0]);
        return 1;
    }
    char* sourceImage = argv[1];
    char* outputImage = argv[2];
   
    FILE* sourceFile = fopen(sourceImage, "rb");
    if (sourceFile == NULL) {
        perror("Error opening source image");
        return 1;
    }

#if __MINGW32__
    struct __stat64 st; 
    _stat64(sourceImage, &st);
    uint64_t partition2SizeMB = (st.st_size / (1024*1024) + 1);
#else
    fseek(sourceFile, 0, SEEK_END);
    uint64_t partition2SizeMB = (ftell(sourceFile) / (1024*1024) + 1);
    rewind(sourceFile);
#endif

    printf("Source Image Size: %ldMB\n",(long)partition2SizeMB);

    FILE* outputFile = fopen(outputImage, "wb");
    if (outputFile == NULL) {
        perror("Error opening output image");
        return 1;
    }

    uint8_t* diskImage = malloc(SECTOR_SIZE * 512);

    // Step 1: Create the MBR and partitions
    printf("Create MBR\n");
    createMBR(diskImage, partition2SizeMB);

    // Write MBR
    printf("Write MBR\n");
    fwrite(diskImage, 1, SECTOR_SIZE, outputFile);

    // Step 2: Write zeros for the content of both partitions
    printf("Zero the content of both partitions\n");
    memset(diskImage, 0, SECTOR_SIZE);

    uint64_t totalSectors = (64ULL * 1024 * 1024) / SECTOR_SIZE + (partition2SizeMB * 1024 * 1024) / SECTOR_SIZE;

    // Write zeros for both partitions
    for (uint64_t i = 0; i < totalSectors; i++) {
        fwrite(diskImage, 1, SECTOR_SIZE, outputFile);
    }
 
    long startSector = 0x800 + (64 * 1024 * 1024) / SECTOR_SIZE;

    printf("0x76 Partition start sector at: %ld\n",startSector );

    printf("Copy the source image file into the 0x76 type partition, this can take a while...\n");
    copyFileContent(sourceFile, outputFile, startSector * SECTOR_SIZE );

    printf("Disk image created successfully.\n");

#if __MINGW32__
    struct __stat64 stout; 
    _stat64(outputImage, &stout);
    uint64_t finalSizeMB = (stout.st_size / (1024*1024) + 1);
#else
    fseek(outputFile, 0, SEEK_END);
    uint64_t finalSizeMB = (ftell(outputFile) / (1024*1024)+1);
#endif
  
    printf("\n");
    printf("Final Disk image size: %ldMB\n",(long)finalSizeMB);
    printf("Make sure that the target SD has sufficent space or shrink the source HDF if needed.\n");
    printf("\n");
    printf("You can now write the image file to a SD card, don't forget to format the 64MB FAT32 partition.\n");
    printf("Copy the emu68 files and kickstart files to this FAT32 partition after formating.\n");

    free(diskImage);
    fclose(outputFile);
    fclose(sourceFile);

    return 0;
}
