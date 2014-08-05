/*****************************************************************
|
|      XAudio Player GUI. Asynchronous Player Logic    
|
|
|      (c) 1996-2000 Xaudio Corporation
|      Author: Gilles Boccon-Gibod (gilles@xaudio.com)
|
****************************************************************/

#ifndef _PLAYER_H_
#define _PLAYER_H_

/*----------------------------------------------------------------------
|       includes
+---------------------------------------------------------------------*/
#include "xaudio.h"
#include "control.h"
#include "decoder.h"

/*----------------------------------------------------------------------
|       constants
+---------------------------------------------------------------------*/
#define XA_PLAYER_MODE_OUTPUT_AUTO_CLOSE_ON_STOP        0x0001
#define XA_PLAYER_MODE_OUTPUT_AUTO_CLOSE_ON_PAUSE       0x0002
#define XA_PLAYER_MODE_OUTPUT_AUTO_CLOSE_ON_EOS         0x0004

#define XA_NOTIFY_MASK_ERROR                            0x0000001
#define XA_NOTIFY_MASK_DEBUG                            0x0000002
#define XA_NOTIFY_MASK_PROGRESS                         0x0000004
#define XA_NOTIFY_MASK_ACK                              0x0000008
#define XA_NOTIFY_MASK_NACK                             0x0000010
#define XA_NOTIFY_MASK_PLAYER_STATE                     0x0000020
#define XA_NOTIFY_MASK_INPUT_STATE                      0x0000040
#define XA_NOTIFY_MASK_INPUT_NAME                       0x0000080
#define XA_NOTIFY_MASK_INPUT_CAPS                       0x0000100
#define XA_NOTIFY_MASK_INPUT_POSITION                   0x0000200
#define XA_NOTIFY_MASK_INPUT_TIMECODE                   0x0000400
#define XA_NOTIFY_MASK_OUTPUT_STATE                     0x0000800
#define XA_NOTIFY_MASK_OUTPUT_NAME                      0x0001000
#define XA_NOTIFY_MASK_OUTPUT_CAPS                      0x0002000
#define XA_NOTIFY_MASK_OUTPUT_POSITION                  0x0004000
#define XA_NOTIFY_MASK_OUTPUT_TIMECODE                  0x0008000
#define XA_NOTIFY_MASK_OUTPUT_VOLUME                    0x0010000
#define XA_NOTIFY_MASK_OUTPUT_BALANCE                   0x0020000
#define XA_NOTIFY_MASK_OUTPUT_PCM_LEVEL                 0x0040000
#define XA_NOTIFY_MASK_OUTPUT_MASTER_LEVEL              0x0080000
#define XA_NOTIFY_MASK_OUTPUT_PORTS                     0x0100000
#define XA_NOTIFY_MASK_STREAM_MIME_TYPE                 0x0200000
#define XA_NOTIFY_MASK_STREAM_DURATION                  0x0400000
#define XA_NOTIFY_MASK_STREAM_PARAMETERS                0x0800000
#define XA_NOTIFY_MASK_STREAM_PROPERTIES                0x1000000
#define XA_NOTIFY_MASK_FEEDBACK_AUDIO_EVENT             0x2000000
#define XA_NOTIFY_MASK_FEEDBACK_TAG_EVENT               0x4000000
#define XA_NOTIFY_MASK_PRIVATE_DATA                     0x8000000

#define XA_OUTPUT_VOLUME_IGNORE_FIELD                   255

#define XA_PLAYER_ARGUMENT_NAME__MODULE_LOADER_HOOK \
        "module-loader-hook"
#define XA_PLAYER_MODULE_LOADER_HOOK_CONTINUE           0
#define XA_PLAYER_MODULE_LOADER_HOOK_SYMBOL_FOUND       1

#define XA_PLAYER_ARGUMENT_NAME__INIT_HOOK \
	    "init-hook"

/*----------------------------------------------------------------------
|       types
+---------------------------------------------------------------------*/
typedef struct {
    const char   *name;
    const void   *data;
    unsigned long size;
} XA_PlayerArgument;

typedef struct {
    unsigned int             nb_arguments;
    const XA_PlayerArgument *arguments;
} XA_PlayerArguments;

typedef enum {
    XA_PLAYER_MODULE_LOADER_HOOK_ACTION_LOAD_INPUT_MODULE,
    XA_PLAYER_MODULE_LOADER_HOOK_ACTION_LOAD_INPUT_FILTER_MODULE,
    XA_PLAYER_MODULE_LOADER_HOOK_ACTION_LOAD_OUTPUT_MODULE,
    XA_PLAYER_MODULE_LOADER_HOOK_ACTION_LOAD_OUTPUT_FILTER_MODULE,
    XA_PLAYER_MODULE_LOADER_HOOK_ACTION_LOAD_CODEC_MODULE
} XA_PlayerModuleLoaderHookAction;

typedef enum {
	XA_PLAYER_MODULE_LOADER_HOOK_CONTEXT_PRE_LOAD,
	XA_PLAYER_MODULE_LOADER_HOOK_CONTEXT_POST_LOAD
} XA_PlayerModuleLoaderHookContext;

typedef struct XA_PlayerModuleLoaderHookInstance XA_PlayerModuleLoaderHookInstance;
typedef int XA_PlayerModuleLoaderHookFunction(
									XA_PlayerModuleLoaderHookInstance *instance,
                                    XA_PlayerModuleLoaderHookAction action,
									XA_PlayerModuleLoaderHookContext context,
                                    XA_Control *control, 
                                    XA_DecoderInfo *decoder,
                                    void **symbol, 
                                    const char *library_name, 
                                    const char *symbol_name);
typedef struct {
    XA_PlayerModuleLoaderHookInstance *instance;
    XA_PlayerModuleLoaderHookFunction *function;
} XA_PlayerModuleLoaderHook;

typedef struct XA_PlayerInitHookInstance XA_PlayerInitHookInstance;
typedef int XA_PlayerInitHookFunction(XA_PlayerInitHookInstance *instance,
									  XA_Control *control,
									  XA_DecoderInfo *decoder);
												                
typedef struct {
	XA_PlayerInitHookInstance *instance;
	XA_PlayerInitHookFunction *function;
} XA_PlayerInitHook;

/*----------------------------------------------------------------------
|       prototypes
+---------------------------------------------------------------------*/
#ifdef __cplusplus
extern "C" {
#endif

extern int XA_EXPORT player_new(XA_Control **control, 
								const XA_ControlArguments *control_args);
extern int XA_EXPORT player_new_ext(XA_Control **control, 
									const XA_ControlArguments *control_args, 
									const XA_PlayerArguments *player_args);
extern int XA_EXPORT player_delete(XA_Control *control);
extern int XA_EXPORT player_set_priority(XA_Control *control, int priority);
extern int XA_EXPORT player_get_priority(XA_Control *control);

#ifdef __cplusplus
}

#define XA_CMSEND control_message_send

class XA_Player {
public:
    XA_Control *control;
    XA_Player() {
        control = (XA_Control *)0;
    }
    virtual ~XA_Player() {
        if (control) player_delete(control);
    }
    int SetPriority(int priority) {
        return player_set_priority(control, priority);
    }
    int GetPriority() {
        return player_get_priority(control);
    }
    int GetMessage(XA_Message *message) {
        return control_message_get(control, message);
    }
    int WaitMessage(XA_Message *message, int timeout) {
        return control_message_wait(control, message, timeout);
    }
    int Synchronize(int timeout) {
        return control_message_sync_to_command_queue(control, timeout);
    }
    int Exit() { 
        return XA_CMSEND(control, XA_MSG_COMMAND_EXIT);
    }
    int Ping(unsigned long tag) {
        return XA_CMSEND(control, XA_MSG_COMMAND_PING, tag);
    }
    int Play() {
        return XA_CMSEND(control, XA_MSG_COMMAND_PLAY);
    }
    int Pause() {
        return XA_CMSEND(control, XA_MSG_COMMAND_PAUSE);
    }
    int Stop() {
        return XA_CMSEND(control, XA_MSG_COMMAND_STOP);
    }
    int Seek(unsigned long offset, unsigned long range) {
        return XA_CMSEND(control, XA_MSG_COMMAND_SEEK, offset, range);
    }
    int InputOpen(const char *name) {
        return XA_CMSEND(control, XA_MSG_COMMAND_INPUT_OPEN, name);
    }
    int InputClose() {
        return XA_CMSEND(control, XA_MSG_COMMAND_INPUT_CLOSE);
    }
    int InputSendMessage(unsigned short type, void *data, unsigned long size) {
        return XA_CMSEND(control, XA_MSG_COMMAND_INPUT_SEND_MESSAGE,
                         type, data, size);
    }
    int InputAddFilter(const char *name, int where) {
        return XA_CMSEND(control, XA_MSG_COMMAND_INPUT_ADD_FILTER, 
                         name, where);
    }
    int InputRemoveFilter(const char *name, int id) {
        return XA_CMSEND(control, XA_MSG_COMMAND_INPUT_REMOVE_FILTER, 
                         name, id);
    }
    int InputFiltersList() {
        return XA_CMSEND(control, XA_MSG_COMMAND_INPUT_FILTERS_LIST);
    }
    int InputModuleRegister(const char *name) {
        return XA_CMSEND(control, XA_MSG_COMMAND_INPUT_MODULE_REGISTER, name);
    }
    int InputModuleQuery(int module) {
        return XA_CMSEND(control, XA_MSG_COMMAND_INPUT_MODULE_QUERY, module);
    }
    int InputModulesList() {
        return XA_CMSEND(control, XA_MSG_COMMAND_INPUT_MODULES_LIST);
    }
    int OutputOpen(const char *name) {
        return XA_CMSEND(control, XA_MSG_COMMAND_OUTPUT_OPEN, name);
    }
    int OutputClose() {
        return XA_CMSEND(control, XA_MSG_COMMAND_OUTPUT_CLOSE);
    }
    int OutputMute() {
        return XA_CMSEND(control, XA_MSG_COMMAND_OUTPUT_MUTE);
    }
    int OutputUnmute() {
        return XA_CMSEND(control, XA_MSG_COMMAND_OUTPUT_UNMUTE);
    }
    int OutputReset() {
        return XA_CMSEND(control, XA_MSG_COMMAND_OUTPUT_RESET);
    }
    int OutputDrain() {
        return XA_CMSEND(control, XA_MSG_COMMAND_OUTPUT_DRAIN);
    }
    int OutputSendMessage(unsigned short type, void *data, unsigned long size) {
        return XA_CMSEND(control, XA_MSG_COMMAND_OUTPUT_SEND_MESSAGE,
                         type, data, size);
    }
    int OutputAddFilter(const char *name, int where) {
        return XA_CMSEND(control, XA_MSG_COMMAND_OUTPUT_ADD_FILTER, 
                         name, where);
    }
    int OutputRemoveFilter(const char *name, int id) {
        return XA_CMSEND(control, XA_MSG_COMMAND_OUTPUT_REMOVE_FILTER, 
                         name, id);
    }
    int OutputFiltersList() {
        return XA_CMSEND(control, XA_MSG_COMMAND_OUTPUT_FILTERS_LIST);
    }
    int OutputModuleRegister(const char *name) {
        return XA_CMSEND(control, XA_MSG_COMMAND_OUTPUT_MODULE_REGISTER, name);
    }
    int OutputModuleQuery(int module) {
        return XA_CMSEND(control, XA_MSG_COMMAND_OUTPUT_MODULE_QUERY, module);
    }
    int OutputModulesList() {
        return XA_CMSEND(control, XA_MSG_COMMAND_OUTPUT_MODULES_LIST);
    }
    int FeedbackHandlerModuleRegister(const char *name) {
        return XA_CMSEND(control, XA_MSG_COMMAND_FEEDBACK_HANDLER_MODULE_REGISTER, name);
    }
    int FeedbackHandlerModuleQuery(int module_id) {
        return XA_CMSEND(control, XA_MSG_COMMAND_FEEDBACK_HANDLER_MODULE_QUERY, module_id);
    }
    int FeedbackHandlerModulesList() {
        return XA_CMSEND(control, XA_MSG_COMMAND_FEEDBACK_HANDLER_MODULES_LIST);
    }
    int FeedbackHandlerStart() {
        return XA_CMSEND(control, XA_MSG_COMMAND_FEEDBACK_HANDLER_START);
    }
    int FeedbackHandlerStop() {
        return XA_CMSEND(control, XA_MSG_COMMAND_FEEDBACK_HANDLER_STOP);
    }
    int FeedbackHandlerSendMessage(unsigned short type, void *data, unsigned long size) {
        return XA_CMSEND(control, XA_MSG_COMMAND_FEEDBACK_HANDLER_SEND_MESSAGE,
                         type, data, size);
    }
    int CodecSendMessage(unsigned short type, void *data, unsigned long size) {
        return XA_CMSEND(control, XA_MSG_COMMAND_CODEC_SEND_MESSAGE,
                         type, data, size);
    }
    int CodecModuleRegister(const char *name) {
        return XA_CMSEND(control, XA_MSG_COMMAND_CODEC_MODULE_REGISTER, name);
    }
    int CodecModuleQuery(int module) {
        return XA_CMSEND(control, XA_MSG_COMMAND_CODEC_MODULE_QUERY, module);
    }
    int CodecModulesList() {
        return XA_CMSEND(control, XA_MSG_COMMAND_CODEC_MODULES_LIST);
    }
    int SetPlayerMode(unsigned long mode) {
         return XA_CMSEND(control, XA_MSG_SET_PLAYER_MODE, mode);
    }
    int GetPlayerMode() {
        return XA_CMSEND(control, XA_MSG_GET_PLAYER_MODE);
    }
    int SetPlayerEnvironmentInteger(const char *name, long value) {
        return XA_CMSEND(control, XA_MSG_SET_PLAYER_ENVIRONMENT_INTEGER,
                         name, value);
    }
    int GetPlayerEnvironmentInteger(const char *name) {
        return XA_CMSEND(control, XA_MSG_GET_PLAYER_ENVIRONMENT_INTEGER, name);
    }
    int SetPlayerEnvironmentString(const char *name, const char *value) {
        return XA_CMSEND(control, XA_MSG_SET_PLAYER_ENVIRONMENT_STRING, 
                         name, value);
    }
    int GetPlayerEnvironmentString(const char *name) {
        return XA_CMSEND(control, XA_MSG_GET_PLAYER_ENVIRONMENT_STRING, name);
    }
    int UnsetPlayerEnvironment(const char *name) {
        return XA_CMSEND(control, XA_MSG_UNSET_PLAYER_ENVIRONMENT, name);
    }
    int SetInputModule(int module) {
        return XA_CMSEND(control, XA_MSG_SET_INPUT_MODULE, module);
    }
    int GetInputModule() {
        return XA_CMSEND(control, XA_MSG_GET_INPUT_MODULE);
    }
    int SetInputName(const char *name) {
        return XA_CMSEND(control, XA_MSG_SET_INPUT_NAME, name);
    }
    int GetInputName() {
        return XA_CMSEND(control, XA_MSG_GET_INPUT_NAME);
    }
    int SetInputPositionRange(unsigned long range) {
        return XA_CMSEND(control, XA_MSG_SET_INPUT_POSITION_RANGE, range);
    }
    int GetInputPositionRange() {
        return XA_CMSEND(control, XA_MSG_GET_INPUT_POSITION_RANGE);
    }
    int SetInputTimecodeGranularity(unsigned long granularity) {
        return XA_CMSEND(control, XA_MSG_SET_INPUT_TIMECODE_GRANULARITY,
                         granularity);
    }
    int GetInputTimecodeGranularity() {
        return XA_CMSEND(control, XA_MSG_GET_INPUT_TIMECODE_GRANULARITY);
    }
    int SetOutputModule(int module) {
        return XA_CMSEND(control, XA_MSG_SET_OUTPUT_MODULE, module);
    }
    int GetOutputModule() {
        return XA_CMSEND(control, XA_MSG_GET_OUTPUT_MODULE);
    }
    int SetOutputName(const char *name) {
        return XA_CMSEND(control, XA_MSG_SET_OUTPUT_NAME, name);
    }
    int GetOutputName() {
        return XA_CMSEND(control, XA_MSG_GET_OUTPUT_NAME);
    }
    int SetOutputVolume(int balance, int pcm_level, int master_level) {
        return XA_CMSEND(control, XA_MSG_SET_OUTPUT_VOLUME,
                         balance, pcm_level, master_level);
    }
    int GetOutputVolume() {
        return XA_CMSEND(control, XA_MSG_GET_OUTPUT_VOLUME);
    }
    int SetOutputChannels(int channels) {
        return XA_CMSEND(control, XA_MSG_SET_OUTPUT_CHANNELS, channels);
    }
    int GetOutputChannels() {
        return XA_CMSEND(control, XA_MSG_GET_OUTPUT_CHANNELS);
    }
    int SetOutputPorts(unsigned char ports) {
        return XA_CMSEND(control, XA_MSG_SET_OUTPUT_PORTS, ports);
    }
    int GetOutputPorts() {
        return XA_CMSEND(control, XA_MSG_GET_OUTPUT_PORTS);
    }
    int SetFeedbackAudioEventRate(int rate) {
        return XA_CMSEND(control, XA_MSG_SET_FEEDBACK_AUDIO_EVENT_RATE, rate);
    }
    int GetFeedbackAudioEventRate() {
        return XA_CMSEND(control, XA_MSG_GET_FEEDBACK_AUDIO_EVENT_RATE);
    }
    int SetFeedbackHandlerName(const char *name) {
        return XA_CMSEND(control, XA_MSG_SET_FEEDBACK_HANDLER_NAME, name);
    }
    int GetFeedbackHandlerName() {
        return XA_CMSEND(control, XA_MSG_GET_FEEDBACK_HANDLER_NAME);
    }
    int SetFeedbackHandlerModule(int module) {
        return XA_CMSEND(control, XA_MSG_SET_FEEDBACK_HANDLER_MODULE, module);
    }
    int GetFeedbackHandlerModule() {
        return XA_CMSEND(control, XA_MSG_GET_FEEDBACK_HANDLER_MODULE);
    }
    int SetFeedbackHandlerEnvironmentInteger(const char *name, long value) {
        return XA_CMSEND(control, XA_MSG_SET_FEEDBACK_HANDLER_ENVIRONMENT_INTEGER,
                         name, value);
    }
    int GetFeedbackHandlerEnvironmentInteger(const char *name) {
        return XA_CMSEND(control, XA_MSG_GET_FEEDBACK_HANDLER_ENVIRONMENT_INTEGER, name);
    }
    int SetFeedbackHandlerEnvironmentString(const char *name, const char *value) {
        return XA_CMSEND(control, XA_MSG_SET_FEEDBACK_HANDLER_ENVIRONMENT_STRING, 
                         name, value);
    }
    int GetFeedbackHandlerEnvironmentString(const char *name) {
        return XA_CMSEND(control, XA_MSG_GET_FEEDBACK_HANDLER_ENVIRONMENT_STRING, name);
    }
    int UnsetFeedbackHandlerEnvironment(const char *name) {
        return XA_CMSEND(control, XA_MSG_UNSET_FEEDBACK_HANDLER_ENVIRONMENT, name);
    }
    int SetCodecQuality(unsigned int quality) {
        return XA_CMSEND(control, XA_MSG_SET_CODEC_QUALITY, quality);
    }
    int GetCodecQuality() {
        return XA_CMSEND(control, XA_MSG_GET_CODEC_QUALITY);
    }
    int SetCodecEqualizer(XA_EqualizerInfo *equalizer) {
        return XA_CMSEND(control, XA_MSG_SET_CODEC_EQUALIZER, equalizer);
    }
    int GetCodecEqualizer() {
        return XA_CMSEND(control, XA_MSG_GET_CODEC_EQUALIZER);
    }
    int SetCodecModule(int module) {
        return XA_CMSEND(control, XA_MSG_SET_CODEC_MODULE, module);
    }
    int GetCodecModule() {
        return XA_CMSEND(control, XA_MSG_GET_CODEC_MODULE);
    }
    int SetNotificationMask(unsigned long mask) {
        return XA_CMSEND(control, XA_MSG_SET_NOTIFICATION_MASK, mask);
    }
    int GetNotificationMask() {
        return XA_CMSEND(control, XA_MSG_GET_NOTIFICATION_MASK);
    }
    int SetDebugLevel(int level) {
        return XA_CMSEND(control, XA_MSG_SET_DEBUG_LEVEL, level);
    }
    int GetDebugLevel() {
        return XA_CMSEND(control, XA_MSG_GET_DEBUG_LEVEL);
    }
    virtual int Run(XA_ControlArguments *control_args) {
        int status;
        status = player_new(&control, control_args);
        if (status) control = (XA_Control *)0;
        return status;
    }
    virtual int Run(XA_ControlArguments *control_args, XA_PlayerArguments *player_args) {
        int status;
        status = player_new_ext(&control, control_args, player_args);
        if (status) control = (XA_Control *)0;
        return status;
    }
    virtual void ProcessMessage(XA_Message *message) {
        switch (message->code) {
          case XA_MSG_NOTIFY_READY:
            OnNotifyReady();
            break;

          case XA_MSG_NOTIFY_ACK:
            OnNotifyAck(message->data.ack);
            break;

          case XA_MSG_NOTIFY_NACK:
            OnNotifyNack(&message->data.nack);
            break;

          case XA_MSG_NOTIFY_PONG:
            OnNotifyPong(message->data.tag);
            break;

          case XA_MSG_NOTIFY_EXITED:
            OnNotifyExited();
            break;

          case XA_MSG_NOTIFY_PLAYER_STATE:
            OnNotifyPlayerState((XA_PlayerState)message->data.state);
            break;

          case XA_MSG_NOTIFY_PLAYER_MODE:
            OnNotifyPlayerMode(message->data.mode);
            break;

          case XA_MSG_NOTIFY_PLAYER_ENVIRONMENT_INTEGER:
            OnNotifyPlayerEnvironmentInteger(
                message->data.environment_info.name,
                message->data.environment_info.value.integer);
            break;

          case XA_MSG_NOTIFY_PLAYER_ENVIRONMENT_STRING:
            OnNotifyPlayerEnvironmentString(
                message->data.environment_info.name,
                message->data.environment_info.value.string);
            break;

          case XA_MSG_NOTIFY_INPUT_STATE:
            OnNotifyInputState((XA_InputState)message->data.state);
            break;

          case XA_MSG_NOTIFY_INPUT_NAME:
            OnNotifyInputName(message->data.name);
            break;

          case XA_MSG_NOTIFY_INPUT_CAPS:
            OnNotifyInputCaps(message->data.caps);
            break;

          case XA_MSG_NOTIFY_INPUT_POSITION:
            OnNotifyInputPosition(message->data.position.offset,
                                  message->data.position.range);
            break;

          case XA_MSG_NOTIFY_INPUT_POSITION_RANGE:
            OnNotifyInputPositionRange(message->data.range);
            break;

          case XA_MSG_NOTIFY_INPUT_TIMECODE:
            OnNotifyInputTimecode(&message->data.timecode);
            break;

          case XA_MSG_NOTIFY_INPUT_TIMECODE_GRANULARITY:
            OnNotifyInputTimecodeGranularity(message->data.granularity);
            break;

          case XA_MSG_NOTIFY_INPUT_MODULE:
            OnNotifyInputModule(message->data.module_id);
            break;

          case XA_MSG_NOTIFY_INPUT_MODULE_INFO:
            OnNotifyInputModuleInfo(&message->data.module_info);
            break;

          case XA_MSG_NOTIFY_INPUT_DEVICE_INFO:
            OnNotifyInputDeviceInfo(&message->data.device_info);
            break;

          case XA_MSG_NOTIFY_INPUT_FILTER_INFO:
            OnNotifyInputFilterInfo(&message->data.filter_info);
            break;

          case XA_MSG_NOTIFY_OUTPUT_STATE:
            OnNotifyOutputState((XA_OutputState)message->data.state);
            break;

          case XA_MSG_NOTIFY_OUTPUT_NAME:
            OnNotifyOutputName(message->data.name);
            break;

          case XA_MSG_NOTIFY_OUTPUT_CAPS:
            OnNotifyOutputCaps(message->data.caps);
            break;

          case XA_MSG_NOTIFY_OUTPUT_POSITION:
            OnNotifyOutputPosition(message->data.position.offset,
                                   message->data.position.range);
            break;

          case XA_MSG_NOTIFY_OUTPUT_TIMECODE:
            OnNotifyOutputTimecode(&message->data.timecode);
            break;

          case XA_MSG_NOTIFY_OUTPUT_VOLUME:
            OnNotifyOutputVolume(&message->data.volume);
            break;

          case XA_MSG_NOTIFY_OUTPUT_BALANCE:
            OnNotifyOutputBalance(message->data.volume.balance);
            break;

          case XA_MSG_NOTIFY_OUTPUT_PCM_LEVEL:
            OnNotifyOutputPcmLevel(message->data.volume.pcm_level);
            break;

          case XA_MSG_NOTIFY_OUTPUT_MASTER_LEVEL:
            OnNotifyOutputMasterLevel(message->data.volume.master_level);
            break;

          case XA_MSG_NOTIFY_OUTPUT_CHANNELS:
            OnNotifyOutputChannels((XA_OutputChannels)message->data.channels);
            break;

          case XA_MSG_NOTIFY_OUTPUT_PORTS:
            OnNotifyOutputPorts(message->data.ports);
            break;

          case XA_MSG_NOTIFY_OUTPUT_MODULE:
            OnNotifyOutputModule(message->data.module_id);
            break;

          case XA_MSG_NOTIFY_OUTPUT_MODULE_INFO:
            OnNotifyOutputModuleInfo(&message->data.module_info);
            break;

          case XA_MSG_NOTIFY_OUTPUT_DEVICE_INFO:
            OnNotifyOutputDeviceInfo(&message->data.device_info);
            break;

          case XA_MSG_NOTIFY_OUTPUT_FILTER_INFO:
            OnNotifyOutputFilterInfo(&message->data.filter_info);
            break;

          case XA_MSG_NOTIFY_STREAM_DURATION:
            OnNotifyStreamDuration(message->data.duration);
            break;

          case XA_MSG_NOTIFY_STREAM_MIME_TYPE:
            OnNotifyStreamMimeType(message->data.mime_type);
            break;

          case XA_MSG_NOTIFY_STREAM_PARAMETERS:
            OnNotifyStreamParameters(&message->data.stream_parameters);
            break;

          case XA_MSG_NOTIFY_STREAM_PROPERTIES:
            OnNotifyStreamProperties(&message->data.properties);
            break;

          case XA_MSG_NOTIFY_CODEC_QUALITY:
            OnNotifyCodecQuality(message->data.quality);
            break;

          case XA_MSG_NOTIFY_CODEC_EQUALIZER:
            OnNotifyCodecEqualizer(message->data.equalizer);
            break;

          case XA_MSG_NOTIFY_CODEC_MODULE:
            OnNotifyCodecModule(message->data.module_id);
            break;

          case XA_MSG_NOTIFY_CODEC_MODULE_INFO:
            OnNotifyCodecModuleInfo(&message->data.module_info);
            break;

          case XA_MSG_NOTIFY_CODEC_DEVICE_INFO:
            OnNotifyCodecDeviceInfo(&message->data.device_info);
            break;

          case XA_MSG_NOTIFY_FEEDBACK_AUDIO_EVENT_RATE:
            OnNotifyFeedbackAudioEventRate(message->data.rate);
            break;

          case XA_MSG_NOTIFY_FEEDBACK_HANDLER_STATE:
            OnNotifyFeedbackHandlerState((XA_FeedbackHandlerState)message->data.state);
            break;

          case XA_MSG_NOTIFY_FEEDBACK_HANDLER_MODULE:
            OnNotifyFeedbackHandlerModule(message->data.module_id);
            break;

          case XA_MSG_NOTIFY_FEEDBACK_HANDLER_MODULE_INFO:
            OnNotifyFeedbackHandlerModuleInfo(&message->data.module_info);
            break;

          case XA_MSG_NOTIFY_FEEDBACK_HANDLER_NAME:
            OnNotifyFeedbackHandlerName(message->data.name);
            break;

          case XA_MSG_NOTIFY_FEEDBACK_HANDLER_INFO:
            OnNotifyFeedbackHandlerInfo(&message->data.device_info);
            break;

          case XA_MSG_NOTIFY_FEEDBACK_HANDLER_ENVIRONMENT_INTEGER:
            OnNotifyFeedbackHandlerEnvironmentInteger(message->data.environment_info.name,
                                                      message->data.environment_info.value.integer);
            break;

          case XA_MSG_NOTIFY_FEEDBACK_HANDLER_ENVIRONMENT_STRING:
            OnNotifyFeedbackHandlerEnvironmentString(message->data.environment_info.name,
                                                     message->data.environment_info.value.string);
            break;

          case XA_MSG_NOTIFY_FEEDBACK_AUDIO_EVENT:
            OnNotifyFeedbackAudioEvent(&message->data.audio_event);
            break;

          case XA_MSG_NOTIFY_FEEDBACK_TAG_EVENT:
            OnNotifyFeedbackTagEvent(&message->data.tag_event);
            break;

          case XA_MSG_NOTIFY_NOTIFICATION_MASK:
            OnNotifyNotificationMask(message->data.notification_mask);
            break;

          case XA_MSG_NOTIFY_DEBUG_LEVEL:
            OnNotifyDebugLevel(message->data.debug_level);
            break;

          case XA_MSG_NOTIFY_PROGRESS:
            OnNotifyProgress(&message->data.progress);
            break;

          case XA_MSG_NOTIFY_DEBUG:
            OnNotifyDebug(&message->data.debug);
            break;

          case XA_MSG_NOTIFY_ERROR:
            OnNotifyError(&message->data.error);
            break;

          case XA_MSG_NOTIFY_PRIVATE_DATA:
            OnNotifyPrivateData(&message->data.private_data);
            break;

          default:
            break;
        }
    }

protected:
    virtual void OnNotifyReady() {};
    virtual void OnNotifyAck(int command) {};
    virtual void OnNotifyNack(XA_NackInfo *info) {};
    virtual void OnNotifyPong(unsigned long tag) {};
    virtual void OnNotifyExited() {};
    virtual void OnNotifyPlayerState(XA_PlayerState state) {};
    virtual void OnNotifyPlayerMode(unsigned long mode) {};
    virtual void OnNotifyPlayerEnvironmentInteger(const char *name, long value) {};
    virtual void OnNotifyPlayerEnvironmentString(const char *name, const char *value) {};
    virtual void OnNotifyInputState(XA_InputState state) {};
    virtual void OnNotifyInputName(const char *name) {};
    virtual void OnNotifyInputCaps(unsigned long caps) {};
    virtual void OnNotifyInputPosition(unsigned long offset, unsigned long range) {};
    virtual void OnNotifyInputPositionRange(unsigned long range) {};
    virtual void OnNotifyInputTimecode(XA_TimecodeInfo *timecode) {};
    virtual void OnNotifyInputTimecodeGranularity(unsigned long granularity) {};
    virtual void OnNotifyInputModule(int module_id) {};
    virtual void OnNotifyInputModuleInfo(XA_ModuleInfo *info) {};
    virtual void OnNotifyInputDeviceInfo(XA_DeviceInfo *info) {};
    virtual void OnNotifyInputFilterInfo(XA_FilterInfo *info) {};
    virtual void OnNotifyOutputState(XA_OutputState state) {};
    virtual void OnNotifyOutputName(const char *name) {};
    virtual void OnNotifyOutputCaps(unsigned long caps) {};
    virtual void OnNotifyOutputPosition(unsigned long offset, unsigned long range) {};
    virtual void OnNotifyOutputTimecode(XA_TimecodeInfo *timecode) {};
    virtual void OnNotifyOutputVolume(XA_VolumeInfo *info) {};
    virtual void OnNotifyOutputBalance(unsigned char balance) {};
    virtual void OnNotifyOutputPcmLevel(unsigned char level) {};
    virtual void OnNotifyOutputMasterLevel(unsigned char level) {};
    virtual void OnNotifyOutputChannels(XA_OutputChannels channels) {};
    virtual void OnNotifyOutputPorts(unsigned long ports) {};
    virtual void OnNotifyOutputModule(int module_id) {};
    virtual void OnNotifyOutputModuleInfo(XA_ModuleInfo *info) {};
    virtual void OnNotifyOutputDeviceInfo(XA_DeviceInfo *info) {};
    virtual void OnNotifyOutputFilterInfo(XA_FilterInfo *info) {};
    virtual void OnNotifyStreamDuration(unsigned long duration) {};
    virtual void OnNotifyStreamMimeType(const char *mime_type) {};
    virtual void OnNotifyStreamParameters(XA_StreamParameters *info) {};
    virtual void OnNotifyStreamProperties(XA_PropertyList *properties) {};
    virtual void OnNotifyCodecQuality(unsigned int quality) {};
    virtual void OnNotifyCodecEqualizer(XA_EqualizerInfo *info) {};
    virtual void OnNotifyCodecModule(int module_id) {};
    virtual void OnNotifyCodecModuleInfo(XA_ModuleInfo *info) {};
    virtual void OnNotifyCodecDeviceInfo(XA_DeviceInfo *info) {};
    virtual void OnNotifyFeedbackAudioEventRate(unsigned long rate) {};
    virtual void OnNotifyFeedbackHandlerState(XA_FeedbackHandlerState state) {};
    virtual void OnNotifyFeedbackHandlerModule(int module_id) {};
    virtual void OnNotifyFeedbackHandlerModuleInfo(XA_ModuleInfo *info) {};
    virtual void OnNotifyFeedbackHandlerName(const char *name) {};
    virtual void OnNotifyFeedbackHandlerInfo(XA_DeviceInfo *info) {};
    virtual void OnNotifyFeedbackHandlerEnvironmentInteger(const char *name, long value) {};
    virtual void OnNotifyFeedbackHandlerEnvironmentString(const char *name, const char *value) {};
    virtual void OnNotifyFeedbackAudioEvent(XA_AudioEvent *event) {};
    virtual void OnNotifyFeedbackTagEvent(XA_TagEvent *event) {};
    virtual void OnNotifyNotificationMask(unsigned long mask) {};
    virtual void OnNotifyDebugLevel(int level) {};
    virtual void OnNotifyProgress(XA_ProgressInfo *info) {};
    virtual void OnNotifyDebug(XA_DebugInfo *info) {};
    virtual void OnNotifyError(XA_ErrorInfo *info) {};
    virtual void OnNotifyPrivateData(XA_PrivateData *data) {};
};

#endif /* __cplusplus */

#endif /* _PLAYER_H_ */
