// license:GPLv3+

#pragma once

// Object to expose global methods and properties to table scripts
class ScriptGlobalTable : 
   public CComObjectRootEx<CComSingleThreadModel>, 
   public IDispatchImpl<ITableGlobal, &IID_ITableGlobal, &LIBID_VPinballLib>, 
   public IScriptable
{
public:
#ifdef __STANDALONE__
   STDMETHOD(GetIDsOfNames)(REFIID /*riid*/, LPOLESTR* rgszNames, UINT cNames, LCID lcid,DISPID* rgDispId);
   STDMETHOD(Invoke)(DISPID dispIdMember, REFIID /*riid*/, LCID lcid, WORD wFlags, DISPPARAMS* pDispParams, VARIANT* pVarResult, EXCEPINFO* pExcepInfo, UINT* puArgErr);
   STDMETHOD(GetDocumentation)(INT index, BSTR *pBstrName, BSTR *pBstrDocString, DWORD *pdwHelpContext, BSTR *pBstrHelpFile);
#endif
   // Headers to support communication between the game and the script.
   STDMETHOD(EndModal)();
   STDMETHOD(BeginModal)();
   STDMETHOD(GetTextFile)(BSTR FileName, /*[out, retval]*/ BSTR *pContents);
   STDMETHOD(GetCustomParam)(/*[in]*/ LONG index, /*[out, retval]*/ BSTR *param);
   STDMETHOD(get_Setting)(BSTR Section, BSTR SettingName, /*[out, retval]*/ BSTR *param);
   STDMETHOD(get_FrameIndex)(/*[out, retval]*/ LONG *pVal);
   STDMETHOD(get_GameTime)(/*[out, retval]*/ LONG *pVal);
   STDMETHOD(get_PreciseGameTime)(/*[out, retval]*/ double *pVal);
   STDMETHOD(get_SystemTime)(/*[out, retval]*/ LONG *pVal);

   STDMETHOD(get_RightFlipperKey)(/*[out, retval]*/ LONG *pVal);
   STDMETHOD(get_LeftFlipperKey)(/*[out, retval]*/ LONG *pVal);
   STDMETHOD(get_StagedRightFlipperKey)(/*[out, retval]*/ LONG *pVal);
   STDMETHOD(get_StagedLeftFlipperKey)(/*[out, retval]*/ LONG *pVal);
   STDMETHOD(get_LeftMagnaSave)(/*[out, retval]*/ LONG *pVal);
   STDMETHOD(get_RightMagnaSave)(/*[out, retval]*/ LONG *pVal);
   STDMETHOD(get_PlungerKey)(/*[out, retval]*/ LONG *pVal);
   STDMETHOD(get_LeftTiltKey)(/*[out, retval]*/ LONG *pVal);
   STDMETHOD(get_RightTiltKey)(/*[out, retval]*/ LONG *pVal);
   STDMETHOD(get_CenterTiltKey)(/*[out, retval]*/ LONG *pVal);
   STDMETHOD(get_AddCreditKey)(/*[out, retval]*/ LONG *pVal);
   STDMETHOD(get_AddCreditKey2)(/*[out, retval]*/ LONG *pVal);
   STDMETHOD(get_StartGameKey)(/*[out, retval]*/ LONG *pVal);
   STDMETHOD(get_MechanicalTilt)(/*[out, retval]*/ LONG *pVal);
   STDMETHOD(get_ExitGame)(/*[out, retval]*/ LONG *pVal);
   STDMETHOD(get_LockbarKey)(/*[out, retval]*/ LONG *pVal);
   STDMETHOD(get_JoyCustomKey)(/*[in]*/ LONG index, /*[out, retval]*/ LONG *pVal);
   STDMETHOD(get_VPXActionKey)(/*[in]*/ LONG index, /*[out, retval]*/ LONG *pVal);

   STDMETHOD(get_ActiveBall)(/*[out, retval]*/ IBall **pVal);
   STDMETHOD(LoadValue)(BSTR TableName, BSTR ValueName, /*[out, retval]*/ VARIANT *Value);
   STDMETHOD(SaveValue)(BSTR TableName, BSTR ValueName, VARIANT Value);
   STDMETHOD(AddObject)(BSTR Name, IDispatch *pdisp);
#ifdef _WIN64
   STDMETHOD(get_GetPlayerHWnd)(/*[out, retval]*/ SIZE_T *pVal);
#else
   STDMETHOD(get_GetPlayerHWnd)(/*[out, retval]*/ LONG *pVal);
#endif
   STDMETHOD(get_UserDirectory)(/*[out, retval]*/ BSTR *pVal);
   STDMETHOD(get_TablesDirectory)(/*[out, retval]*/ BSTR *pVal);
   STDMETHOD(get_MusicDirectory)(/*[optional][in]*/ VARIANT pSubDir, /*[out, retval]*/ BSTR *pVal);
   STDMETHOD(get_ScriptsDirectory)(/*[out, retval]*/ BSTR *pVal);
   STDMETHOD(get_PlatformOS)(/*[out, retval]*/ BSTR *pVal);
   STDMETHOD(get_PlatformCPU)(/*[out, retval]*/ BSTR *pVal);
   STDMETHOD(get_PlatformBits)(/*[out, retval]*/ BSTR *pVal);
   STDMETHOD(put_ShowCursor)(/*[in]*/ VARIANT_BOOL enable);
   STDMETHOD(PlayMusic)(BSTR str, float volume);
   STDMETHOD(put_MusicVolume)(float volume);
   STDMETHOD(EndMusic)();

   STDMETHOD(put_DMDWidth)(/*[in]*/ int pVal);
   STDMETHOD(put_DMDHeight)(/*[in]*/ int pVal);
   STDMETHOD(put_DMDPixels)(/*[in]*/ VARIANT pVal);
   STDMETHOD(put_DMDColoredPixels)(/*[in]*/ VARIANT pVal);

   STDMETHOD(get_DisableStaticPrerendering)(/*[out, retval]*/ VARIANT_BOOL *pVal);
   STDMETHOD(put_DisableStaticPrerendering)(/*[in]*/ VARIANT_BOOL newVal);

   STDMETHOD(get_WindowWidth)(/*[out, retval]*/ int *pVal);
   STDMETHOD(get_WindowHeight)(/*[out, retval]*/ int *pVal);

   STDMETHOD(get_NightDay)(/*[out, retval]*/ int *pVal);
   //STDMETHOD(put_NightDay)(/*[in]*/ int newVal);

   STDMETHOD(get_ShowDT)(/*[out, retval]*/ VARIANT_BOOL *pVal);

   STDMETHOD(get_ShowFSS)(/*[out, retval]*/ VARIANT_BOOL *pVal);

   STDMETHOD(PlaySound)(BSTR sound, LONG LoopCount, float volume, float pan, float randompitch, LONG pitch, VARIANT_BOOL usesame, VARIANT_BOOL restart, float front_rear_fade);
   STDMETHOD(StopSound)(BSTR sound);

   STDMETHOD(FireKnocker)(/*[in]*/ int Count);
   STDMETHOD(QuitPlayer)(/*[in]*/ int CloseType);

   STDMETHOD(Nudge)(float Angle, float Force);
   STDMETHOD(NudgeGetCalibration)(VARIANT *XMax, VARIANT *YMax, VARIANT *XGain, VARIANT *YGain, VARIANT *DeadZone, VARIANT *TiltSensitivity);
   STDMETHOD(NudgeSetCalibration)(int XMax, int YMax, int XGain, int YGain, int DeadZone, int TiltSensitivity);
   STDMETHOD(NudgeSensorStatus)(VARIANT *XNudge, VARIANT *YNudge);
   STDMETHOD(NudgeTiltStatus)(VARIANT *XPlumb, VARIANT *YPlumb, VARIANT *Tilt);

   const WCHAR *get_Name() const final;
   STDMETHOD(get_Name)(BSTR *pVal);

   STDMETHOD(GetBalls)(/*[out, retval]*/ LPSAFEARRAY *pVal);
   STDMETHOD(GetElements)(/*[out, retval]*/ LPSAFEARRAY *pVal);
   STDMETHOD(GetElementByName)(/*[in]*/ BSTR name, /*[out, retval]*/ IDispatch **pVal);
   STDMETHOD(get_ActiveTable)(/*[out, retval]*/ ITable **pVal);

   STDMETHOD(get_Version)(/*[out, retval]*/ int *pVal);
   STDMETHOD(get_VPBuildVersion)(/*[out, retval]*/ double *pVal);
   STDMETHOD(get_VersionMajor)(/*[out, retval]*/ int *pVal);
   STDMETHOD(get_VersionMinor)(/*[out, retval]*/ int *pVal);
   STDMETHOD(get_VersionRevision)(/*[out, retval]*/ int *pVal);

   STDMETHOD(GetSerialDevices)(VARIANT *pVal);
   STDMETHOD(OpenSerial)(/*[in]*/ BSTR device);
   STDMETHOD(CloseSerial)();
   STDMETHOD(FlushSerial)();
   STDMETHOD(SetupSerial)(int baud, int bits, int parity, int stopbit, VARIANT_BOOL rts, VARIANT_BOOL dtr);
   STDMETHOD(ReadSerial)(int size, VARIANT *pVal);
   STDMETHOD(WriteSerial)(VARIANT pVal);

   STDMETHOD(get_RenderingMode)(/*[out, retval]*/ int *pVal);

   STDMETHOD(UpdateMaterial)
   (BSTR pVal, float wrapLighting, float roughness, float glossyImageLerp, float thickness, float edge, float edgeAlpha, float opacity, OLE_COLOR base, OLE_COLOR glossy, OLE_COLOR clearcoat,
      VARIANT_BOOL isMetal, VARIANT_BOOL opacityActive, float elasticity, float elasticityFalloff, float friction, float scatterAngle);
   STDMETHOD(GetMaterial)
   (BSTR pVal, VARIANT *wrapLighting, VARIANT *roughness, VARIANT *glossyImageLerp, VARIANT *thickness, VARIANT *edge, VARIANT *edgeAlpha, VARIANT *opacity, VARIANT *base, VARIANT *glossy,
      VARIANT *clearcoat, VARIANT *isMetal, VARIANT *opacityActive, VARIANT *elasticity, VARIANT *elasticityFalloff, VARIANT *friction, VARIANT *scatterAngle);
   STDMETHOD(UpdateMaterialPhysics)(BSTR pVal, float elasticity, float elasticityFalloff, float friction, float scatterAngle);
   STDMETHOD(GetMaterialPhysics)(BSTR pVal, VARIANT *elasticity, VARIANT *elasticityFalloff, VARIANT *friction, VARIANT *scatterAngle);
   STDMETHOD(MaterialColor)(BSTR pVal, OLE_COLOR newVal);

   STDMETHOD(LoadTexture)(BSTR imageName, BSTR fileName);

   STDMETHOD(CreatePluginObject)(/*[in]*/ BSTR classId, /*[out, retval]*/ IDispatch **pVal);

   void Init(VPinball *vpinball, PinTable *pt);
   ~ScriptGlobalTable();

   IDispatch *GetDispatch() final { return (IDispatch *)this; }
   const IDispatch *GetDispatch() const final { return (const IDispatch *)this; }

   ISelect *GetISelect() final { return nullptr; }
   const ISelect *GetISelect() const final { return nullptr; }

   BEGIN_COM_MAP(ScriptGlobalTable)
   COM_INTERFACE_ENTRY(ITableGlobal)
   COM_INTERFACE_ENTRY(IDispatch)
   END_COM_MAP()

private:
   bool GetTextFileFromDirectory(const string& filename, const string& dirname, BSTR *pContents);

   PinTable *m_pt = nullptr;
   VPinball *m_vpinball = nullptr;
};
