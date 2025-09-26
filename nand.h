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

#define PAGE_DATA_SIZE		32
#define PAGE_SPARE_SIZE		4

/* function prototypes */
int nand_init(int nbanks, int nblks, int npages);
int nand_read(int bank, int blk, int page, void *data, void *spare);
int nand_write(int bank, int blk, int page, void *data, void *spare);
int nand_erase(int bank, int blk);

/* return code */
#define NAND_SUCCESS		0
#define NAND_ERR_INVALID	-1
#define NAND_ERR_OVERWRITE	-2
#define NAND_ERR_POSITION	-3
#define NAND_ERR_EMPTY		-4
