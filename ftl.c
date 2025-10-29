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

// #include "ftl1.h"
#if defined(VERSION_V0)
    #include "ftl.h"
#elif defined(VERSION_V1)
	#include "ftl1.h"
#elif defined(VERSION_V2)
    #include "ftl2.h"
#elif defined(VERSION_V3)
    #include "ftl3.h"
#elif defined(VERSION_V4)
    #include "ftl4.h"
#elif defined(VERSION_V5)
    #include "ftl5.h"
#elif defined(VERSION_V6)
    #include "ftl6.h"
#elif defined(VERSION_V7)
    #include "ftl7.h"
#elif defined(VERSION_V8)
    #include "ftl8.h"
#endif

int used[N_BANKS][BLKS_PER_BANK * PAGES_PER_BLK];
int blk_type[N_BANKS][BLKS_PER_BANK]; // -1: unused, 0: data block, 1: map block
int data_freeblock[N_BANKS];
int trans_freeblock[N_BANKS];
int cmt[N_BANKS][N_CACHED_MAP_PAGE_PB][N_MAP_ENTRIES_PER_PAGE];
int gtd[N_BANKS][N_MAP_PAGES_PB];
int cmt_mvpn[N_BANKS][N_CACHED_MAP_PAGE_PB];
int cmt_dirty[N_BANKS][N_CACHED_MAP_PAGE_PB];
int cmt_ref_time[N_BANKS][N_CACHED_MAP_PAGE_PB];
int cur_data_block[N_BANKS];
int cur_trans_block[N_BANKS];
u32 ref_time = 0;

/* DFTL simulator
 * you must make CMT, GTD to use L2P cache
 * you must increase stats.cache_hit value when L2P is in CMT
 * when you can not find L2P in CMT, you must flush cache 
 * and load target L2P in NAND through GTD and increase stats.cache_miss value
 */
static void map_write(u32 bank, u32 map_page, u32 cache_slot);
static void map_read(u32 bank, u32 map_page, u32 cache_slot);
static void map_garbage_collection(u32 bank);
static void garbage_collection(u32 bank);

void find_next_trans_page(u32 bank, int* block, int* page) {
	// 첫 시작
	if (cur_trans_block[bank] == -1) {
		// find new block
		for (u32 i = 0; i < BLKS_PER_BANK; i++) {
			if (blk_type[bank][i] == -1) {
				blk_type[bank][i] = 1; // map block
				cur_trans_block[bank] = i;
				*block = cur_trans_block[bank];
				*page = 0;
				trans_freeblock[bank]--;
				used[bank][cur_trans_block[bank] * PAGES_PER_BLK + 0] = 1;
				return;
			}
		}
	}

	// cur trans block 정해져 있음
	else {
		for (u32 i = 0; i < PAGES_PER_BLK; i++) {
			// cur trans block의 빈 page
			if (used[bank][cur_trans_block[bank] * PAGES_PER_BLK + i] == 0) {
				*block = cur_trans_block[bank];
				*page = i;
				used[bank][cur_trans_block[bank] * PAGES_PER_BLK + i] = 1;
				return;
			}
		}

		// 새로운 trans block 할당
		for (u32 i = 0; i < BLKS_PER_BANK; i++) {
			if (blk_type[bank][i] == -1) {
				blk_type[bank][i] = 1; // map block
				cur_trans_block[bank] = i;
				*block = cur_trans_block[bank];
				int flag = 0;
				if (trans_freeblock[bank] == 1) {
					// printf("trans GC triggered\n");
					map_garbage_collection(bank);
				}

				for (int j = 0; j < PAGES_PER_BLK; j++) {
					if (used[bank][cur_trans_block[bank] * PAGES_PER_BLK + j] == 0) {
						*page = j;
						trans_freeblock[bank]--;
						used[bank][cur_trans_block[bank] * PAGES_PER_BLK + j] = 1;
						flag = 1;
						break;
					}
				}

				if (flag == 0) {
					printf("ERROR: no free data page\n");
				}

				return;
			}
		}
	}
}

// used = 1 설정
void find_next_data_page(u32 bank, int* block, int* page) {
	if (cur_data_block[bank] == -1) {
		// find new block
		for (u32 i = 0; i < BLKS_PER_BANK; i++) {
			if (blk_type[bank][i] == -1) {
				blk_type[bank][i] = 0; // data block
				cur_data_block[bank] = i;
				*block = cur_data_block[bank];
				*page = 0;
				data_freeblock[bank]--;
				used[bank][cur_data_block[bank] * PAGES_PER_BLK + 0] = 1;
				return;
			}
		}
	}

	else {
		for (u32 i = 0; i < PAGES_PER_BLK; i++) {
			if (used[bank][cur_data_block[bank] * PAGES_PER_BLK + i] == 0) {
				*block = cur_data_block[bank];
				*page = i;
				used[bank][cur_data_block[bank] * PAGES_PER_BLK + i] = 1;
				return;
			}
		}

		for (u32 i = 0; i < BLKS_PER_BANK; i++) {
			if (blk_type[bank][i] == -1) {
				blk_type[bank][i] = 0; // data block
				cur_data_block[bank] = i;
				*block = cur_data_block[bank];
				int flag = 0;
				if (data_freeblock[bank] == 1) {
					// printf("data GC triggered\n");
					garbage_collection(bank);
				}

				for (int j = 0; j < PAGES_PER_BLK; j++) {
					if (used[bank][cur_data_block[bank] * PAGES_PER_BLK + j] == 0) {
						if (j == PAGES_PER_BLK - 1) {
							printf("ERROR: no free data page in new data block\n");
						}
						*page = j;
						data_freeblock[bank]--;
						used[bank][cur_data_block[bank] * PAGES_PER_BLK + j] = 1;
						flag = 1;
						break;
					}
				}
				
				return;
			}
		}

		printf("ERROR: no free data block\n");
	}
}

// gtd가 -1인 경우, cmt slot index 반환(빈 슬롯 없으면 flush 후 반환)
int get_free_cmt_slot(u32 lpn) {
	// int mvpn = (lpn / N_BANKS) / 8;
	int bank = lpn % N_BANKS;

	for (int i = 0; i < N_CACHED_MAP_PAGE_PB; i++) {
		if (cmt_mvpn[bank][i] == -1) {
			return i;
		}
	}

	int victim = 0;
	int min_time = cmt_ref_time[bank][0];

	for (int i = 1; i < N_CACHED_MAP_PAGE_PB; i++) {
		if (cmt_ref_time[bank][i] < min_time) {
			min_time = cmt_ref_time[bank][i];
			victim = i;
		}
	}

	if (cmt_dirty[bank][victim] == 1) {
		// flush
		map_write(bank, cmt_mvpn[bank][victim], victim);
		cmt_dirty[bank][victim] = 0;
	}

	cmt_ref_time[bank][victim] = ref_time++;

	return victim;
}

// 반드시 ppn이 있는 경우, cmt index 반환
int get_cmt_index(u32 lpn) {
	int mvpn = (lpn / N_BANKS) / 8;
	int bank = lpn % N_BANKS;

	// cache 먼저 확인
	for (int i = 0; i < N_CACHED_MAP_PAGE_PB; i++) {
		if (cmt_mvpn[bank][i] == mvpn) {
			// cache hit
			stats.cache_hit++;
			cmt_ref_time[bank][i] = ref_time++;

			return i; // return cmt[i][(lpn / N_BANKS) % 8];
		}
	}

	// cache miss
	stats.cache_miss++;

	int victim = 0;
	int min_time = cmt_ref_time[bank][0];

	for (int i = 1; i < N_CACHED_MAP_PAGE_PB; i++) {
		if (cmt_ref_time[bank][i] < min_time) {
			min_time = cmt_ref_time[bank][i];
			victim = i;
		}
	}

	if (cmt_dirty[bank][victim] == 1) {
		// flush
		map_write(bank, cmt_mvpn[bank][victim], victim);
		cmt_dirty[bank][victim] = 0;
	}

	if (gtd[bank][mvpn] != -1)
		map_read(bank, mvpn, victim);

	cmt_ref_time[bank][victim] = ref_time++;
	cmt_mvpn[bank][victim] = mvpn;

	return victim; // return cmt[victim][(lpn / N_BANKS) % 8];
}

// ppn 수정

static void map_write(u32 bank, u32 map_page, u32 cache_slot)
{
	// printf("map write called\n");
	/* you use this function when you must flush
	 * cache from CMT to NAND MAP area
	 * flush cache with LRU policy and fix GTD!!
     * stats.map_write++ every nand_write call
	 */
	int* buf = (int*)malloc(SECTOR_SIZE * SECTORS_PER_PAGE);
	u32 spare = map_page;
	int block, page;

	int old_addr = gtd[bank][map_page];

	if (old_addr != -1) {
		int old_block = (old_addr - bank * N_PPNS_PB) / PAGES_PER_BLK;
		int old_page = (old_addr - bank * N_PPNS_PB) % PAGES_PER_BLK;

		used[bank][old_block * PAGES_PER_BLK + old_page] = -1;
	}

	find_next_trans_page(bank, &block, &page);
	
	for (u32 i = 0; i < N_MAP_ENTRIES_PER_PAGE; i++) {
		buf[i] = cmt[bank][cache_slot][i];
	}

	nand_write(bank, block, page, buf, &spare);
	stats.map_write++;

	gtd[bank][map_page] = bank * N_PPNS_PB + block * PAGES_PER_BLK + page;

	free(buf);

	return;
}

static void map_read(u32 bank, u32 map_page, u32 cache_slot)
{
	// printf("map read called\n");
	/* you use this function when you must load 
	 * L2P from NAND MAP area to CMT
	 * find L2P MAP with GTD and fill CMT!!
     * stats.map_read++ every nand_read call
	 */
	u32* buf = (u32*)malloc(SECTOR_SIZE * SECTORS_PER_PAGE);
	u32 spare = map_page;
	int addr = gtd[bank][map_page];
	int block = (addr - bank * N_PPNS_PB) / PAGES_PER_BLK;
	int page = (addr - bank * N_PPNS_PB) % PAGES_PER_BLK;

	nand_read(bank, block, page, buf, &spare);
	stats.map_read++;

	for (u32 i = 0; i < N_MAP_ENTRIES_PER_PAGE; i++) {
		cmt[bank][cache_slot][i] = buf[i];
	}

	free(buf);

	return;
}

static void map_garbage_collection(u32 bank)
{
	/* stats.map_gc_cnt++ every map_garbage_collection call
	 * stats.map_gc_copy++ every nand_write call
	 */
	stats.map_gc_cnt++;

	int victim = -1;
	int victim_cnt = -1;

	// select victim block
	for (int i = 0; i < BLKS_PER_BANK; i++) {
		if (blk_type[bank][i] == 1) {
			int cnt = 0;

			for (int j = 0; j < PAGES_PER_BLK; j++) {
				if (used[bank][i * PAGES_PER_BLK + j] == -1) {
					cnt++;
				}
			}

			if (cnt > victim_cnt) {
				victim = i;
				victim_cnt = cnt;
			}
		}
	}

	// valid copy
	int copy_page = 0;
	
	for (int i = 0; i < PAGES_PER_BLK; i++) {
        if (used[bank][victim * PAGES_PER_BLK + i] == 1) {
            u32* buf = (u32*)malloc(SECTOR_SIZE * SECTORS_PER_PAGE);
            u32 spare;
			
            nand_read(bank, victim, i, buf, &spare);
            
            nand_write(bank, cur_trans_block[bank], copy_page, buf, &spare);
            stats.map_gc_copy++;

            used[bank][cur_trans_block[bank] * PAGES_PER_BLK + copy_page] = 1;
			
			gtd[bank][spare] = bank * N_PPNS_PB + cur_trans_block[bank] * PAGES_PER_BLK + copy_page;

            copy_page++;

            free(buf);
        }
    }

	// printf("copy_page: %d\n", copy_page);
	nand_erase(bank, victim);

	blk_type[bank][victim] = -1;

    for (int i = 0; i < PAGES_PER_BLK; i++) {
        used[bank][victim * PAGES_PER_BLK + i] = 0;
    }

	

    trans_freeblock[bank]++;

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
		if (blk_type[bank][i] == 0) {
			int cnt = 0;

			for (int j = 0; j < PAGES_PER_BLK; j++) {
				if (used[bank][i * PAGES_PER_BLK + j] == -1) {
					cnt++;
				}
			}

			if (cnt > victim_cnt) {
				victim = i;
				victim_cnt = cnt;
			}
		}
	}

	int copy_page = 0;

	// valid copy
	for (int i = 0; i < PAGES_PER_BLK; i++) {
        if (used[bank][victim * PAGES_PER_BLK + i] == 1) {
            u32* buf = (u32*)malloc(SECTOR_SIZE * SECTORS_PER_PAGE);
            u32 spare; // lpn

			// data page 복사
            nand_read(bank, victim, i, buf, &spare);

            nand_write(bank, cur_data_block[bank], copy_page, buf, &spare);
			stats.gc_copy++;

            used[bank][cur_data_block[bank] * PAGES_PER_BLK + copy_page] = 1;

			// translation page 수정
			int mvpn = (spare / N_BANKS) / 8;
			int tmp;
			int mppn = gtd[bank][mvpn];
			int mblock = (mppn - bank * N_PPNS_PB) / PAGES_PER_BLK;
			int mpage = (mppn - bank * N_PPNS_PB) % PAGES_PER_BLK;
			int newmblock;
			int newmpage;

			// spare: mvpn
			nand_read(bank, mblock, mpage, buf, &tmp);

			if (tmp != mvpn) {
				printf("ERROR: map page lpn mismatch during GC\n");
			}

			buf[(spare / N_BANKS) % 8] = bank * N_PPNS_PB + cur_data_block[bank] * PAGES_PER_BLK + copy_page;

			used[bank][mblock * PAGES_PER_BLK + mpage] = -1;
			find_next_trans_page(bank, &newmblock, &newmpage);
			nand_write(bank, newmblock, newmpage, buf, &mvpn);

			gtd[bank][mvpn] = bank * N_PPNS_PB + newmblock * PAGES_PER_BLK + newmpage;

			for (int j = 0; j < N_CACHED_MAP_PAGE_PB; j++) {
				if (cmt_mvpn[bank][j] == mvpn) {
					cmt[bank][j][(spare / N_BANKS) % 8] = bank * N_PPNS_PB + cur_data_block[bank] * PAGES_PER_BLK + copy_page;
					break;
				}
			}

			copy_page++;

            free(buf);
        }
    }

	if (copy_page == PAGES_PER_BLK) {
		printf("GC error: all pages are invalid in victim block\n");
	}

	nand_erase(bank, victim);

    for (int i = 0; i < PAGES_PER_BLK; i++) {
        used[bank][victim * PAGES_PER_BLK + i] = 0;
    }

	blk_type[bank][victim] = -1;
	
    data_freeblock[bank]++;

	return;
}

void ftl_open()
{
	nand_init(N_BANKS, BLKS_PER_BANK, PAGES_PER_BLK);

    

    // GC trigger에서 freeblock이 user free block의 개수가 1 ? or 총 free blcok의 개수가 1 ? 아마도 user free block의 개수인듯
    for (int i = 0; i < N_BANKS; i++) {
        data_freeblock[i] = N_USER_BLOCKS_PB + N_USER_OP_BLOCKS_PB;
		trans_freeblock[i] = N_PHY_MAP_BLK;
		cur_data_block[i] = -1;
		cur_trans_block[i] = -1;

		for (u32 j = 0; j < N_MAP_PAGES_PB; j++) {
			gtd[i][j] = -1;
		}

		for (u32 j = 0; j < N_CACHED_MAP_PAGE_PB; j++) {
			cmt_mvpn[i][j] = -1;
			cmt_dirty[i][j] = 0;
			cmt_ref_time[i][j] = 0;
		}

		for (u32 j = 0; j < BLKS_PER_BANK; j++) {
			blk_type[i][j] = -1;
		}
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
			int lpn = (lba + i) / SECTORS_PER_PAGE;
			int bank = lpn % N_BANKS;
			int mvpn = (lpn / N_BANKS) / 8;
			int index;
			int ppn;
			int flag = 0;

			if (gtd[bank][mvpn] == -1) {
				for (int j = 0; j < N_CACHED_MAP_PAGE_PB; j++) {
					if (cmt_mvpn[bank][j] == mvpn) {
						ppn = cmt[bank][j][(lpn / N_BANKS) % 8];
						flag = 1;
						break;
					}
				}
			}

			else {
				flag = 1;
			}

			if (flag == 1) {
				index = get_cmt_index(lpn);
				ppn = cmt[bank][index][(lpn / N_BANKS) % 8];
				if (ppn != -1) {
					int addr = ppn - bank * N_PPNS_PB;
					int block = addr / PAGES_PER_BLK;
					int page = addr % PAGES_PER_BLK;
					int tmp_lpn;

					nand_read(bank, block, page, buf, &tmp_lpn);
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
    int bank = lpn % N_BANKS;
    int block;
    int page;
    int addr;
    u32* buf = (u32*)malloc(SECTOR_SIZE * SECTORS_PER_PAGE);
    // int flag = 0;
	int ppn = -1;
	int mvpn = (lpn / N_BANKS) / 8;

	// translation page가 nand에 아직 없음
	if (gtd[bank][mvpn] == -1) {
		// cmt에 있는지 확인
		for (int i = 0; i < N_CACHED_MAP_PAGE_PB; i++) {
			if (cmt_mvpn[bank][i] == mvpn) {
				ppn = cmt[bank][i][(lpn / N_BANKS) % 8];
				break;
			}
		}
	}

	else {
		ppn = cmt[bank][get_cmt_index(lpn)][(lpn / N_BANKS) % 8];
	}

    // 첫 page alignment이 안된 경우
    if (lba % SECTORS_PER_PAGE != 0) {
        // old data exists
        if (ppn != -1) {
            bank = lpn % N_BANKS;
            addr = ppn - bank * N_PPNS_PB;
            block = addr / PAGES_PER_BLK;
            page = addr % PAGES_PER_BLK;
            
            nand_read(bank, block, page, buf, &lpn);
            // stats.nand_read++;
            // flag = 1;
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

        // buffer 다 채우고 page write
        if ((lba + i) % SECTORS_PER_PAGE == SECTORS_PER_PAGE - 1) {
            int write_lpn = (lba + i) / SECTORS_PER_PAGE;
			int write_mvpn = (write_lpn / N_BANKS) / 8;
			int addr;
			int index = -1;
			int flag = 0;
			int new_block;
			int new_page;
			bank = write_lpn % N_BANKS;

			// cmt check
			for (int j = 0; j < N_CACHED_MAP_PAGE_PB; j++) {
				if (cmt_mvpn[bank][j] == write_mvpn) {
					index = j;
					flag = 1;
					break;
				}
			}

			// nand check
			if (gtd[bank][write_mvpn] != -1) {
				flag = 1;
			}

			// old data exists
			if (flag == 1) {
				index = get_cmt_index(write_lpn);
				int ppn = cmt[bank][index][(write_lpn / N_BANKS) % 8];

				// 원래 data page invalid
				if (ppn != -1) {
					int old_bank = write_lpn % N_BANKS;
					int old_addr = ppn - old_bank * N_PPNS_PB;
					int old_block = old_addr / PAGES_PER_BLK;
					int old_page = old_addr % PAGES_PER_BLK;
					
					used[old_bank][old_block * PAGES_PER_BLK + old_page] = -1;
					stats.nand_read++;
				}
			}
			
			else {
				index = get_free_cmt_slot(write_lpn);

				for (int j = 0; j < N_MAP_ENTRIES_PER_PAGE; j++) {
					cmt[bank][index][j] = -1;
				}
			}

			find_next_data_page(bank, &new_block, &new_page);

			addr = bank * N_PPNS_PB + new_block * PAGES_PER_BLK + new_page;

			cmt[bank][index][(write_lpn / N_BANKS) % 8] = addr;
			cmt_mvpn[bank][index] = write_mvpn;
			cmt_dirty[bank][index] = 1;

			nand_write(bank, new_block, new_page, buf, &write_lpn);
			stats.nand_write++;
		}
    }

	// 마지막 page alignment이 안된 경우
	if ((lba + nsect - 1) % SECTORS_PER_PAGE != SECTORS_PER_PAGE - 1) {
        int write_lpn = (lba + nsect - 1) / SECTORS_PER_PAGE;
		bank = write_lpn % N_BANKS;
		int write_ppn = -1;
		int write_mvpn = (write_lpn / N_BANKS) / 8;
		int index = -1;

		

		// nand에 없음
		if (gtd[bank][write_mvpn] == -1) {
			for (int i = 0; i < N_CACHED_MAP_PAGE_PB; i++) {
				// cmt에 있음
				if (cmt_mvpn[bank][i] == write_mvpn) {
					write_ppn = cmt[bank][i][(write_lpn / N_BANKS) % 8];
					index = i;
					break;
				}
			}
		}

		else {
			index = get_cmt_index(write_lpn);
			write_ppn = cmt[bank][index][(write_lpn / N_BANKS) % 8];
		}

        // old data exists
        if (write_ppn != -1) {
            int old_bank = write_lpn % N_BANKS;
            int old_addr = write_ppn - old_bank * N_PPNS_PB;
            int old_block = old_addr / PAGES_PER_BLK;
            int old_page = old_addr % PAGES_PER_BLK;
            u32* tmp_buf = (u32*)malloc(SECTOR_SIZE * SECTORS_PER_PAGE);

            nand_read(old_bank, old_block, old_page, tmp_buf, &write_lpn);
            // stats.nand_read++;

			used[old_bank][old_block * PAGES_PER_BLK + old_page] = -1;
            stats.nand_read++;
			
			find_next_data_page(bank, &block, &page);
        	addr = bank * N_PPNS_PB + block * PAGES_PER_BLK + page;

            for (int i = lba + nsect;; i++) {
               buf[i % SECTORS_PER_PAGE] = tmp_buf[i % SECTORS_PER_PAGE];

                if (i % SECTORS_PER_PAGE == SECTORS_PER_PAGE - 1)
                    break;
            }

			

			index = get_cmt_index(write_lpn);
			cmt[bank][index][(write_lpn / N_BANKS) % 8] = addr;
			cmt_dirty[bank][index] = 1;
        }

        // no old data
        else {
			find_next_data_page(bank, &block, &page);
        	addr = bank * N_PPNS_PB + block * PAGES_PER_BLK + page;
            for (int i = lba + nsect;; i++) {
                buf[i % SECTORS_PER_PAGE] = 0xffffffff;

                if (i % SECTORS_PER_PAGE == SECTORS_PER_PAGE - 1) {
					if (index == -1) {
						index = get_free_cmt_slot(write_lpn);

						for (int j = 0; j < 8; j++) {
							cmt[bank][index][j] = -1;
						}
					}

					cmt[bank][index][(write_lpn / N_BANKS) % 8] = addr;
					cmt_mvpn[bank][index] = write_mvpn;
					cmt_dirty[bank][index] = 1;

                    break;
				}
            }
        }

		
        nand_write(bank, block, page, buf, &write_lpn);
        stats.nand_write++;
    }

	return;
}