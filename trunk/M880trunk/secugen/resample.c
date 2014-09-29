//======================================================================
//FILE :bl_resample.C
//======================================================================
unsigned short m_ImageHeight = 300;	// Y Original Source Image
unsigned short m_ImageWidth = 400;	// X Original Source Image
unsigned short destHeight = 300;	// Y Corrected Output Image
unsigned short destWidth = 260;	// X Corrected Output Image

unsigned long int digit;	// Accuracy Digit
unsigned long int distortion;	// Intrinsic Distortion of OPP03 in Pixel
unsigned long int trape_ratio;	// Trapezoidal Distortion Ratio (X pixel displacement along wiht Y axis)
unsigned long int ratio, org_x, Dst_x, Intpl_x, Calc_x;
unsigned short pixel, pix1, pix2;
unsigned short flagInvert = 1;	// Image invert
unsigned short m_ContMultifly = 20;	// Image Contrast
unsigned short index0 = 6;	// X axis Offset
//unsigned short destDepth;
unsigned int destDepth;

/* Parameters:
 * unsigned char *m_FrameBuffer;	point to original Image 
 * unsigned char *destImage;            point to destinate Image 
*/
void
Elim_Trape (unsigned char *m_FrameBuffer,unsigned char *destImage)
{
  unsigned short i, line;
  digit = 1000;			// Calculation Accuracy -> 0.001
  distortion = 7;		// Distorted pixel size along with y axis (7 pixels trapezoidal distortion along with 300 y-lines)
  for (line = 0; line < destHeight; line++)
    {
      trape_ratio = (distortion * digit) / m_ImageHeight;	//Trapezoidal distortion ratio
      org_x = trape_ratio * line;	//Start x-coordinate from the bottom. 0/300 -> 7/300
      ratio = ((m_ImageWidth) * digit - 2 * trape_ratio * line) / destWidth;	//Dilation Rate
      destDepth = line * destWidth;

      for (i = 0; i < destWidth; i++)
	{
	  Dst_x = i * ratio + org_x; //Original source Pixel x-coordinate for re-sampling
	  Intpl_x = (Dst_x / digit) * digit;	//Interpolated x-coordiate as integer
	  Calc_x = Dst_x - Intpl_x;	//difference between real position (Dst_x) and interpolated position (0 <= Calc_X < 1) 
	  pix1 = m_FrameBuffer[index0 + line * m_ImageWidth + Intpl_x / digit];	//pixel gray value at interpolated x
//	  pix2 = m_FrameBuffer[index0 + line * m_ImageWidth + Intpl_x / digit + 1];	//next pixel gray value from interpolated x
//	  pixel = (unsigned int) ((((pix1 * (digit - Calc_x)) / digit + (pix2 * Calc_x) / digit) ^ flagInvert) * m_ContMultifly);	//re-sampled value for the target destination
//	  destImage[i+destDepth] = (pixel > 254) ? 254 : pixel;
	  destImage[i+destDepth] =  pix1;
	}
    }
}
