#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include "getcmd.c"

#define dprintf(x) printf x

#include "ssbanger.inc"

#define	SSBLKS	4096 

int fbt[SSBLKS+1];		//an array with size of 4096 to store disk block allocation informations
//fbt[0] must not be used

// Sanity
int fbtfree;

// Macros to manipulate bits in the bitmap
//#define	BM_ISCLR(n)	(!(bm[(n)/BMEBITS] & (1 << ((n) % BMEBITS))))
//#define	BM_SET(n) bm[(n)/BMEBITS] |= (1 << ((n) % BMEBITS))
//#define	BM_CLR(n) bm[(n)/BMEBITS] &= ~(1 << ((n) % BMEBITS))

void
fbt_init()
{
	memset(fbt, 0, sizeof(fbt));	// Mark all blocks as 'free'.

	fbtfree = SSBLKS;
}

int
fbt_alloc(int nblks)
{
	int file_head = 0;  //the start point of created file
	int head;  // search head
	int count = 0;
	int temp;

	// Nested for loop here, to find:
	// Outer loop: Start of a new contigeous free blocks
    if(nblks > fbtfree){    //make sure there are enough free blocks
        printf("A failed\n");
        printf("no enough free blocks\n");
        return -1;
    }
    else if(head > (SSBLKS+1)){ //quit if requested address beyond array limit
        printf("address not accessible\n");
        return -1;
    }
    else{
        for(head = 1; head < SSBLKS+1; head++){
            if(count == nblks){ //quit if all n blocks are allocated
                break;
            }
            else if(fbt[head] == 0){
                if(count > 0){
                    fbt[temp] = head;
                    fbt[head] = 1;
                    temp = head;
                    count++;

                }
                else{   //special treatment for the first link in a file block linked list
                    file_head = head;
                    fbt[head] = 1;
                    temp = head;
                    count++;
                }
            }
        }
    }
    fbt[temp] = -1;
    fbtfree = fbtfree - nblks;
    return file_head;
}

int
fbt_free(int blkno, int nblks)
{
	int head = blkno;
	int temp;

	//free all the blocks in the linked list until tail is detected
    while(fbt[head] != -1){
        if(fbt[head] != 0){
            temp = head;
            head = fbt[head];
            fbt[temp] = 0;
        }
        else
            return -1;
    }
    fbtfree = fbtfree + nblks;
	return 0;
}

//display the linked list information
void
fbt_list(int head)
{
    int count = 0;
    int cnt = 0;
    if (head == 0){ //display all the information
        for(count = 1; count < SSBLKS+1; count++){
            printf("(%d)\t%d\t",count,fbt[count]);
            if(count % 8 ==0)
                printf("\n");
            }
    }
    else if (head == -1){   //display information of all the free blocks
        for(count = 1; count < SSBLKS+1; count++){
            if (fbt[count] == 0){
                cnt++;
                printf("(%d)\t%d\t",count,fbt[count]);
            }
            if(cnt>0 && cnt % 8 == 0)
                printf("\n");
            }
    }
    else if (head > 0 && head < (SSBLKS+1)){    //display information from certain start point
        if (fbt[head] == 0){    //quit when there is an error of list record
            printf("given file head error\n");
            return;
        }
        while(fbt[head] != -1){
            count++;
            printf("(%d)\t%d\t",head, fbt[head]);
            if(count % 8 ==0)
                printf("\n");
            head = fbt[head];
        }
        printf("(%d)\t%d\t",head, fbt[head]);
        printf("\n");
    }
    printf("\n");
}

void
fbt_verify()
{
    int free;
    int count;
    for(count = 1; count < SSBLKS+1; count++){
        if(fbt[count]==0){
            free++;
        }
    }
    printf("TOTAL BLOCK NUMBERS: %d, RECORDED FREE BLOCKS: %d, COUNTED FREE BLOCKS: %d\n", SSBLKS,fbtfree,free);
    if(fbtfree == free)
        printf("OK\n");
    else
        printf("NG\n");

}

void
fbt_interactive()
{
        int bn;
        char cmd;
        int param1, param2;
        int ic;

        for (;;) {
                fputs("fbt>", stdout); fflush(stdout);

                switch((ic = getcmd(&cmd, &param1, &param2))) {
                case 0: // EOF
                        goto out;

                case 1: // Command only
                        if (cmd == 'd')
                                fbt_list(0);
                        else if (cmd == 'v')
                                fbt_verify();
                        break;

                case 2: // Command and a parameter
                        if (cmd == 'a') {
                                bn = fbt_alloc(param1);
                                printf("A %d %d\n",bn, param1);

                        }
#if 0
                        } else if (cmd == 'p') {
                                fbt_print(param1);
#endif
                        else if (cmd == 'd'){
                                fbt_list(param1);
                        }

                        else{
                                fprintf(stderr, "bad command '%c' "
                                                "or invalid parameter(s)\n", cmd);
                        }
                        break;

                case 3: // Command and two parameters
                        if (cmd == 'f') {
                                fbt_free(param1, param2);
                                printf("F %d %d\n", param1, param2);
                        } else {
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
	fbt_init();

//#if 1
	//fbt_interactive();
//#else
	ssbanger(10000, fbt_alloc, fbt_free, NULL, NULL, NULL);
//#endif

	fbt_list(1);
	fbt_verify();
}
