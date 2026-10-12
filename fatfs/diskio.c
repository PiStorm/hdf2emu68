/*-----------------------------------------------------------------------*/
/* Low level disk I/O module SKELETON for FatFs     (C)ChaN, 2019        */
/*-----------------------------------------------------------------------*/
/* If a working storage control module is available, it should be        */
/* attached to the FatFs via a glue function rather than modifying it.   */
/* This is an example of glue functions to attach various exsisting      */
/* storage control modules to the FatFs module with a defined API.       */
/*-----------------------------------------------------------------------*/



#include "ff.h"     /* Obtains integer types */
#include "diskio.h"
#include <fcntl.h>
#include <stdio.h>
#include <sys/types.h>
#include <unistd.h>


FILE *fno = {NULL};

/*-----------------------------------------------------------------------*/
/* Get Drive Status                                                      */
/*-----------------------------------------------------------------------*/

DSTATUS disk_status(BYTE pdrv /* Physical drive nmuber to identify the drive */
) {
  DSTATUS stat;
  return 0;
}

/*-----------------------------------------------------------------------*/
/* Inidialize a Drive                                                    */
/*-----------------------------------------------------------------------*/

DSTATUS disk_initialize(BYTE pdrv /* Physical drive nmuber to identify the drive */
) {
  DSTATUS stat;

  if (fno != NULL)
    return 0;

  fno = fopen(OutputFilename, "r+b");
  if (!fno) {
    fno = fopen(OutputFilename, "w+b");
  }

  if (fno != NULL)
    return 0;

  return STA_NOINIT;
}

/*-----------------------------------------------------------------------*/
/* Read Sector(s)                                                        */
/*-----------------------------------------------------------------------*/

DRESULT disk_read(BYTE pdrv,  /* Physical drive nmuber to identify the drive */
                  BYTE *buff, /* Data buffer to store read data */
                  LBA_t sector, /* Start sector in LBA */
                  UINT count    /* Number of sectors to read */
) {
  DRESULT res;
  int result;

  if (fseek(fno, sector * 512, SEEK_SET))
    return RES_ERROR;

  result = fread(buff, 512, count, fno);

  if (result != count)
    return RES_ERROR;

  return RES_OK;
}

/*-----------------------------------------------------------------------*/
/* Write Sector(s)                                                       */
/*-----------------------------------------------------------------------*/

#if FF_FS_READONLY == 0

DRESULT disk_write(BYTE pdrv, /* Physical drive nmuber to identify the drive */
                   const BYTE *buff, /* Data to be written */
                   LBA_t sector,     /* Start sector in LBA */
                   UINT count        /* Number of sectors to write */
) {
  DRESULT res;
  int result;
  if (fseek(fno, sector * 512, SEEK_SET))
    return RES_ERROR;

  result = fwrite(buff, 512, count, fno);

  if (result != count)
    return RES_ERROR;

  return RES_OK;
}

#endif

/*-----------------------------------------------------------------------*/
/* Miscellaneous Functions                                               */
/*-----------------------------------------------------------------------*/

DRESULT disk_ioctl(BYTE pdrv, /* Physical drive nmuber (0..) */
                   BYTE cmd,  /* Control code */
                   void *buff /* Buffer to send/receive control data */
) {
  DRESULT res;
  int result;

  switch (cmd) {
  case CTRL_SYNC:
    return RES_OK;
    break;

  case GET_SECTOR_COUNT: {
    *((DWORD *)buff) = (133120 - 1) * FF_MAX_SS; // TODO CONSTANT FOR 64M;
    return RES_OK;
  } break;

  case GET_SECTOR_SIZE: {
    *((WORD *)buff) = FF_MAX_SS;
    return RES_OK;
  } break;

  case GET_BLOCK_SIZE: {
    WORD *ss = (WORD *)buff;
    *ss = FF_MAX_SS;
    return RES_OK;
  } break;

  default:
    return RES_PARERR;
    break;
  }
}
