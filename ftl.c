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

u32 pmt[N_LPNS];
int used[N_BANKS][BLKS_PER_BANK * PAGES_PER_BLK];
int freeblock[N_BANKS];
int cmt_cache[N_CACHED_MAP_PAGE_PB][N_MAP_ENTRIES_PER_PAGE];
int gtd_table[N_MAP_PAGES_PB];

/* DFTL simulator
 * you must make CMT, GTD to use L2P cache
 * you must increase stats.cache_hit value when L2P is in CMT
 * when you can not find L2P in CMT, you must flush cache 
 * and load target L2P in NAND through GTD and increase stats.cache_miss value
 */

 void find_next_page(int* bank, int* block, int* page, u32 lpn) {
    *bank = lpn % N_BANKS;
    *block  = -1;
    *page = -1;
    int flag = 0;

    for (int i = 0; i < BLKS_PER_BANK; i++) {
        if (used[*bank][i * PAGES_PER_BLK] == 0) {
            continue;
        }

        for (int j = 1; j < PAGES_PER_BLK; j++) {
            if (used[*bank][i * PAGES_PER_BLK + j] == 0) {
                *block = i;
                *page = j;
                // printf("next page1: bank: %d, block: %d, page: %d\n", *bank, *block, *page);
                used[*bank][i * PAGES_PER_BLK + j] = 1;
                flag = 1;
                break;
            }
        }

        if (flag == 1) {
            break;
        }
    }

    if (flag == 0) {
        for (int i = 0; i < BLKS_PER_BANK * PAGES_PER_BLK; i++) {
            if (used[*bank][i] == 0 && i % PAGES_PER_BLK == 0) {
                if (freeblock[*bank] == 1) {
                    // printf("GC triggered! bank: %d\n", *bank);
                    garbage_collection(*bank);

                    for (int j = i; j < i + PAGES_PER_BLK; j++) {
                        if (used[*bank][j] == 0) {
                            i = j;
                            break;
                        }
                    }
                }

                *block = i / PAGES_PER_BLK;
                *page = i % PAGES_PER_BLK;

                // printf("next page2: bank: %d, block: %d, page: %d\n", *bank, *block, *page);

                used[*bank][i] = 1;

                freeblock[*bank]--;
                // printf("freeblock[%d]: %d\n", *bank, freeblock[*bank]);
                return;
            }
        }
    }
}

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
	stats.gc_cnt++;
	
	int victim = -1;
    int victim_cnt = -1;

    // select victim block
    for (int i = 0; i < BLKS_PER_BANK; i++) {
        int cnt = 0;
        
        for (int j = 0; j < PAGES_PER_BLK; j++) {
            if (used[bank][i * PAGES_PER_BLK + j] == -1) {
                cnt++;
            }
        }

        if (cnt > victim_cnt) {
            victim_cnt = cnt;
            victim = i;
        }
    }

    // printf("victim block: %d in bank: %d\n", victim, bank);

    for (int i = 0; i < PAGES_PER_BLK; i++) {
        if (used[bank][victim * PAGES_PER_BLK + i] == -1) {
            // used[bank][victim * PAGES_PER_BLK + i] = 0;
        }

        if (used[bank][victim * PAGES_PER_BLK + i] == 1) {
            u32* buf = (u32*)malloc(SECTOR_SIZE * SECTORS_PER_PAGE);
            u32 spare;
            // printf("GC copy read: bank: %d, block: %d, page: %d\n", bank, victim, i);
            nand_read(bank, victim, i, buf, &spare);

            int copy_block = -1;
            int copy_page = -1;

            for (int i = 0; i < BLKS_PER_BANK * PAGES_PER_BLK; i++) {
                if (used[bank][i] == 0) {
                    copy_block = i / PAGES_PER_BLK;
                    copy_page = i % PAGES_PER_BLK;
                    used[bank][i] = 1;

                    break;
                }
            }

            
            nand_write(bank, copy_block, copy_page, buf, &spare);
            // printf("GC copy write: bank: %d, block: %d, page: %d, spare: %d, pmt[spare - before]: %d\n", bank, copy_block, copy_page, spare, pmt[spare]);
            stats.gc_write++;

            pmt[spare] = bank * N_PPNS_PB + copy_block * PAGES_PER_BLK + copy_page;

            // printf("pmt[spare - after]: %d\n", pmt[spare]);
            used[bank][copy_block * PAGES_PER_BLK + copy_page] = 1;
            
            // used[bank][victim * PAGES_PER_BLK + i] = 0;

            free(buf);
        }
    }

    nand_erase(bank, victim);

    for (int i = 0; i < PAGES_PER_BLK; i++) {
        used[bank][victim * PAGES_PER_BLK + i] = 0;
    }

    freeblock[bank]++;

	return;
}

void ftl_open()
{
	nand_init(N_BANKS, BLKS_PER_BANK, PAGES_PER_BLK);

    for (u32 i = 0; i < N_LPNS; i++) {
        pmt[i] = -1;
    }

    // GC trigger에서 freeblock이 user free block의 개수가 1 ? or 총 free blcok의 개수가 1 ? 아마도 user free block의 개수인듯
    for (int i = 0; i < N_BANKS; i++) {
        freeblock[i] = BLKS_PER_BANK;
        // printf("freeblock[%d]: %d\n", i, freeblock[i]);
    }

	return;
}

void ftl_read(u32 lba, u32 nsect, u32 *read_buffer)
{
    // stats.nand_read++ every nand_read call
    stats.host_read += nsect;

	u32* buf = (u32*)malloc(SECTOR_SIZE * SECTORS_PER_PAGE);
    
    for (int i = 0; i < nsect; i++) {
        if (i == 0 || (lba + i) % SECTORS_PER_PAGE == 0) {
            if (pmt[(lba + i) / SECTORS_PER_PAGE] != -1) {
                int bank = (lba + i) / SECTORS_PER_PAGE % N_BANKS;
                int addr = pmt[(lba + i) / SECTORS_PER_PAGE] - bank * N_PPNS_PB;
                int block = addr / PAGES_PER_BLK;
                int page = addr % PAGES_PER_BLK;
                int tmp_lpn;

                // printf("bank: %d, block: %d, page: %d, pmt[%d]: %d\n", bank, block, page, (lba + i) / SECTORS_PER_PAGE, pmt[(lba + i) / SECTORS_PER_PAGE]);
                nand_read(bank, block, page, buf, &tmp_lpn);
                // for (int j = 0; j < 8; j++) {
                //     printf("%x ", buf[j]);
                // }
                // printf("\n");
                stats.nand_read++;

                for (int j = 0;; j++) {
                    read_buffer[i + j] = buf[(lba + i + j) % SECTORS_PER_PAGE];
                    
                    if ((lba + i + j) % SECTORS_PER_PAGE == SECTORS_PER_PAGE - 1 || i + j == nsect - 1)
                        break;
                }
                
            }
            
            else {
                for (int j = 0;; j++) {
                    read_buffer[i + j] = 0xffffffff;
                    
                    if ((lba + i + j) % SECTORS_PER_PAGE == SECTORS_PER_PAGE - 1 || i + j == nsect - 1)
                        break;
                }
            }
        }
    }

    free(buf);

	return;
}

void ftl_write(u32 lba, u32 nsect, u32 *write_buffer)
{
    /* stats.nand_read++ every nand_read call
	 * stats.nand_write++ every nand_write call
	 */
	stats.host_write += nsect;

	u32 lpn = lba / SECTORS_PER_PAGE;
    int bank;
    int block;
    int page;
    int addr;
    u32* buf = (u32*)malloc(SECTOR_SIZE * SECTORS_PER_PAGE);
    int flag = 0;

    // 첫 page alignment이 안된 경우
    if (lba % SECTORS_PER_PAGE != 0) {
        // old data exists
        if (pmt[lpn] != -1) {
            bank = lpn % N_BANKS;
            addr = pmt[lpn] - bank * N_PPNS_PB;
            block = addr / PAGES_PER_BLK;
            page = addr % PAGES_PER_BLK;
            
            nand_read(bank, block, page, buf, &lpn);
            // stats.nand_read++;
            flag = 1;
        }

        // no old data
        else {
            for (int i = 0; i < SECTORS_PER_PAGE; i++) {
                buf[i] = 0xffffffff;
            }
        }
    }

    for (int i = 0; i < nsect; i++) {
        buf[(lba + i) % SECTORS_PER_PAGE] = write_buffer[i];
        // printf("writer_buffer[%d]: %x\n", i, write_buffer[i]);

        // buffer 다 채우고 page write
        if ((lba + i) % SECTORS_PER_PAGE == SECTORS_PER_PAGE - 1) {
            int write_lpn = (lba + i) / SECTORS_PER_PAGE;

            find_next_page(&bank, &block, &page, write_lpn);
            addr = bank * N_PPNS_PB + block * PAGES_PER_BLK + page;

            // old page invalid
            if (pmt[write_lpn] != -1) {
                int old_bank = write_lpn % N_BANKS;
                int old_addr = pmt[write_lpn] - old_bank * N_PPNS_PB;
                int old_block = old_addr / PAGES_PER_BLK;
                int old_page = old_addr % PAGES_PER_BLK;

                used[old_bank][old_block * PAGES_PER_BLK + old_page] = -1;

                // if (flag == 0)
                stats.nand_read++;
                // else
                //     flag = 0;
            }

            pmt[write_lpn] = addr;
            
            nand_write(bank, block, page, buf, &write_lpn);
            stats.nand_write++;
        }
    }

    if ((lba + nsect - 1) % SECTORS_PER_PAGE != SECTORS_PER_PAGE - 1) {
        int write_lpn = (lba + nsect - 1) / SECTORS_PER_PAGE;

        // old data exists
        if (pmt[write_lpn] != -1) {
            int old_bank = write_lpn % N_BANKS;
            int old_addr = pmt[write_lpn] - old_bank * N_PPNS_PB;
            int old_block = old_addr / PAGES_PER_BLK;
            int old_page = old_addr % PAGES_PER_BLK;
            u32* tmp_buf = (u32*)malloc(SECTOR_SIZE * SECTORS_PER_PAGE);

            nand_read(old_bank, old_block, old_page, tmp_buf, &write_lpn);
            // stats.nand_read++;

            for (int i = lba + nsect;; i++) {
               buf[i % SECTORS_PER_PAGE] = tmp_buf[i % SECTORS_PER_PAGE];

                if (i % SECTORS_PER_PAGE == SECTORS_PER_PAGE - 1)
                    break;
            }
        }

        // no old data
        else {
            for (int i = lba + nsect;; i++) {
                buf[i % SECTORS_PER_PAGE] = 0xffffffff;

                if (i % SECTORS_PER_PAGE == SECTORS_PER_PAGE - 1)
                    break;
            }
        }

        find_next_page(&bank, &block, &page, write_lpn);
        addr = bank * N_PPNS_PB + block * PAGES_PER_BLK + page;

        // old page invalid
        if (pmt[write_lpn] != -1) {
            int old_bank = write_lpn % N_BANKS;
            int old_addr = pmt[write_lpn] - old_bank * N_PPNS_PB;
            int old_block = old_addr / PAGES_PER_BLK;
            int old_page = old_addr % PAGES_PER_BLK;

            used[old_bank][old_block * PAGES_PER_BLK + old_page] = -1;
            stats.nand_read++;
        }

        pmt[write_lpn] = addr;
        
        nand_write(bank, block, page, buf, &write_lpn);
        stats.nand_write++;
    }

	return;
}
