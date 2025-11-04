/*
 * Lab #5 : ZNS+ Simulator
 *  - Storage Architecture, SSE3069 *
 *
 * TA: Youngjin Kim, Eunji Song
 * Prof: Dongkun Shin
 * Intelligent Embedded Software Laboratory
 * Sungkyunkwan University
 * http://nyx.skku.ac.kr
 */

#include "zns.h"
#include <stdlib.h>
#include <stdio.h>

/************ constants ************/
int NBANK; // total bank
int NBLK; // block per bank
int NPAGE; // page per block

int DEG_ZONE; // block per zone
int MAX_OPEN_ZONE;

int NUM_FCG; // number of chip
/********** do not touch ***********/
int* zone_map;
struct zone_desc* desc_table;
int open_zone_count = 0;
u32** zone_buf;
int* zone_buf_use;
int* zone_buf_start;

typedef struct freeblock_queue {
    int* data; // 데이터를 저장할 배열
    int front; // 큐의 시작 위치 (인덱스)
    int rear;  // 큐의 끝 위치 (인덱스)
    int size;  // 현재 저장된 요소의 개수
} Queue;

Queue* free_block_queues; // 각 FCG별로 관리하는 자유 블록 큐 배열

int isFull(Queue* q) {
    return (q->size == NBLK);
}

int isEmpty(Queue* q) {
    return (q->size == 0);
}

int enqueue(Queue* q, int data) {
    // 1. 큐가 꽉 찼는지 확인
    if (isFull(q)) {
        printf("오류: 큐가 꽉 찼습니다 (Enqueue 실패)\n");
        return 0; // 실패
    }
    
    // 2. rear 위치를 원형으로 한 칸 이동
    // (MAX_QUEUE_SIZE로 나눈 나머지를 사용)
    q->rear = (q->rear + 1) % NBLK;

    // 3. 새 rear 위치에 데이터 삽입
    q->data[q->rear] = data;
    q->size++; // 크기 1 증가
    
    // printf("%d enqueue 완료 (front: %d, rear: %d, size: %d)\n", data, q->front, q->rear, q->size);
    return 1; // 성공
}

int dequeue(Queue* q) {
    // 1. 큐가 비어있는지 확인
    if (isEmpty(q)) {
        printf("오류: 큐가 비어있습니다 (Dequeue 실패)\n");
        return -1; // 오류 값
    }
    
    // 2. front 위치의 데이터를 가져옴
    int data = q->data[q->front];
    
    // 3. front 위치를 원형으로 한 칸 이동
    q->front = (q->front + 1) % NBLK;
    q->size--; // 크기 1 감소
    
    // printf("%d dequeue 완료 (front: %d, rear: %d, size: %d)\n", data, q->front, q->rear, q->size);
    return data;
}

void zns_init(int nbank, int nblk, int npage, int dzone, int max_open_zone)
{
	// constants
	NBANK = nbank;
	NBLK = nblk;
	NPAGE = npage;
	DEG_ZONE = dzone;
	MAX_OPEN_ZONE = max_open_zone;
	NUM_FCG = NBANK / DEG_ZONE;

	// nand
	nand_init(nbank, nblk, npage);

	zone_map = (int*)malloc(sizeof(int) * MAX_ZONE);

	for (int i = 0; i < MAX_ZONE; i++) {
		zone_map[i] = -1; // 초기값 설정
	}

	desc_table = (struct zone_desc*)malloc(sizeof(struct zone_desc) * MAX_ZONE);

	for (int i = 0; i < MAX_ZONE; i++) {
		desc_table[i].state = ZONE_EMPTY;
		desc_table[i].slba = i * ZONE_SIZE;
		desc_table[i].wp = i * ZONE_SIZE;
	}
	
	free_block_queues = (Queue*)malloc(sizeof(Queue) * NUM_FCG);
	
	for (int i = 0; i < NUM_FCG; i++) {
		free_block_queues[i].data = (int*)malloc(sizeof(int) * NBLK);
		free_block_queues[i].front = 0;  // front는 다음에 dequeue 될 위치
		free_block_queues[i].rear = -1;  // rear는 마지막에 enqueue 된 위치
		free_block_queues[i].size = 0;   // 현재 크기는 0

		for (int j = 0; j < NBLK; j++) {
			enqueue(&free_block_queues[i], j);
		}
	}

	zone_buf = (u32**)malloc(sizeof(u32*) * MAX_ZONE);

	for (int i = 0; i < MAX_ZONE; i++) {
		zone_buf[i] = (u32*)malloc(SECT_SIZE * NSECT);
	}

	zone_buf_use = (int*)malloc(sizeof(int) * MAX_ZONE);

	for (int i = 0; i < MAX_ZONE; i++) {
		zone_buf_use[i] = 0; // 초기값 설정
	}

	zone_buf_start = (int*)malloc(sizeof(int) * MAX_ZONE);

	for (int i = 0; i < MAX_ZONE; i++) {
		zone_buf_start[i] = 0; // 초기값 설정
	}
}

int zns_write(int start_lba, int nsect, u32 *data)
{
	int zone = lba_to_zone(start_lba);
	int fcg = zone % NUM_FCG;
	int lba_offset = start_lba - zone * DEG_ZONE * NPAGE * NSECT;
	int lpn_offset = lba_offset / NSECT;
	// int bank = lpn_offset % DEG_ZONE + fcg * DEG_ZONE;
	int block;
	// int page = lpn_offset / DEG_ZONE;
	u32* buf = (u32*)malloc(SECT_SIZE * NSECT);

	for (int i = 0; i < NSECT; i++) {
		buf[i] = 0xFFFFFFFF;
	}

	if (zone_map[zone] == -1) {
		block = dequeue(&free_block_queues[fcg]);

		if (block == -1) {
			printf("FCG %d에 free block이 없습니다\n", fcg);
			free(buf);
			return -1; // 실패
		}

		if (open_zone_count == MAX_OPEN_ZONE) {
			free(buf);
			return -1; // 실패
		}

		zone_map[zone] = block;

		open_zone_count++;
		// printf("open_zone_count: %d\n", open_zone_count);

		desc_table[zone].state = ZONE_OPEN;
		desc_table[zone].slba = start_lba - lba_offset;
		desc_table[zone].wp = start_lba - lba_offset;
	}

	else {
		if (desc_table[zone].state != ZONE_OPEN) {
			// printf("존 %d이 오픈 상태가 아닙니다\n", zone);
			free(buf);
			return -1; // 실패
		}

		if (desc_table[zone].wp - (start_lba - lba_offset) != lba_offset) {
			free(buf);
			return -1; // 실패
		}

		block = zone_map[zone];
	}

	if (zone_buf_use[zone] == 1) {
		for (int i = 0; i < NSECT; i++) {
			buf[i] = zone_buf[zone][i];
		}
	}

	for (int i = 0; i < nsect; i++) {
		buf[(lba_offset + i) % NSECT] = data[i];

		if ((lba_offset + i) % NSECT == NSECT - 1) {
			int write_lpn = (lba_offset + i) / NSECT;
			int write_bank = write_lpn % DEG_ZONE + fcg * DEG_ZONE;
			int write_block = block;
			int write_page = write_lpn / DEG_ZONE;
			int spare = 0;

			nand_write(write_bank, write_block, write_page, buf, &spare);

			for (int j = 0; j < NSECT; j++) {
				buf[j] = 0xFFFFFFFF;
			}

			if (lba_offset + i  == NSECT * NPAGE * DEG_ZONE - 1) {
				desc_table[zone].state = ZONE_FULL;
				open_zone_count--;
				// printf("open_zone_count: %d\n", open_zone_count);
			}

			zone_buf_use[zone] = 0;
		}
	}

	if ((start_lba + nsect - 1) % NSECT != NSECT - 1) {
		for (int i = 0; i < NSECT; i++) {
			zone_buf[zone][i] = buf[i];
		}

		zone_buf_use[zone] = 1;
		zone_buf_start[zone] = (lba_offset + nsect - 1) / NSECT; // start_lba - zone * (DEG_ZONE * NPAGE) * NSECT
	}

	desc_table[zone].wp = desc_table[zone].wp + nsect;

	free(buf);

	return 0;
}

void zns_read(int start_lba, int nsect, u32 *data)
{
	int zone = lba_to_zone(start_lba);
	int fcg = zone % NUM_FCG;
	int lba_offset = start_lba - zone * (DEG_ZONE * NPAGE) * NSECT;
	int lpn_offset = lba_offset / NSECT;
	// int bank = lpn_offset % DEG_ZONE + fcg * DEG_ZONE;
	int block = zone_map[zone];
	// int page = lpn_offset / (DEG_ZONE);
	u32* buf = (u32*)malloc(SECT_SIZE * NSECT);
	int wp = desc_table[zone].wp - (start_lba - lba_offset);

	for (int i = 0; i < nsect; i++) {
		if (lba_offset + i >= wp) {
			data[i] = 0xFFFFFFFF;
		}

		else if (i == 0 || (lba_offset + i) % NSECT == 0) {
			if (zone_buf_use[zone] == 1 && (lba_offset + i) / NSECT == zone_buf_start[zone]) {
				for (int j = 0; j < NSECT; j++) {
					buf[j] = zone_buf[zone][j];
				}
			} 
			
			else {
				int read_lpn = (lba_offset + i) / NSECT;
				int read_bank = read_lpn % DEG_ZONE + fcg * DEG_ZONE;
				int read_block = block;
				int read_page = read_lpn / DEG_ZONE;
				int spare = 0;

				nand_read(read_bank, read_block, read_page, buf, &spare);
			}

			for (int j = 0;; j++) {
				data[i + j] = buf[(start_lba + i + j) % NSECT];
				
				if ((start_lba + i + j) % NSECT == NSECT - 1 || i + j == nsect - 1)
					break;
			}
		}
	}

	free(buf);

	return;
}

int zns_reset(int lba)
{
	int zone = lba_to_zone(lba);
	int fcg = zone % NUM_FCG;
	int lpn_offset = lba / NSECT - zone * (DEG_ZONE * NPAGE);
	int bank = lpn_offset % DEG_ZONE + fcg * DEG_ZONE;
	int block = zone_map[zone];

	if (desc_table[zone].state != ZONE_FULL) {
		return -1;
	}
	
	desc_table[zone].state = ZONE_EMPTY;
	desc_table[zone].slba = zone * ZONE_SIZE;
	desc_table[zone].wp = zone * ZONE_SIZE;

	nand_erase(bank, block);

	enqueue(&free_block_queues[fcg], block);

	return 0;
}

void zns_get_desc(int lba, int nzone, struct zone_desc *descs)
{
	int zone = lba_to_zone(lba);

	for (int i = 0; i < nzone; i++) {
		descs[i].state = desc_table[zone + i].state;
		descs[i].slba = desc_table[zone + i].slba;
		descs[i].wp = desc_table[zone + i].wp;
	}

	return;
}

int zns_izc(int src_zone, int dest_zone, int copy_len, int *copy_list)
{
}

int zns_tl_open(int zone, u8 *valid_arr)
{
}
