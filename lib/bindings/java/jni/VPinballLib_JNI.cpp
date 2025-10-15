#include "core/stdafx.h"

#include "../../../include/vpinball/VPinballLib_C.h"
#include "../../../src/VPinballLib.h"

#include <SDL3/SDL_system.h>
#include <jni.h>

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

JNIEXPORT jint JNICALL Java_org_vpinball_app_jni_VPinballJNI_VPinballResetIni(JNIEnv* env, jobject obj)
{
   return VPinballResetIni();
}

JNIEXPORT void JNICALL Java_org_vpinball_app_jni_VPinballJNI_VPinballUpdateWebServer(JNIEnv* env, jobject obj)
{
   VPinballUpdateWebServer();
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

}
