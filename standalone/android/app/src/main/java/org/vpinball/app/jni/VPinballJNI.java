package org.vpinball.app.jni;

public class VPinballJNI {
    public native String VPinballGetVersionStringFull();
    public native void VPinballInit(VPinballEventCallback callback);
    public native void VPinballLog(int level, String message);
    public native void VPinballResetLog();
    public native int VPinballLoadValueInt(int section, String key, int defaultValue);
    public native float VPinballLoadValueFloat(int section, String key, float defaultValue);
    public native String VPinballLoadValueString(int section, String key, String defaultValue);
    public native void VPinballSaveValueInt(int section, String key, int value);
    public native void VPinballSaveValueFloat(int section, String key, float value);
    public native void VPinballSaveValueString(int section, String key, String value);
    public native int VPinballUncompress(String source);
    public native int VPinballCompress(String source, String destination);
    public native void VPinballUpdateWebServer();
    public native int VPinballResetIni();
    public native int VPinballLoad(String source);
    public native int VPinballExtractScript(String source);
    public native int VPinballPlay();
    public native void VPinballStop();
    public native void VPinballToggleFPS();
    public native void VPinballSetPlayState(int enable);
    public native int VPinballGetCustomTableOptionsCount();
    public native VPinballCustomTableOption VPinballGetCustomTableOption(int index);
    public native void VPinballSetCustomTableOption(VPinballCustomTableOption customTableOption);
    public native void VPinballResetCustomTableOptions();
    public native void VPinballSaveCustomTableOptions();
    public native VPinballTableOptions VPinballGetTableOptions();
    public native void VPinballSetTableOptions(VPinballTableOptions tableOptions);
    public native void VPinballResetTableOptions();
    public native void VPinballSaveTableOptions();
    public native VPinballViewSetup VPinballGetViewSetup();
    public native void VPinballSetViewSetup(VPinballViewSetup viewSetup);
    public native void VPinballSetDefaultViewSetup();
    public native void VPinballResetViewSetup();
    public native void VPinballSaveViewSetup();
}