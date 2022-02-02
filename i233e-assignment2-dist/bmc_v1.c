#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>  

#include "getcmd.c"
#include "ssbanger.inc"

#define dprintf(x) printf x
#define	SSBLKS	4096

// Data type and associated definitions to store the bitmap as integer->
typedef uint32_t bme_t;			// Bitmap element type
#define BMEBITS (sizeof(bme_t) * 8)	// # of bits in the bme_t

// Bitmap storage
bme_t bm[SSBLKS/BMEBITS];		// Large enough to store SSBLKS bits

// Sanity
int bmfree;

// Macros to manipulate bits in the bitmap
#define	BM_ISCLR(n)	(!(bm[(n)/BMEBITS] & (1 << ((n) % BMEBITS))))
#define	BM_SET(n) bm[(n)/BMEBITS] |= (1 << ((n) % BMEBITS))
#define	BM_CLR(n) bm[(n)/BMEBITS] &= ~(1 << ((n) % BMEBITS))

//file record system that record head and blocks of each files
struct _files{
    int file_head;
    int blks;
    struct _files *next;
};

struct _files *header, *tail;

struct _files
*init_file()
{
    header = (struct _files*)malloc(sizeof(struct _files));
    header->next = NULL;
    header->file_head = -1;
    header->blks = -1;

    tail = header;
    return header;
}

void
add_file(int file_head, int blks)
{
    struct _files *node;
    node = (struct _files*)malloc(sizeof(files));
    node->file_head = file_head;
    node->blks = blks;
    node->next = NULL;
    tail->next =  node;
    tail = node;

}

struct _files
*delete_file(int file_head, int blks)
{
    struct _files *p, *q;
    for(p = header; p->next->file_head != file_head; p = p->next){
        if(p->next == NULL){
            printf("given header does not exists\n");
            return header;
        }
    }
    //if deleted node is not tail
    if(blks == p->next->blks && p->next->next != NULL){
        q = p->next;
        p->next = p->next->next;
        free(q);
        return header;
    }
    //if deleted node is tail
    else if(blks == p->next->blks && p->next->next == NULL){
        free(p->next);
        p->next = NULL;
        tail = p;
        return header;
    }
    else{
        p->next->blks -= blks;
        p->next->file_head += blks;
        return header;
    }
}

int
print_files()
{
    int nfiles = -1;
    struct _files *p;
    for(p = header;p->next != NULL; p = p->next){
        printf("%d \t%d\n",p->file_head,p->blks);
        nfiles++;
        }
    printf("number of files is %d\n",nfiles);
    return nfiles;
}

void
bmc_init()
{
	memset(bm, 0, sizeof(bm));	// Mark all blocks as 'free'
	bmfree = SSBLKS;
}

int
bmc_alloc(int nblks)
{

	int count; // current contigeous free block count
	int limit; // search limit
    int head;

	if (nblks > bmfree){    //make sure there are enough free blocks
        printf("A failed\n");
        return -1;
	}

    // Nested for loop here, to find:
	// Outer loop: Start of a new contigeous free blocks
    for (head = 0,limit = SSBLKS - nblks; head < limit; head++){
        if(BM_ISCLR(head)){

    // Inner loop: Test if requested block 'nblks' can be allocated starting at 'head'
            for(count = 1; count < nblks; count++){
                if(!BM_ISCLR(head + count)){
                    head = head + count;
                    goto next;  //n consecutive blocks found
                }
            }
            for(count = 0; count < nblks; count++){
                BM_SET(head + count);
            }

            bmfree = bmfree - nblks;
            add_file(head, nblks);
            return head;
            }

        next:;
    }
	printf("%d consecutive blocks are not available\n", nblks);
	// When requested space is not available, return -1 to indicate an error
	return -1;
}

int
bmc_free(int blkno, int nblks)
{
	int count;

	for(count = 0; count < nblks; count++){
        BM_CLR(blkno + count);
    }
    bmfree = bmfree + nblks;
    delete_file(blkno, nblks);
	return 0;
}

void
bmc_dump(int head)
{
    int count;
    int i;
    count = head;

    if (count >= 0){    //display the management space from given start point
        for(i = 0; i < count; i++){
            putchar('*');   //mark unnecessary blocks as "*"
            if(((1+i) % 32) == 0){
                printf("\n");
            }
        }
        for(; count < SSBLKS; count++){
            putchar(BM_ISCLR(count)? '0' : '1');
            if(((1+count) % 32) == 0){
                printf("\n");
            }
        }
    }
    else if(count = -1){    //display the total management space
        for(count =0; count < SSBLKS; count++){
            putchar(BM_ISCLR(count)? '0' : '*');    //mark unnecessary blocks as "*"
            if(((1+count) % 32) == 0){
                printf("\n");
            }
        }
    }

}

void
bmc_verify()
{
    int free = 0;
    int count;
    for(count = 0; count < SSBLKS; count++){
        if(BM_ISCLR(count)){
            free++;
        }
    }
    printf("TOTAL BLOCK NUMBERS: %d, RECORDED FREE BLOCKS: %d, COUNTED FREE BLOCKS: %d \n", SSBLKS,bmfree,free);

    if(bmfree = free)
        printf("OK\n");
    else
        printf("NG\n");
}

void
bmc_test(){

}

void
bmc_interactive()
{
        int bn;
        char cmd;
        int param1, param2;
        int ic;

        for (;;) {
                fputs("bmc>", stdout); fflush(stdout);

                switch((ic = getcmd(&cmd, &param1, &param2))) {
                case 0: // EOF
                        goto out;

                case 1: // Command only
                        if (cmd == 'd')
                                bmc_dump(0);
                        else if (cmd == 'v')
                                bmc_verify();
                        else if(cmd == 'p'){
                                print_files();
                        }
                        break;

                case 2: // Command and a parameter
                        if (cmd == 'a') {
                                bn = bmc_alloc(param1);
                                printf("A %d %d\n", bn, param1);
                        }
#if 0
                        } else if (cmd == 'p') {
                                bmc_print(param1);

#endif
                        else if(cmd == 'd'){
                                bmc_dump(param1);
                        }

                        else {
                                fprintf(stderr, "bad command '%c' "
                                                "or invalid parameter(s)\n", cmd);
                        }
                        break;

                case 3: // Command and two parameters
                        if (cmd == 'f') {
                                bmc_free(param1, param2);
                                printf("F %d %d\n", param1,param2);
                        }

                        else {
                                fprintf(stderr, "bad command '%c' "
                                                "or invalid parameter(s)\n", cmd);
                        }
                        break;

                case -1: // Error
                        break;
                }
        }

out:;
}

int
main()
{
	bmc_init();
	init_file();

//#if 1
	//bmc_interactive();
//#else
	ssbanger(10000, bmc_alloc, bmc_free, NULL, NULL, NULL);
//#endif

	bmc_dump(0);
	bmc_verify();
}
