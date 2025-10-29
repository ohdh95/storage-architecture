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


#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

#include "ftl1.h"
// #if defined(VERSION_V0)
//     #include "ftl.h"
// #elif defined(VERSION_V1)
//     #include "testcase/ftl1.h"
// #elif defined(VERSION_V2)
//     #include "testcase/ftl2.h"
// #elif defined(VERSION_V3)
//     #include "testcase/ftl3.h"
// #elif defined(VERSION_V4)
//     #include "testcase/ftl4.h"
// #elif defined(VERSION_V5)
//     #include "testcase/ftl5.h"
// #elif defined(VERSION_V6)
//     #include "testcase/ftl6.h"
// #elif defined(VERSION_V7)
//     #include "testcase/ftl7.h"
// #elif defined(VERSION_V8)
//     #include "testcase/ftl8.h"
// #endif

struct ftl_stats stats;

static void show_info(void)
{
	printf("Bank: %d\n", N_BANKS);
	printf("Blocks / Bank: %d blocks\n", BLKS_PER_BANK);
	printf("Pages / Block: %d pages\n", PAGES_PER_BLK);
	printf("Sectors per Page: %lu\n", SECTORS_PER_PAGE);
	printf("OP ratio: %d%%\n", OP_RATIO);
	printf("Physical Blocks: %d\n", N_BLOCKS);
	printf("User Blocks: %d\n", (int)N_USER_BLOCKS);
	printf("OP Blocks: %d\n", (int)N_OP_BLOCKS);
	printf("PPNs: %d\n", N_PPNS);
	printf("LPNs: %d\n", (int)N_LPNS);
	printf("\n");
}

static u32 get_data()
{
	return rand() & 0xff;
}

static void show_stat(void)
{
	printf("\nResults ------\n");
	printf("Host read: %ld(sectors), write: %ld(sectors)\n", stats.host_read, stats.host_write);
	printf("Nand read: %ld(pages), write: %ld(pages)\n", stats.nand_read, stats.nand_write);
	printf("GC copy: %ld\n", stats.gc_copy);
	printf("Number of GCs: %d\n", stats.gc_cnt);
	printf("Valid page copies per GC: %.2f pages\n", (double)stats.gc_copy / stats.gc_cnt);
	printf("MAP writes : %ld\n", stats.map_write);
	printf("Map GC copy: %ld\n", stats.map_gc_copy);
	printf("Number of MAP GCs : %d\n", stats.map_gc_cnt);
	printf("Valid page copies per Map GC: %.2f pages\n", (double)stats.map_gc_copy / stats.map_gc_cnt);
	printf("Cache hit rate : %.2f %%\n", (double)(stats.cache_hit*100. / (stats.cache_hit + stats.cache_miss)));
	printf("WAF: %.2f\n", (double)((stats.host_write + (stats.gc_copy + stats.map_write + stats.map_gc_copy) * 8.0) / stats.host_write));
}

int main(int argc, char **argv)
{
	if (argc >= 2 && !freopen(argv[1], "r", stdin)) {
		perror("freopen in");
		return EXIT_FAILURE;
	}
	if (argc >= 3 && !freopen(argv[2], "w", stdout)) {
		perror("freopen out");
		return EXIT_FAILURE;
	}

	int seed;
	if (scanf("S %d", &seed) < 1) {
		fprintf(stderr, "wrong input format\n");
		return EXIT_FAILURE;
	}
	srand(seed);

	ftl_open();
	show_info();

	while (1) {
		int i;
		char op;
		u32 lba;
		u32 nsect;
		u32 *buf;
		if (scanf(" %c", &op) < 1)
			break;
		switch (op) {
		case 'R':
			scanf("%d %d", &lba, &nsect);
            			assert(lba >= 0 && lba + nsect <= N_LPNS * SECTORS_PER_PAGE);
						
			buf = malloc(SECTOR_SIZE * nsect);
			ftl_read(lba, nsect, buf);
			printf("Read(%u,%u): [ ", lba, nsect);
			for (i = 0; i < nsect; i++)
				printf("%2x ", buf[i]);
			printf("]\n");
                        free(buf);
			break;
		case 'W':
			scanf("%d %d", &lba, &nsect);
                        assert(lba >= 0 && lba + nsect <= N_LPNS * SECTORS_PER_PAGE);
			buf = malloc(SECTOR_SIZE * nsect);
			for (i = 0; i < nsect; i++)
				buf[i] = get_data();
			ftl_write(lba, nsect, buf);
			printf("Write(%u,%u): [ ", lba, nsect);
			for (i = 0; i < nsect; i++)
				printf("%2x ", buf[i]);
			printf("]\n");
                        free(buf);
			break;
		default:
			fprintf(stderr, "Wrong op type\n");
			return EXIT_FAILURE;
		}
	}

	show_stat();
	return 0;
}
