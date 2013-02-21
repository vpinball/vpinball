/*****************************************************************
|
|      Utility Library. Property lists
|
|      (c) 1996-2000 Xaudio Corporation
|      Author: Gilles Boccon-Gibod (gilles@xaudio.com)
|
****************************************************************/

#ifndef _PROPERTIES_H_
#define _PROPERTIES_H_

/*----------------------------------------------------------------------
|       includes
+---------------------------------------------------------------------*/
#include "xaudio.h"

/*----------------------------------------------------------------------
|       types
+---------------------------------------------------------------------*/
typedef enum {
    XA_PROPERTY_TYPE_STRING,
    XA_PROPERTY_TYPE_INTEGER
} XA_PropertyType;

typedef union {
    long        integer;      
    const char *string;
} XA_PropertyValue;

typedef void (*XA_PropertyListenerCallback)(void *listener,
                                            const char *name, 
                                            XA_PropertyValue *value);

typedef struct XA_Properties XA_Properties;

typedef struct {
    const char      *name;
    XA_PropertyType  type;
    XA_PropertyValue value;
} XA_Property;

typedef struct {
    unsigned int nb_properties;
    XA_Property *properties;
} XA_PropertyList;

struct XA_PropertyNode;
typedef struct XA_PropertyNode *XA_PropertyHandle;

/*----------------------------------------------------------------------
|       prototypes
+---------------------------------------------------------------------*/
#ifdef __cplusplus
extern "C" {
#endif

int XA_EXPORT properties_new(XA_Properties **properties);
int XA_EXPORT properties_delete(XA_Properties *properties);
int XA_EXPORT properties_reset(XA_Properties *properties);
int XA_EXPORT properties_set_integer(XA_Properties *properties,
                                     const char *name, long value);
int XA_EXPORT properties_get_integer(const XA_Properties *properties,
                                     const char *name, long *value);
int XA_EXPORT properties_set_string(XA_Properties *properties,
                                    const char *name, const char *value);
int XA_EXPORT properties_get_string(const XA_Properties *properties,
                                    const char *name, char *value);
int XA_EXPORT properties_unset(XA_Properties *properties, const char *name);
int XA_EXPORT properties_add_listener(XA_Properties *properties,
                                      const char *name, void *listener,
                                      XA_PropertyListenerCallback callback,
                                      XA_PropertyHandle *handle);
int XA_EXPORT properties_remove_listener(XA_Properties *properties,
                                         XA_PropertyHandle handle);
int XA_EXPORT properties_get_list(XA_Properties *properties, 
                                  XA_PropertyList *list);
int XA_EXPORT property_list_get_marshaled_size(const XA_PropertyList *list);
int XA_EXPORT property_list_marshal(const XA_PropertyList *list, 
                                    unsigned char *buffer);
int XA_EXPORT property_list_unmarshal(XA_PropertyList *list,
                                      const unsigned char *buffer,
                                      unsigned char *workspace);

#ifdef __cplusplus
}
#endif

#endif /* _PROPERTIES_H_ */




