package org.vpinball.app.jni;

public class VPinballJNI {
    public native String VPinballGetVersionStringFull();
    public native void VPinballInit(VPinballEventCallback callback);
    public native void VPinballLog(int level, String message);
    public native void VPinballResetLog();
    public native int VPinballLoadValueInt(String sectionName, String key, int defaultValue);
    public native float VPinballLoadValueFloat(String sectionName, String key, float defaultValue);
    public native String VPinballLoadValueString(String sectionName, String key, String defaultValue);
    public native void VPinballSaveValueInt(String sectionName, String key, int value);
    public native void VPinballSaveValueFloat(String sectionName, String key, float value);
    public native void VPinballSaveValueString(String sectionName, String key, String value);
    public native int VPinballResetIni();
    public native void VPinballUpdateWebServer();
    public native int VPinballLoadTable(String path);
    public native int VPinballExtractTableScript();
    public native int VPinballPlay();
    public native void VPinballStop();
}