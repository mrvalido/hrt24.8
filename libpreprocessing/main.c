


#include "preprocessing/def.h"
#include "preprocessing/mydef.h"
#include "preprocessing/vmem.h"
#include "preprocessing/ana.h"
#include "preprocessing/arith.h"
#include "../libeve/eve/fixed_point.h"

/* from std c */
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>

#include "FITS_Interface.h"
#include "preprocessing/hrt.h"

void writeImageToFile(int32_t *img, char *fileName, int positionIndex,  int index, uint32_t stdimagesize ){
	FILE *fp;
	char file[20];
	strcpy(file,fileName);

	if(positionIndex >= 0){
		if(index < 10){
			file[positionIndex] = 48 + index;
		}else{
			int decimal = index/10;
			int subDecimal = index-decimal*10;
			file[positionIndex-1] = 48 + decimal;
			file[positionIndex] = 48 + subDecimal;
		}
	}

	fp=fopen(file,"wb");
	fwrite(img,sizeof(int32_t),stdimagesize,fp);
	fclose(fp);

	printf("File %s written successfully!\n", file);
}


/*
 * * * * * * *
 * * MAIN  * *
 * * * * * * *
 */


int main()
{

	int32_t *SDRAM;
	int32_t *img01;
	int32_t *img02;
	int32_t *masks;
	int32_t *cons;
	int32_t *pixCon;
	int32_t *tmp1;
	int32_t *tmp2;
	int32_t *tmp3;
	int32_t *tmp4;
	int32_t *tmp5;
	int32_t *tmp6;

	uint32_t stdimagesize=ROWS*COLS;
	uint32_t numberOfMemoryInput = 16;

	int status = PREPROCESSING_SUCCESSFUL;

	printf ("Start!\n");

	/*
	 * Memory allocation
	 * Corresponds to part of copying images to SDRAM, total size of virtual RAM
	 */
	SDRAM = (int32_t*) malloc(numberOfMemoryInput*stdimagesize*sizeof(int32_t));

	printf("Load images in Virtual RAM!\n");

	/*
	 * * * * * * * * *
	 * memory mapping*
	 * * * * * * * * *
	 */

	//	1.) Load image data to (virtual) SDRAM:
	uint32_t 	img01Sdram = 0;
	uint32_t 	img01Size = stdimagesize;
	uint32_t 	img01DatasetId = 1;

	uint32_t 	img02Sdram = img01Sdram + img01Size;
	uint32_t 	img02Size = stdimagesize;
	uint32_t 	img02DatasetId = 2;

	uint32_t	masksSdram = img02Sdram + img02Size;
	uint32_t	masksSize = stdimagesize;
	uint32_t	masksDatasetId = 3;

	uint32_t	consSdram = masksSdram + masksSize;
	uint32_t	consSize = stdimagesize;
	uint32_t	consDatasetId = 4;

	uint32_t	pixConSdram = consSdram + consSize;
	uint32_t	pixConSize = stdimagesize;
	uint32_t	pixConDatasetId = 5;

	uint32_t	tmp1Sdram = pixConSdram + pixConSize;
	uint32_t	tmp1Size = stdimagesize;
	uint32_t	tmp1DatasetId = 6;

	uint32_t	tmp2Sdram = tmp1Sdram + tmp1Size;
	uint32_t	tmp2Size = stdimagesize;
	uint32_t	tmp2DatasetId = 7;

	uint32_t	tmp3Sdram = tmp2Sdram + tmp2Size;
	uint32_t	tmp3Size = stdimagesize;
	uint32_t	tmp3DatasetId = 8;

	uint32_t	tmp4Sdram = tmp3Sdram + tmp3Size;
	uint32_t	tmp4Size = stdimagesize;
	uint32_t	tmp4DatasetId = 9;

	uint32_t	tmp5Sdram = tmp4Sdram + tmp4Size;
	uint32_t	tmp5Size = stdimagesize;
	uint32_t	tmp5DatasetId = 10;

	uint32_t	tmp6Sdram = tmp5Sdram + tmp5Size;
	uint32_t	tmp6Size = stdimagesize;
	uint32_t	tmp6DatasetId = 11;

	img01=(SDRAM+img01Sdram);
	img02=(SDRAM+img02Sdram);
	masks=(SDRAM+masksSdram);
	cons=(SDRAM+consSdram);
	pixCon=(SDRAM+pixConSdram);
	tmp1=(SDRAM+tmp1Sdram);
	tmp2=(SDRAM+tmp2Sdram);
	tmp3=(SDRAM+tmp3Sdram);
	tmp4=(SDRAM+tmp4Sdram);
	tmp5=(SDRAM+tmp5Sdram);
	tmp6=(SDRAM+tmp6Sdram);

	preprocessing_vmem_setEntry(img01Sdram, img01Size, img01DatasetId, img01);
	preprocessing_vmem_setEntry(img02Sdram, img02Size, img02DatasetId, img02);

	preprocessing_vmem_setEntry(masksSdram, masksSize, masksDatasetId, masks);
	preprocessing_vmem_setEntry(consSdram, consSize, consDatasetId, cons);
	preprocessing_vmem_setEntry(pixConSdram, pixConSize, pixConDatasetId, pixCon);

	preprocessing_vmem_setEntry(tmp1Sdram, tmp1Size, tmp1DatasetId, tmp1);
	preprocessing_vmem_setEntry(tmp2Sdram, tmp2Size, tmp2DatasetId, tmp2);
	preprocessing_vmem_setEntry(tmp3Sdram, tmp3Size, tmp3DatasetId, tmp3);
	preprocessing_vmem_setEntry(tmp4Sdram, tmp4Size, tmp4DatasetId, tmp4);
	preprocessing_vmem_setEntry(tmp5Sdram, tmp5Size, tmp5DatasetId, tmp5);
	preprocessing_vmem_setEntry(tmp6Sdram, tmp6Size, tmp6DatasetId, tmp6);

	preprocessing_vmem_print();

	//NAND FLASH Memory
	int32_t *NANDFLASH;
	int32_t numberOfEntriesNAND = 128;
	int32_t **entriesOfNAND = (int32_t **) malloc(numberOfEntriesNAND*sizeof(int32_t *));
	NANDFLASH = (int32_t*) malloc(numberOfEntriesNAND*stdimagesize*sizeof(int32_t));

	createNANDFLASH(NANDFLASH, entriesOfNAND, stdimagesize, NUMBER_OF_IMAGES);
	//END NAND FLASH Memory

	int xO, yO;
	readNAND(entriesOfNAND[0], ROWS, COLS, img01Sdram);
	CHECK_STATUS(preprocessing_template(img01Sdram, ROWS, COLS, XL, XH, YL, YH, tmp1Sdram))
	writeImageToFile(tmp1, "template.fits", -1, 0, stdimagesize );
	for(unsigned short i = 0; i < NUMBER_OF_IMAGES; i++) {
			printf("Calculating image %d...\n", i);
			readNAND(entriesOfNAND[i], ROWS, COLS, img01Sdram);
			CHECK_STATUS(preprocessing_xCorr(img01Sdram, tmp1Sdram, ROWS, COLS, tmp2Sdram))
			writeImageToFile(tmp2, "myXcoor0.fits", 7, i, stdimagesize );
			CHECK_STATUS(preprocessing_minimumValue(tmp2Sdram, ROWS, COLS, i, tmp3Sdram))
			CHECK_STATUS(preprocessing_zero(ROWS, COLS, tmp2Sdram))

			if(i==0){
				xO = tmp3[0];
				yO = tmp3[1];
			}

			int x = xO - tmp3[i*COORDS_COLS];
			int y = yO - tmp3[i*COORDS_COLS+1];
			printf("Image %d:\tx: %d\ty: %d\n", i, x, y);
	}


	printf("\nCoords:\n");

	for(unsigned int i = 0; i < NUMBER_OF_IMAGES; i++){

		int x = xO - (tmp3[i*COORDS_COLS] + (WINDOW_WIDTH/2));
		int y = yO - (tmp3[i*COORDS_COLS+1] + (WINDOW_HEIGHT/2));
		printf("Image %d:\tx: %d\ty: %d\n", i, x, y);
	}




	printf("Done!\n");
	return 1;

}
