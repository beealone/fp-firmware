/*!ignore \file zkface.h \brief Provides functionality for zkface API. */

/*!ignore
|******************************************************************************|
|*                                                                            *|
|*                         ZKSoftware ZKFace API 2.4                          *|
|*                                                                            *|
|* zkface.h                                                                   *|
|* Header file for ZKFace API                                                 *|
|*                                                                            *|
|* Copyright (C) 2005-2010 ZKSoftware                                         *|
|*                                                                            *|
\******************************************************************************/

#ifndef __ZKFACE__
#define __ZKFACE__

#if defined _WIN32 || defined __CYGWIV__
  #ifdef BUILDING_DLL
    #ifdef __GNUC__
      #define DLL_PUBLIC __attribute__((dllexport))
    #else
      #define DLL_PUBLIC __declspec(dllexport) // Note: actually gcc seems to also supports this syntax.
    #endif
  #else
    #ifdef __GNUC__
      #define DLL_PUBLIC __attribute__((dllimport))
    #else
      #define DLL_PUBLIC __declspec(dllimport) // Note: actually gcc seems to also supports this syntax.
    #endif
  #endif
  #define DLL_LOCAL
#else
  #if __GNUC__ >= 4
    #define DLL_PUBLIC __attribute__ ((visibility("default")))
    #define DLL_LOCAL  __attribute__ ((visibility("hidden")))
  #else
    #define DLL_PUBLIC
    #define DLL_LOCAL
  #endif
#endif

#ifdef __cplusplus
extern "C"
{
#endif

/********  add by caona ***************/
#define FACE_NO_FACE            0
#define FACE_NO_EYES            1
#define FACE_NO_TMP             2
#define FACE_FETCH_OK           3

#define HINT_FPRE               1
#define HINT_FGOING             2
#define HINT_FCHG               3
#define HINT_FFAR               4
#define HINT_FNEAR              5
#define HINT_REGOK              6
#define HINT_REGFAIL            7
#define HINT_VERIFY             8
#define HINT_VERIFYP            9
#define HINT_CHG                0xff

#define  FACE_IMAGE_QUALITY	80

#define FACE_IDETIFY_FAILED	0x70
#define FACE_IDETIFY_OK		0x71


#define  FACE_EXPOSAL_CHG	50

#define  FACE_GRAY_NEAR		180
#define  FACE_GRAY_FAR		95
#define  FACE_GRAY_LOW		85
#define  FACE_GRAY_HIGH		195


#define  FACE_Y_LOW     	390   //420
#define  FACE_Y_HIGH     	320   //320
#define  FACE_Y_MID     	320   //320

#define  FACE_W_MIN             110
#define  FACE_W_MAX             310
#define  FACE_W_OK              200


/************  end   *****************/
 


#define MAX_TEMPLATE_SIZE (2*1024+512)

//for testing the return code of ZKFace API functions
#define ZKFaceFailed(code)	((code)<0)
#define ZKFSucceeded(code) ((code)>=0)

//Init the ZKFace Enginee, return the handle for other function calling.
#define FOR_EXTRACT	1
#define FOR_MATCH	2
DLL_PUBLIC void *ZKFaceInit(int purpose);

//Finalization the ZKFace Enginee, free the resources.
DLL_PUBLIC void ZKFaceFinal(void *handle);



//Defining the parameter indexes for set/get parameters

//the min quality for a face image. The default is 90, valid value is from 10 to 200
#define PARAM_FACE_QUALITY_THRESHOLD	103	
//the min width of a face. The default is 120, valid value is from 80 to 250
#define PARAM_FACE_MIV_WIDTH	105	
//the min width of a face. The default is 300, valid value is from 110 to 350
#define PARAM_FACE_MAX_WIDTH	106		

//the max width/height of a image. if a image is large than this, it will be scaled automatically
#define PARAM_IMAGE_MAX_WIDTH	301

//rotate the image before extract template.
#define PARAM_IMAGE_ROTATION	302
#define PARAM_IMAGE_ROTATIOV_NONE	0
#define PARAM_IMAGE_ROTATIOV_90		1
#define PARAM_IMAGE_ROTATIOV_180	2
#define PARAM_IMAGE_ROTATIOV_270	3

//flip the image before extract template.
#define PARAM_IMAGE_FLIP	303
#define PARAM_IMAGE_FLIP_NONE	0
#define PARAM_IMAGE_FLIP_X	1
#define PARAM_IMAGE_FLIP_Y	2
#define PARAM_IMAGE_FLIP_XY	(PARAM_IMAGE_FLIP_Y|PARAM_IMAGE_FLIP_X)

#define PARAM_IMAGE_FILE_GRAY	304

//Set a specific parameter value.
DLL_PUBLIC int ZKFaceSetParam(void *handle, int paramIndex, int paramValue);

//Get a specific parameter value.
DLL_PUBLIC int ZKFaceGetParam(void *handle, int paramIndex);



//Get the last error message.
DLL_PUBLIC const char *ZKFaceLastErrorMsg(void *handle);



//Extract face template from a image file. JPG, BMP, PNG, TIFF formats are supported.
DLL_PUBLIC int ZKFaceFetchFromFile(void *handle, const char* fileName, unsigned char *template);

//prepare and then extract face template from a jpeg image file buffer.
DLL_PUBLIC int ZKFaceJPGPrepare(void *handle, const unsigned char *buffer, int bufferSize, unsigned char *bmpBuffer, int bmpBufferSize);
//after prepared a jpg buffer, detect the face on the image. if success, return the width of the face, else return a value not great than 0
DLL_PUBLIC int ZKFaceFetchFace(void *handle);
#define QUALITY_DEFINITION	0
#define QUALITY_BANLANCE	1
#define QUALITY_GRAYLEVEL	2
//after detected a face, detect the eyes on the face. if success, return the total quality, else return a value not great than 0
DLL_PUBLIC int ZKFaceFetchEyes(void *handle);
//after detected a face and eyes successfully, etxract face template. if success, return the length of template, else return a value not great than 0
DLL_PUBLIC int ZKFaceFetchTemplate(void *handle, unsigned char *template);
//after prepared a jpg buffer, extract the face template. if success, return the length of template, else return a value not great than 0
DLL_PUBLIC int ZKFaceJPGFetch(void *handle, unsigned char *template);


//Extract face template from a image data. Only the 8-BITs grayscale image is supported.
DLL_PUBLIC int ZKFaceFetchFromGrayscaleData(void *handle, unsigned char *pixelData, int width, int height, unsigned char *template);

//Get the quality of a template
DLL_PUBLIC int ZKFaceTemplateQlt(unsigned char *template);

//Merge multiple templates from same face to a template 
DLL_PUBLIC int ZKFaceMerge(void *handle, unsigned char **templates, int count, unsigned char *gTemplate);

//Defining the indexes for positions
#define POSITIOV_FACE_X			0
#define POSITIOV_FACE_Y			1
#define POSITIOV_FACE_WIDTH		2
#define POSITIOV_FACE_HEIGHT		3
#define POSITIOV_FACE_CONFIDENCE	4
#define POSITIOV_LEFT_EYE_X		5
#define POSITIOV_LEFT_EYE_Y		6
#define POSITIOV_LEFT_EYE_CONFIDENC	7
#define POSITIOV_RIGHT_EYE_X		8
#define POSITIOV_RIGHT_EYE_Y		9
#define POSITIOV_RIGHT_EYE_CONFIDENC	10
#define MAX_POSITIOV_NUMBER		11
//After extract the template, we can get the positions of face and eyes.
//argument positions is a integer buffer, argument count tells the size of positions. it should be MAX_POSITIOV_NUMBER
//If no face be found, it return 0.
//If a face is succeed be detected, it return great than 0.
//If a face be detected but no eye be found, it return less than POSITIOV_LEFT_EYE_X.
//If a face and eyes be detected succussfully, it return MAX_POSITIOV_NUMBER.
DLL_PUBLIC int ZKFaceGetFaceEyesPosition(void *handle, int *positions, int count);
DLL_PUBLIC int ZKFaceGetLastQuality(void *handle, int *quality, int count);

typedef int (*FilterFun)(void *handle, const char *id);
DLL_PUBLIC int ZKFaceSetIdentifyFilter(void *handle, FilterFun filter); //set the filter while do "ZKFaceIdentifyInCache" with its id.

//Matching two templates and return the similarity score
DLL_PUBLIC int ZKFaceVerify(void *handle, unsigned char *template1, unsigned char *template2);
DLL_PUBLIC int ZKFaceIdentifyInCache(void *handle, unsigned char *liveTemplate, char *id, int idSize, int minScore, int maxScore);
DLL_PUBLIC int ZKFaceIdentifyPrepare(void *handle, unsigned char *liveTemplate);
DLL_PUBLIC int ZKFaceIdentify(void *handle, unsigned char *samplaeTemplate);
DLL_PUBLIC int ZKFaceIdentifyEnd(void *handle);



DLL_PUBLIC int ZKFaceCacheSet(void *handle, const char *id, const unsigned char *liveTemplate);
DLL_PUBLIC int ZKFaceCacheGet(void *handle, const char *id, unsigned char *templateBuffer, int templateBufferSize);
typedef int (*TCallbackFun)(void *handle, int index, const char *id, const unsigned char *template, int templateSize, void *param);
DLL_PUBLIC int ZKFaceCacheForAll(void *handle, TCallbackFun f, void *param);
DLL_PUBLIC int ZKFaceCacheDel(void *handle, const char *id);
DLL_PUBLIC int ZKFaceCacheReset(void *handle);
DLL_PUBLIC int ZKFaceCacheSort(void *handle, int increase);
DLL_PUBLIC int ZKFaceCacheSaveToFile(void *handle, const char* fileName);
DLL_PUBLIC int ZKFaceCacheLoadFromFile(void *handle, const char* fileName); 

#ifdef __cplusplus
}
#endif


#endif
