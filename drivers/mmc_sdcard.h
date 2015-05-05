/*
The MIT License (MIT)

Copyright (c) 2014 UnicornRaceEngineering

Permission is hereby granted, free of charge, to any person obtaining a copy of
this software and associated documentation files (the "Software"), to deal in
the Software without restriction, including without limitation the rights to
use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of
the Software, and to permit persons to whom the Software is furnished to do so,
subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS
FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR
COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER
IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 */

/**
 * @file mmc_sdcard.c
 * Low level driver for reading and writing SD cards using SPI. Information on
 * the specification can be found at:
 * https://www.sdcard.org/downloads/pls/simplified_specs/
 * In the "Physical Layer Simplified Specification" document.
 */

#ifndef MMC_SDCARD_H
#define MMC_SDCARD_H

#include <stdint.h>
#include <stddef.h>
#include <utils.h>

#define SD_BLOCKSIZE 512

#define CSD_SIZE	16 // csd size in bytes (128bits)
#define CID_SIZE	16 // cid size in bytes (128bits)

#define CSD_STRUCTURE(csd)				EXTRACT_AT_INDEX(csd, CSD_SIZE, 126, 2)

#define CSD_V1_TAAC(csd)				EXTRACT_AT_INDEX(csd, CSD_SIZE, 112, 8)
#define CSD_V1_NSAC(csd)				EXTRACT_AT_INDEX(csd, CSD_SIZE, 104, 8)
#define CSD_V1_TRAN_SPEED(csd)			EXTRACT_AT_INDEX(csd, CSD_SIZE, 96, 8)
#define CSD_V1_CCC(csd) 				MERGE_BYTE(EXTRACT_AT_INDEX(csd, CSD_SIZE, 84+8, 12-8), EXTRACT_AT_INDEX(csd, CSD_SIZE, 84, 8))
#define CSD_V1_READ_BL_LEN(csd) 		EXTRACT_AT_INDEX(csd, CSD_SIZE, 80, 4)
#define CSD_V1_READ_BL_PARTIAL(csd) 	EXTRACT_AT_INDEX(csd, CSD_SIZE, 79, 1)
#define CSD_V1_WRITE_BLK_MISALIGN(csd) 	EXTRACT_AT_INDEX(csd, CSD_SIZE, 78, 1)
#define CSD_V1_READ_BLK_MISALIGN(csd) 	EXTRACT_AT_INDEX(csd, CSD_SIZE, 77, 1)
#define CSD_V1_DSR_IMP(csd) 			EXTRACT_AT_INDEX(csd, CSD_SIZE, 76, 1)
#define CSD_V1_C_SIZE(csd) 				MERGE_BYTE(EXTRACT_AT_INDEX(csd, CSD_SIZE, 62+8, 12-8), EXTRACT_AT_INDEX(csd, CSD_SIZE, 62, 8))
#define CSD_V1_VDD_R_CURR_MIN(csd) 		EXTRACT_AT_INDEX(csd, CSD_SIZE, 59, 3)
#define CSD_V1_VDD_R_CURR_MAX(csd) 		EXTRACT_AT_INDEX(csd, CSD_SIZE, 56, 3)
#define CSD_V1_VDD_W_CURR_MIN(csd) 		EXTRACT_AT_INDEX(csd, CSD_SIZE, 53, 3)
#define CSD_V1_VDD_W_CURR_MAX(csd) 		EXTRACT_AT_INDEX(csd, CSD_SIZE, 50, 3)
#define CSD_V1_C_SIZE_MULT(csd) 		EXTRACT_AT_INDEX(csd, CSD_SIZE, 47, 3)
#define CSD_V1_ERASE_BLK_EN(csd) 		EXTRACT_AT_INDEX(csd, CSD_SIZE, 46, 1)
#define CSD_V1_SECTOR_SIZE(csd) 		EXTRACT_AT_INDEX(csd, CSD_SIZE, 39, 7)
#define CSD_V1_WP_GRP_SIZE(csd) 		EXTRACT_AT_INDEX(csd, CSD_SIZE, 32, 7)
#define CSD_V1_WP_GRP_ENABLE(csd) 		EXTRACT_AT_INDEX(csd, CSD_SIZE, 31, 1)
#define CSD_V1_R2W_FACTOR(csd) 			EXTRACT_AT_INDEX(csd, CSD_SIZE, 26, 3)
#define CSD_V1_WRITE_BL_LEN(csd) 		EXTRACT_AT_INDEX(csd, CSD_SIZE, 22, 4)
#define CSD_V1_WRITE_BL_PARTIAL(csd) 	EXTRACT_AT_INDEX(csd, CSD_SIZE, 21, 1)
#define CSD_V1_FILE_FORMAT_GRP(csd) 	EXTRACT_AT_INDEX(csd, CSD_SIZE, 15, 1)
#define CSD_V1_COPY(csd) 				EXTRACT_AT_INDEX(csd, CSD_SIZE, 14, 1)
#define CSD_V1_PERM_WRITE_PROTECT(csd) 	EXTRACT_AT_INDEX(csd, CSD_SIZE, 13, 1)
#define CSD_V1_TMP_WRITE_PROTECT(csd) 	EXTRACT_AT_INDEX(csd, CSD_SIZE, 12, 1)
#define CSD_V1_FILE_FORMAT(csd) 		EXTRACT_AT_INDEX(csd, CSD_SIZE, 10, 2)
#define CSD_V1_CRC(csd) 				EXTRACT_AT_INDEX(csd, CSD_SIZE, 1, 7)

#define CSD_V2_TAAC(csd)				EXTRACT_AT_INDEX(csd, CSD_SIZE, 112, 8)
#define CSD_V2_NSAC(csd)				EXTRACT_AT_INDEX(csd, CSD_SIZE, 104, 8)
#define CSD_V2_TRAN_SPEED(csd)			EXTRACT_AT_INDEX(csd, CSD_SIZE, 96, 8)
#define CSD_V2_CCC(csd)					MERGE_BYTE(EXTRACT_AT_INDEX(csd, CSD_SIZE, 84+8, 12-8), EXTRACT_AT_INDEX(csd, CSD_SIZE, 84, 8))
#define CSD_V2_READ_BL_LEN(csd)			EXTRACT_AT_INDEX(csd, CSD_SIZE, 80, 4)
#define CSD_V2_READ_BL_PARTIAL(csd)		EXTRACT_AT_INDEX(csd, CSD_SIZE, 79, 1)
#define CSD_V2_WRITE_BLK_MISALIGN(csd)	EXTRACT_AT_INDEX(csd, CSD_SIZE, 78, 1)
#define CSD_V2_READ_BLK_MISALIGN(csd)	EXTRACT_AT_INDEX(csd, CSD_SIZE, 77, 1)
#define CSD_V2_DSR_IMP(csd)				EXTRACT_AT_INDEX(csd, CSD_SIZE, 76, 1)
#define CSD_V2_C_SIZE(csd)				MERGE_U32(0, EXTRACT_AT_INDEX(csd, CSD_SIZE, 48+8+8, 22-(8+8)), EXTRACT_AT_INDEX(csd, CSD_SIZE, 48+8, 8), EXTRACT_AT_INDEX(csd, CSD_SIZE, 48, 8))
#define CSD_V2_ERASE_BLK_EN(csd)		EXTRACT_AT_INDEX(csd, CSD_SIZE, 46, 1)
#define CSD_V2_SECTOR_SIZE(csd)			EXTRACT_AT_INDEX(csd, CSD_SIZE, 39, 7)
#define CSD_V2_WP_GRP_SIZE(csd)			EXTRACT_AT_INDEX(csd, CSD_SIZE, 32, 7)
#define CSD_V2_WP_GRP_ENABLE(csd)		EXTRACT_AT_INDEX(csd, CSD_SIZE, 31, 1)
#define CSD_V2_R2W_FACTOR(csd)			EXTRACT_AT_INDEX(csd, CSD_SIZE, 26, 3)
#define CSD_V2_WRITE_BL_LEN(csd)		EXTRACT_AT_INDEX(csd, CSD_SIZE, 22, 4)
#define CSD_V2_WRITE_BL_PARTIAL(csd)	EXTRACT_AT_INDEX(csd, CSD_SIZE, 21, 1)
#define CSD_V2_FILE_FORMAT_GRP(csd)		EXTRACT_AT_INDEX(csd, CSD_SIZE, 15, 1)
#define CSD_V2_COPY(csd)				EXTRACT_AT_INDEX(csd, CSD_SIZE, 14, 1)
#define CSD_V2_PERM_WRITE_PROTECT(csd)	EXTRACT_AT_INDEX(csd, CSD_SIZE, 13, 1)
#define CSD_V2_TMP_WRITE_PROTECT(csd)	EXTRACT_AT_INDEX(csd, CSD_SIZE, 12, 1)
#define CSD_V2_FILE_FORMAT(csd)			EXTRACT_AT_INDEX(csd, CSD_SIZE, 10, 2)
#define CSD_V2_CRC(csd)					EXTRACT_AT_INDEX(csd, CSD_SIZE, 1, 7)


int8_t sd_init(void);
int8_t sd_read(uint8_t *buff, uint32_t sector, size_t n);
int8_t sd_write(const uint8_t *buf, uint32_t sector, size_t n);

int sd_sync(void);

int read_csd(uint8_t csd[static CSD_SIZE]);
int read_cid(uint8_t cid[static CID_SIZE]);

int get_memory_capacity(uint32_t *memory_capacity);

#endif /* MMC_SDCARD_H */
