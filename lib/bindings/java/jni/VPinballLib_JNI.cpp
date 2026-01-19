// license:GPLv3+

#include "core/stdafx.h"

#include "../../../include/vpinball/VPinballLib_C.h"
#include "../../../src/VPinballLib.h"

#include <SDL3/SDL_system.h>
#include <jni.h>

#ifdef ENABLE_XR
#define XR_USE_PLATFORM_ANDROID
#include <openxr/openxr.h>
#include <openxr/openxr_platform.h>
#endif

using namespace VPinballLib;

static jobject gJNICallbackObject = nullptr;
static jmethodID gJNIOnEventMethod = nullptr;

void VPinballJNI_OnEventCallback(VPINBALL_EVENT event, const char* jsonData)
{
   if (!gJNICallbackObject || !gJNIOnEventMethod)
      return;

   JNIEnv* env = (JNIEnv*)SDL_GetAndroidJNIEnv();

   jstring jsonDataString = jsonData ? env->NewStringUTF(jsonData) : nullptr;
   env->CallVoidMethod(gJNICallbackObject, gJNIOnEventMethod, (jint)event, jsonDataString);

   if (jsonDataString)
      env->DeleteLocalRef(jsonDataString);

   if (env->ExceptionCheck())
      env->ExceptionClear();
}

extern "C" {

JNIEXPORT jstring JNICALL Java_org_vpinball_app_jni_VPinballJNI_VPinballGetVersionStringFull(JNIEnv* env, jobject obj)
{
   return env->NewStringUTF(VPinballGetVersionStringFull());
}

JNIEXPORT void JNICALL Java_org_vpinball_app_jni_VPinballJNI_VPinballInit(JNIEnv* env, jobject obj, jobject callback)
{
   if (!callback)
      return;

   gJNICallbackObject = env->NewGlobalRef(callback);
   gJNIOnEventMethod = env->GetMethodID(env->GetObjectClass(gJNICallbackObject), "onEvent", "(ILjava/lang/String;)V");

   VPinballInit(VPinballJNI_OnEventCallback);
}

JNIEXPORT void JNICALL Java_org_vpinball_app_jni_VPinballJNI_VPinballLog(JNIEnv* env, jobject obj, jint level, jstring message)
{
   const char* pMessage = env->GetStringUTFChars(message, nullptr);
   VPinballLog(static_cast<VPINBALL_LOG_LEVEL>(level), pMessage);
   env->ReleaseStringUTFChars(message, pMessage);
}

JNIEXPORT void JNICALL Java_org_vpinball_app_jni_VPinballJNI_VPinballResetLog(JNIEnv* env, jobject obj)
{
   VPinballResetLog();
}

JNIEXPORT jint JNICALL Java_org_vpinball_app_jni_VPinballJNI_VPinballLoadValueInt(JNIEnv* env, jobject obj, jstring sectionName, jstring key, jint defaultValue)
{
   const char* pSectionName = env->GetStringUTFChars(sectionName, nullptr);
   const char* pKey = env->GetStringUTFChars(key, nullptr);
   int result = VPinballLoadValueInt(pSectionName, pKey, defaultValue);
   env->ReleaseStringUTFChars(key, pKey);
   env->ReleaseStringUTFChars(sectionName, pSectionName);
   return result;
}

JNIEXPORT jfloat JNICALL Java_org_vpinball_app_jni_VPinballJNI_VPinballLoadValueFloat(JNIEnv* env, jobject obj, jstring sectionName, jstring key, jfloat defaultValue)
{
   const char* pSectionName = env->GetStringUTFChars(sectionName, nullptr);
   const char* pKey = env->GetStringUTFChars(key, nullptr);
   float result = VPinballLoadValueFloat(pSectionName, pKey, defaultValue);
   env->ReleaseStringUTFChars(key, pKey);
   env->ReleaseStringUTFChars(sectionName, pSectionName);
   return result;
}

JNIEXPORT jstring JNICALL Java_org_vpinball_app_jni_VPinballJNI_VPinballLoadValueString(JNIEnv* env, jobject obj, jstring sectionName, jstring key, jstring defaultValue)
{
   const char* pSectionName = env->GetStringUTFChars(sectionName, nullptr);
   const char* pKey = env->GetStringUTFChars(key, nullptr);
   const char* pDefaultValue = env->GetStringUTFChars(defaultValue, nullptr);
   const char* pResult = VPinballLoadValueString(pSectionName, pKey, pDefaultValue);
   env->ReleaseStringUTFChars(defaultValue, pDefaultValue);
   env->ReleaseStringUTFChars(key, pKey);
   env->ReleaseStringUTFChars(sectionName, pSectionName);
   return env->NewStringUTF(pResult);
}

JNIEXPORT jboolean JNICALL Java_org_vpinball_app_jni_VPinballJNI_VPinballLoadValueBool(JNIEnv* env, jobject obj, jstring sectionName, jstring key, jboolean defaultValue)
{
   const char* pSectionName = env->GetStringUTFChars(sectionName, nullptr);
   const char* pKey = env->GetStringUTFChars(key, nullptr);
   int result = VPinballLoadValueBool(pSectionName, pKey, defaultValue ? 1 : 0);
   env->ReleaseStringUTFChars(key, pKey);
   env->ReleaseStringUTFChars(sectionName, pSectionName);
   return result != 0;
}

JNIEXPORT void JNICALL Java_org_vpinball_app_jni_VPinballJNI_VPinballSaveValueInt(JNIEnv* env, jobject obj, jstring sectionName, jstring key, jint value)
{
   const char* pSectionName = env->GetStringUTFChars(sectionName, nullptr);
   const char* pKey = env->GetStringUTFChars(key, nullptr);
   VPinballSaveValueInt(pSectionName, pKey, value);
   env->ReleaseStringUTFChars(key, pKey);
   env->ReleaseStringUTFChars(sectionName, pSectionName);
}

JNIEXPORT void JNICALL Java_org_vpinball_app_jni_VPinballJNI_VPinballSaveValueFloat(JNIEnv* env, jobject obj, jstring sectionName, jstring key, jfloat value)
{
   const char* pSectionName = env->GetStringUTFChars(sectionName, nullptr);
   const char* pKey = env->GetStringUTFChars(key, nullptr);
   VPinballSaveValueFloat(pSectionName, pKey, value);
   env->ReleaseStringUTFChars(key, pKey);
   env->ReleaseStringUTFChars(sectionName, pSectionName);
}

JNIEXPORT void JNICALL Java_org_vpinball_app_jni_VPinballJNI_VPinballSaveValueString(JNIEnv* env, jobject obj, jstring sectionName, jstring key, jstring value)
{
   const char* pSectionName = env->GetStringUTFChars(sectionName, nullptr);
   const char* pKey = env->GetStringUTFChars(key, nullptr);
   const char* pValue = env->GetStringUTFChars(value, nullptr);
   VPinballSaveValueString(pSectionName, pKey, pValue);
   env->ReleaseStringUTFChars(value, pValue);
   env->ReleaseStringUTFChars(key, pKey);
   env->ReleaseStringUTFChars(sectionName, pSectionName);
}

JNIEXPORT void JNICALL Java_org_vpinball_app_jni_VPinballJNI_VPinballSaveValueBool(JNIEnv* env, jobject obj, jstring sectionName, jstring key, jboolean value)
{
   const char* pSectionName = env->GetStringUTFChars(sectionName, nullptr);
   const char* pKey = env->GetStringUTFChars(key, nullptr);
   VPinballSaveValueBool(pSectionName, pKey, value ? 1 : 0);
   env->ReleaseStringUTFChars(key, pKey);
   env->ReleaseStringUTFChars(sectionName, pSectionName);
}

JNIEXPORT jint JNICALL Java_org_vpinball_app_jni_VPinballJNI_VPinballResetIni(JNIEnv* env, jobject obj)
{
   return VPinballResetIni();
}

JNIEXPORT void JNICALL Java_org_vpinball_app_jni_VPinballJNI_VPinballUpdateWebServer(JNIEnv* env, jobject obj)
{
   VPinballUpdateWebServer();
}

JNIEXPORT jstring JNICALL Java_org_vpinball_app_jni_VPinballJNI_VPinballGetPath(JNIEnv* env, jobject obj, jint pathType)
{
   return env->NewStringUTF(VPinballGetPath(static_cast<VPINBALL_PATH>(pathType)));
}

JNIEXPORT jint JNICALL Java_org_vpinball_app_jni_VPinballJNI_VPinballLoadTable(JNIEnv* env, jobject obj, jstring path)
{
   const char* pPath = env->GetStringUTFChars(path, nullptr);
   VPINBALL_STATUS status = VPinballLoadTable(pPath);
   env->ReleaseStringUTFChars(path, pPath);
   return status;
}

JNIEXPORT jint JNICALL Java_org_vpinball_app_jni_VPinballJNI_VPinballExtractTableScript(JNIEnv* env, jobject obj)
{
   return VPinballExtractTableScript();
}

JNIEXPORT jint JNICALL Java_org_vpinball_app_jni_VPinballJNI_VPinballPlay(JNIEnv* env, jobject obj)
{
   return VPinballPlay();
}

JNIEXPORT jint JNICALL Java_org_vpinball_app_jni_VPinballJNI_VPinballStop(JNIEnv* env, jobject obj)
{
   return VPinballStop();
}

struct ZipCallbackContext {
   JNIEnv* env;
   jobject callback;
   jmethodID onProgressMethod;
};

static ZipCallbackContext* g_zipCallbackContext = nullptr;

static void ZipProgressCallbackHandler(int current, int total, const char* filename)
{
   if (!g_zipCallbackContext || !g_zipCallbackContext->callback)
      return;

   JNIEnv* env = g_zipCallbackContext->env;
   jstring filenameStr = filename ? env->NewStringUTF(filename) : nullptr;
   env->CallVoidMethod(g_zipCallbackContext->callback, g_zipCallbackContext->onProgressMethod, current, total, filenameStr);

   if (filenameStr)
      env->DeleteLocalRef(filenameStr);
}

JNIEXPORT jint JNICALL Java_org_vpinball_app_jni_VPinballJNI_VPinballZipCreate(JNIEnv* env, jobject obj, jstring sourcePath, jstring destPath, jobject callback)
{
   const char* pSourcePath = env->GetStringUTFChars(sourcePath, nullptr);
   const char* pDestPath = env->GetStringUTFChars(destPath, nullptr);

   ZipCallbackContext context = { env, nullptr, nullptr };
   if (callback) {
      context.callback = callback;
      jclass callbackClass = env->GetObjectClass(callback);
      context.onProgressMethod = env->GetMethodID(callbackClass, "onProgress", "(IILjava/lang/String;)V");
      g_zipCallbackContext = &context;
   }

   VPINBALL_STATUS status = VPinballZipCreate(pSourcePath, pDestPath, callback ? ZipProgressCallbackHandler : nullptr);

   g_zipCallbackContext = nullptr;
   env->ReleaseStringUTFChars(destPath, pDestPath);
   env->ReleaseStringUTFChars(sourcePath, pSourcePath);

   return status;
}

JNIEXPORT jint JNICALL Java_org_vpinball_app_jni_VPinballJNI_VPinballZipExtract(JNIEnv* env, jobject obj, jstring sourcePath, jstring destPath, jobject callback)
{
   const char* pSourcePath = env->GetStringUTFChars(sourcePath, nullptr);
   const char* pDestPath = env->GetStringUTFChars(destPath, nullptr);

   ZipCallbackContext context = { env, nullptr, nullptr };
   if (callback) {
      context.callback = callback;
      jclass callbackClass = env->GetObjectClass(callback);
      context.onProgressMethod = env->GetMethodID(callbackClass, "onProgress", "(IILjava/lang/String;)V");
      g_zipCallbackContext = &context;
   }

   VPINBALL_STATUS status = VPinballZipExtract(pSourcePath, pDestPath, callback ? ZipProgressCallbackHandler : nullptr);

   g_zipCallbackContext = nullptr;
   env->ReleaseStringUTFChars(destPath, pDestPath);
   env->ReleaseStringUTFChars(sourcePath, pSourcePath);

   return status;
}

#ifdef ENABLE_XR
JNIEXPORT jboolean JNICALL Java_org_vpinball_app_jni_VPinballJNI_VPinballInitOpenXR(JNIEnv* env, jobject obj, jobject activity)
{
   JavaVM* vm;
   env->GetJavaVM(&vm);

   jobject globalActivity = env->NewGlobalRef(activity);

   XrLoaderInitInfoAndroidKHR loaderInitInfo = {XR_TYPE_LOADER_INIT_INFO_ANDROID_KHR};
   loaderInitInfo.applicationVM = vm;
   loaderInitInfo.applicationContext = globalActivity;

   PFN_xrInitializeLoaderKHR xrInitializeLoaderKHR;
   xrGetInstanceProcAddr(XR_NULL_HANDLE, "xrInitializeLoaderKHR", (PFN_xrVoidFunction*)&xrInitializeLoaderKHR);

   if (xrInitializeLoaderKHR != nullptr)
   {
      XrResult result = xrInitializeLoaderKHR((const XrLoaderInitInfoBaseHeaderKHR*)&loaderInitInfo);
      return result == XR_SUCCESS;
   }
   return false;
}
#endif

}
