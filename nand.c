/*
 * Lab #1 : NAND Simulator
 *  - Storage Architecture, SSE3069
 *
 * TA: Youngjin Kim, Eunji Song
 * Prof: Dongkun Shin
 * Intelligent Embedded Systems Laboratory
 * Sungkyunkwan University
 * http://nyx.skku.ac.kr
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "nand.h"

#define DEBUG 1
/*
 * define your own data structure for NAND flash implementation
 */
unsigned char* nand = NULL;
unsigned char* map = NULL;
int NBANKS = -1;
int NBLKS = -1;
int NPAGES = -1;
/*
 * initialize the NAND flash memory
 * @nbanks: number of bank
 * @nblks: number of blocks per bank
 * @npages: number of pages per block
 *
 * Returns:
 *   0 on success
 *   NAND_ERR_INVALID if given dimension is invalid
 */
int nand_init(int nbanks, int nblks, int npages)
{
	if (nbanks < 0 || nblks < 0 || npages < 0) {
		if (DEBUG)
			printf("NAND_ERR_INVALID_INIT\n");
		return NAND_ERR_INVALID;
	}

	nand = (unsigned char*)malloc(nbanks * nblks * npages * (PAGE_DATA_SIZE + PAGE_SPARE_SIZE));
	map = (unsigned char*)malloc(nbanks * nblks * npages);
	memset(map, 0, nbanks * nblks * npages);
	NBANKS = nbanks;
	NBLKS = nblks;
	NPAGES = npages;
	
	return NAND_SUCCESS;
}

/*
 * write data and spare into the NAND flash memory page
 *
 * Returns:
 *   0 on success
 *   NAND_ERR_INVALID if target flash page address is invalid
 *   NAND_ERR_OVERWRITE if target page is already written
 *   NAND_ERR_POSITION if target page is empty but not the position to be written
 */
int nand_write(int bank, int blk, int page, void *data, void *spare)
{
	if (bank < 0 || blk < 0 || page < 0 || bank >= NBANKS || blk >= NBLKS || page >=NPAGES) {
		return NAND_ERR_INVALID;
	}

	int addr = bank * NBLKS * NPAGES + blk * NPAGES + page;

	if (map[addr] == 1) {
		if (DEBUG)
			printf("NAND_ERR_OVERWRITE_WRITE\n");
		return NAND_ERR_OVERWRITE;
	}

	int startBlock = addr - (addr % NPAGES);

	for (int i = 0; i < (addr % NPAGES); i++) {
		if (map[startBlock + i] == 0) {
			if (DEBUG)
				printf("NAND_ERR_POSITION_WRITE\n");
			return NAND_ERR_POSITION;
		}
	}

	map[addr] = 1;

	unsigned char* start = nand + addr * (PAGE_DATA_SIZE + PAGE_SPARE_SIZE);

	memcpy(start, data, PAGE_DATA_SIZE);
	memcpy(start + PAGE_DATA_SIZE, spare, PAGE_SPARE_SIZE);

	return NAND_SUCCESS;
}


/*
 * read data and spare from the NAND flash memory page
 *
 * Returns:
 *   0 on success
 *   NAND_ERR_INVALID if target flash page address is invalid
 *   NAND_ERR_EMPTY if target page is empty
 */
int nand_read(int bank, int blk, int page, void *data, void *spare)
{
	if (bank < 0 || blk < 0 || page < 0 || bank >= NBANKS || blk >= NBLKS || page >=NPAGES) {
		if (DEBUG)
			printf("NAND_ERR_INVALID_READ\n");
		return NAND_ERR_INVALID;
	}

	int addr = bank * NBLKS * NPAGES + blk * NPAGES + page;

	if (map[addr] == 0) {
		if (DEBUG)
			printf("NAND_ERR_EMPTY_READ\n");
		return NAND_ERR_EMPTY;
	}

	unsigned char* start = nand + addr * (PAGE_DATA_SIZE + PAGE_SPARE_SIZE);

	memcpy(data, start, PAGE_DATA_SIZE);
	memcpy(spare, start + PAGE_DATA_SIZE, PAGE_SPARE_SIZE);

	return NAND_SUCCESS;
}

/*
 * erase the NAND flash memory block
 *
 * Returns:
 *   0 on success
 *   NAND_ERR_INVALID if target flash block address is invalid
 *   NAND_ERR_EMPTY if target block is already erased
 */
int nand_erase(int bank, int blk)
{
	if (bank < 0 || blk < 0 || bank >= NBANKS || blk >= NBLKS) {
		if (DEBUG)
			printf("NAND_ERR_INVALID_ERASE\n");
		return NAND_ERR_INVALID;
	}

	int addr = bank * NBLKS * NPAGES + blk * NPAGES;
	int flag = 0;

	for (int i = 0; i < NPAGES; i++) {
		if (map[addr + i] == 1) {
			flag = 1;
		}
	}

	if (flag == 0) {
		if (DEBUG)
			printf("NAND_ERR_EMPTY_ERASE\n");
		return NAND_ERR_EMPTY;
	}

	for (int i = 0; i < NPAGES; i++) {
		map[addr + i] = 0;
	}

	return NAND_SUCCESS;
}