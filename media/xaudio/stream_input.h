/*****************************************************************
|
|      Network Library. Stream Input Module
|
|      (c) 1996-2000 Xaudio Corporation
|      Author: Gilles Boccon-Gibod (gilles@xaudio.com)
|
 ****************************************************************/

#ifndef _STREAM_INPUT_H_
#define _STREAM_INPUT_H_

/*----------------------------------------------------------------------
|       includes
+---------------------------------------------------------------------*/
#include "decoder.h"


/*----------------------------------------------------------------------
|       constants
+---------------------------------------------------------------------*/
enum {
    XA_PROGRESS_CODE_STREAM_CONNECTING = 0x10,
    XA_PROGRESS_CODE_STREAM_CONNECTED,
    XA_PROGRESS_CODE_STREAM_SENDING_REQUEST,
    XA_PROGRESS_CODE_STREAM_BUFFER_UNDERFLOW,
    XA_PROGRESS_CODE_STREAM_BUFFER_OK,
    XA_PROGRESS_CODE_STREAM_PREBUFFER_FULLNESS,
    XA_PROGRESS_CODE_STREAM_BUFFER_FULLNESS,
    XA_PROGRESS_CODE_STREAM_REACHED_EOS
};

#define XA_STREAM_NETWORK_HTTP_PROXY_HOSTNAME_ENV \
    "NETWORK.HTTP.PROXY.HOSTNAME"
#define XA_STREAM_NETWORK_HTTP_PROXY_PORT_ENV \
    "NETWORK.HTTP.PROXY.PORT"
#define XA_STREAM_NETWORK_HTTP_CUSTOM_HEADER_ENV \
    "NETWORK.HTTP.CUSTOM-HEADER"
#define XA_STREAM_NETWORK_HTTP_CONNECT_TIMEOUT \
    "NETWORK.HTTP.CONNECT-TIMEOUT"
#define XA_STREAM_NETWORK_HTTP_READ_TIMEOUT \
    "NETWORK.HTTP.READ-TIMEOUT"

/*----------------------------------------------------------------------
|       prototypes
+---------------------------------------------------------------------*/
#ifdef __cplusplus
extern "C" {
#endif

int XA_EXPORT stream_input_module_register(XA_InputModule *module);

#ifdef __cplusplus
}
#endif
 
#endif /* _STREAM_INPUT_H_ */
