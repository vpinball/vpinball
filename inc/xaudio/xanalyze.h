/*****************************************************************
|
|      MPEG Audio Analyzer
|
|      (c) 1996-2000 Xaudio Corporation
|      Author: Gilles Boccon-Gibod (gilles@xaudio.com)
|
 ****************************************************************/

#ifndef _XANALYSE_H_
#define _XANALYZE_H_

/*----------------------------------------------------------------------
|       macros
+---------------------------------------------------------------------*/
#if defined(WIN32) || defined(_WIN32)
#define XA_EXPORT __stdcall
#else
#define XA_EXPORT
#endif                               

/*-------------------------------------------------------------------------
|       error codes
+-------------------------------------------------------------------------*/
#define XANALYZE_SUCCESS                ( 0)
#define XANALYZE_FAILURE                (-1)

#define XANALYZE_ERROR_INTERNAL         (-2)
#define XANALYSE_ERROR_OUT_OF_MEMORY    (-3)
#define XANALYZE_ERROR_NO_SUCH_FILE     (-4)
#define XANALYZE_ERROR_CANNOT_OPEN      (-5)
#define XANALYZE_ERROR_INVALID_FRAME    (-6)
#define XANALYZE_ERROR_EOS              (-7)
#define XANALYZE_ERROR_STOP_CONDITION   (-8)
#define XANALYZE_ERROR_WATCHDOG         (-9)

#define XANALYZE_INVALID_VALUE          (-1)
#define XANALYZE_ALL_FRAMES             (0)

#define XANALYZE_CALLBACK_CONTINUE      (0)
#define XANALYZE_CALLBACK_STOP          (-1)

#define XANALYZE_TRUE                   1
#define XANALYZE_FALSE                  0

/*-------------------------------------------------------------------------
|       constants
+-------------------------------------------------------------------------*/
#define MPEG_ID_MPEG_1                          1
#define MPEG_ID_MPEG_2                          0
#define MPEG_ID_MPEG_2_5                        2

#define MPEG_MODE_STEREO                        0
#define MPEG_MODE_JOINT_STEREO                  1
#define MPEG_MODE_DUAL_CHANNEL                  2
#define MPEG_MODE_SINGLE_CHANNEL                3

#define XANALYZE_REPORT_CHANGING_LEVEL          0x001
#define XANALYZE_REPORT_CHANGING_LAYER          0x002
#define XANALYZE_REPORT_CHANGING_BITRATE        0x004
#define XANALYZE_REPORT_CHANGING_FREQUENCY      0x008
#define XANALYZE_REPORT_CHANGING_MODE           0x010
#define XANALYZE_REPORT_CHANGING_CHANNELS       0x020
#define XANALYZE_REPORT_HAS_META_DATA           0x040
#define XANALYZE_REPORT_HAS_ID3V1_HEADER        0x080
#define XANALYZE_REPORT_HAS_ID3V2_HEADER        0x100

#define XANALYZE_REPORT_ANY_CHANGE_MASK         0x3F

/*-------------------------------------------------------------------------
|       target specific macros
+-------------------------------------------------------------------------*/
#if defined(WIN32) || defined(_WIN32)
#define XA_EXPORT __stdcall
#define XA_IMPORT __stdcall
#else
#define XA_EXPORT
#define XA_IMPORT
#endif                                      

/*-------------------------------------------------------------------------
|       types
+-------------------------------------------------------------------------*/
typedef struct {
    int  level;
    int  layer;
    int  bitrate;
    int  frequency;
    int  mode;
    int  channels;
} XA_AnalyzerMpegInfo;

typedef struct {
    unsigned int         index;
    XA_AnalyzerMpegInfo  info;
    unsigned int         offset;
    unsigned int         nb_samples;
} XA_AnalyzerMpegFrameInfo;

typedef struct {
    const char   *title;
    const char   *artist;
    const char   *album;
    const char   *year;
    const char   *comment;
    unsigned char genre;
} XA_AnalyzerTrackInfo;

typedef struct {
    XA_AnalyzerMpegInfo  stream_type;
    XA_AnalyzerTrackInfo track;
    unsigned long        frames;
    unsigned long        duration;
    unsigned long        flags;
} XA_AnalyzerInfo;

typedef struct XA_AnalyzerCallbackClient XA_AnalyzerCallbackClient;
typedef int (*XA_AnalyzerCallbackFunction)(XA_AnalyzerCallbackClient *client, 
                                           XA_AnalyzerMpegFrameInfo *frame);

typedef struct XA_Analyzer XA_Analyzer;

/*-------------------------------------------------------------------------
|       prototypes
+-------------------------------------------------------------------------*/
#ifdef __cplusplus
extern "C" {
#endif
                        
int XA_EXPORT xanalyzer_new(XA_Analyzer **analyzer);
int XA_EXPORT xanalyzer_delete(XA_Analyzer *analyzer);
int XA_EXPORT xanalyzer_process_file(XA_Analyzer *analyzer, 
                                     const char *name, 
                                     XA_AnalyzerCallbackFunction callback_function,
                                     XA_AnalyzerCallbackClient *callback_client,
                                     XA_AnalyzerInfo *info,
                                     unsigned long stop_mask,
                                     unsigned long watchdog,
                                     unsigned long max_frames);

#ifdef __cplusplus
}
#endif           

#endif /* _XANALYZE_H_ */

