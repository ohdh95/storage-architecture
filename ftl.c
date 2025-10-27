/*
 * Lab #4 : DFTL Simulator
 *  - Storage Architecture, SSE3069
 *
 * TA: Youngjin Kim, Eunji Song
 * Prof: Dongkun Shin
 * Intelligent Embedded Software Laboratory
 * Sungkyunkwan University
 * http://nyx.skku.ac.kr
 */

#include "ftl.h"

/* DFTL simulator
 * you must make CMT, GTD to use L2P cache
 * you must increase stats.cache_hit value when L2P is in CMT
 * when you can not find L2P in CMT, you must flush cache 
 * and load target L2P in NAND through GTD and increase stats.cache_miss value
 */

static void map_write(u32 bank, u32 map_page, u32 cache_slot)
{
	/* you use this function when you must flush
	 * cache from CMT to NAND MAP area
	 * flush cache with LRU policy and fix GTD!!
     * stats.map_write++ every nand_write call
	 */
	return;
}

static void map_read(u32 bank, u32 map_page, u32 cache_slot)
{
	/* you use this function when you must load 
	 * L2P from NAND MAP area to CMT
	 * find L2P MAP with GTD and fill CMT!!
     * stats.map_read++ every nand_read call
	 */
	return;
}

static void map_garbage_collection(u32 bank)
{
	/* stats.map_gc_cnt++ every map_garbage_collection call
	 * stats.map_gc_copy++ every nand_write call
	 */
	return;
}
static void garbage_collection(u32 bank)
{
	/* stats.gc_cnt++ every garbage_collection call
	 * stats.gc_copy++ every nand_write call
	 */
	return;
}

void ftl_open()
{

	return;
}

void ftl_read(u32 lba, u32 nsect, u32 *read_buffer)
{
    // stats.nand_read++ every nand_read call
    stats.host_read += nsect;

	return;
}

void ftl_write(u32 lba, u32 nsect, u32 *write_buffer)
{
    /* stats.nand_read++ every nand_read call
	 * stats.nand_write++ every nand_write call
	 */
	stats.host_write += nsect;

	return;
}
