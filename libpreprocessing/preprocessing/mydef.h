/*
 * mydef.h
 *
 *  Created on: 13 jul. 2017
 *      Author: zaca
 */

#ifndef LIBPREPROCESSING_PREPROCESSING_MYDEF_H_
#define LIBPREPROCESSING_PREPROCESSING_MYDEF_H_

#define ROWS 1024
#define COLS 1024

#define XC (ROWS/2) // center coordinate of CCD
#define YC (COLS/2) // center coordinate of CCD

#define NUMBER_OF_IMAGES 9

#define WINDOW_WIDTH 10
#define WINDOW_HEIGHT 10

#define XL (XC-WINDOW_WIDTH/2)
#define XH (XC+WINDOW_WIDTH/2)
#define YL (YC-WINDOW_HEIGHT/2)
#define YH (YC+WINDOW_HEIGHT/2)

#define RW (ROWS-WINDOW_WIDTH+1)
#define RH (COLS-WINDOW_HEIGHT+1)

#define COORDS_COLS 2

#define MASK_INDEX  NUMBER_OF_IMAGES

#define CHECK_STATUS(x) 	if((status = x ) != PREPROCESSING_SUCCESSFUL){ printf("Status Error\n");  return status;}


#endif /* LIBPREPROCESSING_PREPROCESSING_MYDEF_H_ */
