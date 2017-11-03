/*
 * Flatfield.c
 *
 *  Created on: 16 may. 2017
 *      Author: zaca
 */


#include "preprocessing/hrt.h"

#include "preprocessing/ana.h"
#include "preprocessing/arith.h"

#include "preprocessing/def.h"
#include "preprocessing/mydef.h"
#include "preprocessing/vmem.h"
#include "../libeve/eve/fixed_point.h"

/* from std c */
#include <limits.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

//NAND FLASH METHODS

void createNANDFLASH(int32_t *NANDFLASH, int32_t **entriesOfNAND, int stdimagesize,  int numberOfImages){

	int nkeys;
	char **header;
	int *inputimg = (int*) malloc((uint32_t)stdimagesize*sizeof(int));			//Only one image

	printf("Load images in NAND FLASH!\n");
	char fileName[13] = "im/im00.fits";
	for(unsigned int i = 0; i < numberOfImages; i++) {
		fileName[6] = 48 + i;
		printf("%s\n", fileName);
		FITS_getImage(fileName, inputimg, stdimagesize, &nkeys, &header);
		for (int j = 0; j < stdimagesize; j++)
			NANDFLASH[i*stdimagesize + j]=(int32_t)eve_fp_int2s32(inputimg[j], FP32_FWL );
	}

	/*
	char maskFileName[13] = "im/mask.fits";
	printf("%s\n", maskFileName);
	FITS_getImage(maskFileName, inputimg, stdimagesize, &nkeys, &header);
	for (int j = 0; j < stdimagesize; j++)
		NANDFLASH[MASK_INDEX*stdimagesize + j]=(int32_t)eve_fp_int2s32(inputimg[j], FP32_FWL );
*/

	//	1.) Load image data to NAND Flash:
	uint32_t 	img1Nand = 0;
	uint32_t 	img2Nand = img1Nand + stdimagesize;
	uint32_t	img3Nand = img2Nand + stdimagesize;
	uint32_t	img4Nand = img3Nand + stdimagesize;
	uint32_t	img5Nand = img4Nand + stdimagesize;
	uint32_t	img6Nand = img5Nand + stdimagesize;
	uint32_t	img7Nand = img6Nand + stdimagesize;
	uint32_t	img8Nand = img7Nand + stdimagesize;
	uint32_t	img9Nand = img8Nand + stdimagesize;
	uint32_t	maskNand = img9Nand + stdimagesize;
	uint32_t	dispNand = maskNand + stdimagesize;

	entriesOfNAND[0] =(NANDFLASH+img1Nand);
	entriesOfNAND[1] =(NANDFLASH+img2Nand);
	entriesOfNAND[2] =(NANDFLASH+img3Nand);
	entriesOfNAND[3] =(NANDFLASH+img4Nand);
	entriesOfNAND[4] =(NANDFLASH+img5Nand);
	entriesOfNAND[5] =(NANDFLASH+img6Nand);
	entriesOfNAND[6] =(NANDFLASH+img7Nand);
	entriesOfNAND[7] =(NANDFLASH+img8Nand);
	entriesOfNAND[8] =(NANDFLASH+img9Nand);


	printf("Images loaded successfully!\n");
}

int readNAND(int32_t *nandSrc, uint16_t rows, uint16_t cols, uint32_t sdDst){
	int status = PREPROCESSING_SUCCESSFUL;
	unsigned int size = (unsigned int)(rows) * cols;
	unsigned int p = 0;

	int32_t* dst = preprocessing_vmem_getDataAddress(sdDst);

	// Check whether given rows and columns are in a valid range.
	if (!preprocessing_vmem_isProcessingSizeValid(sdDst, rows, cols)){
		return PREPROCESSING_INVALID_SIZE;
	}

	// Process.
	for (unsigned int r = 0; r < rows; r++)
	{
		for (unsigned int c = 0; c < cols; c++)
		{
			p = r * cols + c;

			// Check for valid pointer position.
			PREPROCESSING_DEF_CHECK_POINTER(dst, p, size);

			dst[p] = nandSrc[p];

			if (dst[p] == EVE_FP32_NAN)
			{
				status = PREPROCESSING_INVALID_NUMBER;
			}
		}
	}

	return status;
}

int writeNAND(uint32_t sdSrc, uint16_t rows, uint16_t cols, int32_t *nandDst){
	int status = PREPROCESSING_SUCCESSFUL;
	unsigned int size = (unsigned int)(rows) * cols;
	unsigned int p = 0;

	const int32_t* src = preprocessing_vmem_getDataAddress(sdSrc);

	// Check whether given rows and columns are in a valid range.
	if (!preprocessing_vmem_isProcessingSizeValid(sdSrc, rows, cols)){
		return PREPROCESSING_INVALID_SIZE;
	}

	// Process.
	for (unsigned int r = 0; r < rows; r++)
	{
		for (unsigned int c = 0; c < cols; c++)
		{
			p = r * cols + c;

			// Check for valid pointer position.
			PREPROCESSING_DEF_CHECK_POINTER(src, p, size);

			nandDst[p]=src[p];

			if (src[p] == EVE_FP32_NAN)
			{
				status = PREPROCESSING_INVALID_NUMBER;
			}
		}
	}

	return status;
}
//END OF NAND FLASH METHODS

//*******************
int preprocessing_zero(uint16_t rows, uint16_t cols, uint32_t sdDst)
{
    int status = PREPROCESSING_SUCCESSFUL;
    unsigned int size = (unsigned int)(rows) * cols;
    unsigned int p = 0;

    int32_t* dst = preprocessing_vmem_getDataAddress(sdDst);

    // Check whether given rows and columns are in a valid range.
    if (!preprocessing_vmem_isProcessingSizeValid(sdDst, rows, cols))
    {
        return PREPROCESSING_INVALID_SIZE;
    }

    // Process.
    for (unsigned int r = 0; r < rows; r++)
    {
        for (unsigned int c = 0; c < cols; c++)
        {
            p = r * cols + c;

            // Check for valid pointer position.
            PREPROCESSING_DEF_CHECK_POINTER(dst, p, size);

				dst[p] = 0;

            if (dst[p] == EVE_FP32_NAN)
            {
                status = PREPROCESSING_INVALID_NUMBER;
            }
        }
    }

    return status;
}

int preprocessing_arith_ROI(uint32_t sdSrc, uint16_t rows, uint16_t cols,
		   int16_t dx, int16_t dy, uint32_t sdDst){

	int status = PREPROCESSING_SUCCESSFUL;
	unsigned int size = (unsigned int)(rows) * cols;
	unsigned int p = 0;
	unsigned int roiPoint = 0;

	//Calculate window edges
	unsigned int jyl = max(0, -dy);				//Row
	unsigned int jyh = min(0, -dy) + rows; 	//Row
	unsigned int jxl = max(0, -dx); 				//Column
	unsigned int jxh = min(0, -dx) + cols; 	//Column

	const int32_t* src = preprocessing_vmem_getDataAddress(sdSrc);
	int32_t* dst = preprocessing_vmem_getDataAddress(sdDst);

	// Check whether given rows and columns are in a valid range.
	if ((!preprocessing_vmem_isProcessingSizeValid(sdSrc, rows, cols))
			|| (!preprocessing_vmem_isProcessingSizeValid(sdDst, rows, cols)))
	{
		return PREPROCESSING_INVALID_SIZE;
	}

	//Calculate ROI
	for(int y=jyl; y < jyh; y++){
		for(int x=jxl; x < jxh; x++){

			p = y * cols + x;
			roiPoint = (y-jyl) * cols + (x-jxl);

			// Check for valid pointer position.
			PREPROCESSING_DEF_CHECK_POINTER(src, p, size);
			PREPROCESSING_DEF_CHECK_POINTER(dst, roiPoint, size);

			dst[roiPoint] = src[p];

			if (dst[roiPoint] == EVE_FP32_NAN)
			{
				status = PREPROCESSING_INVALID_NUMBER;
			}
		}
	}

	return status;
}

int preprocessing_arith_addROI(uint32_t sdSrc1, uint32_t sdSrc2, uint16_t rows, uint16_t cols,
		   int16_t dx, int16_t dy, uint32_t sdDst){

	int status = PREPROCESSING_SUCCESSFUL;
	unsigned int size = (unsigned int)(rows) * cols;
	unsigned int p = 0;
	unsigned int roiPoint = 0;

	// Calculate window edges
	unsigned int jyl = max(0, -dy), jyh = min(0, -dy) + rows; 	// ROWS
	unsigned int jxl = max(0, -dx), jxh = min(0, -dx) + cols; 		// COLUMNS

	const int32_t* src1 = preprocessing_vmem_getDataAddress(sdSrc1);
	const int32_t* src2 = preprocessing_vmem_getDataAddress(sdSrc2);
	int32_t* dst = preprocessing_vmem_getDataAddress(sdDst);

	// Check whether given rows and columns are in a valid range.
	if ((!preprocessing_vmem_isProcessingSizeValid(sdSrc1, rows, cols))
			|| (!preprocessing_vmem_isProcessingSizeValid(sdSrc2, rows, cols))
			|| (!preprocessing_vmem_isProcessingSizeValid(sdDst, rows, cols)))
	{
		return PREPROCESSING_INVALID_SIZE;
	}

	//Calculate sum
	for(int y=jyl; y < jyh; y++){
		for(int x=jxl; x < jxh; x++){

			p = y * cols + x;
			roiPoint = (y-jyl) * cols + (x-jxl);

			// Check for valid pointer position.
			PREPROCESSING_DEF_CHECK_POINTER(src1, p, size);
			PREPROCESSING_DEF_CHECK_POINTER(src2, roiPoint, size);
			PREPROCESSING_DEF_CHECK_POINTER(dst, p, size);

			dst[p] = eve_fp_add32(src1[p], src2[roiPoint]);

			if (dst[p] == EVE_FP32_NAN)
			{
				status = PREPROCESSING_INVALID_NUMBER;
			}
		}
	}

	return status;
}

int preprocessing_arith_substractROI(uint32_t sdSrc1, uint32_t sdSrc2, uint16_t rows, uint16_t cols,
		   int16_t dx, int16_t dy, uint32_t sdDst){

	int status = PREPROCESSING_SUCCESSFUL;
	unsigned int size = (unsigned int)(rows) * cols;
	unsigned int p = 0;
	unsigned int roiPoint = 0;

	// Calculate window edges
	unsigned int jyl = max(0, -dy), jyh = min(0, -dy) + rows; 	// ROWS
	unsigned int jxl = max(0, -dx), jxh = min(0, -dx) + cols; 		// COLUMNS

	const int32_t* src1 = preprocessing_vmem_getDataAddress(sdSrc1);
	const int32_t* src2 = preprocessing_vmem_getDataAddress(sdSrc2);
	int32_t* dst = preprocessing_vmem_getDataAddress(sdDst);

	// Check whether given rows and columns are in a valid range.
	if ((!preprocessing_vmem_isProcessingSizeValid(sdSrc1, rows, cols))
			|| (!preprocessing_vmem_isProcessingSizeValid(sdSrc2, rows, cols))
			|| (!preprocessing_vmem_isProcessingSizeValid(sdDst, rows, cols)))
	{
		return PREPROCESSING_INVALID_SIZE;
	}

	//Calculate sum
	for(int y=jyl; y < jyh; y++){
		for(int x=jxl; x < jxh; x++){

			p = y * cols + x;
			roiPoint = (y-jyl) * cols + (x-jxl);

			// Check for valid pointer position.
			PREPROCESSING_DEF_CHECK_POINTER(src1, p, size);
			PREPROCESSING_DEF_CHECK_POINTER(src2, roiPoint, size);
			PREPROCESSING_DEF_CHECK_POINTER(dst, p, size);

			dst[p] = eve_fp_subtract32(src1[p], src2[roiPoint]);

			if (dst[p] == EVE_FP32_NAN)
			{
				status = PREPROCESSING_INVALID_NUMBER;
			}
		}
	}

	return status;
}

int preprocessing_arith_equalImages(uint32_t sdSrc, uint16_t rows, uint16_t cols, uint32_t sdDst){

	int status = PREPROCESSING_SUCCESSFUL;
	unsigned int size = (unsigned int)(rows) * cols;
	unsigned int p = 0;

	const int32_t* src = preprocessing_vmem_getDataAddress(sdSrc);
	int32_t* dst = preprocessing_vmem_getDataAddress(sdDst);

	// Check whether given rows and columns are in a valid range.
	if ((!preprocessing_vmem_isProcessingSizeValid(sdSrc, rows, cols))
			|| (!preprocessing_vmem_isProcessingSizeValid(sdDst, rows, cols)))
	{
		return PREPROCESSING_INVALID_SIZE;
	}

	// Process.
	for (unsigned int r = 0; r < rows; r++)
	{
		for (unsigned int c = 0; c < cols; c++)
		{
			p = r * cols + c;

			// Check for valid pointer position.
			PREPROCESSING_DEF_CHECK_POINTER(src, p, size);
			PREPROCESSING_DEF_CHECK_POINTER(dst, p, size);

			dst[p] = src[p];

			if (dst[p] == EVE_FP32_NAN)
			{
				status = PREPROCESSING_INVALID_NUMBER;
			}
		}
	}

	return status;
}

int16_t max(int16_t a, int16_t b){
	if(a >= b) return a;
	return b;
}

int16_t min(int16_t a, int16_t b){
	if(a <= b) return a;
	return b;
}

int32_t eve_fp_double2s32rounded(double value, unsigned int fractionBits)
{
    int32_t result
        = (int32_t)(round(fabs(value) * (1 << fractionBits)));

    if (value < 0)
    {
        result = (int32_t)(-result);
    }

    return result;
}

int preprocessing_getMask(uint32_t sdSrc, uint16_t rows, uint16_t cols, uint16_t index, uint32_t sdDst)
{
    int status = PREPROCESSING_SUCCESSFUL;
    unsigned int size = (unsigned int)(rows) * cols;
    unsigned int p = 0;

    const int32_t* src = preprocessing_vmem_getDataAddress(sdSrc);
    int32_t* dst = preprocessing_vmem_getDataAddress(sdDst);

    // Check whether given rows and columns are in a valid range.
    if ((!preprocessing_vmem_isProcessingSizeValid(sdSrc, rows, cols))
            || (!preprocessing_vmem_isProcessingSizeValid(sdDst, rows, cols)))
    {
        return PREPROCESSING_INVALID_SIZE;
    }

    // Process.
    for (unsigned int r = 0; r < rows; r++)
    {
        for (unsigned int c = 0; c < cols; c++)
        {
            p = r * cols + c;

            // Check for valid pointer position.
            PREPROCESSING_DEF_CHECK_POINTER(src, p, size);
            PREPROCESSING_DEF_CHECK_POINTER(dst, p, size);

			dst[p] = (src[p] & (FP32_BINARY_TRUE << index)) >> index;

            if (dst[p] == EVE_FP32_NAN)
            {
                status = PREPROCESSING_INVALID_NUMBER;
            }
        }
    }

    return status;
}

//*******************

int preprocessing_template(uint32_t sdSrc, uint16_t rows, uint16_t cols, uint16_t xl, uint16_t xh, uint16_t yl, uint16_t yh, uint32_t sdDst){
	int status = PREPROCESSING_SUCCESSFUL;
	unsigned int size = (unsigned int)(rows) * cols;
	unsigned int p = 0;
	unsigned int p2 = 0;

	const int32_t* src = preprocessing_vmem_getDataAddress(sdSrc);
	int32_t* dst = preprocessing_vmem_getDataAddress(sdDst);

	// Check whether given rows and columns are in a valid range.
	if ((!preprocessing_vmem_isProcessingSizeValid(sdSrc, rows, cols))
			|| (!preprocessing_vmem_isProcessingSizeValid(sdDst, rows, cols)))
	{
		return PREPROCESSING_INVALID_SIZE;
	}

	// Process.
	for (unsigned int r = xl; r < xh; r++)
	{
		for (unsigned int c = yl; c < yh; c++)
		{
			p = r * cols + c;
			p2 = (r-xl) * cols + (c-yl);

			// Check for valid pointer position.
			PREPROCESSING_DEF_CHECK_POINTER(src, p, size);
			PREPROCESSING_DEF_CHECK_POINTER(dst, p2, size);

			dst[p2] = src[p];

			if (dst[p2] == EVE_FP32_NAN){
				status = PREPROCESSING_INVALID_NUMBER;
			}
		}
	}

	return status;
}

int preprocessing_xCorr(uint32_t sdSrc1, uint32_t sdSrc2, uint16_t rows, uint16_t cols, uint32_t sdDst){
	int status = PREPROCESSING_SUCCESSFUL;
	unsigned int size = (unsigned int)(rows) * cols;
	unsigned int p = 0;
	unsigned int p2 = 0;
	unsigned int p3 = 0;
	double sum = 0;
	double tmp = 0;
	int32_t temp;

	const int32_t* src1 = preprocessing_vmem_getDataAddress(sdSrc1);	//Image
	const int32_t* src2 = preprocessing_vmem_getDataAddress(sdSrc2);	//Mask
	int32_t* dst = preprocessing_vmem_getDataAddress(sdDst);


	// Check whether given rows and columns are in a valid range.
	if ((!preprocessing_vmem_isProcessingSizeValid(sdSrc1, rows, cols))
			|| (!preprocessing_vmem_isProcessingSizeValid(sdSrc2, rows, cols))
			|| (!preprocessing_vmem_isProcessingSizeValid(sdDst, rows, cols)))
	{
		return PREPROCESSING_INVALID_SIZE;
	}

	// Process.
	for (unsigned int r = 0; r < RW; r++)
	{
 		for (unsigned int c = 0; c < RH; c++)
		{
			sum = 0;
			for(unsigned int i = 0; i < WINDOW_WIDTH; i++){

				for(unsigned int j = 0; j < WINDOW_HEIGHT; j++){

					p = (r+i) * cols + (c+j);
					p2 = i * cols + j;

					// Check for valid pointer position.
					PREPROCESSING_DEF_CHECK_POINTER(src1, p, size);
					PREPROCESSING_DEF_CHECK_POINTER(src2, p2, size);

					tmp = eve_fp_signed32ToDouble(eve_fp_subtract32(src2[p2], src1[p]), FP32_FWL);
					sum = sum + tmp*tmp;
				}
			}

			p3 = r * cols + c;

			// Check for valid pointer position.
			PREPROCESSING_DEF_CHECK_POINTER(dst, p3, size);

			temp = eve_fp_double2s32(sum, FP32_FWL);
			dst[p3] = (temp == EVE_FP32_NAN) ? EVE_FP32_MAX : temp;

			if (dst[p3] == EVE_FP32_NAN)
			{
				status = PREPROCESSING_INVALID_NUMBER;
			}
		}
	}

	/************
 #define ind( y, x ) ( y*W+x )
 #define ind2( y, x ) ( y*w+x )
	for(int y = 0; y <(H-h+1); y++) {
		for(int x = 0; x <(W-w+1); x++) {
			sum = 0.0;
			for(int k = 0; k < h;k++) {
				for(int j = 0; j <w; j++) {
					float A=(float)mask[ind2(k,j)]-(float)img[ind(int(y + k), int(x + j))];
					sum = sum + A*A;
				}
			}
			tmp[ind3(y,x)] = sum;
		}
	}

	/*************/

	return status;
}

int preprocessing_minimumValue(uint32_t sdSrc, uint16_t rows, uint16_t cols, uint16_t index, uint32_t sdDst){
	int status = PREPROCESSING_SUCCESSFUL;
	unsigned int size = (unsigned int)(rows) * cols;
	unsigned int p = 0;
	unsigned int min = EVE_FP32_MAX;

	const int32_t* src = preprocessing_vmem_getDataAddress(sdSrc);
	int32_t* dst = preprocessing_vmem_getDataAddress(sdDst);

	// Check whether given rows and columns are in a valid range.
	if ((!preprocessing_vmem_isProcessingSizeValid(sdSrc, rows, cols))
			|| (!preprocessing_vmem_isProcessingSizeValid(sdDst, rows, cols)))
	{
		return PREPROCESSING_INVALID_SIZE;
	}

	// Check for valid pointer position.
	uint32_t pos = index * COORDS_COLS;
	PREPROCESSING_DEF_CHECK_POINTER(dst, pos, size);
	PREPROCESSING_DEF_CHECK_POINTER(dst, pos+1, size);

	// Process.
	for (unsigned int r = 0; r < RW; r++)
	{
		for (unsigned int c = 0; c < RH; c++)
		{
			p = r * cols + c;

			// Check for valid pointer position.
			PREPROCESSING_DEF_CHECK_POINTER(src, p, size);

			if(eve_fp_compare32(src + p, &min) == -1){
				dst[pos] = r;
				dst[pos+1] = c;
				min = src[p];
				//printf("Min: %d\n", min);
			}

			if (dst[pos] == EVE_FP32_NAN || dst[pos+1] == EVE_FP32_NAN)
			{
				status = PREPROCESSING_INVALID_NUMBER;
			}
		}
	}

	return status;
}

