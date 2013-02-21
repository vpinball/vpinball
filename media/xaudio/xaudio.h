/*****************************************************************
|
|      Xaudio General Definitions
|
|      (c) 1996-2000 Xaudio Corporation
|      Author: Gilles Boccon-Gibod (gilles@xaudio.com)
|
 ****************************************************************/

#ifndef _XAUDIO_H_
#define _XAUDIO_H_

/*----------------------------------------------------------------------
|       types
+---------------------------------------------------------------------*/
typedef void (*XA_ProgressNotificationFunction)(void *client,
                                                int source,
                                                int code,
                                                long value,
                                                const char *message);

typedef void (*XA_DebugNotificationFunction)(void *client,
                                             int source, 
                                             int level,
                                             const char *message, 
                                             const char *reason);

typedef void (*XA_ErrorNotificationFunction)(void *client,
                                             int source,
                                             int code,
                                             const char *message,
                                             const char *reason);

typedef void (*XA_PrivateDataNotificationFunction)(void *client,
                                                   int source,
                                                   int type,
                                                   const void *data,
                                                   unsigned long size);

typedef struct {
    void                              *client;
    XA_ProgressNotificationFunction    notify_progress;
    XA_DebugNotificationFunction       notify_debug;
    XA_ErrorNotificationFunction       notify_error;
	XA_PrivateDataNotificationFunction notify_private_data;
} XA_NotificationClient;

#define XA_NOTIFY_PROGRESS(_client, _source, _code, _value, _message)    \
if ((_client) && (_client)->notify_progress)                             \
    (*(_client)->notify_progress)((_client)->client,                     \
     _source, _code, _value, _message)

#define XA_NOTIFY_DEBUG(_client, _source, _level, _message, _reason)     \
if ((_client) && (_client)->notify_debug)                                \
    (*(_client)->notify_debug)((_client)->client,                        \
     _source, _level, _message, _reason)

#define XA_NOTIFY_ERROR(_client, _source, _code, _message, _reason)      \
if ((_client) && (_client)->notify_error)                                \
    (*(_client)->notify_error)((_client)->client,                        \
     _source, _code, _message, _reason)

#define XA_NOTIFY_PRIVATE_DATA(_client, _source, _type, _data, _size)    \
if ((_client) && (_client)->notify_private_data)                              \
    (*(_client)->notify_private_data)((_client)->client,                 \
     _source, _type, _data, _size)

/*----------------------------------------------------------------------
|       macros
+---------------------------------------------------------------------*/
#if defined(WIN32) || defined(_WIN32) || defined(_WIN32_WCE)
#if UNDER_CE
#define XA_EXPORT __cdecl
#define XA_IMPORT __cdecl
#else
#define XA_EXPORT __stdcall
#define XA_IMPORT __stdcall
#endif /* UNDER_CE */
#else
#define XA_EXPORT
#define XA_IMPORT
#endif

/*----------------------------------------------------------------------
|       version indexes
+---------------------------------------------------------------------*/
#define XA_VERSION_ID_SYNC_API       1
#define XA_VERSION_ID_ASYNC_API      2 
#define XA_VERSION_ID_IMPLEMENTATION 3

/*----------------------------------------------------------------------
|       version constants
+---------------------------------------------------------------------*/
#define XA_IMPLEMENTATION_MAJOR     3
#define XA_IMPLEMENTATION_MINOR     2  
#define XA_IMPLEMENTATION_REVISION  9
#define XA_IMPLEMENTATION_VERSION          \
 XA_VERSION_ID(XA_IMPLEMENTATION_MAJOR,    \
               XA_IMPLEMENTATION_MINOR,    \
               XA_IMPLEMENTATION_REVISION)

/*----------------------------------------------------------------------
|       macros
+---------------------------------------------------------------------*/
#define XA_VERSION_ID(major, minor, revision) \
    (((unsigned long)(major) << 16)  |             \
     ((unsigned long)(minor) << 8)   |             \
     ((unsigned long)(revision)))
#define XA_VERSION_ID_MAJOR(id)    ((((unsigned long)id)>>16)&0xFF)
#define XA_VERSION_ID_MINOR(id)    ((((unsigned long)id)>> 8)&0xFF)
#define XA_VERSION_ID_REVISION(id) ((((unsigned long)id)    )&0xFF)
#define XA_VERSION_ID_MAJOR_MINOR_MASK 0xFFFF00

/*----------------------------------------------------------------------
|       error codes
+---------------------------------------------------------------------*/
#define XA_SUCCESS                           ( 0)
#define XA_FAILURE                           (-1)

/* general error codes */
#define XA_ERROR_BASE_GENERAL              (-100)
#define XA_ERROR_OUT_OF_MEMORY             (XA_ERROR_BASE_GENERAL - 0)
#define XA_ERROR_OUT_OF_RESOURCES          (XA_ERROR_BASE_GENERAL - 1)
#define XA_ERROR_INVALID_PARAMETERS        (XA_ERROR_BASE_GENERAL - 2)
#define XA_ERROR_INTERNAL                  (XA_ERROR_BASE_GENERAL - 3)
#define XA_ERROR_TIMEOUT                   (XA_ERROR_BASE_GENERAL - 4)
#define XA_ERROR_VERSION_EXPIRED           (XA_ERROR_BASE_GENERAL - 5)
#define XA_ERROR_VERSION_MISMATCH          (XA_ERROR_BASE_GENERAL - 6)

/* network error codes */
#define XA_ERROR_BASE_NETWORK              (-200)
#define XA_ERROR_CONNECT_TIMEOUT           (XA_ERROR_BASE_NETWORK -  0)
#define XA_ERROR_CONNECT_FAILED            (XA_ERROR_BASE_NETWORK -  1)
#define XA_ERROR_CONNECTION_REFUSED        (XA_ERROR_BASE_NETWORK -  2)
#define XA_ERROR_ACCEPT_FAILED             (XA_ERROR_BASE_NETWORK -  3)
#define XA_ERROR_LISTEN_FAILED             (XA_ERROR_BASE_NETWORK -  4)
#define XA_ERROR_SOCKET_FAILED             (XA_ERROR_BASE_NETWORK -  5)
#define XA_ERROR_SOCKET_CLOSED             (XA_ERROR_BASE_NETWORK -  6)
#define XA_ERROR_BIND_FAILED               (XA_ERROR_BASE_NETWORK -  7)
#define XA_ERROR_HOST_UNKNOWN              (XA_ERROR_BASE_NETWORK -  8)
#define XA_ERROR_HTTP_INVALID_REPLY        (XA_ERROR_BASE_NETWORK -  9)
#define XA_ERROR_HTTP_ERROR_REPLY          (XA_ERROR_BASE_NETWORK - 10)
#define XA_ERROR_HTTP_FAILURE              (XA_ERROR_BASE_NETWORK - 11)
#define XA_ERROR_FTP_INVALID_REPLY         (XA_ERROR_BASE_NETWORK - 12)
#define XA_ERROR_FTP_ERROR_REPLY           (XA_ERROR_BASE_NETWORK - 13)
#define XA_ERROR_FTP_FAILURE               (XA_ERROR_BASE_NETWORK - 14)

/* control error codes */
#define XA_ERROR_BASE_CONTROL              (-300)
#define XA_ERROR_PIPE_FAILED               (XA_ERROR_BASE_CONTROL - 0)
#define XA_ERROR_FORK_FAILED               (XA_ERROR_BASE_CONTROL - 1)
#define XA_ERROR_SELECT_FAILED             (XA_ERROR_BASE_CONTROL - 2)
#define XA_ERROR_PIPE_CLOSED               (XA_ERROR_BASE_CONTROL - 3)
#define XA_ERROR_PIPE_READ_FAILED          (XA_ERROR_BASE_CONTROL - 4)
#define XA_ERROR_PIPE_WRITE_FAILED         (XA_ERROR_BASE_CONTROL - 5)
#define XA_ERROR_INVALID_MESSAGE           (XA_ERROR_BASE_CONTROL - 6)
#define XA_ERROR_CIRQ_FULL                 (XA_ERROR_BASE_CONTROL - 7)
#define XA_ERROR_POST_FAILED               (XA_ERROR_BASE_CONTROL - 8)

/* url error codes */
#define XA_ERROR_BASE_URL                  (-400)
#define XA_ERROR_URL_UNSUPPORTED_SCHEME    (XA_ERROR_BASE_URL - 0)
#define XA_ERROR_URL_INVALID_SYNTAX        (XA_ERROR_BASE_URL - 1)

/* i/o error codes */
#define XA_ERROR_BASE_IO                   (-500)
#define XA_ERROR_OPEN_FAILED               (XA_ERROR_BASE_IO -  0)
#define XA_ERROR_CLOSE_FAILED              (XA_ERROR_BASE_IO -  1)
#define XA_ERROR_READ_FAILED               (XA_ERROR_BASE_IO -  2)
#define XA_ERROR_WRITE_FAILED              (XA_ERROR_BASE_IO -  3)
#define XA_ERROR_PERMISSION_DENIED         (XA_ERROR_BASE_IO -  4)
#define XA_ERROR_NO_DEVICE                 (XA_ERROR_BASE_IO -  5)
#define XA_ERROR_IOCTL_FAILED              (XA_ERROR_BASE_IO -  6)
#define XA_ERROR_MODULE_NOT_FOUND          (XA_ERROR_BASE_IO -  7)
#define XA_ERROR_UNSUPPORTED_INPUT         (XA_ERROR_BASE_IO -  8)
#define XA_ERROR_UNSUPPORTED_OUTPUT        (XA_ERROR_BASE_IO -  9)
#define XA_ERROR_UNSUPPORTED_FORMAT        (XA_ERROR_BASE_IO - 10)
#define XA_ERROR_DEVICE_BUSY               (XA_ERROR_BASE_IO - 11)
#define XA_ERROR_NO_SUCH_DEVICE            (XA_ERROR_BASE_IO - 12)
#define XA_ERROR_NO_SUCH_FILE              (XA_ERROR_BASE_IO - 13)
#define XA_ERROR_INPUT_EOS                 (XA_ERROR_BASE_IO - 14)

/* codec error codes */
#define XA_ERROR_BASE_CODEC                (-600)
#define XA_ERROR_NO_CODEC                  (XA_ERROR_BASE_CODEC - 0)

/* bitstream error codes */
#define XA_ERROR_BASE_BITSTREAM            (-700)
#define XA_ERROR_INVALID_FRAME             (XA_ERROR_BASE_BITSTREAM - 0)

/* dynamic linking error codes */
#define XA_ERROR_BASE_DYNLINK              (-800)
#define XA_ERROR_DLL_NOT_FOUND             (XA_ERROR_BASE_DYNLINK - 0)
#define XA_ERROR_SYMBOL_NOT_FOUND          (XA_ERROR_BASE_DYNLINK - 1)

/* environment variables / porperties  error codes */
#define XA_ERROR_BASE_ENVIRONMENT          (-900)
#define XA_ERROR_NO_SUCH_ENVIRONMENT       (XA_ERROR_BASE_ENVIRONMENT - 0)
#define XA_ERROR_NO_SUCH_PROPERTY          (XA_ERROR_BASE_ENVIRONMENT - 0)
#define XA_ERROR_ENVIRONMENT_TYPE_MISMATCH (XA_ERROR_BASE_ENVIRONMENT - 1)
#define XA_ERROR_PROPERTY_TYPE_MISMATCH    (XA_ERROR_BASE_ENVIRONMENT - 1)

/* modules */
#define XA_ERROR_BASE_MODULES              (-1000)
#define XA_ERROR_NO_SUCH_INTERFACE         (XA_ERROR_BASE_MODULES - 0)

/*----------------------------------------------------------------------
|       error strings
+---------------------------------------------------------------------*/
#ifdef __cplusplus
extern "C" {
#endif

const char * XA_EXPORT xaudio_error_string(int code);
unsigned long XA_EXPORT xaudio_get_version(unsigned int version_index);

#ifdef __cplusplus
}

class XaudioException {
public:
    XaudioException(int code):error_code(code) {};
    int GetErrorCode() { return error_code; };
    const char *GetErrorString() { return xaudio_error_string(error_code); };

private:
    int error_code;
};

#endif /* __cplusplus */

#endif /* _XAUDIO_H_ */


