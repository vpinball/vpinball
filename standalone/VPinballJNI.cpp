#include "core/stdafx.h"

#include "VPinball.h"

#include <SDL3/SDL_system.h>
#include <jni.h>

static jobject gJNICallbackObject = nullptr;
static jmethodID gJNIOnEventMethod = nullptr;

static jclass gJNIProgressDataClass = nullptr;
static jclass gJNIScriptErrorDataClass = nullptr;
static jclass gJNIRumbleDataClass = nullptr;
static jclass gJNIWebServerDataClass = nullptr;
static jclass gJNITableOptionsClass = nullptr;
static jclass gJNICustomTableOptionClass = nullptr;
static jclass gJNIViewSetupClass = nullptr;

static jclass gJNIScriptErrorTypeClass = nullptr;
static jclass gJNISettingsSectionClass = nullptr;
static jclass gJNIOptionUnitClass = nullptr;
static jclass gJNIToneMapperClass = nullptr;
static jclass gJNIViewLayoutClass = nullptr;

void* VPinballJNI_OnEventCallback(VPINBALL_EVENT event, void* data)
{
   if (!gJNICallbackObject || !gJNIOnEventMethod)
      return nullptr;

   jobject eventDataObject = nullptr;

   JNIEnv* env = (JNIEnv*)SDL_GetAndroidJNIEnv();

   switch(event) {
      case VPINBALL_EVENT_ARCHIVE_UNCOMPRESSING:
      case VPINBALL_EVENT_ARCHIVE_COMPRESSING:
      case VPINBALL_EVENT_LOADING_ITEMS:
      case VPINBALL_EVENT_LOADING_IMAGES:
      case VPINBALL_EVENT_LOADING_SOUNDS:
      case VPINBALL_EVENT_LOADING_FONTS:
      case VPINBALL_EVENT_LOADING_COLLECTIONS:
      case VPINBALL_EVENT_PRERENDERING:
      {
         if (gJNIProgressDataClass) {
            jmethodID constructorMethod = env->GetMethodID(gJNIProgressDataClass, "<init>", "(I)V");
            if (constructorMethod) {
               VPinballProgressData* pData = (VPinballProgressData*)(data);
               if (pData)
                  eventDataObject = env->NewObject(gJNIProgressDataClass, constructorMethod, pData->progress);
            }
         }
         break;
      }
      case VPINBALL_EVENT_SCRIPT_ERROR:
      {
         jmethodID constructorMethod = env->GetMethodID(gJNIScriptErrorDataClass, "<init>", "(Lorg/vpinball/app/jni/VPinballScriptErrorType;IILjava/lang/String;)V");
         jmethodID fromIntMethod = env->GetStaticMethodID(gJNIScriptErrorTypeClass, "fromInt", "(I)Lorg/vpinball/app/jni/VPinballScriptErrorType;");
         if (constructorMethod && fromIntMethod) {
            VPinballScriptErrorData* pData = (VPinballScriptErrorData*)(data);
            if (pData) {
               jobject errorType = env->CallStaticObjectMethod(gJNIScriptErrorTypeClass, fromIntMethod, (jint)pData->error);
               jstring descriptionString = env->NewStringUTF(pData->pDescription ? pData->pDescription : "");
               jobject scriptErrorObject = env->NewObject(gJNIScriptErrorDataClass, constructorMethod, errorType, pData->line, pData->position, descriptionString);
               env->DeleteLocalRef(descriptionString);
               env->DeleteLocalRef(errorType);
               eventDataObject = scriptErrorObject;
            }
         }
         break;
      }
      case VPINBALL_EVENT_RUMBLE:
      {
          if (gJNIRumbleDataClass) {
            jmethodID constructorMethod = env->GetMethodID(gJNIRumbleDataClass, "<init>", "(III)V");
            if (constructorMethod) {
               VPinballRumbleData* pData = (VPinballRumbleData*)(data);
               if (pData) {
                  eventDataObject = env->NewObject(gJNIRumbleDataClass, constructorMethod,
                     (jint)pData->lowFrequencyRumble,
                     (jint)pData->highFrequencyRumble,
                     (jint)pData->durationMs);
               }
            }
         }
         break;
      }
      case VPINBALL_EVENT_WEB_SERVER:
      {
          if (gJNIWebServerDataClass) {
            jmethodID constructorMethod = env->GetMethodID(gJNIWebServerDataClass, "<init>", "(Ljava/lang/String;)V");
            if (constructorMethod) {
               VPinballWebServerData* pData = (VPinballWebServerData*)(data);
               if (pData) {
                  jstring urlString = env->NewStringUTF(pData->pUrl ? pData->pUrl : "");
                  eventDataObject = env->NewObject(gJNIWebServerDataClass, constructorMethod, urlString);
                  env->DeleteLocalRef(urlString);
               }
            }
         }
         break;
      }
      default:
         break;
   }

   jobject result = env->CallObjectMethod(gJNICallbackObject, gJNIOnEventMethod, (jint)event, eventDataObject);

   if (env->ExceptionCheck()) {
      env->ExceptionClear();
      return nullptr;
   }

   void* nativeResult = nullptr;
   if (result) {
      nativeResult = env->GetDirectBufferAddress(result);
      env->DeleteLocalRef(result);
   }

   if (eventDataObject) {
      env->DeleteLocalRef(eventDataObject);
   }

   return nativeResult;
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
   gJNIOnEventMethod = env->GetMethodID(env->GetObjectClass(gJNICallbackObject), "onEvent", "(ILjava/lang/Object;)Ljava/lang/Object;");

   gJNIProgressDataClass = (jclass)env->NewGlobalRef(env->FindClass("org/vpinball/app/jni/VPinballProgressData"));
   gJNIScriptErrorDataClass = (jclass)env->NewGlobalRef(env->FindClass("org/vpinball/app/jni/VPinballScriptErrorData"));
   gJNIRumbleDataClass = (jclass)env->NewGlobalRef(env->FindClass("org/vpinball/app/jni/VPinballRumbleData"));
   gJNIWebServerDataClass = (jclass)env->NewGlobalRef(env->FindClass("org/vpinball/app/jni/VPinballWebServerData"));
   gJNITableOptionsClass = (jclass)env->NewGlobalRef(env->FindClass("org/vpinball/app/jni/VPinballTableOptions"));
   gJNICustomTableOptionClass = (jclass)env->NewGlobalRef(env->FindClass("org/vpinball/app/jni/VPinballCustomTableOption"));
   gJNIViewSetupClass = (jclass)env->NewGlobalRef(env->FindClass("org/vpinball/app/jni/VPinballViewSetup"));

   gJNIScriptErrorTypeClass = (jclass)env->NewGlobalRef(env->FindClass("org/vpinball/app/jni/VPinballScriptErrorType"));
   gJNISettingsSectionClass = (jclass)env->NewGlobalRef(env->FindClass("org/vpinball/app/jni/VPinballSettingsSection"));
   gJNIOptionUnitClass = (jclass)env->NewGlobalRef(env->FindClass("org/vpinball/app/jni/VPinballOptionUnit"));
   gJNIToneMapperClass = (jclass)env->NewGlobalRef(env->FindClass("org/vpinball/app/jni/VPinballToneMapper"));
   gJNIViewLayoutClass = (jclass)env->NewGlobalRef(env->FindClass("org/vpinball/app/jni/VPinballViewLayoutMode"));
 
   VPinballInit(VPinballJNI_OnEventCallback);
}

JNIEXPORT void JNICALL Java_org_vpinball_app_jni_VPinballJNI_VPinballLog(JNIEnv* env, jobject obj, jint level, jstring message)
{
   const char* msg = env->GetStringUTFChars(message, nullptr);
   VPinballLog(static_cast<VPINBALL_LOG_LEVEL>(level), msg);
   env->ReleaseStringUTFChars(message, msg);
}

JNIEXPORT void JNICALL Java_org_vpinball_app_jni_VPinballJNI_VPinballResetLog(JNIEnv* env, jobject obj)
{
   VPinballResetLog();
}

JNIEXPORT jint JNICALL Java_org_vpinball_app_jni_VPinballJNI_VPinballLoadValueInt(JNIEnv* env, jobject obj, jint section, jstring key, jint defaultValue)
{
   const char* k = env->GetStringUTFChars(key, nullptr);
   int result = VPinballLoadValueInt(static_cast<VPINBALL_SETTINGS_SECTION>(section), k, defaultValue);
   env->ReleaseStringUTFChars(key, k);
   return result;
}

JNIEXPORT jfloat JNICALL Java_org_vpinball_app_jni_VPinballJNI_VPinballLoadValueFloat(JNIEnv* env, jobject obj, jint section, jstring key, jfloat defaultValue)
{
   const char* k = env->GetStringUTFChars(key, nullptr);
   float result = VPinballLoadValueFloat(static_cast<VPINBALL_SETTINGS_SECTION>(section), k, defaultValue);
   env->ReleaseStringUTFChars(key, k);
   return result;
}

JNIEXPORT jstring JNICALL Java_org_vpinball_app_jni_VPinballJNI_VPinballLoadValueString(JNIEnv* env, jobject obj, jint section, jstring key, jstring defaultValue)
{
   const char* k = env->GetStringUTFChars(key, nullptr);
   const char* def = env->GetStringUTFChars(defaultValue, nullptr);
   const char* result = VPinballLoadValueString(static_cast<VPINBALL_SETTINGS_SECTION>(section), k, def);
   env->ReleaseStringUTFChars(key, k);
   env->ReleaseStringUTFChars(defaultValue, def);
   return env->NewStringUTF(result);
}

JNIEXPORT void JNICALL Java_org_vpinball_app_jni_VPinballJNI_VPinballSaveValueInt(JNIEnv* env, jobject obj, jint section, jstring key, jint value)
{
   const char* k = env->GetStringUTFChars(key, nullptr);
   VPinballSaveValueInt(static_cast<VPINBALL_SETTINGS_SECTION>(section), k, value);
   env->ReleaseStringUTFChars(key, k);
}

JNIEXPORT void JNICALL Java_org_vpinball_app_jni_VPinballJNI_VPinballSaveValueFloat(JNIEnv* env, jobject obj, jint section, jstring key, jfloat value)
{
   const char* k = env->GetStringUTFChars(key, nullptr);
   VPinballSaveValueFloat(static_cast<VPINBALL_SETTINGS_SECTION>(section), k, value);
   env->ReleaseStringUTFChars(key, k);
}

JNIEXPORT void JNICALL Java_org_vpinball_app_jni_VPinballJNI_VPinballSaveValueString(JNIEnv* env, jobject obj, jint section, jstring key, jstring value)
{
   const char* k = env->GetStringUTFChars(key, nullptr);
   const char* v = env->GetStringUTFChars(value, nullptr);
   VPinballSaveValueString(static_cast<VPINBALL_SETTINGS_SECTION>(section), k, v);
   env->ReleaseStringUTFChars(key, k);
   env->ReleaseStringUTFChars(value, v);
}

JNIEXPORT jint JNICALL Java_org_vpinball_app_jni_VPinballJNI_VPinballUncompress(JNIEnv* env, jobject obj, jstring source)
{
   const char* src = env->GetStringUTFChars(source, nullptr);
   VPINBALL_STATUS status = VPinballUncompress(src);
   env->ReleaseStringUTFChars(source, src);
   return status;
}

JNIEXPORT jint JNICALL Java_org_vpinball_app_jni_VPinballJNI_VPinballCompress(JNIEnv* env, jobject obj, jstring source, jstring destination)
{
   const char* src = env->GetStringUTFChars(source, nullptr);
   const char* dest = env->GetStringUTFChars(destination, nullptr);
   VPINBALL_STATUS status = VPinballCompress(src, dest);
   env->ReleaseStringUTFChars(source, src);
   env->ReleaseStringUTFChars(destination, dest);
   return status;
}

JNIEXPORT void JNICALL Java_org_vpinball_app_jni_VPinballJNI_VPinballUpdateWebServer(JNIEnv* env, jobject obj)
{
   VPinballUpdateWebServer();
}

JNIEXPORT jint JNICALL Java_org_vpinball_app_jni_VPinballJNI_VPinballResetIni(JNIEnv* env, jobject obj)
{
   return VPinballResetIni();
}

JNIEXPORT jint JNICALL Java_org_vpinball_app_jni_VPinballJNI_VPinballLoad(JNIEnv* env, jobject obj, jstring source)
{
   const char* src = env->GetStringUTFChars(source, nullptr);
   VPINBALL_STATUS status = VPinballLoad(src);
   env->ReleaseStringUTFChars(source, src);
   return status;
}

JNIEXPORT jint JNICALL Java_org_vpinball_app_jni_VPinballJNI_VPinballExtractScript(JNIEnv* env, jobject obj, jstring source)
{
   const char* src = env->GetStringUTFChars(source, nullptr);
   VPINBALL_STATUS status = VPinballExtractScript(src);
   env->ReleaseStringUTFChars(source, src);
   return status;
}

JNIEXPORT jint JNICALL Java_org_vpinball_app_jni_VPinballJNI_VPinballPlay(JNIEnv* env, jobject obj)
{
   return VPinballPlay();
}

JNIEXPORT jint JNICALL Java_org_vpinball_app_jni_VPinballJNI_VPinballStop(JNIEnv* env, jobject obj)
{
   return VPinballStop();
}

JNIEXPORT void JNICALL Java_org_vpinball_app_jni_VPinballJNI_VPinballSetPlayState(JNIEnv* env, jobject obj, jint enable)
{
   VPinballSetPlayState(enable);
}

JNIEXPORT void JNICALL Java_org_vpinball_app_jni_VPinballJNI_VPinballToggleFPS(JNIEnv* env, jobject obj)
{
   VPinballToggleFPS();
}

JNIEXPORT jobject JNICALL Java_org_vpinball_app_jni_VPinballJNI_VPinballGetTableOptions(JNIEnv* env, jobject obj)
{
   if (!gJNITableOptionsClass || !gJNIToneMapperClass)
      return nullptr;

   VPinballTableOptions tableOptions = {};
   VPinballGetTableOptions(&tableOptions);

   jmethodID constructor = env->GetMethodID(gJNITableOptionsClass, "<init>", "()V");
   jobject setupObject = env->NewObject(gJNITableOptionsClass, constructor);
   if (!constructor || !setupObject)
      return nullptr;

   env->SetFloatField(setupObject, env->GetFieldID(gJNITableOptionsClass, "globalEmissionScale", "F"), tableOptions.globalEmissionScale);
   env->SetFloatField(setupObject, env->GetFieldID(gJNITableOptionsClass, "globalDifficulty", "F"), tableOptions.globalDifficulty);
   env->SetFloatField(setupObject, env->GetFieldID(gJNITableOptionsClass, "exposure", "F"), tableOptions.exposure);

   jfieldID toneMapperField = env->GetFieldID(gJNITableOptionsClass, "toneMapper", "Lorg/vpinball/app/jni/VPinballToneMapper;");
   jmethodID toneMapperFromIntMethod = env->GetStaticMethodID(gJNIToneMapperClass, "fromInt", "(I)Lorg/vpinball/app/jni/VPinballToneMapper;");
   if (toneMapperField && toneMapperFromIntMethod) {
      jobject toneMapperObject = env->CallStaticObjectMethod(gJNIToneMapperClass, toneMapperFromIntMethod, tableOptions.toneMapper);
      if (toneMapperObject)
         env->SetObjectField(setupObject, toneMapperField, toneMapperObject);
   }

   env->SetIntField(setupObject, env->GetFieldID(gJNITableOptionsClass, "musicVolume", "I"), tableOptions.musicVolume);
   env->SetIntField(setupObject, env->GetFieldID(gJNITableOptionsClass, "soundVolume", "I"), tableOptions.soundVolume);

   return setupObject;
}

JNIEXPORT void JNICALL Java_org_vpinball_app_jni_VPinballJNI_VPinballSetTableOptions(JNIEnv* env, jobject obj, jobject setup) {
   if (!setup || !gJNITableOptionsClass || !gJNIToneMapperClass)
      return;

   VPinballTableOptions tableOptions = {};

   tableOptions.globalEmissionScale = env->GetFloatField(setup, env->GetFieldID(gJNITableOptionsClass, "globalEmissionScale", "F"));
   tableOptions.globalDifficulty = env->GetFloatField(setup, env->GetFieldID(gJNITableOptionsClass, "globalDifficulty", "F"));
   tableOptions.exposure = env->GetFloatField(setup, env->GetFieldID(gJNITableOptionsClass, "exposure", "F"));

   jfieldID toneMapperField = env->GetFieldID(gJNITableOptionsClass, "toneMapper", "Lorg/vpinball/app/jni/VPinballToneMapper;");
   jmethodID toneMapperGetValueMethod = env->GetMethodID(gJNIToneMapperClass, "getValue", "()I");
   if (toneMapperField && toneMapperGetValueMethod) {
      jobject toneMapperObject = env->GetObjectField(setup, toneMapperField);
      if (toneMapperObject)
         tableOptions.toneMapper = env->CallIntMethod(toneMapperObject, toneMapperGetValueMethod);
   }

   tableOptions.musicVolume = env->GetIntField(setup, env->GetFieldID(gJNITableOptionsClass, "musicVolume", "I"));
   tableOptions.soundVolume = env->GetIntField(setup, env->GetFieldID(gJNITableOptionsClass, "soundVolume", "I"));

   VPinballSetTableOptions(&tableOptions);
}

JNIEXPORT void JNICALL Java_org_vpinball_app_jni_VPinballJNI_VPinballResetTableOptions(JNIEnv* env, jobject obj)
{
   VPinballResetTableOptions();
}

JNIEXPORT void JNICALL Java_org_vpinball_app_jni_VPinballJNI_VPinballSaveTableOptions(JNIEnv* env, jobject obj)
{
   VPinballSaveTableOptions();
}

JNIEXPORT jint JNICALL Java_org_vpinball_app_jni_VPinballJNI_VPinballGetCustomTableOptionsCount(JNIEnv* env, jobject obj)
{
   return VPinballGetCustomTableOptionsCount();
}

JNIEXPORT jobject JNICALL Java_org_vpinball_app_jni_VPinballJNI_VPinballGetCustomTableOption(JNIEnv* env, jobject obj, jint index)
{
   if (!gJNICustomTableOptionClass || !gJNISettingsSectionClass || !gJNIOptionUnitClass)
      return nullptr;

   VPinballCustomTableOption customTableOption = {};
   VPinballGetCustomTableOption(index, &customTableOption);

   jmethodID constructor = env->GetMethodID(gJNICustomTableOptionClass, "<init>", "()V");
   jobject setupObject = env->NewObject(gJNICustomTableOptionClass, constructor);
   if (!constructor || !setupObject)
      return nullptr;

   jfieldID sectionField = env->GetFieldID(gJNICustomTableOptionClass, "section", "Lorg/vpinball/app/jni/VPinballSettingsSection;");
   jmethodID settingsSectionFromIntMethod = env->GetStaticMethodID(gJNISettingsSectionClass, "fromInt", "(I)Lorg/vpinball/app/jni/VPinballSettingsSection;");
   if (sectionField && settingsSectionFromIntMethod) {
      jobject settingsSectionObject = env->CallStaticObjectMethod(gJNISettingsSectionClass, settingsSectionFromIntMethod, (jint)customTableOption.section);
      if (settingsSectionObject)
         env->SetObjectField(setupObject, sectionField, settingsSectionObject);
   }

   env->SetObjectField(setupObject, env->GetFieldID(gJNICustomTableOptionClass, "id", "Ljava/lang/String;"), env->NewStringUTF(customTableOption.id ? customTableOption.id : ""));
   env->SetObjectField(setupObject, env->GetFieldID(gJNICustomTableOptionClass, "name", "Ljava/lang/String;"), env->NewStringUTF(customTableOption.name ? customTableOption.name : ""));
   env->SetIntField(setupObject, env->GetFieldID(gJNICustomTableOptionClass, "showMask", "I"), customTableOption.showMask);
   env->SetFloatField(setupObject, env->GetFieldID(gJNICustomTableOptionClass, "minValue", "F"), customTableOption.minValue);
   env->SetFloatField(setupObject, env->GetFieldID(gJNICustomTableOptionClass, "maxValue", "F"), customTableOption.maxValue);
   env->SetFloatField(setupObject, env->GetFieldID(gJNICustomTableOptionClass, "step", "F"), customTableOption.step);
   env->SetFloatField(setupObject, env->GetFieldID(gJNICustomTableOptionClass, "defaultValue", "F"), customTableOption.defaultValue);

   jfieldID unitField = env->GetFieldID(gJNICustomTableOptionClass, "unit", "Lorg/vpinball/app/jni/VPinballOptionUnit;");
   jmethodID optionUnitFromIntMethod = env->GetStaticMethodID(gJNIOptionUnitClass, "fromInt", "(I)Lorg/vpinball/app/jni/VPinballOptionUnit;");
   if (unitField && optionUnitFromIntMethod) {
      jobject optionUnitObject = env->CallStaticObjectMethod(gJNIOptionUnitClass, optionUnitFromIntMethod, (jint)customTableOption.unit);
      if (optionUnitObject)
         env->SetObjectField(setupObject, unitField, optionUnitObject);
   }

   env->SetObjectField(setupObject, env->GetFieldID(gJNICustomTableOptionClass, "literals", "Ljava/lang/String;"), env->NewStringUTF(customTableOption.literals ? customTableOption.literals : ""));
   env->SetFloatField(setupObject, env->GetFieldID(gJNICustomTableOptionClass, "value", "F"), customTableOption.value);

   return setupObject;
}

JNIEXPORT void JNICALL Java_org_vpinball_app_jni_VPinballJNI_VPinballSetCustomTableOption(JNIEnv* env, jobject obj, jobject setup)
{
   if (!setup || !gJNICustomTableOptionClass)
      return;

   VPinballCustomTableOption customTableOption = {};

   jstring idString = (jstring)env->GetObjectField(setup, env->GetFieldID(gJNICustomTableOptionClass, "id", "Ljava/lang/String;"));
   const char* id = env->GetStringUTFChars(idString, nullptr);
   customTableOption.id = id;

   customTableOption.value = env->GetFloatField(setup, env->GetFieldID(gJNICustomTableOptionClass, "value", "F"));

   VPinballSetCustomTableOption(&customTableOption);

   env->ReleaseStringUTFChars(idString, id);
}

JNIEXPORT void JNICALL Java_org_vpinball_app_jni_VPinballJNI_VPinballResetCustomTableOptions(JNIEnv* env, jobject obj)
{
   VPinballResetCustomTableOptions();
}

JNIEXPORT void JNICALL Java_org_vpinball_app_jni_VPinballJNI_VPinballSaveCustomTableOptions(JNIEnv* env, jobject obj)
{
   VPinballSaveCustomTableOptions();
}

JNIEXPORT jobject JNICALL Java_org_vpinball_app_jni_VPinballJNI_VPinballGetViewSetup(JNIEnv* env, jobject obj) {
   if (!gJNIViewSetupClass || !gJNIViewLayoutClass)
      return nullptr;

   VPinballViewSetup viewSetup = {};
   VPinballGetViewSetup(&viewSetup);

   jmethodID constructor = env->GetMethodID(gJNIViewSetupClass, "<init>", "()V");
   jobject setupObject = env->NewObject(gJNIViewSetupClass, constructor);
   if (!constructor || !setupObject)
      return nullptr;

   jfieldID viewModeField = env->GetFieldID(gJNIViewSetupClass, "viewMode", "Lorg/vpinball/app/jni/VPinballViewLayoutMode;");
   jmethodID viewModeFromIntMethod = env->GetStaticMethodID(gJNIViewLayoutClass, "fromInt", "(I)Lorg/vpinball/app/jni/VPinballViewLayoutMode;");
   if (viewModeField && viewModeFromIntMethod) {
      jobject viewModeObject = env->CallStaticObjectMethod(gJNIViewLayoutClass, viewModeFromIntMethod, viewSetup.viewMode);
      if (viewModeObject)
         env->SetObjectField(setupObject, viewModeField, viewModeObject);
   }

   env->SetFloatField(setupObject, env->GetFieldID(gJNIViewSetupClass, "sceneScaleX", "F"), viewSetup.sceneScaleX);
   env->SetFloatField(setupObject, env->GetFieldID(gJNIViewSetupClass, "sceneScaleY", "F"), viewSetup.sceneScaleY);
   env->SetFloatField(setupObject, env->GetFieldID(gJNIViewSetupClass, "sceneScaleZ", "F"), viewSetup.sceneScaleZ);
   env->SetFloatField(setupObject, env->GetFieldID(gJNIViewSetupClass, "viewX", "F"), viewSetup.viewX);
   env->SetFloatField(setupObject, env->GetFieldID(gJNIViewSetupClass, "viewY", "F"), viewSetup.viewY);
   env->SetFloatField(setupObject, env->GetFieldID(gJNIViewSetupClass, "viewZ", "F"), viewSetup.viewZ);
   env->SetFloatField(setupObject, env->GetFieldID(gJNIViewSetupClass, "lookAt", "F"), viewSetup.lookAt);
   env->SetFloatField(setupObject, env->GetFieldID(gJNIViewSetupClass, "viewportRotation", "F"), viewSetup.viewportRotation);
   env->SetFloatField(setupObject, env->GetFieldID(gJNIViewSetupClass, "fov", "F"), viewSetup.fov);
   env->SetFloatField(setupObject, env->GetFieldID(gJNIViewSetupClass, "layback", "F"), viewSetup.layback);
   env->SetFloatField(setupObject, env->GetFieldID(gJNIViewSetupClass, "viewHOfs", "F"), viewSetup.viewHOfs);
   env->SetFloatField(setupObject, env->GetFieldID(gJNIViewSetupClass, "viewVOfs", "F"), viewSetup.viewVOfs);
   env->SetFloatField(setupObject, env->GetFieldID(gJNIViewSetupClass, "windowTopZOfs", "F"), viewSetup.windowTopZOfs);
   env->SetFloatField(setupObject, env->GetFieldID(gJNIViewSetupClass, "windowBottomZOfs", "F"), viewSetup.windowBottomZOfs);

   return setupObject;
}

JNIEXPORT void JNICALL Java_org_vpinball_app_jni_VPinballJNI_VPinballSetViewSetup(JNIEnv* env, jobject obj, jobject setup) {
   if (!setup || !gJNIViewSetupClass || !gJNIViewLayoutClass)
      return;

   VPinballViewSetup viewSetup = {};

   jfieldID viewModeField = env->GetFieldID(gJNIViewSetupClass, "viewMode", "Lorg/vpinball/app/jni/VPinballViewLayoutMode;");
   jmethodID viewModeGetValueMethod = env->GetMethodID(gJNIViewLayoutClass, "getValue", "()I");
   if (viewModeField && viewModeGetValueMethod) {
      jobject viewModeObject = env->GetObjectField(setup, viewModeField);
      if (viewModeObject)
         viewSetup.viewMode = env->CallIntMethod(viewModeObject, viewModeGetValueMethod);
   }

   viewSetup.sceneScaleX = env->GetFloatField(setup, env->GetFieldID(gJNIViewSetupClass, "sceneScaleX", "F"));
   viewSetup.sceneScaleY = env->GetFloatField(setup, env->GetFieldID(gJNIViewSetupClass, "sceneScaleY", "F"));
   viewSetup.sceneScaleZ = env->GetFloatField(setup, env->GetFieldID(gJNIViewSetupClass, "sceneScaleZ", "F"));
   viewSetup.viewX = env->GetFloatField(setup, env->GetFieldID(gJNIViewSetupClass, "viewX", "F"));
   viewSetup.viewY = env->GetFloatField(setup, env->GetFieldID(gJNIViewSetupClass, "viewY", "F"));
   viewSetup.viewZ = env->GetFloatField(setup, env->GetFieldID(gJNIViewSetupClass, "viewZ", "F"));
   viewSetup.lookAt = env->GetFloatField(setup, env->GetFieldID(gJNIViewSetupClass, "lookAt", "F"));
   viewSetup.viewportRotation = env->GetFloatField(setup, env->GetFieldID(gJNIViewSetupClass, "viewportRotation", "F"));
   viewSetup.fov = env->GetFloatField(setup, env->GetFieldID(gJNIViewSetupClass, "fov", "F"));
   viewSetup.layback = env->GetFloatField(setup, env->GetFieldID(gJNIViewSetupClass, "layback", "F"));
   viewSetup.viewHOfs = env->GetFloatField(setup, env->GetFieldID(gJNIViewSetupClass, "viewHOfs", "F"));
   viewSetup.viewVOfs = env->GetFloatField(setup, env->GetFieldID(gJNIViewSetupClass, "viewVOfs", "F"));
   viewSetup.windowTopZOfs = env->GetFloatField(setup, env->GetFieldID(gJNIViewSetupClass, "windowTopZOfs", "F"));
   viewSetup.windowBottomZOfs = env->GetFloatField(setup, env->GetFieldID(gJNIViewSetupClass, "windowBottomZOfs", "F"));

   VPinballSetViewSetup(&viewSetup);
}

JNIEXPORT void JNICALL Java_org_vpinball_app_jni_VPinballJNI_VPinballSetDefaultViewSetup(JNIEnv* env, jobject obj)
{
   VPinballSetDefaultViewSetup();
}

JNIEXPORT void JNICALL Java_org_vpinball_app_jni_VPinballJNI_VPinballResetViewSetup(JNIEnv* env, jobject obj)
{
   VPinballResetViewSetup();
}

JNIEXPORT void JNICALL Java_org_vpinball_app_jni_VPinballJNI_VPinballSaveViewSetup(JNIEnv* env, jobject obj)
{
   VPinballSaveViewSetup();
}

}