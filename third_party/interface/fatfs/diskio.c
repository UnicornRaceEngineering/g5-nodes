/*-----------------------------------------------------------------------*/
/* Low level disk I/O module skeleton for FatFs     (C)ChaN, 2014        */
/*-----------------------------------------------------------------------*/
/* If a working storage control module is available, it should be        */
/* attached to the FatFs via a glue function rather than modifying it.   */
/* This is an example of glue functions to attach various exsisting      */
/* storage control modules to the FatFs module with a defined API.       */
/*-----------------------------------------------------------------------*/

#include <fatfs/diskio.h>		/* FatFs lower layer API */
#include <mmc_sdcard.h>

/* Definitions of physical drive number for each drive */

volatile static DSTATUS status = STA_NOINIT;	/* Disk status */

// TODO
// This is a stub. We must get the correct time from the RTC
// DWORD get_fattime(void) {
// 	return 0xFFFF;
// }


/*-----------------------------------------------------------------------*/
/* Get Drive Status                                                      */
/*-----------------------------------------------------------------------*/

DSTATUS disk_status (
	BYTE pdrv		/* Physical drive nmuber to identify the drive */
)
{
	if (pdrv != 0) return STA_NOINIT;
	return status;
}



/*-----------------------------------------------------------------------*/
/* Inidialize a Drive                                                    */
/*-----------------------------------------------------------------------*/

DSTATUS disk_initialize (
	BYTE pdrv				/* Physical drive nmuber to identify the drive */
)
{
	if (pdrv != 0) return STA_NOINIT;
	if (sd_init() != 0) return STA_NOINIT;
	status &= ~STA_NOINIT;

	return status;
}



/*-----------------------------------------------------------------------*/
/* Read Sector(s)                                                        */
/*-----------------------------------------------------------------------*/

DRESULT disk_read (
	BYTE pdrv,		/* Physical drive nmuber to identify the drive */
	BYTE *buff,		/* Data buffer to store read data */
	DWORD sector,	/* Sector address in LBA */
	UINT count		/* Number of sectors to read */
)
{
	if (pdrv != 0 || !count) return RES_PARERR;
	if (status & STA_NOINIT) return RES_NOTRDY;
	return (sd_read(buff, sector, count) != 0) ? RES_ERROR: RES_OK;
}



/*-----------------------------------------------------------------------*/
/* Write Sector(s)                                                       */
/*-----------------------------------------------------------------------*/

#if _USE_WRITE
DRESULT disk_write (
	BYTE pdrv,			/* Physical drive nmuber to identify the drive */
	const BYTE *buff,	/* Data to be written */
	DWORD sector,		/* Sector address in LBA */
	UINT count			/* Number of sectors to write */
)
{
	if (pdrv || !count) return RES_PARERR;
	if (status & STA_NOINIT) return RES_NOTRDY;
	if (status & STA_PROTECT) return RES_WRPRT;
	return (sd_write(buff, sector, count) != 0) ? RES_ERROR : RES_OK;
}
#endif


/*-----------------------------------------------------------------------*/
/* Miscellaneous Functions                                               */
/*-----------------------------------------------------------------------*/

#if _USE_IOCTL
DRESULT disk_ioctl (
	BYTE pdrv,		/* Physical drive nmuber (0..) */
	BYTE cmd,		/* Control code */
	void *buff		/* Buffer to send/receive control data */
)
{
	if (pdrv != 0) return RES_PARERR;
	if (status & STA_NOINIT) return RES_NOTRDY;

	switch(cmd) {
		case CTRL_SYNC:
			if (sd_sync() != 0) return RES_ERROR;
			break;
#if 0
		case GET_SECTOR_COUNT:
			if (get_memory_capacity((uint32_t*)buff) != 0) return RES_ERROR;
			break;

		case GET_BLOCK_SIZE:
			*(DWORD*)buff = SD_BLOCKSIZE; //128;
			break;
#endif
		default: return RES_PARERR;
	}

	return RES_OK;
}
#endif
