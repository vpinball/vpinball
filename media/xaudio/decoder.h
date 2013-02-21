/*****************************************************************
|
|      Xaudio Decoder. SYNC API
|
|      (c) 1996-2000 Xaudio Corporation
|      Author: Gilles Boccon-Gibod (gilles@xaudio.com)
|
 ****************************************************************/

#ifndef _DECODER_H_
#define _DECODER_H_

/*----------------------------------------------------------------------
|       includes
+---------------------------------------------------------------------*/
#include "xaudio.h"
#include "properties.h"

/*----------------------------------------------------------------------
|       constants
+---------------------------------------------------------------------*/
#define XA_DECODER_INPUT_SEEKABLE                  0x0001

#define XA_DECODER_DEVICE_HAS_MASTER_LEVEL_CONTROL 0x00000001
#define XA_DECODER_DEVICE_HAS_PCM_LEVEL_CONTROL    0x00000002
#define XA_DECODER_DEVICE_HAS_BALANCE_CONTROL      0x00000004
#define XA_DECODER_DEVICE_HAS_LINE_OUT             0x00000008
#define XA_DECODER_DEVICE_HAS_SPEAKER_OUT          0x00000010
#define XA_DECODER_DEVICE_HAS_HEADPHONE_OUT        0x00000020
#define XA_DECODER_DEVICE_HAS_08_000_KHZ           0x00000040
#define XA_DECODER_DEVICE_HAS_11_025_KHZ           0x00000080
#define XA_DECODER_DEVICE_HAS_12_000_KHZ           0x00000100
#define XA_DECODER_DEVICE_HAS_16_000_KHZ           0x00000200
#define XA_DECODER_DEVICE_HAS_22_050_KHZ           0x00000400
#define XA_DECODER_DEVICE_HAS_24_000_KHZ           0x00000800
#define XA_DECODER_DEVICE_HAS_32_000_KHZ           0x00001000
#define XA_DECODER_DEVICE_HAS_44_100_KHZ           0x00002000
#define XA_DECODER_DEVICE_HAS_48_000_KHZ           0x00004000
#define XA_DECODER_DEVICE_HAS_8_BITS               0x00008000
#define XA_DECODER_DEVICE_HAS_MU_LAW               0x00010000
#define XA_DECODER_DEVICE_HAS_16_BITS              0x00020000
#define XA_DECODER_DEVICE_HAS_MUTE                 0x00040000
#define XA_DECODER_DEVICE_HAS_RESET                0x00080000
#define XA_DECODER_DEVICE_HAS_PAUSE                0x00100000
#define XA_DECODER_DEVICE_HAS_DRAIN                0x00200000
#define XA_DECODER_DEVICE_HAS_BUFFER_STATUS        0x00400000

#define XA_DECODER_DEVICE_FREQUENCIES_OFFSET       6
#define XA_DECODER_DEVICE_FREQUENCIES_MASK         0x1FF

#define XA_DECODER_CONTROL_BPS                     0x0001
#define XA_DECODER_CONTROL_CHANNELS                0x0002
#define XA_DECODER_CONTROL_SAMPLE_RATE             0x0004
#define XA_DECODER_CONTROL_MASTER_LEVEL            0x0008
#define XA_DECODER_CONTROL_PCM_LEVEL               0x0010
#define XA_DECODER_CONTROL_BALANCE                 0x0020
#define XA_DECODER_CONTROL_PORTS                   0x0040
#define XA_DECODER_CONTROL_ALL_INFO                0x007F
#define XA_DECODER_CONTROL_MUTE                    0x0080
#define XA_DECODER_CONTROL_UNMUTE                  0x0100
#define XA_DECODER_CONTROL_RESET                   0x0200
#define XA_DECODER_CONTROL_PAUSE                   0x0400
#define XA_DECODER_CONTROL_RESTART                 0x0800
#define XA_DECODER_CONTROL_DRAIN                   0x1000

#define XA_DECODER_CONTROL_OUTPUT_LINE             0x01
#define XA_DECODER_CONTROL_OUTPUT_SPEAKER          0x02
#define XA_DECODER_CONTROL_OUTPUT_HEADPHONE        0x04

#define XA_DECODER_CODEC_CONFIG_CHANNELS           0x01
#define XA_DECODER_CODEC_CONFIG_QUALITY            0x02
#define XA_DECODER_CODEC_CONFIG_EQUALIZER          0x04
#define XA_DECODER_CODEC_CONFIG_ALL_MASK           0x07

#define XA_DECODER_CODEC_HAS_EQUALIZER             0x01
#define XA_DECODER_CODEC_HAS_VARIABLE_QUALITY      0x02

#define XA_DECODER_ENVIRONMENT_MAX_NAME_LENGTH     255
#define XA_DECODER_ENVIRONMENT_MAX_STRING_LENGTH   1023

#define XA_DECODER_INPUT_QUERY_MODULE_NAME         0x01
#define XA_DECODER_INPUT_QUERY_NB_DEVICES          0x02
#define XA_DECODER_INPUT_QUERY_DEVICE_NAME         0x04

#define XA_DECODER_INPUT_QUERY_NAME_IS_GENERIC     0x01
#define XA_DECODER_INPUT_QUERY_DEVICE_IS_DEFAULT   0x02

#define XA_DECODER_INPUT_FILTER_QUERY_MODULE_NAME  0x01

#define XA_DECODER_OUTPUT_QUERY_MODULE_NAME        0x01
#define XA_DECODER_OUTPUT_QUERY_NB_DEVICES         0x02
#define XA_DECODER_OUTPUT_QUERY_DEVICE_NAME        0x04

#define XA_DECODER_OUTPUT_QUERY_NAME_IS_GENERIC    0x01
#define XA_DECODER_OUTPUT_QUERY_DEVICE_IS_DEFAULT  0x02
#define XA_DECODER_OUTPUT_QUERY_DEVICE_IS_SHAREABLE 0x04

#define XA_DECODER_OUTPUT_FILTER_QUERY_MODULE_NAME 0x01

#define XA_DECODER_CODEC_QUERY_MODULE_NAME         0x01
#define XA_DECODER_CODEC_QUERY_NB_DEVICES          0x02
#define XA_DECODER_CODEC_QUERY_DEVICE_NAME         0x04
#define XA_DECODER_CODEC_QUERY_BUFFER_SIZE         0x08
#define XA_DECODER_CODEC_QUERY_CAPS                0x10

#define XA_DECODER_CODEC_QUERY_NAME_IS_GENERIC     0x01
#define XA_DECODER_CODEC_QUERY_DEVICE_IS_DEFAULT   0x02

#define XA_DECODER_MAX_NAME_LENGTH                 255
#define XA_DECODER_MAX_DESCRIPTION_LENGTH          255

#define XA_DECODER_INPUT_AUTOSELECT                (-1)
#define XA_DECODER_OUTPUT_AUTOSELECT               (-1)
#define XA_DECODER_CODEC_AUTOSELECT                (-1)

#define XA_DECODER_INPUT_FILTER_FIRST              (-1)
#define XA_DECODER_INPUT_FILTER_LAST               ( 0)
#define XA_DECODER_INPUT_FILTER_BY_NAME            (-2)

#define XA_DECODER_OUTPUT_FILTER_FIRST             (-1)
#define XA_DECODER_OUTPUT_FILTER_LAST              ( 0)
#define XA_DECODER_OUTPUT_FILTER_BY_NAME           (-2)

#define XA_TIMECODE_FRACTIONS_PER_SECOND           (100)
#define XA_TIMECODE_FRACTIONS_PER_MINUTE        (100*60)
#define XA_TIMECODE_FRACTIONS_PER_HOUR       (100*60*60)

#define XA_DECODER_CODEC_QUALITY_HIGH               0
#define XA_DECODER_CODEC_QUALITY_MEDIUM             1
#define XA_DECODER_CODEC_QUALITY_LOW                2

#define XA_DECODER_EQUALIZER_NB_BANDS               32
#define XA_DECODER_FEEDBACK_NB_BANDS                32

#define XA_DECODER_STREAM_PROPERTIES_CHANGED        0x01
#define XA_DECODER_STREAM_PARAMETERS_CHANGED        0x02
#define XA_DECODER_STREAM_MIME_TYPE_CHANGED         0x04

/*----------------------------------------------------------------------
|       version constants
+---------------------------------------------------------------------*/
#define XA_SYNC_API_MAJOR     4
#define XA_SYNC_API_MINOR     4  
#define XA_SYNC_API_REVISION  0
#define XA_SYNC_API_VERSION          \
 XA_VERSION_ID(XA_SYNC_API_MAJOR,    \
               XA_SYNC_API_MINOR,    \
               XA_SYNC_API_REVISION)

/*----------------------------------------------------------------------
|       types
+---------------------------------------------------------------------*/
typedef XA_PropertyListenerCallback XA_EnvironmentCallback;

typedef struct {
    unsigned int h;
    unsigned int m;
    unsigned int s;
    unsigned int f;
} XA_TimeCode;

typedef struct {
    long seconds;
    long microseconds;
} XA_AbsoluteTime;

enum XA_IOState { XA_IO_STATE_CLOSED = 0, XA_IO_STATE_OPEN };

typedef struct {
    int           index;
    unsigned long flags;
    char          name[XA_DECODER_MAX_NAME_LENGTH];
    char          description[XA_DECODER_MAX_DESCRIPTION_LENGTH];
} XA_InputModuleQuery;

typedef struct {
    int   id;
    void *global;
} XA_InputModuleClassInfo;

typedef struct XA_DecoderInfo XA_DecoderInfo;

typedef struct XA_InputInstance XA_InputInstance;

typedef struct {
    unsigned long api_version_id;
    int  (*input_module_probe)(const char *name);
    int  (*input_module_query)(XA_InputModuleQuery *query, 
                               unsigned long query_mask);
    int  (*input_new)(XA_InputInstance **input, const char *name,
                      XA_InputModuleClassInfo *class_info,
                      XA_DecoderInfo *decoder);
    int  (*input_delete)(XA_InputInstance *input);
    int  (*input_open)(XA_InputInstance *input);
    int  (*input_close)(XA_InputInstance *input);
    int  (*input_read)(XA_InputInstance *input, void *buffer, 
                       unsigned long size);
    int  (*input_seek)(XA_InputInstance *input, unsigned long offset);
    long (*input_get_caps)(XA_InputInstance *input);
    long (*input_get_size)(XA_InputInstance *input);
    int  (*input_send_message)(XA_InputInstance *input, int type, 
                               const void *data, unsigned long size);
} XA_InputModule;

typedef struct {
    unsigned long flags;
    char          name[XA_DECODER_MAX_NAME_LENGTH];
    char          description[XA_DECODER_MAX_DESCRIPTION_LENGTH];
} XA_InputFilterModuleQuery;

typedef struct XA_InputFilterNode XA_InputFilterNode;

typedef struct XA_InputFilterInstance XA_InputFilterInstance;

typedef struct {
    unsigned long api_version_id;
    int  (*filter_module_query)(XA_InputFilterModuleQuery *query,
		                        unsigned long query_mask);
    int  (*filter_new)(XA_InputFilterInstance **filter, int id,
                       XA_DecoderInfo *decoder);
    int  (*filter_delete)(XA_InputFilterInstance *filter);
    int  (*input_open)(XA_InputFilterNode *node);
    int  (*input_close)(XA_InputFilterNode *node);
    int  (*input_read)(XA_InputFilterNode *node, 
                       void *buffer, unsigned long size);
    int  (*input_seek)(XA_InputFilterNode *node, 
                       unsigned long offset);
    long (*input_get_caps)(XA_InputFilterNode *node);
    long (*input_get_size)(XA_InputFilterNode *node);
    int  (*input_send_message)(XA_InputFilterNode *node, 
                               int type, const void *data, unsigned long size);
} XA_InputFilterModule;

struct XA_InputFilterNode {
    int                   id;
    void                 *instance;
    XA_InputFilterModule  vtable;
    XA_InputFilterNode   *next;
    XA_InputFilterNode   *prev;
};

typedef struct {
    enum XA_IOState       state;
    unsigned long         caps;
    unsigned long         offset;
    unsigned long         size;
    XA_InputInstance     *instance;
    const XA_InputModule *module;
    const char           *name;
    const char           *mime_type;
} XA_InputInfo;

typedef enum { 
    XA_OUTPUT_STEREO, 
    XA_OUTPUT_MONO_LEFT,         
    XA_OUTPUT_MONO_RIGHT,
    XA_OUTPUT_MONO_MIX
} XA_OutputChannelsMode;

typedef struct {
    unsigned int  changed;
    short        *pcm_samples;
    unsigned int  nb_samples;
    unsigned int  bytes_per_sample;
    unsigned int  nb_channels;
    unsigned int  sample_rate;
    unsigned long delay;
} XA_OutputBufferInfo;

typedef struct {
    unsigned long delay;
} XA_OutputStatus;

typedef struct {
    unsigned int bytes_per_sample;
    unsigned int sample_rate;
    unsigned int nb_channels;
    unsigned int master_level;
    unsigned int pcm_level;
    unsigned int balance;
    unsigned int ports;
} XA_OutputControl;

typedef struct {
    int           index;
    unsigned long flags;
    char          name[XA_DECODER_MAX_NAME_LENGTH];
    char          description[XA_DECODER_MAX_DESCRIPTION_LENGTH];
} XA_OutputModuleQuery;

typedef struct {
    int   id;
    void *global;
} XA_OutputModuleClassInfo;

typedef struct XA_OutputInstance XA_OutputInstance;

typedef struct {
    unsigned long api_version_id;
    int   (*output_module_probe)(const char *name);
    int   (*output_module_query)(XA_OutputModuleQuery *query, 
                                 unsigned long query_mask);
    int   (*output_new)(XA_OutputInstance **output, const char *name,
                        XA_OutputModuleClassInfo *class_info,
                        XA_DecoderInfo *decoder);
    int   (*output_delete)(XA_OutputInstance *output);
    int   (*output_open)(XA_OutputInstance *output);
    int   (*output_close)(XA_OutputInstance *output);
    int   (*output_write)(XA_OutputInstance *output, const void *buffer, 
                          unsigned long size, unsigned int bytes_per_sample,
                          unsigned int nb_channels, unsigned int sample_rate);
    void* (*output_get_buffer)(XA_OutputInstance *output, unsigned long size); 
    int   (*output_set_control)(XA_OutputInstance *output, 
                                XA_OutputControl *control, 
                                unsigned long control_mask);
    int   (*output_get_control)(XA_OutputInstance *output, 
                                XA_OutputControl *control, 
                                unsigned long control_mask);
    int   (*output_get_status)(XA_OutputInstance *output,
                               XA_OutputStatus *status);
    long  (*output_get_caps)(XA_OutputInstance *output);
    int   (*output_send_message)(XA_OutputInstance *output, int type, 
                                 const void *data, unsigned long size);
} XA_OutputModule;

typedef struct {
    unsigned long flags;
    char          name[XA_DECODER_MAX_NAME_LENGTH];
    char          description[XA_DECODER_MAX_DESCRIPTION_LENGTH];
} XA_OutputFilterModuleQuery;

typedef struct XA_OutputFilterNode XA_OutputFilterNode;

typedef struct XA_OutputFilterInstance XA_OutputFilterInstance;

typedef struct {
    unsigned long api_version_id;
    int  (*filter_module_query)(XA_OutputFilterModuleQuery *query,
		                        unsigned long query_mask);
    int  (*filter_new)(XA_OutputFilterInstance **filter, int id,
                       XA_DecoderInfo *decoder);
    int  (*filter_delete)(XA_OutputFilterInstance *filter);
    int  (*output_open)(XA_OutputFilterNode *node);
    int  (*output_close)(XA_OutputFilterNode *node);
    int  (*output_write)(XA_OutputFilterNode *node, 
                         void *buffer, 
                         unsigned long size, unsigned int bytes_per_sample,
                         unsigned int nb_channels, unsigned int sample_rate);
    void* (*output_get_buffer)(XA_OutputFilterNode *node, 
                               unsigned long size); 
    int  (*output_set_control)(XA_OutputFilterNode *node, 
                               XA_OutputControl *control, 
                               unsigned long control_mask);
    int  (*output_get_control)(XA_OutputFilterNode *node, 
                               XA_OutputControl *control, 
                               unsigned long control_mask);
    int  (*output_get_status)(XA_OutputFilterNode *node,
                              XA_OutputStatus *status);
    long (*output_get_caps)(XA_OutputFilterNode *node);
    int  (*output_send_message)(XA_OutputFilterNode *node, 
                                int type, const void *data, 
                                unsigned long size);
} XA_OutputFilterModule;

struct XA_OutputFilterNode {
    int                    id;
    void                  *instance;
    XA_OutputFilterModule  vtable;
    XA_OutputFilterNode   *next;
    XA_OutputFilterNode   *prev;
};

typedef struct {
    signed char left[XA_DECODER_EQUALIZER_NB_BANDS];
    signed char right[XA_DECODER_EQUALIZER_NB_BANDS];
} XA_EqualizerInfo;

typedef struct {
    enum XA_IOState            state;
    unsigned long              caps;
    XA_OutputInstance         *instance;
    const XA_OutputModule     *module;
    const char                *name;
} XA_OutputInfo;

typedef struct {
    XA_OutputChannelsMode channels;
    unsigned int          quality;
    XA_EqualizerInfo     *equalizer;
} XA_CodecConfig;

typedef struct {
    int   id;
    void *global;
} XA_CodecModuleClassInfo;

typedef struct {
    int           index;
    unsigned long flags;
    char          name[XA_DECODER_MAX_NAME_LENGTH];
    char          description[XA_DECODER_MAX_DESCRIPTION_LENGTH];
    unsigned long min_buffer_size;
} XA_CodecModuleQuery;

typedef struct {
    unsigned int     changed;
    unsigned long    duration;
    unsigned long    frequency;
    unsigned long    bitrate;
    unsigned int     nb_channels;
    const char      *mime_type;
    XA_Properties   *properties;
} XA_StreamInfo;

typedef struct XA_CodecInstance XA_CodecInstance;

typedef struct {
    unsigned long api_version_id;
    int   (*codec_module_probe)(const char *mime_type, const char *name);
    int   (*codec_module_query)(XA_CodecModuleQuery *query, unsigned long query_mask);
    int   (*codec_new)(XA_CodecInstance **codec, 
                       const char *mime_type, const char *name,
                       XA_CodecModuleClassInfo *class_info,
                       XA_DecoderInfo *decoder);
    int   (*codec_delete)(XA_CodecInstance *codec);
    int   (*codec_decode)(XA_CodecInstance *codec, XA_OutputBufferInfo *buffer,
                          XA_StreamInfo *info);
    int   (*codec_get_stream_info)(XA_CodecInstance *codec,
                                   XA_StreamInfo *info);
    float (*codec_get_stream_position)(XA_CodecInstance *codec,
                                       unsigned long offset);
    float (*codec_get_seek_position)(XA_CodecInstance *codec, float position);
    int   (*codec_notify_input_open)(XA_CodecInstance *codec, 
                                     const char *mime_type,
                                     const char *name);
    int   (*codec_notify_input_close)(XA_CodecInstance *codec);
    int   (*codec_notify_input_seek)(XA_CodecInstance *codec, 
                                     unsigned long offset);
    int   (*codec_notify_input_discontinuity)(XA_CodecInstance *codec);
    int   (*codec_set_config)(XA_CodecInstance *codec, 
                              XA_CodecConfig *config, 
                              unsigned long config_mask);
    int   (*codec_get_config)(XA_CodecInstance *codec, 
                              XA_CodecConfig *config,
                              unsigned long config_mask);
    int   (*codec_send_message)(XA_CodecInstance *codec, int type, 
                                const void *data, unsigned long size);
} XA_CodecModule;

typedef struct {
    XA_CodecInstance     *instance;
    const XA_CodecModule *module;
    unsigned long         min_buffer_size;
    unsigned long         caps;
} XA_CodecInfo;

typedef struct {
    int nb_input_modules;
    int nb_output_modules;
    int nb_codec_modules;
} XA_ModulesInfo;

typedef struct {
    float         position;
    XA_StreamInfo info;
    XA_TimeCode   timecode;
} XA_StatusInfo;

struct XA_DecoderPrivateInfo;

typedef struct {
    unsigned long api_version_id;
    int (XA_EXPORT *input_read)(XA_DecoderInfo *decoder, 
                                void *buffer, unsigned long size);
    int (XA_EXPORT *input_seek)(XA_DecoderInfo *decoder, 
                                unsigned long offset);
    int (XA_EXPORT *notify_input_discontinuity)(XA_DecoderInfo *decoder);
    int (XA_EXPORT *set_stream_property_integer)(XA_DecoderInfo *decoder,
                                                 const char *name,
                                                 long value);
    int (XA_EXPORT *get_stream_property_integer)(XA_DecoderInfo *decoder,
                                                 const char *name,
                                                 long *value);
    int (XA_EXPORT *set_stream_property_string)(XA_DecoderInfo *decoder,
                                                const char *name,
                                                const char *value);    
    int (XA_EXPORT *get_stream_property_string)(XA_DecoderInfo *decoder,
                                                const char *name,
                                                char *value);
    int (XA_EXPORT *unset_stream_property)(XA_DecoderInfo *decoder, 
                                           const char *name);
    int (XA_EXPORT *set_environment_integer)(XA_DecoderInfo *decoder,
                                             const char *name,
                                             long value);
    int (XA_EXPORT *get_environment_integer)(XA_DecoderInfo *decoder,
                                             const char *name,
                                             long *value);
    int (XA_EXPORT *set_environment_string)(XA_DecoderInfo *decoder,
                                            const char *name,
                                            const char *value);
    int (XA_EXPORT *get_environment_string)(XA_DecoderInfo *decoder,
                                            const char *name,
                                            char *value);    
    int (XA_EXPORT *unset_environment)(XA_DecoderInfo *decoder, 
                                       const char *name);
    int (XA_EXPORT *add_environment_listener)(XA_DecoderInfo *decoder,
                                              const char *name,
                                              void *client,
                                              XA_EnvironmentCallback callback,
                                              XA_PropertyHandle *handle);
    int (XA_EXPORT *remove_environment_listener)(XA_DecoderInfo *decoder,
                                                 XA_PropertyHandle handle);
} XA_DecoderCallbackTable;

struct XA_DecoderInfo {
    XA_InputInfo                 *input;
    XA_OutputInfo                *output;
    XA_CodecInfo                 *codec;
    XA_ModulesInfo               *modules;
    XA_StatusInfo                *status;
    XA_OutputBufferInfo          *output_buffer;
    XA_NotificationClient         notification_client;
    XA_DecoderCallbackTable       callbacks;
    struct XA_DecoderPrivateInfo *hidden;
};

/*----------------------------------------------------------------------
|       prototypes
+---------------------------------------------------------------------*/
#ifdef __cplusplus
extern "C" {
#endif

int XA_EXPORT decoder_new(XA_DecoderInfo **decoder);
int XA_EXPORT decoder_delete(XA_DecoderInfo *decoder);
int XA_EXPORT decoder_input_module_register(XA_DecoderInfo *decoder, 
                                            const XA_InputModule *module);
int XA_EXPORT decoder_input_module_query(XA_DecoderInfo *decoder,
                                         int module,
                                         XA_InputModuleQuery *query,
                                         unsigned long query_mask);
int XA_EXPORT decoder_input_add_filter(XA_DecoderInfo *decoder, 
                                       const XA_InputFilterModule *module,
                                       int where);
int XA_EXPORT decoder_input_remove_filter(XA_DecoderInfo *decoder, 
                                          const char *name, int id); 
int XA_EXPORT decoder_input_filters_list(XA_DecoderInfo *decoder,
                                         void *client,
                                         void (*callback)
                                         (void *client, 
                                          XA_InputFilterNode *node));
int XA_EXPORT decoder_input_new(XA_DecoderInfo *decoder, 
                                const char *name, int module_id);
int XA_EXPORT decoder_input_delete(XA_DecoderInfo *decoder);
int XA_EXPORT decoder_input_open(XA_DecoderInfo *decoder);
int XA_EXPORT decoder_input_close(XA_DecoderInfo *decoder);
int XA_EXPORT decoder_input_read(XA_DecoderInfo *decoder, 
                                 void *buffer, unsigned long size);
int XA_EXPORT decoder_input_seek_to_offset(XA_DecoderInfo *decoder, 
                                           unsigned long offset);
int XA_EXPORT decoder_input_seek_to_position(XA_DecoderInfo *decoder, 
                                             float position);
int XA_EXPORT decoder_input_seek_to_time(XA_DecoderInfo *decoder, 
                                         float seconds);
int XA_EXPORT decoder_input_seek_to_timecode(XA_DecoderInfo *decoder, 
                                             const XA_TimeCode *timecode);
int XA_EXPORT decoder_input_send_message(XA_DecoderInfo *decoder,
                                         int type,
                                         const void *data,
                                         unsigned long size);
int XA_EXPORT decoder_output_module_register(XA_DecoderInfo *decoder,
                                             const XA_OutputModule *module);
int XA_EXPORT decoder_output_module_query(XA_DecoderInfo *decoder,
                                          int module,
                                          XA_OutputModuleQuery *query,
                                          unsigned long query_mask);
int XA_EXPORT decoder_output_add_filter(XA_DecoderInfo *decoder, 
                                        const XA_OutputFilterModule *module,
                                        int where);
int XA_EXPORT decoder_output_remove_filter(XA_DecoderInfo *decoder, 
                                           const char *name, int id); 
int XA_EXPORT decoder_output_filters_list(XA_DecoderInfo *decoder,
                                          void *client,
                                          void (*callback)
                                          (void *client, 
                                           XA_OutputFilterNode *node));
int XA_EXPORT decoder_output_new(XA_DecoderInfo *decoder,
                                 const char *name, int module_id);
int XA_EXPORT decoder_output_delete(XA_DecoderInfo *decoder);
int XA_EXPORT decoder_output_open(XA_DecoderInfo *decoder); 
int XA_EXPORT decoder_output_close(XA_DecoderInfo *decoder);
int XA_EXPORT decoder_output_write(XA_DecoderInfo *decoder, 
                                   void           *buffer, 
                                   unsigned long size, 
                                   unsigned int bytes_per_sample, 
                                   unsigned int channels, 
                                   unsigned int sample_rate);
int XA_EXPORT decoder_output_set_control(XA_DecoderInfo *decoder, 
                                         XA_OutputControl *control,
                                         unsigned long control_mask);
int XA_EXPORT decoder_output_get_control(XA_DecoderInfo *decoder, 
                                         XA_OutputControl *control,
                                         unsigned long control_mask);
int XA_EXPORT decoder_output_send_message(XA_DecoderInfo *decoder,
                                          int type,
                                          const void *data,
                                          unsigned long size);
int XA_EXPORT decoder_codec_module_register(XA_DecoderInfo *decoder,
                                            const XA_CodecModule *module);
int XA_EXPORT decoder_codec_module_query(XA_DecoderInfo *decoder,
                                         int module,
                                         XA_CodecModuleQuery *query,
                                         unsigned long query_mask);
int XA_EXPORT decoder_codec_set_channels(XA_DecoderInfo *decoder,
                                         XA_OutputChannelsMode channels);
int XA_EXPORT decoder_codec_get_channels(XA_DecoderInfo *decoder);
int XA_EXPORT decoder_codec_set_quality(XA_DecoderInfo *decoder,
                                        unsigned int quality);
int XA_EXPORT decoder_codec_get_quality(XA_DecoderInfo *decoder);
int XA_EXPORT decoder_codec_set_equalizer(XA_DecoderInfo *decoder,
                                          XA_EqualizerInfo *equalizer);
int XA_EXPORT decoder_codec_get_equalizer(XA_DecoderInfo *decoder,
                                          XA_EqualizerInfo *equalizer);
int XA_EXPORT decoder_stream_set_property_integer(XA_DecoderInfo *decoder,
                                                  const char *name,
                                                  long value);
int XA_EXPORT decoder_stream_get_property_integer(XA_DecoderInfo *decoder,
                                                  const char *name,
                                                  long *value);
int XA_EXPORT decoder_stream_set_property_string(XA_DecoderInfo *decoder,
                                                 const char *name,
                                                 const char *value);
int XA_EXPORT decoder_stream_get_property_string(XA_DecoderInfo *decoder,
                                                 const char *name,
                                                 char *value);
int XA_EXPORT decoder_stream_unset_property(XA_DecoderInfo *decoder,
                                            const char *name);
int XA_EXPORT decoder_set_environment_integer(XA_DecoderInfo *decoder,
                                              const char *name,
                                              long value);
int XA_EXPORT decoder_get_environment_integer(XA_DecoderInfo *decoder,
                                              const char *name,
                                              long *value);
int XA_EXPORT decoder_set_environment_string(XA_DecoderInfo *decoder,
                                             const char *name,
                                             const char *value);
int XA_EXPORT decoder_get_environment_string(XA_DecoderInfo *decoder,
                                             const char *name,
                                             char *value);
int XA_EXPORT decoder_unset_environment(XA_DecoderInfo *decoder,
                                        const char *name);
int XA_EXPORT decoder_add_environment_listener(XA_DecoderInfo *decoder,
                                               const char *name,
                                               void *client,
                                               XA_EnvironmentCallback callback,
                                               XA_PropertyHandle *handle);
int XA_EXPORT decoder_remove_environment_listener(XA_DecoderInfo *decoder,
                                                  XA_PropertyHandle handle);

int XA_EXPORT decoder_notify_input_discontinuity(XA_DecoderInfo *decoder);
int XA_EXPORT decoder_decode(XA_DecoderInfo *decoder, void *output_buffer);
int XA_EXPORT decoder_play(XA_DecoderInfo *decoder);

#ifdef __cplusplus
}

class XA_Decoder {
public:
    XA_DecoderInfo *info;
    XA_Decoder() { 
        info = (XA_DecoderInfo *)0; 
        decoder_new(&info); 
    }
    ~XA_Decoder() { 
        decoder_delete(info); 
    }
    int InputModuleRegister(const XA_InputModule *module) {
        return decoder_input_module_register(info, module);
    }
    int InputModuleQuery(int module, XA_InputModuleQuery *query,
                         unsigned long query_mask) {
        return decoder_input_module_query(info, module, query, query_mask);
    }
    int InputAddFilter(const XA_InputFilterModule *module, 
                       int where= XA_DECODER_INPUT_FILTER_FIRST) {
        return decoder_input_add_filter(info, module, where);
    }
    int InputRemoveFilter(const char *name, int id) {
        return decoder_input_remove_filter(info, name, id);
    }
    int InputNew(const char *name, int module_id = XA_DECODER_INPUT_AUTOSELECT) {
        return decoder_input_new(info, name, module_id);
    }
    int InputDelete() {
        return decoder_input_delete(info);
    }
    int InputOpen() {
        return decoder_input_open(info);
    }
    int InputClose() {
        return decoder_input_close(info);
    }
    int InputSeekToOffset(unsigned long offset) {
        return decoder_input_seek_to_offset(info, offset);
    }
    int InputSeekToPosition(float position) {
        return decoder_input_seek_to_position(info, position);
    }
    int InputSeekToTime(float seconds) {
        return decoder_input_seek_to_time(info, seconds);
    }
    int InputSeekToTimecode(const XA_TimeCode *timecode) {
        return decoder_input_seek_to_timecode(info, timecode);
    }
    int InputSendMessage(int type, const void *data, unsigned long size) {
        return decoder_input_send_message(info, type, data, size);
    }
    int OutputModuleRegister(const XA_OutputModule *module) {
        return decoder_output_module_register(info, module);
    }
    int OutputModuleQuery(int module, XA_OutputModuleQuery *query,
                          unsigned long query_mask) {
        return decoder_output_module_query(info, module, query, query_mask);
    }
    int OutputAddFilter(const XA_OutputFilterModule *module, 
                        int where = XA_DECODER_OUTPUT_FILTER_FIRST) {
        return decoder_output_add_filter(info, module, where);
    }
    int OutputRemoveFilter(const char *name, int id) {
        return decoder_output_remove_filter(info, name, id);
    }
    int OutputNew(const char *name, int module_id = XA_DECODER_OUTPUT_AUTOSELECT) {
        return decoder_output_new(info, name, module_id);
    }
    int OutputDelete() {
        return decoder_output_delete(info);
    }
    int OutputOpen() {
        return decoder_output_open(info);
    }
    int OutputClose() {
        return decoder_output_close(info);
    }
    int OutputSetControl(XA_OutputControl *control, 
                         unsigned long control_mask) {
        return decoder_output_set_control(info, control, control_mask);
    }
    int OutputGetControl(XA_OutputControl *control,
                         unsigned long control_mask) {
        return decoder_output_get_control(info, control, control_mask);
    }
    int OutputSendMessage(int type, const void *data, unsigned long size) {
        return decoder_output_send_message(info, type, data, size);
    }
    int CodecModuleRegister(const XA_CodecModule *module) {
        return decoder_codec_module_register(info, module);
    }
    int CodecModuleQuery(int module, XA_CodecModuleQuery *query,
                         unsigned long query_mask) {
        return decoder_codec_module_query(info, module, query, query_mask);
    }
    int CodecSetChannels(XA_OutputChannelsMode channels) {
        return decoder_codec_set_channels(info, channels);
    }
    int CodecGetChannels() {
        return decoder_codec_get_channels(info);
    }
    int CodecSetQuality(unsigned int quality) {
        return decoder_codec_set_quality(info, quality);
    }
    int CodecGetQuality() {
        return decoder_codec_get_quality(info);
    }
    int CodecSetEqualizer(XA_EqualizerInfo *equalizer) {
        return decoder_codec_set_equalizer(info, equalizer);
    }
    int CodecGetEqualizer(XA_EqualizerInfo *equalizer) {
        return decoder_codec_get_equalizer(info, equalizer);
    }
    int SetEnvironmentInteger(const char *name, long value) {
        return decoder_set_environment_integer(info, name, value);
    }
    int GetEnvironmentInteger(const char *name, long *value) {
        return decoder_get_environment_integer(info, name, value);
    }
    int SetEnvironmentString(const char *name, const char *value) {
        return decoder_set_environment_string(info, name, value);
    }
    int GetEnvironmentString(const char *name, char *value) {
        return decoder_get_environment_string(info, name, value);
    }
    int UnsetEnvironment(const char *name) {
        return decoder_unset_environment(info, name);
    }
    int AddEnvironmentListener(const char *name, void *client,
                               XA_EnvironmentCallback callback,
                               XA_PropertyHandle *handle) {
        return decoder_add_environment_listener(info, name, client, 
                                                callback, handle);
    }
    int RemoveEnvironmentListener(XA_PropertyHandle handle) {
        return decoder_remove_environment_listener(info, handle);
    }
    int Decode(void *output_buffer) {
        return decoder_decode(info, output_buffer);
    }
    int Play() {
        return decoder_play(info);
    }
};
#endif

#endif /* _DECODER_H_ */



















