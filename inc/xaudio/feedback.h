/*****************************************************************
|
|      XAudio Player. Feddback API
|
|      (c) 1996-2001 Xaudio Corporation
|      Author: Gilles Boccon-Gibod (gilles@xaudio.com)
|
 ****************************************************************/

#ifndef _FEEDBACK_H_
#define _FEEDBACK_H_

 /*----------------------------------------------------------------------
|       includes
+---------------------------------------------------------------------*/
#include "xaudio.h"
#include "properties.h"
#include "control.h"

/*----------------------------------------------------------------------
|       constants
+---------------------------------------------------------------------*/
#define XA_FEEDBACK_HANDLER_AUTOSELECT                    (-1)
#define XA_FEEDBACK_HANDLER_MAX_NAME_LENGTH                255
#define XA_FEEDBACK_HANDLER_MAX_DESCRIPTION_LENGTH         255

#define XA_FEEDBACK_HANDLER_QUERY_MODULE_NAME             0x01
#define XA_FEEDBACK_HANDLER_QUERY_NB_HANDLERS             0x02
#define XA_FEEDBACK_HANDLER_QUERY_HANDLER_NAME            0x04

#define XA_FEEDBACK_HANDLER_QUERY_NAME_IS_GENERIC         0x01
#define XA_FEEDBACK_HANDLER_QUERY_HANDLER_IS_DEFAULT      0x02

#define XA_FEEDBACK_HANDLER_ENVIRONMENT_MAX_STRING_LENGTH 1023
#define XA_FEEDBACK_HANDLER_ENVIRONMENT_MAX_NAME_LENGTH    255

/*----------------------------------------------------------------------
|       types
+---------------------------------------------------------------------*/
typedef struct XA_FeedbackHandlerCallbackClient XA_FeedbackHandlerCallbackClient;

typedef struct {
    int (XA_EXPORT *get_environment_integer)(
        XA_FeedbackHandlerCallbackClient *client,
        const char *name,
        long *value);
    int (XA_EXPORT *get_environment_string)(
        XA_FeedbackHandlerCallbackClient *client,
        const char *name,
        char *value);    
    int (XA_EXPORT *add_environment_listener)(
        XA_FeedbackHandlerCallbackClient *client,
        const char *name,
        void *listener,
        XA_PropertyListenerCallback callback,
        XA_PropertyHandle *handle);
    int (XA_EXPORT *remove_environment_listener)(
        XA_FeedbackHandlerCallbackClient *client,
        XA_PropertyHandle handle);
    int (XA_EXPORT *forward_message)(
        XA_FeedbackHandlerCallbackClient *client,
        XA_Message *message);
} XA_FeedbackHandlerCallbackTable;

typedef struct {
    int   id;
    void *global;
} XA_FeedbackHandlerModuleClassInfo;

typedef struct {
    int           index;
    unsigned long flags;
    char          name[XA_FEEDBACK_HANDLER_MAX_NAME_LENGTH];
    char          description[XA_FEEDBACK_HANDLER_MAX_DESCRIPTION_LENGTH];
} XA_FeedbackHandlerModuleQuery;

typedef struct XA_FeedbackHandlerInstance XA_FeedbackHandlerInstance;

typedef struct {
    unsigned long api_version_id;
    int  (*feedback_handler_module_probe)(const char *name);
    int  (*feedback_handler_module_query)(XA_FeedbackHandlerModuleQuery *query, 
                                         unsigned long query_mask);
    int  (*feedback_handler_new)(XA_FeedbackHandlerInstance **feedback_handler, 
                                 const char *name,
                                 XA_FeedbackHandlerModuleClassInfo *class_info,
                                 XA_FeedbackHandlerCallbackClient *client,
                                 XA_FeedbackHandlerCallbackTable *callbacks);
    int  (*feedback_handler_delete)(XA_FeedbackHandlerInstance *feedback_handler);
    int  (*feedback_handler_start)(XA_FeedbackHandlerInstance *feedback_handler);
    int  (*feedback_handler_stop)(XA_FeedbackHandlerInstance *feedback_handler);
    int  (*feedback_handler_send_message)(XA_FeedbackHandlerInstance *feedback_handler, 
                                         int type, const void *data, unsigned long size);
    int  (*feedback_handler_event)(XA_FeedbackHandlerInstance *feedback_handler, XA_AudioEvent *event);
} XA_FeedbackHandlerModule;

#endif /* _FEEDBACK_H_ */
