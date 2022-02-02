#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#include "banggen.inc"

#define	MAXBANG	4096

struct fentry {
	int	blkno;	// First block for this file
	int nblks;	// # of blocks for this file
} files[MAXBANG];

int nfile;

typedef int (*fp_alloc_t)(int);
typedef int (*fp_free_t)(int, int); 
typedef void (*fp_vinit_t)(void);
typedef void (*fp_vref_t)(int, int);
typedef void (*fp_vchk_t)(void);

void
ssbanger(int nbang, fp_alloc_t fp_alloc, fp_free_t fp_free,
			fp_vinit_t fp_vinit, fp_vref_t fp_vref, fp_vchk_t fp_vchk)
{
	int i;
	int fe;
	uint32_t nb;
	int nallocreq = 0;
	int nallocok = 0;
	int nallocng = 0;

	srand(1);

	for (fe = 0 ; fe < MAXBANG ; fe++) {
		files[fe].blkno = -1;
	}

	nfile = 0;

	for (i = 0 ; i < nbang ; i++) {
		int aorf;
		aorf = rand();
		aorf = ((aorf >> 24) & 0xff)
			^ ((aorf >> 16) & 0xff)
			^ ((aorf >> 8) & 0xff)
			^ ((aorf >> 0) & 0xff);

		if (aorf >= 64) {
			// Alloc
			if (nfile == MAXBANG)
				continue;

			nallocreq++;

			// Find an empty slot in files[];
			for (fe = 0 ; fe < MAXBANG ; fe++) {
				if (files[fe].blkno < 0)
					break;
			}
			if (fe == MAXBANG) {
				// Panic; nfile != MAXBANG but no free in files[].
				fprintf(stderr, "ssbanger: panic: no free files[] entry\n");
				exit(1);
			}

			// Calculate request size
			do {
				nb = banggen(0.0, 0.5, 4);
			} while (nb == 0);

			dprintf(("ssbanger(%d): a %d\n", nallocreq, nb));

			if ((files[fe].blkno = (*fp_alloc)(nb)) >= 0) {
				files[fe].nblks = nb;
				printf("ssbanger: A %d %d\n", files[fe].blkno, files[fe].nblks);
				nfile++;
				nallocok++;
			} else {
				printf("ssbanger: A failed for %d\n", nb);
				nallocng++;
			}
		} else {
			// Free
			if (nfile == 0)
				continue;

			// Select an entry to free
			int re;
			re = rand() % nfile;

			// printf("ssbanger: re %d\n", re);

			for (fe = 0 ; fe < MAXBANG ; fe++) {
				if (files[fe].blkno >= 0) {
					if (--re < 0) {
						// This is the entry to free
						dprintf(("ssbanger: f %d %d\n", files[fe].blkno, files[fe].nblks));
						(*fp_free)(files[fe].blkno, files[fe].nblks);
						printf("ssbanger: F %d %d\n", files[fe].blkno, files[fe].nblks);
						nfile--;
						files[fe].blkno = -1;
						break;
					}
				}
			}

			if (fe == MAXBANG) {
				fprintf(stderr, "ssbanger: panic: failed to select a release entry\n");
				exit(1);
			}
		}
	}

	printf("ssbanger: total %d allocreq %d success %d fail %d\n", nbang, nallocreq, nallocok, nallocng);

#if 1
	// Debuggin'

	printf("ssbanger: dumping\n");

	for (fe = 0 ; fe < MAXBANG ; fe++) {
		if (files[fe].blkno < 0)
			continue;
		printf("[%d] %d %d\n", fe, files[fe].blkno, files[fe].nblks);
	}

#endif

	if (fp_vinit && fp_vref && fp_vchk) {
		(*fp_vinit)();
		for (fe = 0 ; fe < MAXBANG ; fe++) {
			if (files[fe].blkno >= 0)
				(*fp_vref)(files[fe].blkno, files[fe].nblks);
		}
		(*fp_vchk)();
	}
}
