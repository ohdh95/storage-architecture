/*
 * Lab #2 : Page Mapping FTL Simulator
 *  - Storage Architecture, SSE3069
 *
 * TA: Jinwoo Jeong, Hyunbin Kang
 * Prof: Dongkun Shin
 * Intelligent Embedded Software Laboratory
 * Sungkyunkwan University
 * http://nyx.skku.ac.kr
 *
 */

#include "ftl.h"

struct ftl_stats stats;

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

static void show_stat(void)
{
	printf("\nResults ------\n");
	printf("Host read: %d(sectors), write: %d(sectors)\n", stats.host_read, stats.host_write);
	printf("Nand read: %d(pages), write: %d(pages)\n", stats.nand_read, stats.nand_write);
	printf("GC read: %d, GC write: %d\n", stats.gc_read, stats.gc_write);
	printf("Number of GCs: %d\n", stats.gc_cnt);
	printf("Valid copies per GC: %.2f pages\n", (double)stats.gc_write / stats.gc_cnt);
	printf("WAF: %.2f\n", (stats.host_write + (double)(stats.gc_write*SECTORS_PER_PAGE)) / stats.host_write);
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
			buf = malloc(SECTOR_SIZE * nsect);
			ftl_read(lba, nsect, buf);
			printf("Read(%u,%u): [ ", lba, nsect);
			for (i = 0; i < nsect; i++)
				printf("%2x ", buf[i]);
			printf("]\n");
			//free(buf);
			break;
		case 'W':
			scanf("%d %d", &lba, &nsect);
			buf = malloc(SECTOR_SIZE * nsect);
			for (i = 0; i < nsect; i++)
				buf[i] = get_data();
			ftl_write(lba, nsect, buf);
			printf("Write(%u,%u): [ ", lba, nsect);
			for(i = 0; i < nsect ; i++)
				printf("%2x ",buf[i]);
			printf("]\n");
			//free(buf);
			break;
		default:
			fprintf(stderr, "Wrong op type\n");
			return EXIT_FAILURE;
		}
	}

	show_stat();
	return 0;
}
