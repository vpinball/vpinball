package org.vpinball.app.jni;

public class VPinballJNI {
    public native String VPinballGetVersionStringFull();
    public native void VPinballInit(VPinballEventCallback callback);
    public native void VPinballLog(int level, String message);
    public native void VPinballResetLog();
    public native int VPinballLoadValueInt(String sectionName, String key, int defaultValue);
    public native float VPinballLoadValueFloat(String sectionName, String key, float defaultValue);
    public native String VPinballLoadValueString(String sectionName, String key, String defaultValue);
    public native boolean VPinballLoadValueBool(String sectionName, String key, boolean defaultValue);
    public native void VPinballSaveValueInt(String sectionName, String key, int value);
    public native void VPinballSaveValueFloat(String sectionName, String key, float value);
    public native void VPinballSaveValueString(String sectionName, String key, String value);
    public native void VPinballSaveValueBool(String sectionName, String key, boolean value);
    public native int VPinballResetIni();
    public native void VPinballUpdateWebServer();
    public native String VPinballGetPath(int pathType);
    public native int VPinballLoadTable(String path);
    public native int VPinballExtractTableScript();
    public native int VPinballPlay();
    public native void VPinballStop();
    public native boolean VPinballInitOpenXR(Object activity);
    public native int VPinballZipCreate(String sourcePath, String destPath, VPinballZipCallback callback);
    public native int VPinballZipExtract(String sourcePath, String destPath, VPinballZipCallback callback);
}