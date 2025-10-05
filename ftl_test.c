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

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

#include "ftl1.h"
// #if defined(VERSION_V0)
//     #include "ftl.h"
// #elif defined(VERSION_V1)
//     #include "ftl1.h"
// #elif defined(VERSION_V2)
//     #include "ftl2.h"
// #elif defined(VERSION_V3)
//     #include "ftl3.h"
// #elif defined(VERSION_V4)
//     #include "ftl4.h"
// #elif defined(VERSION_V5)
//     #include "ftl5.h"
// #elif defined(VERSION_V6)
//     #include "ftl6.h"
// #elif defined(VERSION_V7)
//     #include "ftl7.h"
// #elif defined(VERSION_V8)
//     #include "ftl8.h"
// #endif
// struct ftl_stats stats;

static void show_info(void)
{
	printf("Bank: %d\n", N_BANKS);
	printf("Blocks / Bank: %d blocks\n", BLKS_PER_BANK);
	printf("Pages / Block: %d pages\n", PAGES_PER_BLK);
	printf("Sectors per Page: %lu\n", SECTORS_PER_PAGE);
	printf("OP ratio: %d%%\n", OP_RATIO);
	printf("Physical Blocks: %d\n", N_BLOCKS);
	printf("User Blocks: %d\n", N_USER_BLOCKS);
	printf("OP Blocks: %d\n", N_OP_BLOCKS);
	printf("PPNs: %d\n", N_PPNS);
	printf("LPNs: %d\n", N_LPNS);
	printf("\n");
}

static u32 get_data()
{
	return rand() & 0xff;
}

/* static void show_stat(void)
{
	printf("\nResults ------\n");
	printf("Host writes: %d\n", stats.host_write);
	printf("NAND reads: %d\n", stats.nand_read*8);
	printf("NAND writes: %d\n", stats.nand_write*8);
	printf("GC writes: %d\n", stats.gc_write*8);
	printf("Number of GCs: %d\n", stats.gc_cnt);
	printf("Valid pages per GC: %.2f pages\n", (double)stats.gc_write / stats.gc_cnt);
	printf("WAF: %.2f\n", (double)(stats.nand_write + stats.gc_write) * 8 / stats.host_write);
} */

int main(int argc, char **argv)
{
	int NBANKS, NBLOCKS, NPAGES;
	if (argc >= 2 && !freopen(argv[1], "r", stdin)) {
		perror("freopen in");
		return EXIT_FAILURE;
	}
	if (argc >= 3 && !freopen(argv[2], "w", stdout)) {
		perror("freopen out");
		return EXIT_FAILURE;
	}

	int seed, nbank, nblk, npage;
	if (scanf("S %d %d %d %d", &seed, &nbank, &nblk, &npage) < 4) {
		fprintf(stderr, "wrong input format\n");
		return EXIT_FAILURE;
	}
	assert(N_BANKS == nbank && BLKS_PER_BANK == nblk && PAGES_PER_BLK == npage);
	srand(seed);

	ftl_open();
	show_info();

	while (1) {
		int i;
		char op;
		u32 lba, lpn;
		u32 nsect, npage;
		u32 *buf;
		if (scanf(" %c", &op) < 1)
			break;
		switch (op) {
		case 'R':
			scanf("%d %d", &lba, &nsect);
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
			buf = malloc(SECTOR_SIZE * nsect);
			for (i = 0; i < nsect; i++)
				buf[i] = get_data();
			ftl_write(lba, nsect, buf);
			printf("Write(%u,%u): [", lba, nsect);
			for (i = 0; i < nsect; i++)
				printf("%2x ", buf[i]);
			printf("]\n");
			free(buf);
			break;
		case 'T':
			scanf("%d %d", &lpn, &npage);
			ftl_trim(lpn, npage);
			printf("Trim(%u,%u)\n", lpn, npage);
			break;
		case 'F':
			ftl_flush();
			printf("Flush()\n");
			break;
		default:
			fprintf(stderr, "Wrong op type\n");
			return EXIT_FAILURE;
		}
	}

	// show_stat();
	return 0;
}
