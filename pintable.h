// PinTable.h: interface for the PinTable class.
//
//////////////////////////////////////////////////////////////////////
#pragma once
#if !defined(AFX_PINTABLE_H__D14A2DAB_2984_4FE7_A102_D0283ECE31B4__INCLUDED_)
#define AFX_PINTABLE_H__D14A2DAB_2984_4FE7_A102_D0283ECE31B4__INCLUDED_

#include <unordered_map>
#include <atomic>
#include "hash.h"
#include "SearchSelectDialog.h"

#define VIEW_PLAYFIELD 1
#define VIEW_BACKGLASS 2

#define MIN_ZOOM 0.126f // purposely make them offset from powers to 2 to account for roundoff error
#define MAX_ZOOM 63.9f

#define DISABLE_SCRIPT_EDITING  0x00000002 // cannot open script windows (stops editing and viewing)
#define DISABLE_EVERYTHING      0x80000000 // everything is off limits (including future locks)

#define MAX_LAYERS              11

struct LightSource {
   COLORREF emission;
   Vertex3Ds pos;
};

struct ProtectionData {
   long				fileversion;
   long				size;
   unsigned char	paraphrase[16 + 8];
   unsigned long	flags;
   int				keyversion;
   int				spare1;
   int				spare2;
};

class ScriptGlobalTable;

class PinTableMDI : public CMDIChild
{
public:
    PinTableMDI(VPinball *vpinball);
    virtual ~PinTableMDI();
    CComObject<PinTable> *GetTable()
    {
        return m_table;
    }
    bool CanClose() const;

protected:
    virtual void PreCreate(CREATESTRUCT &cs);
    virtual int  OnCreate(CREATESTRUCT &cs);
    virtual void OnClose();
    virtual LRESULT OnMDIActivate(UINT msg, WPARAM wparam, LPARAM lparam);

private:
    CComObject<PinTable> *m_table;
    VPinball             *m_vpinball;
    CMenu                 m_menu;
};

class ProgressDialog : public CDialog
{
public:
    ProgressDialog();
    void SetProgress(const int value)
    {
        m_progressBar.SetPos(value);
    }

    void SetName(const std::string& text)
    {
        m_progressName.SetWindowText(text.c_str());
    }

protected:
    virtual BOOL OnInitDialog();

private:
    CProgressBar m_progressBar;
    CStatic      m_progressName;
};

class PinTable:
   public CWnd,
   public CComObjectRootEx<CComSingleThreadModel>,
   public IDispatchImpl<ITable, &IID_ITable, &LIBID_VPinballLib>,
   public IConnectionPointContainerImpl<PinTable>,
   public EventProxy<PinTable, &DIID_ITableEvents>,
   // IProvideClassInfo provides an ITypeInfo for the whole coclass
   // allowing VBScript to get the set of events to sync to.
   // VBA does not need this interface for some reason
   public IProvideClassInfo2Impl<&CLSID_Table, &DIID_ITableEvents, &LIBID_VPinballLib>,
   public ISelect,
   public IScriptable,
   public IScriptableHost,
   public IEditable,
   public IPerPropertyBrowsing	// Ability to fill in dropdown in property browser
{
public:
   STDMETHOD(get_BallFrontDecal)(/*[out, retval]*/ BSTR *pVal);
   STDMETHOD(put_BallFrontDecal)(/*[in]*/ BSTR newVal);
   STDMETHOD(get_YieldTime)(/*[out, retval]*/ long *pVal);
   STDMETHOD(put_YieldTime)(/*[in]*/ long newVal);
   STDMETHOD(get_BallImage)(/*[out, retval]*/ BSTR *pVal);
   STDMETHOD(put_BallImage)(/*[in]*/ BSTR newVal);

   STDMETHOD(get_Gravity)(/*[out, retval]*/ float *pVal);
   STDMETHOD(put_Gravity)(/*[in]*/ float newVal);
   STDMETHOD(get_Friction)(/*[out, retval]*/ float *pVal);
   STDMETHOD(put_Friction)(/*[in]*/ float newVal);
   STDMETHOD(get_Elasticity)(/*[out, retval]*/ float *pVal);
   STDMETHOD(put_Elasticity)(/*[in]*/ float newVal);
   STDMETHOD(get_ElasticityFalloff)(/*[out, retval]*/ float *pVal);
   STDMETHOD(put_ElasticityFalloff)(/*[in]*/ float newVal);
   STDMETHOD(get_Scatter)(/*[out, retval]*/ float *pVal);
   STDMETHOD(put_Scatter)(/*[in]*/ float newVal);
   STDMETHOD(get_DefaultScatter)(/*[out, retval]*/ float *pVal);
   STDMETHOD(put_DefaultScatter)(/*[in]*/ float newVal);
   STDMETHOD(get_NudgeTime)(/*[out, retval]*/ float *pVal);
   STDMETHOD(put_NudgeTime)(/*[in]*/ float newVal);
   STDMETHOD(get_PlungerNormalize)(/*[out, retval]*/ int *pVal);
   STDMETHOD(put_PlungerNormalize)(/*[in]*/ int newVal);

   STDMETHOD(get_PlungerFilter)(/*[out, retval]*/ VARIANT_BOOL *pVal);
   STDMETHOD(put_PlungerFilter)(/*[in]*/ VARIANT_BOOL newVal);

   STDMETHOD(get_PhysicsLoopTime)(/*[out, retval]*/ int *pVal);
   STDMETHOD(put_PhysicsLoopTime)(/*[in]*/ int newVal);

   STDMETHOD(get_FieldOfView)(/*[out, retval]*/ float *pVal);
   STDMETHOD(put_FieldOfView)(/*[in]*/ float newVal);
   STDMETHOD(get_Inclination)(/*[out, retval]*/ float *pVal);
   STDMETHOD(put_Inclination)(/*[in]*/ float newVal);
   STDMETHOD(get_Layback)(/*[out, retval]*/ float *pVal);
   STDMETHOD(put_Layback)(/*[in]*/ float newVal);
   STDMETHOD(get_Rotation)(/*[out, retval]*/ float *pVal);
   STDMETHOD(put_Rotation)(/*[in]*/ float newVal);
   STDMETHOD(get_Scalex)(/*[out, retval]*/ float *pVal);
   STDMETHOD(put_Scalex)(/*[in]*/ float newVal);
   STDMETHOD(get_Scaley)(/*[out, retval]*/ float *pVal);
   STDMETHOD(put_Scaley)(/*[in]*/ float newVal);
   STDMETHOD(get_Scalez)(/*[out, retval]*/ float *pVal);
   STDMETHOD(put_Scalez)(/*[in]*/ float newVal);
   STDMETHOD(get_Xlatex)(/*[out, retval]*/ float *pVal);
   STDMETHOD(put_Xlatex)(/*[in]*/ float newVal);
   STDMETHOD(get_Xlatey)(/*[out, retval]*/ float *pVal);
   STDMETHOD(put_Xlatey)(/*[in]*/ float newVal);
   STDMETHOD(get_Xlatez)(/*[out, retval]*/ float *pVal);
   STDMETHOD(put_Xlatez)(/*[in]*/ float newVal);

   STDMETHOD(get_SlopeMax)(/*[out, retval]*/ float *pVal);
   STDMETHOD(put_SlopeMax)(/*[in]*/ float newVal);
   STDMETHOD(get_SlopeMin)(/*[out, retval]*/ float *pVal);
   STDMETHOD(put_SlopeMin)(/*[in]*/ float newVal);
   STDMETHOD(get_ColorGradeImage)(/*[out, retval]*/ BSTR *pVal);
   STDMETHOD(put_ColorGradeImage)(/*[in]*/ BSTR newVal);
   STDMETHOD(get_BackdropImage_DT)(/*[out, retval]*/ BSTR *pVal);
   STDMETHOD(put_BackdropImage_DT)(/*[in]*/ BSTR newVal);
   STDMETHOD(get_BackdropImage_FS)(/*[out, retval]*/ BSTR *pVal);
   STDMETHOD(put_BackdropImage_FS)(/*[in]*/ BSTR newVal);
   STDMETHOD(get_BackdropImage_FSS)(/*[out, retval]*/ BSTR *pVal);
   STDMETHOD(put_BackdropImage_FSS)(/*[in]*/ BSTR newVal);
   STDMETHOD(get_BackdropImageApplyNightDay)(/*[out, retval]*/ VARIANT_BOOL *pVal);
   STDMETHOD(put_BackdropImageApplyNightDay)(/*[in]*/ VARIANT_BOOL newVal);
   STDMETHOD(get_BackdropColor)(/*[out, retval]*/ OLE_COLOR *pVal);
   STDMETHOD(put_BackdropColor)(/*[in]*/ OLE_COLOR newVal);
   STDMETHOD(get_PlayfieldMaterial)(/*[out, retval]*/ BSTR *pVal);
   STDMETHOD(put_PlayfieldMaterial)(/*[in]*/ BSTR newVal);
   STDMETHOD(get_BackglassMode)(/*[out, retval]*/ BackglassIndex *pVal);
   STDMETHOD(put_BackglassMode)(/*[in]*/ BackglassIndex newVal);

   STDMETHOD(get_LightAmbient)(/*[out, retval]*/ OLE_COLOR *pVal);
   STDMETHOD(put_LightAmbient)(/*[in]*/ OLE_COLOR newVal);
   STDMETHOD(get_Light0Emission)(/*[out, retval]*/ OLE_COLOR *pVal);
   STDMETHOD(put_Light0Emission)(/*[in]*/ OLE_COLOR newVal);
   STDMETHOD(get_LightHeight)(/*[out, retval]*/ float *pVal);
   STDMETHOD(put_LightHeight)(/*[in]*/ float newVal);
   STDMETHOD(get_LightRange)(/*[out, retval]*/ float *pVal);
   STDMETHOD(put_LightRange)(/*[in]*/ float newVal);
   STDMETHOD(get_EnvironmentEmissionScale)(/*[out, retval]*/ float *pVal);
   STDMETHOD(put_EnvironmentEmissionScale)(/*[in]*/ float newVal);
   STDMETHOD(get_LightEmissionScale)(/*[out, retval]*/ float *pVal);
   STDMETHOD(put_LightEmissionScale)(/*[in]*/ float newVal);
   STDMETHOD(get_NightDay)(/*[out, retval]*/ int *pVal);
   STDMETHOD(put_NightDay)(/*[in]*/ int newVal);
   STDMETHOD(get_AOScale)(/*[out, retval]*/ float *pVal);
   STDMETHOD(put_AOScale)(/*[in]*/ float newVal);
   STDMETHOD(get_SSRScale)(/*[out, retval]*/ float *pVal);
   STDMETHOD(put_SSRScale)(/*[in]*/ float newVal);
   STDMETHOD(get_BallReflection)(/*[out, retval]*/ UserDefaultOnOff *pVal);
   STDMETHOD(put_BallReflection)(/*[in]*/ UserDefaultOnOff newVal);
   STDMETHOD(get_PlayfieldReflectionStrength)(/*[out, retval]*/ int *pVal);
   STDMETHOD(put_PlayfieldReflectionStrength)(/*[in]*/ int newVal);
   STDMETHOD(get_BallTrail)(/*[out, retval]*/ UserDefaultOnOff *pVal);
   STDMETHOD(put_BallTrail)(/*[in]*/ UserDefaultOnOff newVal);
   STDMETHOD(get_TrailStrength)(/*[out, retval]*/ int *pVal);
   STDMETHOD(put_TrailStrength)(/*[in]*/ int newVal);
   STDMETHOD(get_BallPlayfieldReflectionScale)(/*[out, retval]*/ float *pVal);
   STDMETHOD(put_BallPlayfieldReflectionScale)(/*[in]*/ float newVal);
   STDMETHOD(get_DefaultBulbIntensityScale)(/*[out, retval]*/ float *pVal);
   STDMETHOD(put_DefaultBulbIntensityScale)(/*[in]*/ float newVal);

   STDMETHOD(get_Height)(/*[out, retval]*/ float *pVal);
   STDMETHOD(put_Height)(/*[in]*/ float newVal);
   STDMETHOD(get_Width)(/*[out, retval]*/ float *pVal);
   STDMETHOD(put_Width)(/*[in]*/ float newVal);
   STDMETHOD(get_GlassHeight)(/*[out, retval]*/ float *pVal);
   STDMETHOD(put_GlassHeight)(/*[in]*/ float newVal);
   STDMETHOD(get_TableHeight)(/*[out, retval]*/ float *pVal);
   STDMETHOD(put_TableHeight)(/*[in]*/ float newVal);
   STDMETHOD(get_DisplayBackdrop)(/*[out, retval]*/ VARIANT_BOOL *pVal);
   STDMETHOD(put_DisplayBackdrop)(/*[in]*/ VARIANT_BOOL newVal);
   STDMETHOD(get_DisplayGrid)(/*[out, retval]*/ VARIANT_BOOL *pVal);
   STDMETHOD(put_DisplayGrid)(/*[in]*/ VARIANT_BOOL newVal);
   STDMETHOD(get_Image)(/*[out, retval]*/ BSTR *pVal);
   STDMETHOD(put_Image)(/*[in]*/ BSTR newVal);

   STDMETHOD(PlaySound)(BSTR bstr, int loopcount, float volume, float pan, float randompitch, int pitch, VARIANT_BOOL usesame, VARIANT_BOOL restart, float front_rear_fade);
   STDMETHOD(FireKnocker)(/*[in]*/ int Count);
   STDMETHOD(QuitPlayer)(/*[in]*/ int CloseType);

   STDMETHOD(get_FileName)(/*[out, retval]*/ BSTR *pVal);

   STDMETHOD(get_Name)(/*[out, retval]*/ BSTR *pVal);
   STDMETHOD(put_Name)(/*[in]*/ BSTR newVal);
   STDMETHOD(get_EnableAntialiasing)(/*[out, retval]*/ UserDefaultOnOff *pVal);
   STDMETHOD(put_EnableAntialiasing)(/*[in]*/ UserDefaultOnOff newVal);
   STDMETHOD(get_EnableAO)(/*[out, retval]*/ UserDefaultOnOff *pVal);
   STDMETHOD(put_EnableAO)(/*[in]*/ UserDefaultOnOff newVal);
   STDMETHOD(get_EnableSSR)(/*[out, retval]*/ UserDefaultOnOff *pVal);
   STDMETHOD(put_EnableSSR)(/*[in]*/ UserDefaultOnOff newVal);
   STDMETHOD(get_EnableFXAA)(/*[out, retval]*/ FXAASettings *pVal);
   STDMETHOD(put_EnableFXAA)(/*[in]*/ FXAASettings newVal);

   STDMETHOD(get_BloomStrength)(/*[out, retval]*/ float *pVal);
   STDMETHOD(put_BloomStrength)(/*[in]*/ float newVal);

   STDMETHOD(get_OverridePhysics)(/*[out, retval]*/ PhysicsSet *pVal);
   STDMETHOD(put_OverridePhysics)(/*[in]*/ PhysicsSet newVal);
   STDMETHOD(get_OverridePhysicsFlippers)(/*[out, retval]*/ VARIANT_BOOL *pVal);
   STDMETHOD(put_OverridePhysicsFlippers)(/*[in]*/ VARIANT_BOOL newVal);

   STDMETHOD(ImportPhysics)();
   STDMETHOD(ExportPhysics)();

   STDMETHOD(get_MaxSeparation)(/*[out, retval]*/ float *pVal);
   STDMETHOD(put_MaxSeparation)(/*[in]*/ float newVal);
   STDMETHOD(get_ZPD)(/*[out, retval]*/ float *pVal);
   STDMETHOD(put_ZPD)(/*[in]*/ float newVal);
   STDMETHOD(get_Offset)(/*[out, retval]*/ float *pVal);
   STDMETHOD(put_Offset)(/*[in]*/ float newVal);

   STDMETHOD(get_EnableDecals)(/*[out, retval]*/ VARIANT_BOOL *pVal);
   STDMETHOD(put_EnableDecals)(/*[in]*/ VARIANT_BOOL newVal);
   STDMETHOD(get_EnableEMReels)(/*[out, retval]*/ VARIANT_BOOL *pVal);
   STDMETHOD(put_EnableEMReels)(/*[in]*/ VARIANT_BOOL newVal);
   STDMETHOD(get_ShowDT)(/*[out, retval]*/ VARIANT_BOOL *pVal);
   STDMETHOD(put_ShowDT)(/*[in]*/ VARIANT_BOOL newVal);
   STDMETHOD(get_ShowFSS)(/*[out, retval]*/ VARIANT_BOOL *pVal);
   STDMETHOD(put_ShowFSS)(/*[in]*/ VARIANT_BOOL newVal);
   STDMETHOD(get_ReflectElementsOnPlayfield)(/*[out, retval]*/ VARIANT_BOOL *pVal);
   STDMETHOD(put_ReflectElementsOnPlayfield)(/*[in]*/ VARIANT_BOOL newVal);

   STDMETHOD(get_GlobalDifficulty)(/*[out, retval]*/ float *pVal);
   STDMETHOD(put_GlobalDifficulty)(/*[in]*/ float newVal);

   STDMETHOD(get_Accelerometer)(/*[out, retval]*/ VARIANT_BOOL *pVal); //!! remove?!
   STDMETHOD(put_Accelerometer)(/*[in]*/ VARIANT_BOOL newVal);	 //!! remove?!
   STDMETHOD(get_AccelNormalMount)(/*[out, retval]*/ VARIANT_BOOL *pVal); //!! remove?!
   STDMETHOD(put_AccelNormalMount)(/*[in]*/ VARIANT_BOOL newVal); //!! remove?!
   STDMETHOD(get_AccelerometerAngle)(/*[out, retval]*/ float *pVal);  //!! remove?!
   STDMETHOD(put_AccelerometerAngle)(/*[in]*/ float newVal); //!! remove?!

   STDMETHOD(get_DeadZone)(/*[out, retval]*/  int *pVal); //!! remove?!
   STDMETHOD(put_DeadZone)(/*[in]*/ int newVal); //!! remove?!

#ifdef UNUSED_TILT
   STDMETHOD(get_JoltAmount)(/*[out, retval]*/  int *pVal);
   STDMETHOD(put_JoltAmount)(/*[in]*/ int newVal);
   STDMETHOD(get_TiltAmount)(/*[out, retval]*/ int *pVal);
   STDMETHOD(put_TiltAmount)(/*[in]*/ int newVal);
   STDMETHOD(get_JoltTriggerTime)(/*[out, retval]*/ int *pVal);
   STDMETHOD(put_JoltTriggerTime)(/*[in]*/ int newVal);
   STDMETHOD(get_TiltTriggerTime)(/*[out, retval]*/ int *pVal);
   STDMETHOD(put_TiltTriggerTime)(/*[in]*/ int newVal);
#endif

   STDMETHOD(get_TableSoundVolume)(/*[out, retval]*/ int *pVal);
   STDMETHOD(put_TableSoundVolume)(/*[in]*/ int newVal);
   STDMETHOD(get_TableMusicVolume)(/*[out, retval]*/ int *pVal);
   STDMETHOD(put_TableMusicVolume)(/*[in]*/ int newVal);

   STDMETHOD(get_TableAdaptiveVSync)(/*[out, retval]*/ int *pVal);
   STDMETHOD(put_TableAdaptiveVSync)(/*[in]*/ int newVal);

   STDMETHOD(get_DetailLevel)(/*[out, retval]*/ int *pVal);
   STDMETHOD(put_DetailLevel)(/*[in]*/ int newVal);

   STDMETHOD(get_GlobalAlphaAcc)(/*[out, retval]*/ VARIANT_BOOL *pVal);
   STDMETHOD(put_GlobalAlphaAcc)(/*[in]*/ VARIANT_BOOL newVal);

   STDMETHOD(get_GlobalDayNight)(/*[out, retval]*/ VARIANT_BOOL *pVal);
   STDMETHOD(put_GlobalDayNight)(/*[in]*/ VARIANT_BOOL newVal);

   STDMETHOD(get_GlobalStereo3D)(/*[out, retval]*/ VARIANT_BOOL *pVal);
   STDMETHOD(put_GlobalStereo3D)(/*[in]*/ VARIANT_BOOL newVal);

   STDMETHOD(get_BallDecalMode)(/*[out, retval]*/ VARIANT_BOOL *pVal);
   STDMETHOD(put_BallDecalMode)(/*[in]*/ VARIANT_BOOL newVal);

   STDMETHOD(get_EnvironmentImage)(/*[out, retval]*/ BSTR *pVal);
   STDMETHOD(put_EnvironmentImage)(/*[in]*/ BSTR newVal);

   STDMETHOD(get_Version)(/*[out, retval]*/ int *pVal);
   STDMETHOD(get_VPBuildVersion)(/*[out, retval]*/ int *pVal);
   STDMETHOD(get_VersionMajor)(/*[out, retval]*/ int *pVal);
   STDMETHOD(get_VersionMinor)(/*[out, retval]*/ int *pVal);
   STDMETHOD(get_VersionRevision)(/*[out, retval]*/ int *pVal);

   /////////////////////////////////////////////

   PinTable();
   virtual ~PinTable();

   void InitBuiltinTable(VPinball * const pvp, const bool useBlankTable = false);
   void InitPostLoad(VPinball *pvp);

   virtual HRESULT GetTypeName(BSTR *pVal);

   void SetCaption(const string& szCaption);
   void SetMouseCapture();
   int  ShowMessageBox(const char *text) const;
   POINT GetScreenPoint() const;

   HRESULT InitVBA();
   void CloseVBA();

   void UIRenderPass2(Sur * const psur);
   void Paint(HDC hdc);
   ISelect *HitTest(const int x, const int y);
   void SetDirtyDraw();

   void Render3DProjection(Sur * const psur);

   bool GetDecalsEnabled() const;
   bool GetEMReelsEnabled() const;

   void Copy(int x, int y);
   void Paste(const bool atLocation, const int x, const int y);

   void ExportBlueprint();
   void ExportTableMesh();
   void ImportBackdropPOV(const string& filename);
   void ExportBackdropPOV(const string& filename);
   void ImportVPP(const string& filename);

   //void FireVoidEvent(int dispid);
   void FireKeyEvent(int dispid, int keycode);

   // also creates Player instance
   void Play(const bool cameraMode);
   // called before Player instance gets deleted
   void StopPlaying();

   void ImportSound(const HWND hwndListView, const string& filename);
   void ReImportSound(const HWND hwndListView, PinSound * const pps, const string& filename);
   bool ExportSound(PinSound * const pps, const char * const filename);
   void ListSounds(HWND hwndListView);
   int AddListSound(HWND hwndListView, PinSound * const pps);
   void RemoveSound(PinSound * const pps);
   HRESULT SaveSoundToStream(PinSound * const pps, IStream *pstm);
   HRESULT LoadSoundFromStream(IStream *pstm, const int LoadFileVersion);
   bool ExportImage(Texture * const ppi, const char * const filename);
   void ImportImage(HWND hwndListView, const string& filename);
   void ReImportImage(Texture * const ppi, const string& filename);
   void ListImages(HWND hwndListView);
   int AddListImage(HWND hwndListView, Texture * const ppi);
   void RemoveImage(Texture * const ppi);
   HRESULT LoadImageFromStream(IStream *pstm, unsigned int idx, int version);
   Texture* GetImage(const std::string &szName) const;
   bool GetImageLink(Texture * const ppi) const;
   PinBinary *GetImageLinkBinary(const int id);

   void ListCustomInfo(HWND hwndListView);
   int AddListItem(HWND hwndListView, const string& szName, const string& szValue1, LPARAM lparam);

   void ImportFont(HWND hwndListView, const string& filename);
   void ListFonts(HWND hwndListView);
   int AddListBinary(HWND hwndListView, PinBinary *ppb);
   void RemoveFont(PinFont * const ppf);

   void NewCollection(const HWND hwndListView, const bool fFromSelection);
   void ListCollections(HWND hwndListView);
   int AddListCollection(HWND hwndListView, CComObject<Collection> *pcol);
   void RemoveCollection(CComObject<Collection> *pcol);
   void SetCollectionName(Collection *pcol, const char *szName, HWND hwndList, int index);

   void DoContextMenu(int x, int y, const int menuid, ISelect *psel);
   virtual void DoCommand(int icmd, int x, int y);
   bool FMutilSelLocked();

   virtual void SelectItem(IScriptable *piscript);
   virtual void DoCodeViewCommand(int command);
   virtual void SetDirtyScript(SaveDirtyState sds);
   virtual void ExportMesh(FILE *f);

   // Multi-object manipulation
   virtual Vertex2D GetCenter() const;
   virtual void PutCenter(const Vertex2D& pv);
   virtual void FlipY(const Vertex2D& pvCenter);
   virtual void FlipX(const Vertex2D& pvCenter);
   virtual void Rotate(const float ang, const Vertex2D& pvCenter, const bool useElementCenter);
   virtual void Scale(const float scalex, const float scaley, const Vertex2D& pvCenter, const bool useElementCenter);
   virtual void Translate(const Vertex2D &pvOffset);

   // IEditable (mostly bogus for now)
   virtual void UIRenderPass1(Sur * const psur);
   virtual ItemTypeEnum GetItemType() const { return eItemTable; }
   virtual HRESULT InitLoad(IStream *pstm, PinTable *ptable, int *pid, int version, HCRYPTHASH hcrypthash, HCRYPTKEY hcryptkey);
   virtual HRESULT InitPostLoad();
   virtual HRESULT InitVBA(BOOL fNew, int id, WCHAR * const wzName);
   virtual ISelect *GetISelect() { return (ISelect *)this; }
   virtual const ISelect *GetISelect() const { return (const ISelect *)this; }
   virtual void SetDefaults(bool fromMouseClick);
   virtual IScriptable *GetScriptable() { return (IScriptable *)this; }
   virtual void SetDefaultPhysics(bool fromMouseClick);

   virtual PinTable *GetPTable() { return this; }
   virtual const PinTable *GetPTable() const { return this; }
   const char *GetElementName(IEditable *pedit) const;

   IEditable *GetElementByName(const char * const name);
   void OnDelete();

   void DoLeftButtonDown(int x, int y, bool zoomIn = true);
   void OnLeftButtonUp(int x, int y);
   void OnRightButtonDown(int x, int y);
   void FillCollectionContextMenu(CMenu &mainMenu, CMenu &colSubMenu, ISelect *psel);
   void OnRightButtonUp(int x, int y);
   void DoMouseMove(int x, int y);
   void OnLeftDoubleClick(int x, int y);
   void UseTool(int x, int y, int tool);
   void OnKeyDown(int key);

   // Transform editor window coordinates to table coordinates
   Vertex2D TransformPoint(int x, int y) const;

   void ClearMultiSel(ISelect *newSel = NULL);
   bool MultiSelIsEmpty();
   ISelect *GetSelectedItem() const { return m_vmultisel.ElementAt(0); }
   void AddMultiSel(ISelect *psel, const bool add, const bool update, const bool contextClick);

   void BeginAutoSaveCounter();
   void EndAutoSaveCounter();
   void AutoSave();

   HRESULT TableSave();
   HRESULT SaveAs();
   virtual HRESULT ApcProject_Save();
   HRESULT Save(const bool saveAs);
   HRESULT SaveToStorage(IStorage *pstg);
   HRESULT SaveInfo(IStorage* pstg, HCRYPTHASH hcrypthash);
   HRESULT SaveCustomInfo(IStorage* pstg, IStream *pstmTags, HCRYPTHASH hcrypthash);
   HRESULT WriteInfoValue(IStorage* pstg, const WCHAR * const wzName, const string& szValue, HCRYPTHASH hcrypthash);
   HRESULT ReadInfoValue(IStorage* pstg, const WCHAR * const wzName, char **pszValue, HCRYPTHASH hcrypthash);
   HRESULT SaveData(IStream* pstm, HCRYPTHASH hcrypthash, const bool backupForPlay);
   HRESULT LoadGameFromFilename(const string& szFileName);
   HRESULT LoadGameFromStorage(IStorage *pstgRoot);
   HRESULT LoadInfo(IStorage* pstg, HCRYPTHASH hcrypthash, int version);
   HRESULT LoadCustomInfo(IStorage* pstg, IStream *pstmTags, HCRYPTHASH hcrypthash, int version);
   HRESULT LoadData(IStream* pstm, int& csubobj, int& csounds, int& ctextures, int& cfonts, int& ccollection, int version, HCRYPTHASH hcrypthash, HCRYPTKEY hcryptkey);
   void ReadAccelerometerCalibration();
   virtual IEditable *GetIEditable() { return (IEditable *)this; }
   virtual const IEditable *GetIEditable() const { return (const IEditable *)this; }
   virtual void Delete() {} // Can't delete table itself
   virtual void Uncreate() {}
   virtual bool LoadToken(const int id, BiffReader * const pbr);

   virtual IDispatch *GetPrimary() { return this->GetDispatch(); }
   virtual IDispatch *GetDispatch() { return (IDispatch *)this; }
   virtual const IDispatch *GetDispatch() const { return (const IDispatch *)this; }
   virtual IFireEvents *GetIFireEvents() { return (IFireEvents *)this; }
   virtual IDebugCommands *GetDebugCommands() { return NULL; }

   void SetZoom(float zoom);
   void SetMyScrollInfo();

   void BackupForPlay();
   void RestoreBackup();

   void BeginUndo();
   void EndUndo();
   void Undo();

   void Uncreate(IEditable *pie);
   void Undelete(IEditable *pie);

   STDMETHOD(GetDisplayString)(DISPID dispID, BSTR *pbstr) { return hrNotImplemented; }
   STDMETHOD(MapPropertyToPage)(DISPID dispID, CLSID *pclsid) { return hrNotImplemented; }
   STDMETHOD(GetPredefinedStrings)(DISPID dispID, CALPOLESTR *pcaStringsOut, CADWORD *pcaCookiesOut);
   STDMETHOD(GetPredefinedValue)(DISPID dispID, DWORD dwCookie, VARIANT *pVarOut);

   STDMETHOD(GetPredefinedStrings)(DISPID dispID, CALPOLESTR *pcaStringsOut, CADWORD *pcaCookiesOut, IEditable *piedit);
   STDMETHOD(GetPredefinedValue)(DISPID dispID, DWORD dwCookie, VARIANT *pVarOut, IEditable *piedit);

   virtual void OnLButtonDown(int x, int y);
   virtual void OnLButtonUp(int x, int y);
   virtual void OnMouseMove(int x, int y);
   void OnMouseMove(const short x, const short y);

   void SetDefaultView();
   void GetViewRect(FRect *pfrect) const;

   bool IsNameUnique(const WCHAR * const wzName) const;
   void GetUniqueName(const ItemTypeEnum type, WCHAR * const wzUniqueName) const;
   void GetUniqueName(const WCHAR * const prefix, WCHAR * const wzUniqueName) const;
   void GetUniqueNamePasting(const int type, WCHAR * const wzUniqueName);

   float GetSurfaceHeight(const char * const szName, float x, float y) const;

   void SetLoadDefaults();

   void SetDirty(SaveDirtyState sds);
   void SetNonUndoableDirty(SaveDirtyState sds);
   void CheckDirty();
   bool FDirty();

   void FVerifySaveToClose();

   //void Play2(const LPDIRECTSOUNDBUFFER &pdsb, const int &decibelvolume, float randompitch, int pitch, PinSound * pps, PinDirectSound * pDS, float pan, float front_rear_fade, const int &flags, const VARIANT_BOOL &restart);
   //void Play(PinSoundCopy * ppsc, const int &decibelvolume, float randompitch, const LPDIRECTSOUNDBUFFER &pdsb, int pitch, PinDirectSound * pDS, float pan, float front_rear_fade, const int &flags, const VARIANT_BOOL &restart);

   HRESULT StopSound(BSTR Sound);
   void StopAllSounds();

   void DeleteFromLayer(IEditable *obj);
   void UpdateCollection(const int index);
   void MoveCollectionUp(CComObject<Collection> *pcol);
   void MoveCollectionDown(CComObject<Collection> *pcol);

   int GetDetailLevel() const; // used for rubber, ramp and ball
   void SetDetailLevel(const int value);
   float GetZPD() const;
   float GetMaxSeparation() const;
   float Get3DOffset() const;

   FRect3D GetBoundingBox() const;

   bool RenderSolid() const { return m_renderSolid; }

   void InvokeBallBallCollisionCallback(Ball *b1, Ball *b2, float hitVelocity);

   BEGIN_COM_MAP(PinTable)
      COM_INTERFACE_ENTRY(ITable)
      COM_INTERFACE_ENTRY(IDispatch)
      COM_INTERFACE_ENTRY_IMPL(IConnectionPointContainer)
      COM_INTERFACE_ENTRY(IPerPropertyBrowsing)
      COM_INTERFACE_ENTRY(IProvideClassInfo)
      COM_INTERFACE_ENTRY(IProvideClassInfo2)
   END_COM_MAP()

   BEGIN_CONNECTION_POINT_MAP(PinTable)
      CONNECTION_POINT_ENTRY(DIID_ITableEvents)
   END_CONNECTION_POINT_MAP()

   void ListMaterials(HWND hwndListView);
   int AddListMaterial(HWND hwndListView, Material * const pmat);
   void RemoveMaterial(Material * const pmat);
   void AddDbgLight(Light * const plight);
   void UpdateDbgLight();
   void AddMaterial(Material * const pmat);
   void AddDbgMaterial(Material * const pmat);
   void UpdateDbgMaterial();

   bool IsMaterialNameUnique(const std::string &name) const;
   Material* GetMaterial(const std::string &szName) const;
   Material* GetSurfaceMaterial(const char * const szName) const;
   Texture* GetSurfaceImage(const char * const szName) const;

   bool GetCollectionIndex(const ISelect * const element, int &collectionIndex, int &elementIndex);

   void LockElements();

   string m_szFileName;
   string m_szTitle;

   VPinball *m_pvp;

   // editor viewport
   Vertex2D m_offset;
   float m_zoom;

   //ISelect *m_pselcur;
   VectorProtected<ISelect> m_vmultisel;

   float m_left; // always zero for now
   float m_top; // always zero for now
   float m_right;
   float m_bottom;

   float m_glassheight;
   float m_tableheight;

   float m_3DmaxSeparation;
   float m_global3DMaxSeparation;
   float m_3DZPD;
   float m_global3DZPD;
   float m_3DOffset;
   float m_global3DOffset;
   float m_defaultBulbIntensityScaleOnBall;

   unsigned int m_BG_current_set;
   float m_BG_inclination[NUM_BG_SETS];
   float m_BG_FOV[NUM_BG_SETS];
   float m_BG_layback[NUM_BG_SETS];
   float m_BG_rotation[NUM_BG_SETS];
   float m_BG_scalex[NUM_BG_SETS];
   float m_BG_scaley[NUM_BG_SETS];
   float m_BG_scalez[NUM_BG_SETS];
   float m_BG_xlatex[NUM_BG_SETS];
   float m_BG_xlatey[NUM_BG_SETS];
   float m_BG_xlatez[NUM_BG_SETS];
   char  m_BG_szImage[NUM_BG_SETS][MAXTOKEN];

   bool  m_BG_enable_FSS;
   int   m_currentBackglassMode;

   float m_angletiltMax;
   float m_angletiltMin;

   int   m_overridePhysics;
   float m_fOverrideGravityConstant, m_fOverrideContactFriction, m_fOverrideElasticity, m_fOverrideElasticityFalloff, m_fOverrideScatterAngle;
   float m_fOverrideMinSlope, m_fOverrideMaxSlope;

   unsigned int m_PhysicsMaxLoops;

   float m_Gravity;

   float m_friction;
   float m_elasticity;
   float m_elasticityFalloff;
   float m_scatter;

   float m_defaultScatter;
   float m_nudgeTime;
   int   m_plungerNormalize;
   bool  m_plungerFilter;

   bool  m_overridePhysicsFlipper;

   bool  m_tblAutoStartEnabled;
   bool  m_tblMirrorEnabled;		// Mirror tables left to right.  This is activated by a cheat during table selection.

   bool  m_tblAccelerometer;		// true if electronic accelerometer enabled
   bool  m_tblAccelNormalMount;	// true is Normal Mounting (Left Hand Coordinates)
   float m_tblAccelAngle;			// 0 degrees rotated counterclockwise (GUI is lefthand coordinates)
   float m_tblAccelAmpX;			// Accelerometer gain X axis
   float m_tblAccelAmpY;			// Accelerometer gain Y axis
   int   m_tblAccelMaxX;           // Accelerometer max value X axis
   int   m_tblAccelMaxY;			// Accelerometer max value Y axis

   float m_tblNudgeReadX;
   float m_tblNudgeReadY;
   float m_tblNudgeReadTilt;
   float m_tblNudgePlumbX;
   float m_tblNudgePlumbY;

   U32   m_tblAutoStart;           // msecs before trying an autostart if doing once-only method .. 0 is automethod
   U32   m_tblAutoStartRetry;      // msecs before retrying to autostart.
   float m_tblVolmod;              // volume modulation for doing audio balancing
   U32   m_tblExitConfirm;         // msecs for esc button to be pressed to exit completely
   float m_globalDifficulty;       // table Difficulty Level

   short m_oldMousePosX;
   short m_oldMousePosY;

   ProtectionData m_protectionData;

   char m_szImage[MAXTOKEN];
   char m_szPlayfieldMaterial[MAXNAMEBUFFER];
   COLORREF m_colorbackdrop;
   bool m_ImageBackdropNightDay;

   char m_szImageColorGrade[MAXTOKEN];

   char m_szBallImage[MAXTOKEN];
   char m_szBallImageDecal[MAXTOKEN];
   bool m_BallDecalMode;

   char m_szEnvImage[MAXTOKEN];

   //CComObject<Surface> *m_psur;

   vector< IEditable* > m_vedit;
   vector< IEditable* > m_layer[MAX_LAYERS];
   vector< ISelect* >   m_allHitElements;


   vector< Texture* > m_vimage;
   const vector<Texture *>& GetImageList() const
   {
       return m_vimage;
   }

   int m_numMaterials;
   vector< Material* > m_materials;
   const vector<Material *>& GetMaterialList() const
   {
       return m_materials;
   }

   vector< PinSound* > m_vsound;

   vector< PinFont* > m_vfont;

   VectorProtected< CComObject<Collection> > m_vcollection;

   COLORREF m_rgcolorcustom[16];		// array for the choosecolor in property browser

   float m_TableSoundVolume;
   float m_TableMusicVolume;

   int m_TableAdaptiveVSync;

   FRect m_rcDragRect; // Multi-select

   HBITMAP m_hbmOffScreen; // Buffer for drawing the editor window

   PinUndo m_undo;

   CComObject<CodeViewer> *m_pcv;

   CComObject<ScriptGlobalTable> *m_psgt; // Object to expose to script for global functions

   SaveDirtyState m_sdsDirtyProp;
   SaveDirtyState m_sdsDirtyScript;
   SaveDirtyState m_sdsNonUndoableDirty;
   SaveDirtyState m_sdsCurrentDirtyState;

   // Table info
   string m_szTableName;
   string m_szAuthor;
   string m_szVersion;
   string m_szReleaseDate;
   string m_szAuthorEMail;
   string m_szWebSite;
   string m_szBlurb;
   string m_szDescription;
   string m_szRules;
   string m_szScreenShot;
   string m_szDateSaved;
   unsigned int m_numTimesSaved;

   PinBinary *m_pbTempScreenshot; // Holds contents of screenshot image until the image asks for it

   vector<string> m_vCustomInfoTag;
   vector<string> m_vCustomInfoContent;

   vector<HANDLE> m_vAsyncHandles;

   int  m_globalDetailLevel;
   int  m_userDetailLevel;
   bool m_overwriteGlobalDetailLevel;

   bool m_overwriteGlobalDayNight;

   LightSource m_Light[MAX_LIGHT_SOURCES];
   COLORREF m_lightAmbient;
   float m_lightHeight;
   float m_lightRange;
   float m_lightEmissionScale;
   float m_envEmissionScale;
   float m_globalEmissionScale;
   float m_AOScale;
   float m_SSRScale;

   int   m_useReflectionForBalls;
   float m_playfieldReflectionStrength;
   int   m_useTrailForBalls;
   float m_ballTrailStrength;
   float m_ballPlayfieldReflectionStrength;
   int   m_useAA;
   int   m_useFXAA;
   int   m_useAO;
   int   m_useSSR;
   float m_bloom_strength;

   HWND  m_hMaterialManager;
   SearchSelectDialog m_searchSelectDlg;
   ProgressDialog m_progressDialog;

   volatile std::atomic<bool> m_savingActive;

   bool  m_dirtyDraw; // Whether our background bitmap is up to date
   bool  m_renderSolid;

   bool  m_grid; // Display grid or not
   bool  m_backdrop;
   bool  m_renderDecals;
   bool  m_renderEMReels;
   bool  m_overwriteGlobalStereo3D;
   bool  m_reflectElementsOnPlayfield;
   bool  m_reflectionEnabled;

   vector<Material*> m_dbgChangedMaterials;

   struct DebugLightData
   {
       char name[MAX_PATH];
       float falloff;
       float falloffPower;
       float intensity;
       float bulbModulateVsAdd;
       float transmissionScale;
       float fadeSpeedUp;
       float fadeSpeedDown;
       COLORREF color1;
       COLORREF color2;
       LightState lightstate;
   };

   vector<DebugLightData*> m_dbgChangedLights;

   float m_backupInclination;
   float m_backupFOV;
   float m_backupRotation;
   Vertex3Ds m_backupScale;
   Vertex3Ds m_backupOffset;
   Vertex3Ds m_backupCamera;
   float m_backupLayback;
   float m_backupEmisionScale;
   float m_backupLightRange;
   float m_backupLightHeight;
   float m_backupEnvEmissionScale;

#ifdef UNUSED_TILT //!! currently unused (see NudgeGetTilt())
   int   m_jolt_amount;
   int   m_tilt_amount;
   int   m_jolt_trigger_time;
   int   m_tilt_trigger_time;
#endif

   virtual void OnInitialUpdate();
   virtual LRESULT WndProc(UINT uMsg, WPARAM wParam, LPARAM lParam);
   virtual BOOL OnCommand(WPARAM wparam, LPARAM lparam);

   void SetMouseCursor();
   void OnLeftButtonDown(const short x, const short y);
   void OnMouseWheel(const short x, const short y, const short zDelta);
   void OnSize();
   void Set3DOffset(const float value);
   void SetZPD(const float value);
   void SetMaxSeparation(const float value);
   bool GetShowDT() const;
   void SetShowDT(const bool enable);
   bool GetShowFSS() const;
   void SetShowFSS(const bool enable);
   void PutGlobalAlphaAcc(const bool enable);
   int GetBallTrailStrength() const;
   void SetBallTrailStrength(const int value);
   int GetGlobalEmissionScale() const;
   void SetGlobalEmissionScale(const int value);
   float GetGlobalDifficulty() const;
   void SetGlobalDifficulty(const float value);
   int GetTableSoundVolume() const;
   void SetTableSoundVolume( const int value);
   int GetTableMusicVolume() const;
   void SetTableMusicVolume(const int value);
   int GetPlayfieldReflectionStrength() const;
   void SetPlayfieldReflectionStrength(const int value);
   float GetGravity() const;
   void SetGravity(const float value);
   void SetFriction(const float value);
   void SetPlungerNormalize(const int value);
   float GetTableWidth() const;
   void SetTableWidth(const float value);
   float GetHeight() const;
   void SetHeight(const float value);

   void SetMDITable(PinTableMDI * const table)
   {
       m_mdiTable = table;
   }
   PinTableMDI *GetMDITable() const
   {
       return m_mdiTable;
   }

   WCHAR* GetCollectionNameByElement(const ISelect* const element);
   void RefreshProperties();

   void SetNotesText(const CString& text) 
   { 
      m_notesText = text; 
      SetDirtyDraw(); 
   }
   CString GetNotesText() const
   {
      return m_notesText;
   }

private:
   PinTableMDI *m_mdiTable;
   bool m_moving;
   CString m_notesText;
   std::unordered_map<const char*, Texture *, StringHashFunctor, StringComparator> m_textureMap;  // hash table to speed up texture lookup by name
   std::unordered_map<const char*, Material*, StringHashFunctor, StringComparator> m_materialMap; // hash table to speed up material lookup by name
};

class ScriptGlobalTable :
   public CComObjectRootEx<CComSingleThreadModel>,
   public IDispatchImpl<ITableGlobal, &IID_ITableGlobal, &LIBID_VPinballLib>,
   public IScriptable
{
public:
   // Headers to support communication between the game and the script.
   STDMETHOD(EndModal)();
   STDMETHOD(BeginModal)();
   STDMETHOD(GetTextFile)(BSTR FileName, /*[out, retval]*/ BSTR *pContents);
   STDMETHOD(GetCustomParam)(long index, /*[out, retval]*/ BSTR *param);
   STDMETHOD(get_GameTime)(/*[out, retval]*/ long *pVal);
   STDMETHOD(get_SystemTime)(/*[out, retval]*/ long *pVal);
   STDMETHOD(get_AddCreditKey)(/*[out, retval]*/ long *pVal);
   STDMETHOD(get_AddCreditKey2)(/*[out, retval]*/ long *pVal);
   STDMETHOD(get_ActiveBall)(/*[out, retval]*/ IBall **pVal);
   STDMETHOD(LoadValue)(BSTR TableName, BSTR ValueName, /*[out, retval]*/ VARIANT *Value);
   STDMETHOD(SaveValue)(BSTR TableName, BSTR ValueName, VARIANT Value);
   STDMETHOD(StopSound)(BSTR Sound);
   STDMETHOD(AddObject)(BSTR Name, IDispatch *pdisp);
#ifdef _WIN64
   STDMETHOD(get_GetPlayerHWnd)(/*[out, retval]*/ SIZE_T *pVal);
#else
   STDMETHOD(get_GetPlayerHWnd)(/*[out, retval]*/ long *pVal);
#endif
   STDMETHOD(get_UserDirectory)(/*[out, retval]*/ BSTR *pVal);
   STDMETHOD(get_StartGameKey)(/*[out, retval]*/ long *pVal);
   STDMETHOD(PlayMusic)(BSTR str, float volume);
   STDMETHOD(put_MusicVolume)(float volume);
   STDMETHOD(EndMusic)();
   STDMETHOD(get_PlungerKey)(/*[out, retval]*/ long *pVal);
   STDMETHOD(get_CenterTiltKey)(/*[out, retval]*/ long *pVal);
   STDMETHOD(get_RightTiltKey)(/*[out, retval]*/ long *pVal);
   STDMETHOD(get_LeftTiltKey)(/*[out, retval]*/ long *pVal);
   STDMETHOD(get_RightFlipperKey)(/*[out, retval]*/ long *pVal);
   STDMETHOD(get_LeftFlipperKey)(/*[out, retval]*/ long *pVal);

   STDMETHOD(put_DMDWidth)(/*[in]*/ int pVal);
   STDMETHOD(put_DMDHeight)(/*[in]*/ int pVal);
   STDMETHOD(put_DMDPixels)(/*[in]*/ VARIANT pVal);
   STDMETHOD(put_DMDColoredPixels)(/*[in]*/ VARIANT pVal);

   STDMETHOD(get_WindowWidth)(/*[out, retval]*/ int *pVal);
   STDMETHOD(get_WindowHeight)(/*[out, retval]*/ int *pVal);

   STDMETHOD(get_NightDay)(/*[out, retval]*/ int *pVal);
   //STDMETHOD(put_NightDay)(/*[in]*/ int newVal);

   STDMETHOD(get_ShowDT)(/*[out, retval]*/ VARIANT_BOOL *pVal);
   //STDMETHOD(put_ShowDT)(/*[in]*/ VARIANT_BOOL newVal);

   STDMETHOD(get_ShowFSS)(/*[out, retval]*/ VARIANT_BOOL *pVal);
   //STDMETHOD(put_ShowFSS)(/*[in]*/ VARIANT_BOOL newVal);

   STDMETHOD(PlaySound)(BSTR bstr, long LoopCount, float volume, float pan, float randompitch, long pitch, VARIANT_BOOL usesame, VARIANT_BOOL restart, float front_rear_fade);
   STDMETHOD(FireKnocker)(/*[in]*/ int Count);
   STDMETHOD(QuitPlayer)(/*[in]*/ int CloseType);

   STDMETHOD(Nudge)(float Angle, float Force);
   STDMETHOD(NudgeGetCalibration)(VARIANT *XMax, VARIANT *YMax, VARIANT *XGain, VARIANT *YGain, VARIANT *DeadZone, VARIANT *TiltSensitivty);
   STDMETHOD(NudgeSetCalibration)(int XMax, int YMax, int XGain, int YGain, int DeadZone, int TiltSensitivty);
   STDMETHOD(NudgeSensorStatus)(VARIANT *XNudge, VARIANT *YNudge);
   STDMETHOD(NudgeTiltStatus)(VARIANT *XPlumb, VARIANT *YPlumb, VARIANT *Tilt);

   STDMETHOD(get_Name)(BSTR *pVal);
   STDMETHOD(get_MechanicalTilt)(/*[out, retval]*/ long *pVal);
   STDMETHOD(get_LeftMagnaSave)(/*[out, retval]*/ long *pVal);
   STDMETHOD(get_RightMagnaSave)(/*[out, retval]*/ long *pVal);
   STDMETHOD(get_ExitGame)(/*[out, retval]*/ long *pVal);
   STDMETHOD(get_LockbarKey)(/*[out, retval]*/ long *pVal);

   STDMETHOD(GetBalls)(/*[out, retval]*/ LPSAFEARRAY *pVal);
   STDMETHOD(GetElements)(/*[out, retval]*/ LPSAFEARRAY *pVal);
   STDMETHOD(GetElementByName)(/*[in]*/ BSTR name, /*[out, retval]*/ IDispatch* *pVal);
   STDMETHOD(get_ActiveTable)(/*[out, retval]*/ ITable **pVal);

   STDMETHOD(get_Version)(/*[out, retval]*/ int *pVal);
   STDMETHOD(get_VPBuildVersion)(/*[out, retval]*/ int *pVal);
   STDMETHOD(get_VersionMajor)(/*[out, retval]*/ int *pVal);
   STDMETHOD(get_VersionMinor)(/*[out, retval]*/ int *pVal);
   STDMETHOD(get_VersionRevision)(/*[out, retval]*/ int *pVal);

   STDMETHOD(UpdateMaterial)(BSTR pVal, float wrapLighting, float roughness, float glossyImageLerp, float thickness, float edge, float edgeAlpha, float opacity,
      OLE_COLOR base, OLE_COLOR glossy, OLE_COLOR clearcoat, VARIANT_BOOL isMetal, VARIANT_BOOL opacityActive,
      float elasticity, float elasticityFalloff, float friction, float scatterAngle);
   STDMETHOD(GetMaterial)(BSTR pVal, float *wrapLighting, float *roughness, float *glossyImageLerp, float *thickness, float *edge, float *edgeAlpha, float *opacity,
      OLE_COLOR *base, OLE_COLOR *glossy, OLE_COLOR *clearcoat, VARIANT_BOOL *isMetal, VARIANT_BOOL *opacityActive,
      float *elasticity, float *elasticityFalloff, float *friction, float *scatterAngle);
   STDMETHOD(UpdateMaterialPhysics)(BSTR pVal, float elasticity, float elasticityFalloff, float friction, float scatterAngle);
   STDMETHOD(GetMaterialPhysics)(BSTR pVal, float *elasticity, float *elasticityFalloff, float *friction, float *scatterAngle);
   STDMETHOD(MaterialColor)(BSTR pVal, OLE_COLOR newVal);

   void Init(VPinball *vpinball, PinTable *pt);

   virtual IDispatch *GetDispatch() { return (IDispatch *)this; }
   virtual const IDispatch *GetDispatch() const { return (const IDispatch *)this; }

   virtual ISelect *GetISelect() { return NULL; }
   virtual const ISelect *GetISelect() const { return NULL; }

   BEGIN_COM_MAP(ScriptGlobalTable)
      COM_INTERFACE_ENTRY(ITableGlobal)
      COM_INTERFACE_ENTRY(IDispatch)
   END_COM_MAP()

private:
   bool GetTextFileFromDirectory(const char * const szfilename, const char * const dirname, BSTR *pContents);

   PinTable *m_pt;
   VPinball *m_vpinball;
};

#endif // !defined(AFX_PINTABLE_H__D14A2DAB_2984_4FE7_A102_D0283ECE31B4__INCLUDED_)
