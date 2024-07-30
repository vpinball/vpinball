// PinTable.h: interface for the PinTable class.
//
//////////////////////////////////////////////////////////////////////
#pragma once
#if !defined(AFX_PINTABLE_H__D14A2DAB_2984_4FE7_A102_D0283ECE31B4__INCLUDED_)
#define AFX_PINTABLE_H__D14A2DAB_2984_4FE7_A102_D0283ECE31B4__INCLUDED_

#include "robin_hood.h"

#include <atomic>
#include "hash.h"
#include "SearchSelectDialog.h"
#include "renderer/RenderProbe.h"
#include "renderer/ViewSetup.h"

#define VIEW_PLAYFIELD 1
#define VIEW_BACKGLASS 2

#define MIN_ZOOM 0.126f // purposely make them offset from powers to 2 to account for roundoff error
#define MAX_ZOOM 63.9f

#define DISABLE_SCRIPT_EDITING 0x00000002 // cannot open script windows (stops editing and viewing)
#define DISABLE_EVERYTHING 0x80000000 // everything is off limits (including future locks)

#define MAX_LAYERS 11

struct LightSource
{
   COLORREF emission;
   Vertex3Ds pos;
};

struct ProtectionData
{
   int32_t fileversion;
   int32_t size;
   uint8_t paraphrase[16 + 8];
   uint32_t flags;
   int32_t keyversion;
   int32_t spare1;
   int32_t spare2;
};

class ScriptGlobalTable;

class PinTableMDI : public CMDIChild
{
public:
   PinTableMDI(VPinball *vpinball);
   virtual ~PinTableMDI();
   CComObject<PinTable> *GetTable() { return m_table; }
   bool CanClose() const;

protected:
   virtual void PreCreate(CREATESTRUCT &cs) override;
   virtual int OnCreate(CREATESTRUCT &cs) override;
   virtual void OnClose() override;
   virtual LRESULT OnMDIActivate(UINT msg, WPARAM wparam, LPARAM lparam) override;
   virtual BOOL OnEraseBkgnd(CDC &dc) override;

private:
   CComObject<PinTable> *m_table;
   VPinball *m_vpinball;
};

class ProgressDialog : public CDialog
{
public:
   ProgressDialog();
   void SetProgress(const int value) { m_progressBar.SetPos(value); }

   void SetName(const string &text) { m_progressName.SetWindowText(text.c_str()); }

protected:
   BOOL OnInitDialog() final;

private:
   CProgressBar m_progressBar;
   CStatic m_progressName;
};

class PinTable : public CWnd,
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
                 public IPerPropertyBrowsing // Ability to fill in dropdown in property browser
{
public:
   STDMETHOD(get_BallFrontDecal)(/*[out, retval]*/ BSTR *pVal);
   STDMETHOD(put_BallFrontDecal)(/*[in]*/ BSTR newVal);
   // deprecated
   STDMETHOD(get_YieldTime)(/*[out, retval]*/ long *pVal);
   // deprecated
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
   STDMETHOD(get_ShowFSS)(/*[out, retval]*/ VARIANT_BOOL *pVal);
   STDMETHOD(get_ReflectElementsOnPlayfield)(/*[out, retval]*/ VARIANT_BOOL *pVal);
   STDMETHOD(put_ReflectElementsOnPlayfield)(/*[in]*/ VARIANT_BOOL newVal);

   STDMETHOD(get_GlobalDifficulty)(/*[out, retval]*/ float *pVal);
   STDMETHOD(put_GlobalDifficulty)(/*[in]*/ float newVal);

   STDMETHOD(get_Accelerometer)(/*[out, retval]*/ VARIANT_BOOL *pVal); //!! remove?!
   STDMETHOD(put_Accelerometer)(/*[in]*/ VARIANT_BOOL newVal); //!! remove?!
   STDMETHOD(get_AccelNormalMount)(/*[out, retval]*/ VARIANT_BOOL *pVal); //!! remove?!
   STDMETHOD(put_AccelNormalMount)(/*[in]*/ VARIANT_BOOL newVal); //!! remove?!
   STDMETHOD(get_AccelerometerAngle)(/*[out, retval]*/ float *pVal); //!! remove?!
   STDMETHOD(put_AccelerometerAngle)(/*[in]*/ float newVal); //!! remove?!

   STDMETHOD(get_DeadZone)(/*[out, retval]*/ int *pVal); //!! remove?!
   STDMETHOD(put_DeadZone)(/*[in]*/ int newVal); //!! remove?!

#ifdef UNUSED_TILT
   STDMETHOD(get_JoltAmount)(/*[out, retval]*/ int *pVal);
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
   STDMETHOD(get_VPBuildVersion)(/*[out, retval]*/ double *pVal);
   STDMETHOD(get_VersionMajor)(/*[out, retval]*/ int *pVal);
   STDMETHOD(get_VersionMinor)(/*[out, retval]*/ int *pVal);
   STDMETHOD(get_VersionRevision)(/*[out, retval]*/ int *pVal);

   STDMETHOD(get_Option)(BSTR optionName, float minValue, float maxValue, float step, float defaultValue, int unit, /*[optional][in]*/ VARIANT values, /*[out, retval]*/ float *param);
   STDMETHOD(put_Option)(BSTR optionName, float minValue, float maxValue, float step, float defaultValue, int unit, /*[optional][in]*/ VARIANT values, /*[in]*/ float val);

   /////////////////////////////////////////////

   PinTable();
   virtual ~PinTable();

   void ClearForOverwrite() final;
   void InitBuiltinTable(const size_t tableId);
   void InitTablePostLoad();
   void RemoveInvalidReferences();

   HRESULT GetTypeName(BSTR *pVal) override;

   void SetCaption(const string &szCaption);
   void SetMouseCapture();
   int ShowMessageBox(const char *text) const;
   POINT GetScreenPoint() const;

   HRESULT InitVBA();
   void CloseVBA();

   void UIRenderPass2(Sur *const psur) override;
   void Paint(HDC hdc);
   ISelect *HitTest(const int x, const int y);
   void SetDirtyDraw();

   void Render3DProjection(Sur *const psur);

   bool GetDecalsEnabled() const;
   bool GetEMReelsEnabled() const;

   void Copy(int x, int y);
   void Paste(const bool atLocation, const int x, const int y);

   void ExportBlueprint();
   void ExportTableMesh();
   void ImportBackdropPOV(const string &filename);
   void ExportBackdropPOV();
   void ImportVPP(const string &filename);

   //void FireVoidEvent(int dispid);
   void FireKeyEvent(int dispid, int keycode);

   void HandleLoadFailure();
   
   void Play(const int playMode); // Duplicate table into a live instance, create a player to run it, suspend edit mode
   void StopPlaying(); // Called on a live instance of the table (copy for playing) before Player instance gets deleted

   void ImportSound(const HWND hwndListView, const string &filename);
   void ReImportSound(const HWND hwndListView, PinSound *const pps, const string &filename);
   bool ExportSound(PinSound *const pps, const char *const filename);
   void ListSounds(HWND hwndListView);
   int AddListSound(HWND hwndListView, PinSound *const pps);
   void RemoveSound(PinSound *const pps);
   HRESULT SaveSoundToStream(const PinSound *const pps, IStream *pstm);
   HRESULT LoadSoundFromStream(IStream *pstm, const int LoadFileVersion);
   bool ExportImage(const Texture *const ppi, const char *const filename);
   Texture* ImportImage(const string &filename, const string &imageName);
   void ListImages(HWND hwndListView);
   int AddListImage(HWND hwndListView, Texture *const ppi);
   void RemoveImage(Texture *const ppi);
   HRESULT LoadImageFromStream(IStream *pstm, size_t idx, int version, bool resize_on_low_mem);
   Texture *GetImage(const string &szName) const;
   bool GetImageLink(const Texture *const ppi) const;
   PinBinary *GetImageLinkBinary(const int id);
   Light *GetLight(const string &szName) const;
   RenderProbe *GetRenderProbe(const string &szName) const;

   void AuditTable() const;

   void ListCustomInfo(HWND hwndListView);
   int AddListItem(HWND hwndListView, const string &szName, const string &szValue1, LPARAM lparam);

   void ImportFont(HWND hwndListView, const string &filename);
   void ListFonts(HWND hwndListView);
   int AddListBinary(HWND hwndListView, PinBinary *ppb);
   void RemoveFont(PinFont *const ppf);

   void NewCollection(const HWND hwndListView, const bool fFromSelection);
   void ListCollections(HWND hwndListView);
   int AddListCollection(HWND hwndListView, CComObject<Collection> *pcol);
   void RemoveCollection(CComObject<Collection> *pcol);
   void SetCollectionName(Collection *pcol, const char *szName, HWND hwndList, int index);

   void DoContextMenu(int x, int y, const int menuid, ISelect *psel);
   virtual void DoCommand(int icmd, int x, int y) override;
   bool FMutilSelLocked();

   virtual void SelectItem(IScriptable *piscript) override;
   virtual void DoCodeViewCommand(int command) override;
   virtual void SetDirtyScript(SaveDirtyState sds) override;
   virtual void ExportMesh(ObjLoader &loader) override;

   // Multi-object manipulation
   virtual Vertex2D GetCenter() const override;
   virtual void PutCenter(const Vertex2D &pv) override;
   virtual void FlipY(const Vertex2D &pvCenter) override;
   virtual void FlipX(const Vertex2D &pvCenter) override;
   virtual void Rotate(const float ang, const Vertex2D &pvCenter, const bool useElementCenter) override;
   virtual void Scale(const float scalex, const float scaley, const Vertex2D &pvCenter, const bool useElementCenter) override;
   virtual void Translate(const Vertex2D &pvOffset) override;

   // IEditable (mostly bogus for now)
   virtual void UIRenderPass1(Sur *const psur) override;
   virtual ItemTypeEnum GetItemType() const override { return eItemTable; }
   virtual HRESULT InitLoad(IStream *pstm, PinTable *ptable, int *pid, int version, HCRYPTHASH hcrypthash, HCRYPTKEY hcryptkey) override;
   virtual HRESULT InitPostLoad() override;
   virtual HRESULT InitVBA(BOOL fNew, int id, WCHAR *const wzName) override;
   virtual ISelect *GetISelect() override { return (ISelect *)this; }
   virtual const ISelect *GetISelect() const override { return (const ISelect *)this; }
   virtual void SetDefaults(const bool fromMouseClick) override;
   virtual IScriptable *GetScriptable() override { return (IScriptable *)this; }
   virtual void SetDefaultPhysics(const bool fromMouseClick) override;

   virtual PinTable *GetPTable() override { return this; }
   virtual const PinTable *GetPTable() const override { return this; }
   const char *GetElementName(IEditable *pedit) const;

   IEditable *GetElementByName(const char *const name);
   void OnDelete();

   void DoLeftButtonDown(int x, int y, bool zoomIn);
   void OnLeftButtonUp(int x, int y);
   void OnRightButtonDown(int x, int y);
   void FillCollectionContextMenu(CMenu &mainMenu, CMenu &colSubMenu, ISelect *psel);
   void FillLayerContextMenu(CMenu &mainMenu, CMenu &layerSubMenu, ISelect *psel);
   void OnRightButtonUp(int x, int y);
   void DoMouseMove(int x, int y);
   void OnLeftDoubleClick(int x, int y);
   void UseTool(int x, int y, int tool);
   void OnKeyDown(int key);

   // Transform editor window coordinates to table coordinates
   Vertex2D TransformPoint(int x, int y) const;

   void ClearMultiSel(ISelect *newSel = nullptr);
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
   HRESULT SaveInfo(IStorage *pstg, HCRYPTHASH hcrypthash);
   HRESULT SaveCustomInfo(IStorage *pstg, IStream *pstmTags, HCRYPTHASH hcrypthash);
   HRESULT WriteInfoValue(IStorage *pstg, const WCHAR *const wzName, const string &szValue, HCRYPTHASH hcrypthash);
   HRESULT ReadInfoValue(IStorage *pstg, const WCHAR *const wzName, string &output, HCRYPTHASH hcrypthash);
   HRESULT SaveData(IStream *pstm, HCRYPTHASH hcrypthash, const bool saveForUndo) override;
   HRESULT LoadGameFromFilename(const string &szFileName);
   HRESULT LoadInfo(IStorage *pstg, HCRYPTHASH hcrypthash, int version);
   HRESULT LoadCustomInfo(IStorage *pstg, IStream *pstmTags, HCRYPTHASH hcrypthash, int version);
   HRESULT LoadData(IStream *pstm, int &csubobj, int &csounds, int &ctextures, int &cfonts, int &ccollection, int version, HCRYPTHASH hcrypthash, HCRYPTKEY hcryptkey);
   void ReadAccelerometerCalibration();
   virtual IEditable *GetIEditable() override { return (IEditable *)this; }
   virtual const IEditable *GetIEditable() const override { return (const IEditable *)this; }
   virtual void Delete() override { } // Can't delete table itself
   virtual void Uncreate() override { }
   virtual bool LoadToken(const int id, BiffReader *const pbr) override;

   virtual IDispatch *GetPrimary() { return this->GetDispatch(); }
   virtual IDispatch *GetDispatch() override { return (IDispatch *)this; }
   virtual const IDispatch *GetDispatch() const override { return (const IDispatch *)this; }
   virtual IFireEvents *GetIFireEvents() override { return (IFireEvents *)this; }
   virtual IDebugCommands *GetDebugCommands() { return nullptr; }

   void SetZoom(float zoom);
   void SetMyScrollInfo();

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

   virtual void OnLButtonDown(int x, int y) override;
   virtual void OnLButtonUp(int x, int y) override;
   virtual void OnMouseMove(int x, int y) override;
   void OnMouseMove(const short x, const short y);

   void SetDefaultView();
   void GetViewRect(FRect *pfrect) const;

   bool IsNameUnique(const WCHAR *const wzName) const;
   void GetUniqueName(const ItemTypeEnum type, WCHAR *const wzUniqueName, const DWORD wzUniqueName_maxlength) const;
   void GetUniqueName(const WCHAR *const prefix, WCHAR *const wzUniqueName, const DWORD wzUniqueName_maxlength) const;
   void GetUniqueNamePasting(const int type, WCHAR *const wzUniqueName, const DWORD wzUniqueName_maxlength);

   float GetSurfaceHeight(const string &name, float x, float y) const;

   void SetLoadDefaults();

   void SetDirty(SaveDirtyState sds);
   void SetNonUndoableDirty(SaveDirtyState sds);
   void CheckDirty();
   bool FDirty() const;

   void FVerifySaveToClose();

   //void Play2(const LPDIRECTSOUNDBUFFER &pdsb, const int &decibelvolume, float randompitch, int pitch, PinSound * pps, PinDirectSound * pDS, float pan, float front_rear_fade, const int &flags, const VARIANT_BOOL &restart);
   //void Play(PinSoundCopy * ppsc, const int &decibelvolume, float randompitch, const LPDIRECTSOUNDBUFFER &pdsb, int pitch, PinDirectSound * pDS, float pan, float front_rear_fade, const int &flags, const VARIANT_BOOL &restart);

   HRESULT StopSound(BSTR Sound);
   void StopAllSounds();

   void DeleteFromLayer(IEditable *obj);
   void UpdateCollection(const int index);
   void MoveCollectionUp(CComObject<Collection> *pcol);
   void MoveCollectionDown(CComObject<Collection> *pcol);
   void UpdatePropertyImageList();
   void UpdatePropertyMaterialList();
   int GetDetailLevel() const { return m_settings.LoadValueWithDefault(Settings::Player, "AlphaRampAccuracy"s, 10); }; // used for rubber, ramp and ball
   float GetZPD() const;
   float GetMaxSeparation() const;
   float Get3DOffset() const;

   FRect3D GetBoundingBox() const;
   void ComputeNearFarPlane(const Matrix3D& matWorldView, const float scale, float &near, float &far) const;

   bool RenderSolid() const { return m_renderSolid; }

   void InvokeBallBallCollisionCallback(const Ball *b1, const Ball *b2, float hitVelocity);

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
   int AddListMaterial(HWND hwndListView, Material *const pmat);
   void RemoveMaterial(Material *const pmat);
   void AddMaterial(Material *const pmat);

   bool IsMaterialNameUnique(const string &name) const;
   Material *GetMaterial(const string &name) const;
   Material *GetSurfaceMaterial(const string &name) const;
   Texture *GetSurfaceImage(const string &name) const;

   bool GetCollectionIndex(const ISelect *const element, int &collectionIndex, int &elementIndex);

   void LockElements();

   string m_szFileName;
   string m_szTitle;

   // Flag that disables all table edition. Lock toggles are counted to identify version changes in a table (for example to guarantee untouched table for tournament)
   bool IsLocked() const { return (m_locked & 1) != 0; }
   void ToggleLock() { BeginUndo(); MarkForUndo(); m_locked++; EndUndo(); SetDirtyDraw(); }

   bool TournamentModePossible() const { return IsLocked() && !FDirty() && m_pcv->external_script_name.empty(); }

private:
   unsigned int m_locked = 0;

public:
   void SetSettingsFileName(const string &path)
   {
      m_szIniFileName = FileExists(path) ? path : string();
      m_settings.LoadFromFile(GetSettingsFileName(), false);
   }
   string GetSettingsFileName() const
   {
      if (!m_szIniFileName.empty() && FileExists(m_szIniFileName))
         return m_szIniFileName;
      string szINIFilename = m_szFileName;
      if (ReplaceExtensionFromFilename(szINIFilename, "ini"s))
         return szINIFilename;
      return string();
   }
   string m_szIniFileName;
   Settings m_settings; // Settings for this table (apply overrides above application settings)

   bool m_isLiveInstance = false; // true for live shallow copy of a table
   robin_hood::unordered_map<void *, void *> m_startupToLive; // For live table, maps back and forth to startup table editable parts, materials,...
   robin_hood::unordered_map<void *, void *> m_liveToStartup;

   // editor viewport
   Vertex2D m_offset;
   float m_zoom;

   //ISelect *m_pselcur;
   VectorProtected<ISelect> m_vmultisel;

   float m_left; // always zero for now
   float m_top; // always zero for now
   float m_right;
   float m_bottom;

   float m_glassBottomHeight; // Height of glass above playfield at bottom of playfield
   float m_glassTopHeight; // Height of glass above playfield at top of playfield

   float m_3DmaxSeparation;
   float m_global3DMaxSeparation;
   float m_3DZPD;
   float m_global3DZPD;
   float m_3DOffset;
   float m_global3DOffset;
   float m_defaultBulbIntensityScaleOnBall;

   bool m_BG_enable_FSS; // Flag telling if this table supports Full Single Screen POV (defaults is to use it in desktop mode if available)
   ViewSetupID m_BG_override = BG_INVALID; // Allow to easily override the POV for testing (not persisted)
   ViewSetupID m_BG_current_set; // Cache of the active view setup ID (depends on table but also on application settings and user overriding it)
   ViewSetup mViewSetups[NUM_BG_SETS];
   string m_BG_image[NUM_BG_SETS];
   ViewSetupID m_currentBackglassMode; // POV shown in the UI (not persisted)

   float m_angletiltMax;
   float m_angletiltMin;

   int m_overridePhysics;
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
   int m_plungerNormalize;
   bool m_plungerFilter;

   bool m_overridePhysicsFlipper;

   bool m_tblAutoStartEnabled;
   bool m_tblMirrorEnabled; // Mirror tables left to right.  This is activated by a cheat during table selection.

   bool m_tblAccelerometer; // true if electronic accelerometer enabled
   bool m_tblAccelNormalMount; // true is Normal Mounting (Left Hand Coordinates)
   float m_tblAccelAngle; // 0 degrees rotated counterclockwise (GUI is lefthand coordinates)
   Vertex2D m_tblAccelAmp; // Accelerometer gain X/Y axis
   int2 m_tblAccelMax; // Accelerometer max value X/Y axis

   Vertex2D m_tblNudgeRead;
   float m_tblNudgeReadTilt;
   Vertex2D m_tblNudgePlumb;

   U32 m_tblAutoStart; // msecs before trying an autostart if doing once-only method .. 0 is automethod
   U32 m_tblAutoStartRetry; // msecs before retrying to autostart.
   float m_tblVolmod; // volume modulation for doing audio balancing
   U32 m_tblExitConfirm; // msecs for exit (or esc if not disabled) button to be pressed to exit completely
   float m_difficulty; // table difficulty Level
   float m_globalDifficulty; // global difficulty, that is to say table difficulty eventually overriden from settings

   short2 m_oldMousePos;

   ProtectionData m_protectionData;

   string m_image;
   string m_playfieldMaterial;
   COLORREF m_colorbackdrop;
   bool m_ImageBackdropNightDay;

   string m_imageColorGrade;

   string m_ballImage;
   bool m_ballSphericalMapping = true;
   string m_ballImageDecal;
   bool m_BallDecalMode;

   string m_envImage;

   vector<IEditable *> m_vedit;
   vector<IEditable *> m_layer[MAX_LAYERS];
   vector<ISelect *> m_allHitElements;

   vector<Texture *> m_vimage;
   vector<Texture *> m_vliveimage;
   const vector<Texture *> &GetImageList() const { return m_vimage; }

   int m_numMaterials;
   vector<Material *> m_materials;
   const vector<Material *> &GetMaterialList() const { return m_materials; }

   vector<PinSound *> m_vsound;

   vector<PinFont *> m_vfont;

   VectorProtected<CComObject<Collection>> m_vcollection;

   vector<RenderProbe *> m_vrenderprobe;
   void RemoveRenderProbe(RenderProbe *pb) { m_vrenderprobe.erase(std::remove(m_vrenderprobe.begin(), m_vrenderprobe.end(), pb), m_vrenderprobe.end()); }
   RenderProbe *NewRenderProbe() { auto pb = new RenderProbe(); m_vrenderprobe.push_back(pb); return pb; }
   const vector<RenderProbe *> &GetRenderProbeList() const { return m_vrenderprobe; }
   const vector<RenderProbe *> GetRenderProbeList(RenderProbe::ProbeType type) const
   {
      std::vector<RenderProbe *> list;
      std::copy_if(m_vrenderprobe.begin(), m_vrenderprobe.end(), std::back_inserter(list), [type](RenderProbe *rp) { return rp->GetType() == type; });
      return list;
   }

   COLORREF m_rgcolorcustom[16]; // array for the choosecolor in property browser

   float m_TableSoundVolume;
   float m_TableMusicVolume;

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

   LightSource m_Light[MAX_LIGHT_SOURCES];
   COLORREF m_lightAmbient;
   float m_lightHeight;
   float m_lightRange;
   float m_lightEmissionScale;
   float m_envEmissionScale;
   float m_globalEmissionScale;
   float m_AOScale;
   float m_SSRScale;

   float m_playfieldReflectionStrength; // default (implicit) playfield reflection strength (0 to disable playfield reflection)

   float m_ballPlayfieldReflectionStrength;
   bool m_enableAO;
   bool m_enableSSR;
   float m_bloom_strength;

   HWND m_hMaterialManager;
   SearchSelectDialog m_searchSelectDlg;
   ProgressDialog m_progressDialog;

   volatile std::atomic<bool> m_savingActive;

   bool m_dirtyDraw; // Whether our background bitmap is up to date
   bool m_renderSolid;

   bool m_grid; // Display grid or not
   bool m_backdrop;
   bool m_renderDecals;
   bool m_renderEMReels;
   bool m_overwriteGlobalStereo3D;

#ifdef UNUSED_TILT //!! currently unused (see NudgeGetTilt())
   int m_jolt_amount;
   int m_tilt_amount;
   int m_jolt_trigger_time;
   int m_tilt_trigger_time;
#endif

   virtual void OnInitialUpdate() override;
   virtual LRESULT WndProc(UINT uMsg, WPARAM wParam, LPARAM lParam) override;
   virtual BOOL OnEraseBkgnd(CDC &dc) override;

   void SetMouseCursor();
   void OnLeftButtonDown(const short x, const short y);
   void OnMouseWheel(const short x, const short y, const short zDelta);
   void OnSize();
   void Set3DOffset(const float value);
   void SetZPD(const float value);
   void SetMaxSeparation(const float value);
   bool IsFSSEnabled() const;
   void EnableFSS(const bool enable);
   ViewSetupID GetViewSetupOverride() const { return m_BG_override; }
   void SetViewSetupOverride(const ViewSetupID v) { m_BG_override = v; UpdateCurrentBGSet(); }
   void UpdateCurrentBGSet();
   int GetGlobalEmissionScale() const;
   void SetGlobalEmissionScale(const int value);
   float GetGlobalDifficulty() const;
   void SetGlobalDifficulty(const float value);
   int GetTableSoundVolume() const;
   void SetTableSoundVolume(const int value);
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

   void SetMDITable(PinTableMDI *const table) { m_mdiTable = table; }
   PinTableMDI *GetMDITable() const { return m_mdiTable; }

   WCHAR *GetCollectionNameByElement(const ISelect *const element);
   void RefreshProperties();

   void SetNotesText(const CString &text)
   {
      m_notesText = text;
      SetDirtyDraw();
   }
   CString GetNotesText() const { return m_notesText; }

   ToneMapper GetToneMapper() const { return m_toneMapper; }
   void SetToneMapper(const ToneMapper& tm) { m_toneMapper = tm; }

private:
   PinTableMDI *m_mdiTable = nullptr;
   CString m_notesText;
   robin_hood::unordered_map<string, Texture *, StringHashFunctor, StringComparator> m_textureMap; // hash table to speed up texture lookup by name
   robin_hood::unordered_map<string, Material *, StringHashFunctor, StringComparator> m_materialMap; // hash table to speed up material lookup by name
   robin_hood::unordered_map<string, Light *, StringHashFunctor, StringComparator> m_lightMap; // hash table to speed up light lookup by name
   robin_hood::unordered_map<string, RenderProbe *, StringHashFunctor, StringComparator> m_renderprobeMap; // hash table to speed up renderprobe lookup by name
   bool m_moving;

   ToneMapper m_toneMapper = ToneMapper::TM_TONY_MC_MAPFACE;
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
   STDMETHOD(GetCustomParam)(/*[in]*/ long index, /*[out, retval]*/ BSTR *param);
   STDMETHOD(get_Setting)(BSTR Section, BSTR SettingName, /*[out, retval]*/ BSTR *param);
   STDMETHOD(get_FrameIndex)(/*[out, retval]*/ long *pVal);
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
   STDMETHOD(get_TablesDirectory)(/*[out, retval]*/ BSTR *pVal);
   STDMETHOD(get_MusicDirectory)(/*[optional][in]*/ VARIANT pSubDir, /*[out, retval]*/ BSTR *pVal);
   STDMETHOD(get_ScriptsDirectory)(/*[out, retval]*/ BSTR *pVal);
   STDMETHOD(get_PlatformOS)(/*[out, retval]*/ BSTR *pVal);
   STDMETHOD(get_PlatformCPU)(/*[out, retval]*/ BSTR *pVal);
   STDMETHOD(get_PlatformBits)(/*[out, retval]*/ BSTR *pVal);
   STDMETHOD(put_ShowCursor)(/*[in]*/ VARIANT_BOOL enable);
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
   STDMETHOD(get_StagedRightFlipperKey)(/*[out, retval]*/ long *pVal);
   STDMETHOD(get_StagedLeftFlipperKey)(/*[out, retval]*/ long *pVal);

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
   STDMETHOD(get_JoyCustomKey)(/*[in]*/ long index, /*[out, retval]*/ long *pVal);

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

   void Init(VPinball *vpinball, PinTable *pt);

   IDispatch *GetDispatch() override { return (IDispatch *)this; }
   const IDispatch *GetDispatch() const override { return (const IDispatch *)this; }

   ISelect *GetISelect() override { return nullptr; }
   const ISelect *GetISelect() const override { return nullptr; }

   BEGIN_COM_MAP(ScriptGlobalTable)
   COM_INTERFACE_ENTRY(ITableGlobal)
   COM_INTERFACE_ENTRY(IDispatch)
   END_COM_MAP()

private:
   bool GetTextFileFromDirectory(const string& szfilename, const string& dirname, BSTR *pContents);

   PinTable *m_pt;
   VPinball *m_vpinball;
};

#endif // !defined(AFX_PINTABLE_H__D14A2DAB_2984_4FE7_A102_D0283ECE31B4__INCLUDED_)
