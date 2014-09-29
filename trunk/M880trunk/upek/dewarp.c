/*************************************************
                                           
 ZEM 200                                          
                                                    
 dewarp.c
                                                      
 Copyright (C) 2003-2005, ZKSoftware Inc.      		
                                                      
*************************************************/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "arca.h"
#include "sensor.h"

typedef struct _CORRECTION_COEFF {
   int   iXmin;
   int   iXmax;
   int   iYmin;
   int   iYmax;   float fk2;
   float fScaleFactor;
   float r1[3];   /* r1 and r2 are the first and second column of the rotation matrix R */
   float r2[3];
   float T[3];    /* the translation vector */
   float f[2];    /* focal lengths */
   float t[2];    /* image coordinates of the intersection of the optical axis with the image plane */
} CORRECTION_COEFF, * PCORRECTION_COEFF;
                                                                                                               
typedef struct _CENTERING_INFO {
   float deltaX;
   float deltaY;
} CENTERING_INFO, * PCENTERING_INFO;

/*Image Correction Calibration*/
                                                                                                               
/* reduction factor */
#define DW_SCALEFACTOR 55.95f // 54.95f
                                                                                                               
/* range of coords for out image */
#define DW_xMin -124
#define DW_xMax 330
#define DW_yMin -43
#define DW_yMax 456
                                                                                                               
/* k2: coefficient of the 2nd order term in the radial distortion of the lens */
#define DW_k2 -0.17068028036701f // -0.503324996581264f
                                                                                                               
/* r1 is the first column of the rotation matrix R */
#define DW_r1 {0.999996967790909f, 0.00205820137943802f, -0.00135211540555979f} //{0.998666979348832f, 0.00896129866039453f, 0.0508326615927072f}
                                                                                                               
/* r2 is the second column of the rotation matrix R */
#define DW_r2 {-0.00204698747083614f, 0.389487102785644f, -0.921029644803f} // {0.0320381977108188f, 0.664526776156987f, -0.746577335349691f}
                                                                                                               
/* T: the translation vector */
#define DW_T {-1.81041561965488f, -0.80600051095657f, 32.3533646594741f}  // {-3.85427329462583f, -4.99559723361522f, 28.6053746050197f}
                                                                                                               
/* f: focal lengths */
#define DW_f {1544.63849266742f, 2474.4623498026f} // {1300.53580524408f, 1383.47519785042f}
                                                                                                               
/* t: image coordinates of the intersection of the optical axis with the image plane */
#define DW_t {191.328302439083f, 121.006524591601f} // {270.86595642124f, 255.749540327182f}

const CORRECTION_COEFF  cCorrectionCoeff = {
   DW_xMin,                                                             // iXmin;
   DW_xMax,                                                             // iXmax;
   DW_yMin,                                                             // iYmin;
   DW_yMax,                                                             // iYmax;
   DW_k2,                                                               // fk2;
   DW_SCALEFACTOR,                                                      // fScaleFactor;
   DW_r1,                                                               // r1[3];
   DW_r2,                                                               // r2[3];
   DW_T,                                                                // T[3];
   DW_f,                                                                // f[2];
   DW_t                                                                 // t[3];
};

//IMAGE SIZE
#define  DW_IN_ROWS 		289
#define  DW_IN_COLS 		367
                                                                                                               
#define  DW_OUT_ROWS       	550
#define  DW_OUT_COLS        	500
                                                                                                               
#define  DW_X_PATCH_ETALON  	204
#define  DW_Y_PATCH_ETALON  	137
                                                                                                               
#define  DW_VALID_ROWS     	508
#define  DW_VALID_COLS      	410

// output width of the Trim operation; increase to show margin
#define  IMAGE_TRIMMED_WIDTH     (367)
                                                                                                               
// imager: optical black margins
#define  IMAGER_OBLACK_WIDTH     (13)
#define  IMAGER_OWHITE_WIDTH     (367)
                                                                                                               
#define  DEV_FAKE_STRIP_SIZE     (90)
                                                                                                               
// start of image relative to the beginning of data line;
// 13 pixels before it are optical black
#define  DEV_IMAGE_OWHITE_START  (11)
// the value 11 gives us the fillowing format of the data line:
// 11 black margin pixels + 367 significant image pixels + 6 black margin pixels

void StretchLinearImage(int width, int height, BYTE *image, int dst_width, int dst_height, BYTE *dst_image)
{
        int i, j;
        BYTE *ps, *pd, *pp;
	                                                                                                              
        for(i = 0, pd = dst_image; i < dst_height; i++, pd += dst_width)
        {
                ps = image + (i * height / dst_height) * width;
                for(j = 0, pp = pd; j < dst_width; j++, pp++)
                        *pp = *(ps + (j * width / dst_width));
        }
}

#define DW_BACKGROUND 255

/********************************************************************/

/* Computes the 3D coordinates of a point given the camera parameters
   and its projection onto the image */
void inverse_projection_local(float Pin[2], 
			const CORRECTION_COEFF * pCorCoeff, 
			float P3d[3])
{
  float Pnorm[2];
  float b[2];
  float A[2][2];
  float d[2];
  float detA;
  float x, y;

  b[0] = (Pin[0] - pCorCoeff->t[0]) / (pCorCoeff->f[0]);
  b[1] = (Pin[1] - pCorCoeff->t[1]) / (pCorCoeff->f[1]);

  Pnorm[0] = b[0];
  Pnorm[1] = b[1];

  /* builds a system A*[x y]' = d, where (x,y) are the coords of Pout */
  A[0][0] = -1;
  A[0][1] = (Pnorm[0]*pCorCoeff->r2[2]);   
  A[1][0] = 0;
  A[1][1] = (Pnorm[1]*pCorCoeff->r2[2] - pCorCoeff->r2[1]);

  d[0] = (pCorCoeff->T[0] - Pnorm[0]*pCorCoeff->T[2]);
  d[1] = (pCorCoeff->T[1] - Pnorm[1]*pCorCoeff->T[2]);

  detA = A[0][0]*A[1][1] - A[0][1]*A[1][0];

  x = (A[1][1]*d[0] - A[0][1]*d[1]) / detA;
  y = (-A[1][0]*d[0] + A[0][0]*d[1]) / detA;

  P3d[0] = x;
  P3d[1] = y;
  P3d[2] = 0;
}

static void
trim_optical_black_local (
   unsigned char *Dst,
   unsigned int DstSize,
   unsigned char *Src,
   unsigned int SrcSize
   )
{
  unsigned nLinesSrc = SrcSize / DEV_IMAGE_WIDTH;
  unsigned nLinesDst = DstSize / IMAGE_TRIMMED_WIDTH;
  unsigned j = 0;
  unsigned nLines = nLinesSrc; //min(nLinesSrc,nLinesDst);

  if (nLines>nLinesDst) nLines=nLinesDst;
  
  for (j = 0; j < nLines; j++) {
    // line by line copy
    memmove (Dst + j * IMAGE_TRIMMED_WIDTH,
	     Src + j * DEV_IMAGE_WIDTH + DEV_IMAGE_OWHITE_START,
	     IMAGE_TRIMMED_WIDTH
	     );
  }
}

static void
dewarp_local (
	int               inRows, 
	int               inCols, 
	unsigned char     *inImg, 
	int               outRows, 
	int               outCols,
	unsigned char     *outImg,
	const CORRECTION_COEFF * pCorCoeff,   // provides device-specific calibration coefficients
	CENTERING_INFO * pCenterInfo)
{
  int i, j, 
    PinY; 
 float y, 
   PtransfY, 
   InvPtransfZ, 
   PnormY;
 int         iColNb;
 float       yr20T0;
 float       yr21T1;
 float       yr22T2;
 int         iXmin;
 int         iXmax;
 int         iYmin;
 int         iYmax;
 float       fk2;           
 float       reductionFactor;
 float       r10;
 float       r11;
 float       r12;
 float       r20;
 float       r21;
 float       r22;
 float       T0;
 float       T1;
 float       T2;
 float       f0;
 float       f1;
 float       t0;
 float       t1;
 float       t0_5;
 float       t1_5;
 float       deltaX=0;
 float       deltaY=0;
 float       distDeltaX=0;
 float       distDeltaY=0;
 float       Pin[2] = {0, 0};
 float       P3d[3] = {0, 0, 0};
 float       xt;
 const int   P = 16;
 float       InvPtransfZf0;
 int xReductionFactor;
 int x;
 int PinYinCols;

 int iImgMaxX, iImgMaxY, iImgMinX, iImgMinY, iValidWidth, iValidHeight, iValidTop, iValidLeft; 
 unsigned char *pValidPos;

//calculate valid image area
      iImgMaxX		= 0;  
      iImgMaxY 		= 0;
      iImgMinX 		= outCols - 1;
      iImgMinY		= 0;

// for speed improvement we use local variable instead of the structure 
// Simpler compution based on the stack
      iXmin             = pCorCoeff->iXmin;
      iXmax             = pCorCoeff->iXmax;
      iYmin             = pCorCoeff->iYmin;
      iYmax             = pCorCoeff->iYmax;
      fk2               = pCorCoeff->fk2;
      reductionFactor   = 1 / pCorCoeff->fScaleFactor;
      r10               = pCorCoeff->r1[0];
      r11               = pCorCoeff->r1[1];
      r12               = pCorCoeff->r1[2];
      r20               = pCorCoeff->r2[0];
      r21               = pCorCoeff->r2[1];
      r22               = pCorCoeff->r2[2];
      T0                = pCorCoeff->T[0];
      T1                = pCorCoeff->T[1];
      T2                = pCorCoeff->T[2];
      f0                = pCorCoeff->f[0];
      f1                = pCorCoeff->f[1];
      t0                = pCorCoeff->t[0];
      t1                = pCorCoeff->t[1];
      t0_5              = t0 + 0.5f;
      t1_5              = t1 + 0.5f;

      if (pCenterInfo != NULL) {
        deltaX         = pCenterInfo->deltaX;
        deltaY         = pCenterInfo->deltaY;
	
        Pin[0] = -deltaX;
        Pin[1] = -deltaY;
        inverse_projection_local(Pin, pCorCoeff, P3d);

        distDeltaX = P3d[0]/reductionFactor;
        distDeltaY = P3d[1]/reductionFactor;
      } else {
        distDeltaX = (float) iXmin;
        distDeltaY = (float) iYmin;
      }

      xt = distDeltaX * reductionFactor;
      
      for(i=0; i<outRows; i++)
      {
         y = (i + distDeltaY) * reductionFactor;
         
         iColNb = i*outCols;
         yr20T0 = T0;
         yr21T1 = y*r21 + T1;
         yr22T2 = y*r22 + T2;

         PtransfY = yr21T1;
         InvPtransfZ = 1 / (yr22T2);
         PnormY = PtransfY * InvPtransfZ;
         PinY = (int) (PnormY*f1 + t1_5 + deltaY);
         if (PinY >= inRows)
         {
            memset((outImg+i*outCols),DW_BACKGROUND,(outRows-i)*outCols);
            break;
         }

         InvPtransfZf0 = InvPtransfZ * f0;
         xReductionFactor = (int) ((reductionFactor * InvPtransfZf0) * (1<<P));
         x = (int) (((xt + yr20T0)*InvPtransfZf0 + t0 + deltaX) * (1<<P));
         PinYinCols = PinY*inCols;

         for(j=0; j<outCols; j++)
         {
            int x0 = x;
            int x1 = x0 >> P;
            int x2 = x1 + 1;
            x0 &= (1<<P)-1;

            if (x2 >= inCols)
            {
                memset((outImg + iColNb),DW_BACKGROUND,(outCols-j));
                if (iImgMaxX < (outCols-j)){
			iImgMaxX = outCols-j-1;
			iImgMaxY = outRows-1-i;
		}
		 
                if (iImgMinX > (outCols-j)){
			iImgMinX = outCols-j-1;
			iImgMinY = outRows-1-i;
		}
               //memset((outImg + iColNb + j),DW_BACKGROUND,(outCols-j));
               break;
            }
            else
            {
               if((PinY >= 0) && (x1 >= 0))
                  outImg[iColNb + outCols - j - 1] = (((1<<P) - x0) * inImg[PinYinCols + x1] + 
                                                 x0  * inImg[PinYinCols + x2]) >> P;
               else
                  outImg[iColNb + outCols - j - 1] = DW_BACKGROUND;

            }
            x += xReductionFactor;
         }
      }

      //Add by David clip the valid image area for impovement speed 2004.11.18	
      DBPRINTF("MaxX = %d MaxY = %d MinX = %d MinY = %d \n", iImgMaxX, iImgMaxY, iImgMinX, iImgMinY);
      DBPRINTF("Left = %d Top = %d\n", iImgMinX + (iImgMaxX-iImgMinX)/2, iImgMaxY);		
      DBPRINTF("700 = Width = %d Height = %d\n", outCols-iImgMaxX + 1, iImgMinY - iImgMaxY + 1);	
      DBPRINTF("500 = Width = %d Height = %d\n", (outCols-iImgMaxX + 1)*5/7, (iImgMinY - iImgMaxY + 1)*5/7);	
      	
      iValidWidth	= outCols - iImgMaxX + 1;
      if (iValidWidth > DW_VALID_COLS) 
		iValidWidth = DW_VALID_COLS;
      iValidHeight	= iImgMinY - iImgMaxY + 1;
      iValidLeft 	= iImgMinX + (iImgMaxX-iImgMinX)/2;	
      iValidTop		= iImgMaxY;
      
      RotationImageVetical(outImg, outCols, outRows);
            
      pValidPos = outImg;	

      for(i = 0; i < DW_VALID_ROWS; i++){
	      if (i < iValidHeight){
		      memcpy(pValidPos + i*DW_VALID_COLS, 
			     outImg + (i + iImgMaxY)*outCols + iValidLeft, iValidWidth);	
		      if (iValidWidth < DW_VALID_COLS)
			      memset(pValidPos + i*DW_VALID_COLS + iValidWidth, 
				     DW_BACKGROUND, DW_VALID_COLS - iValidWidth);
	      }
	      else
		      memset(pValidPos + i*DW_VALID_COLS, DW_BACKGROUND, DW_VALID_COLS);
      }
}

void dewarpImageAndStretch(unsigned char *src, unsigned char *dest)
{
    CENTERING_INFO cei = { 0.0, 0.0 };
    BYTE byPatchCenterX, byPatchCenterY;
 
    trim_optical_black_local(src+64, DW_IN_ROWS * DW_IN_COLS,
		       src+64, DEV_IMAGE_WIDTH * DEV_IMAGE_HEIGHT);
                                                                                                                    
    byPatchCenterX = src[8];
    byPatchCenterY = src[9];
    cei.deltaX = (float) ((signed) byPatchCenterX - (signed) DW_X_PATCH_ETALON);
    cei.deltaY = (float) ((signed) byPatchCenterY - (signed) DW_Y_PATCH_ETALON);
                                                                                                                                      
    dewarp_local(DW_IN_ROWS, DW_IN_COLS, src+64,
	   DW_OUT_ROWS, DW_OUT_COLS, dest,
	   &cCorrectionCoeff,
	   &cei);
       
    StretchLinearImage(DW_VALID_COLS, DW_VALID_ROWS, dest, uru4000_width, uru4000_height, src); 
}
