/*
 * Lab #2 : Page usedping FTL Simulator
 *  - Storage Architecture, SSE3069
 *
 * TA: Jinwoo Jeong, Hyunbin Kang
 * Prof: Dongkun Shin
 * Intelligent Embedded Software Laboratory
 * Sungkyunkwan University
 * http://nyx.skku.ac.kr
 */

// #include "ftl.h"
// #include "lab2_tc/ftl2.h"
#if defined(VERSION_V0)
    #include "ftl.h"
#elif defined(VERSION_V1)
    #include "lab2_tc/ftl1.h"
#elif defined(VERSION_V2)
    #include "lab2_tc/ftl2.h"
#elif defined(VERSION_V3)
    #include "lab2_tc/ftl3.h"
#elif defined(VERSION_V4)
    #include "lab2_tc/ftl4.h"
#elif defined(VERSION_V5)
    #include "lab2_tc/ftl5.h"
#elif defined(VERSION_V6)
    #include "lab2_tc/ftl6.h"
#elif defined(VERSION_V7)
    #include "lab2_tc/ftl7.h"
#elif defined(VERSION_V8)
    #include "lab2_tc/ftl8.h"
#endif


u32 pmt[N_LPNS];
int used[N_BANKS][BLKS_PER_BANK * PAGES_PER_BLK];
int freeblock[N_BANKS];


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
            // printf("GC copy read: victim: %d, bank: %d, block: %d, page: %d\n", victim, bank, victim / PAGES_PER_BLK, victim % PAGES_PER_BLK);
            nand_read(bank, victim, i, buf, &spare);
            stats.gc_read++;

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
            // printf("GC copy: bank: %d, block: %d, page: %d, spare: %d, pmt[spare - before]: %d, ", bank, copy_block, copy_page, spare, pmt[spare]);
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
    u32 lpn = lba / SECTORS_PER_PAGE;

    if (pmt[lpn] == -1) {
        memset(read_buffer, 0xff, nsect * SECTOR_SIZE);
    }

    else {
        int addr = pmt[lpn];
        int bank = lpn % N_BANKS;
        int block;
        int page;
        u32* buf = (u32*)malloc(SECTOR_SIZE * SECTORS_PER_PAGE);
        u32 spare;

        addr = addr - bank * N_PPNS_PB;

        block = addr / PAGES_PER_BLK;
        page = addr % PAGES_PER_BLK;

        for (int i = 0; i < nsect; i++) {
            if (i == 0 || (lba + i) % SECTORS_PER_PAGE == 0) {
                // printf("bank: %d, block: %d, page: %d, lpn: %d, pmt[lpn]: %d\n", bank, block, page, lpn, pmt[lpn]);
                nand_read(bank, block, page, buf, &spare);
                // for (int j = 0; j < 8; j++) {
                //     // printf("%x ", buf[j]);
                // }
                // printf("\n");
                stats.nand_read++;

                lpn++;
                bank = lpn % N_BANKS;
                addr = pmt[lpn] - bank * N_PPNS_PB;
                block = addr / PAGES_PER_BLK;
                page = addr % PAGES_PER_BLK;
            }

            read_buffer[i] = buf[(lba + i) % SECTORS_PER_PAGE];
        }

        free(buf);
    }

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
        int bank = lpn % N_BANKS;
        int addr = pmt[lpn] - bank * N_PPNS_PB;
        int block = addr / PAGES_PER_BLK;
        int page = addr % PAGES_PER_BLK;
        u32* buf = (u32*)malloc(SECTOR_SIZE * SECTORS_PER_PAGE);
        int tmp_lpn = lpn;
        if (lba % SECTORS_PER_PAGE != 0) {
            // ftl_read(lba - (lba % SECTORS_PER_PAGE), lba % SECTORS_PER_PAGE, buf);
            nand_read(bank, block, page, buf, &tmp_lpn);
            stats.nand_read++;
            // printf("old data bank: %d, page: %d\n", bank, block * PAGES_PER_BLK + page);
        }
        
        used[bank][block * PAGES_PER_BLK + page] = -1;

        for (int i = 1; i < nsect; i++) {
            if ((lba + i) % SECTORS_PER_PAGE == 0) {
                tmp_lpn++;
                bank = tmp_lpn % N_BANKS;
                addr = pmt[tmp_lpn] - bank * N_PPNS_PB;
                block = addr / PAGES_PER_BLK;
                page = addr % PAGES_PER_BLK;

                if (used[bank][block * PAGES_PER_BLK + page] == 1) {
                    // printf("old data bank: %d, page: %d\n", bank, block * PAGES_PER_BLK + page);
                    used[bank][block * PAGES_PER_BLK + page] = -1;
                }
            }
        }
        

        for (int i = 0; i < nsect; i++) {
            buf[(lba + i) % SECTORS_PER_PAGE] = write_buffer[i];
            // printf("writer_buffer[%d]: %x\n", i, write_buffer[i]);

            if ((lba + i) % SECTORS_PER_PAGE == SECTORS_PER_PAGE - 1) {
                find_next_page(&bank, &block, &page, lpn);
                addr = bank * N_PPNS_PB + block * PAGES_PER_BLK + page;
                pmt[lpn] = addr;
                // for (int j = 0; j < 8; j++) {
                //     // printf("%x ", buf[j]);
                // }
                // printf("\n");
                // printf("bank: %d, block: %d, page: %d, pmt[lpn]: %d\n", bank, block, page, pmt[lpn]);
                nand_write(bank, block, page, buf, &lpn);
                stats.nand_write++;

                lpn++;
            }
        }

        if ((lba + nsect - 1) % SECTORS_PER_PAGE != SECTORS_PER_PAGE - 1) {
            tmp_lpn = (lba + nsect - 1) / SECTORS_PER_PAGE;

            if (pmt[tmp_lpn] != -1) {
                bank = tmp_lpn % N_BANKS;
                addr = pmt[tmp_lpn] - bank * N_PPNS_PB;
                block = addr / PAGES_PER_BLK;
                page = addr % PAGES_PER_BLK;
                u32* tmp_buf = (u32*)malloc(SECTOR_SIZE * SECTORS_PER_PAGE);
                nand_read(bank, block, page, tmp_buf, &tmp_lpn);
                stats.nand_read++;

                for (int i = nsect;; i++) {
                    buf[(lba + i) % SECTORS_PER_PAGE] = tmp_buf[(lba + i) % SECTORS_PER_PAGE];

                    if ((lba + i) % SECTORS_PER_PAGE == SECTORS_PER_PAGE - 1) {
                        find_next_page(&bank, &block, &page, lpn);
                        addr = bank * N_PPNS_PB + block * PAGES_PER_BLK + page;
                        pmt[lpn] = addr;
                        // printf("bank: %d, block: %d, page: %d, pmt[lpn]: %d\n", bank, block, page, pmt[lpn]);
                        nand_write(bank, block, page, buf, &lpn);
                        stats.nand_write++;
                        break;
                    }
                }

                free(tmp_buf);
            }
            
            else {
                for (int i = nsect;; i++) {
                    buf[(lba + i) % SECTORS_PER_PAGE] = 0xffffffff;

                    if ((lba + i) % SECTORS_PER_PAGE == SECTORS_PER_PAGE - 1) {
                        find_next_page(&bank, &block, &page, lpn);
                        addr = bank * N_PPNS_PB + block * PAGES_PER_BLK + page;
                        pmt[lpn] = addr;
                        // printf("bank: %d, block: %d, page: %d, pmt[lpn]: %d\n", bank, block, page, pmt[lpn]);
                        nand_write(bank, block, page, buf, &lpn);
                        stats.nand_write++;
                        break;
                    }
                }
            }
        }

        free(buf);
    }

    // old data does not exist
    else if (pmt[lpn] == -1) {
        int bank;
        int block;
        int page;
        int addr;
        u32* buf = (u32*)malloc(SECTOR_SIZE * SECTORS_PER_PAGE);

        memset(buf, 0xff, (lba % SECTORS_PER_PAGE) * SECTOR_SIZE);

        for (int i = 0; i < nsect; i++) {
            
            buf[(lba + i) % SECTORS_PER_PAGE] = write_buffer[i];

            if ((lba + i) % SECTORS_PER_PAGE == SECTORS_PER_PAGE - 1) {
                find_next_page(&bank, &block, &page, lpn);

                addr = bank * N_PPNS_PB + block * PAGES_PER_BLK + page;

                pmt[lpn] = addr;
                // printf("bank: %d, block: %d, page: %d, pmt[lpn]: %d\n", bank, block, page, pmt[lpn]);
                // for (int j = 0; j < 8; j++) {
                //     // printf("%x ", buf[j]);
                // }
                // printf("\n");
                // printf("bank: %d, block: %d, page: %d, pmt[lpn]: %d\n", bank, block, page, pmt[lpn]);
                nand_write(bank, block, page, buf, &lpn);
                stats.nand_write++;

                lpn++;
            }
        }

        if ((lba + nsect - 1) % SECTORS_PER_PAGE != SECTORS_PER_PAGE - 1) {
            for (int i = nsect;; i++) {
                buf[(lba + i) % SECTORS_PER_PAGE] = 0xffffffff;

                if ((lba + i) % SECTORS_PER_PAGE == SECTORS_PER_PAGE - 1) {
                    find_next_page(&bank, &block, &page, lpn);

                    addr = bank * N_PPNS_PB + block * PAGES_PER_BLK + page;

                    pmt[lpn] = addr;
                    // for (int j = 0; j < 8; j++) {
                    //     // printf("%x ", buf[j]);
                    // }
                    // printf("\n");
                    // printf("bank: %d, block: %d, page: %d, pmt[lpn]: %d\n", bank, block, page, pmt[lpn]);
                    nand_write(bank, block, page, buf, &lpn);
                    stats.nand_write++;
                    break;
                }
            }
        }

        free(buf);
    }

    

    return;
}
