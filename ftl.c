/*
 * Lab #3 : Page Mapping Simulator II
 *  - Storage Architecture, SSE3069
 *
 *
 * TA: Youngjin Kim, Eunji Song
 * Prof: Dongkun Shin
 * Intelligent Embedded Software Laboratory
 * Sungkyunkwan University
 * http://nyx.skku.ac.kr
 */
#include "ftl.h"

static void garbage_collection(u32 bank)
{

}

void ftl_open()
{
	nand_init(N_BANKS, BLKS_PER_BANK, PAGES_PER_BLK);

}

void ftl_read(u32 lba, u32 nsect, u32 *read_buffer)
{

}

void ftl_write(u32 lba, u32 nsect, u32 *write_buffer)
{

}

void ftl_flush()
{

}

void ftl_trim(u32 lpn, u32 npage)
{

}