/*
 * Lab #2 : Page Mapping FTL Simulator
 *  - Storage Architecture, SSE3069
 *
 * TA: Jinwoo Jeong, Hyunbin Kang
 * Prof: Dongkun Shin
 * Intelligent Embedded Software Laboratory
 * Sungkyunkwan University
 * http://nyx.skku.ac.kr
 */

#include "ftl.h"

u32 pmt[N_LPNS];
unsigned char map[N_BANKS][N_USER_BLOCKS_PB];
int freeblock;

static void garbage_collection(u32 bank)
{
    stats.gc_cnt++;
/***************************************
Add

stats.gc_write++;
for every nand_write call (every valid page copy)
that you issue in this function

stats.gc_read++;
for every nand_read call (every valid page copy)
that you issue in this function
***************************************/
    return;
}

void ftl_open()
{
    nand_init(N_BANKS, BLKS_PER_BANK, PAGES_PER_BLK);

    for (u32 i = 0; i < N_LPNS; i++) {
        pmt[i] = -1;
    }

    // GC trigger에서 freeblock이 user free block의 개수가 1 ? or 총 free blcok의 개수가 1 ?
    freeblock = N_BANKS * N_USER_BLOCKS_PB;
}

void ftl_read(u32 lba, u32 nsect, u32 *read_buffer)
{
    stats.host_read += nsect;
/***************************************
Add

stats.nand_read++;

for every nand_read call 
that you issue in this function
***************************************/

    return;
}

void ftl_write(u32 lba, u32 nsect, u32 *write_buffer)
{
    stats.host_write += nsect;
/***************************************
Add

stats.nand_write++;

for every nand_write call 
that you issue in this function
***************************************/
    u32 lpn = lba / SECTORS_PER_PAGE;

    // old data exists
    if (pmt[lpn] != -1) {

    }

    // old data does not exist
    else if (pmt[lpn] == -1) {
        int bank = lpn % N_BANKS;
        int block  = -1;
        int page = -1;

        for (int i = 0; i < N_USER_BLOCKS_PB; i++) {
            if (map[bank][i] == 0) {
                block = i / PAGES_PER_BLK;
                page = i % PAGES_PER_BLK;
                map[bank][i] = 1;
                break;
            }
        }

        for (int i = 0; i < nsect; i++) {
        
        }
    }

    

    return;
}
