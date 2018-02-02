// PinTable.h: interface for the PinTable class.
//
//////////////////////////////////////////////////////////////////////
#pragma once
#if !defined(AFX_PINTABLE_H__D14A2DAB_2984_4FE7_A102_D0283ECE31B4__INCLUDED_)
#define AFX_PINTABLE_H__D14A2DAB_2984_4FE7_A102_D0283ECE31B4__INCLUDED_

#if 0
#include <tr1/unordered_map>
using namespace std::tr1;
#else
#include <unordered_map>
#endif

#include "hash.h"
#include "SearchSelectDialog.h"

#define VIEW_PLAYFIELD 1
#define VIEW_BACKGLASS 2

#define MIN_ZOOM 0.126f // purposely make them offset from powers to 2 to account for roundoff error
#define MAX_ZOOM 63.9f

// define table protection flags
#define DISABLE_TABLE_SAVE		0x00000001      // cannot save table (or export)
#define DISABLE_SCRIPT_EDITING	0x00000002		// cannot open script windows (stops editing and viewing)
#define DISABLE_OPEN_MANAGERS	0x00000004		// cannot open file managers (image, sound etc..)
#define DISABLE_CUTCOPYPASTE	0x00000008		// cannot cuy, copy and paste items to/from the clipboard
#define DISABLE_TABLEVIEW		0x00000010		// cannot view the table design
#define DISABLE_TABLE_SAVEPROT	0x00000020		// connot save a protected table (stops hijacking)
#define DISABLE_DEBUGGER		0x00000040		// cannot use the visual table debugger

#define DISABLE_EVERYTHING		0x80000000		// everything is off limits (including future locks)

#define	PROT_DATA_VERSION		1
#define	PROT_PASSWORD_LENGTH	16
#define	PROT_CIPHER_LENGTH		PROT_PASSWORD_LENGTH+8
#define	PROT_KEYVERSION			0


struct LightSource {
   COLORREF emission;
   Vertex3Ds pos;
};

typedef struct {
   long				fileversion;
   long				size;
   unsigned char	paraphrase[PROT_CIPHER_LENGTH];
   unsigned long	flags;
   int				keyversion;
   int				spare1;
   int				spare2;
} _protectionData;

class ScriptGlobalTable;

class PinTable :
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
   STDMETHOD(get_EnableFXAA)(/*[out, retval]*/ FXAASettings *pVal);
   STDMETHOD(put_EnableFXAA)(/*[in]*/ FXAASettings newVal);

   STDMETHOD(get_BloomStrength)(/*[out, retval]*/ float *pVal);
   STDMETHOD(put_BloomStrength)(/*[in]*/ float newVal);

   STDMETHOD(get_OverridePhysics)(/*[out, retval]*/ PhysicsSet *pVal);
   STDMETHOD(put_OverridePhysics)(/*[in]*/ PhysicsSet newVal);

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

   void Init(VPinball *pvp, const bool useBlankTable=false);
   void InitPostLoad(VPinball *pvp);

   virtual HRESULT GetTypeName(BSTR *pVal);
   virtual void GetDialogPanes(Vector<PropertyPane> *pvproppane);

   void CreateTableWindow();
   void SetCaption(char *szCaption);

   HRESULT InitVBA();
   void CloseVBA();

   void Render(Sur * const psur);
   void Paint(HDC hdc);
   ISelect *HitTest(const int x, const int y);
   void SetDirtyDraw();

   void Render3DProjection(Sur * const psur);

   BOOL GetDecalsEnabled() const;
   BOOL GetEMReelsEnabled() const;

   void Copy(int x, int y);
   void Paste(BOOL fAtLocation, int x, int y);

   void ExportBlueprint();
   void ExportTableMesh();
   void ImportBackdropPOV(const char *filename = NULL);
   void ExportBackdropPOV();

   //void FireVoidEvent(int dispid);
   void FireKeyEvent(int dispid, int keycode);

   void Play(bool _cameraMode = false);
   void StopPlaying();

   void ImportSound(HWND hwndListView, char *filename, BOOL fPlay);
   void ReImportSound(HWND hwndListView, PinSound *pps, char *filename, BOOL fPlay);
   bool ExportSound(HWND hwndListView, PinSound *pps, char *filename);
   void ListSounds(HWND hwndListView);
   int AddListSound(HWND hwndListView, PinSound *pps);
   void RemoveSound(PinSound *pps);
   HRESULT SaveSoundToStream(PinSound *pps, IStream *pstm);
   HRESULT LoadSoundFromStream(IStream *pstm, const int LoadFileVersion);
   void ClearOldSounds();
   bool ExportImage(HWND hwndListView, Texture *ppi, char *filename);
   void ImportImage(HWND hwndListView, char *filename);
   void ReImportImage(HWND hwndListView, Texture *ppi, char *filename);
   void ListImages(HWND hwndListView);
   int AddListImage(HWND hwndListView, Texture *ppi);
   void RemoveImage(Texture *ppi);
   HRESULT LoadImageFromStream(IStream *pstm, int version);
   Texture *GetImage(char * const szName) const;
   void CreateGDIBackdrop();
   int GetImageLink(Texture *ppi);
   PinBinary *PinTable::GetImageLinkBinary(int id);

   void ListCustomInfo(HWND hwndListView);
   int AddListItem(HWND hwndListView, char *szName, char *szValue1, LPARAM lparam);

   void ImportFont(HWND hwndListView, char *filename);
   void ListFonts(HWND hwndListView);
   int AddListBinary(HWND hwndListView, PinBinary *ppb);
   void RemoveFont(PinFont *ppf);

   void NewCollection(HWND hwndListView, BOOL fFromSelection);
   void ListCollections(HWND hwndListView);
   int AddListCollection(HWND hwndListView, CComObject<Collection> *pcol);
   void RemoveCollection(CComObject<Collection> *pcol);
   void SetCollectionName(Collection *pcol, char *szName, HWND hwndList, int index);

   void DoContextMenu(int x, int y, int menuid, ISelect *psel);
   virtual void DoCommand(int icmd, int x, int y);
   bool FMutilSelLocked();

   virtual void SelectItem(IScriptable *piscript);
   virtual void DoCodeViewCommand(int command);
   virtual void SetDirtyScript(SaveDirtyState sds);
   virtual void ExportMesh(FILE *f);

   // Multi-object manipulation
   virtual void GetCenter(Vertex2D * const pv) const;
   virtual void PutCenter(const Vertex2D * const pv);
   virtual void FlipY(Vertex2D * const pvCenter);
   virtual void FlipX(Vertex2D * const pvCenter);
   virtual void Rotate(float ang, Vertex2D *pvCenter, const bool useElementCenter = false);
   virtual void Scale(float scalex, float scaley, Vertex2D *pvCenter, const bool useElementsCenter=false);
   virtual void Translate(Vertex2D *pvOffset);

   // IEditable (mostly bogus for now)
   virtual void PreRender(Sur * const psur);
   virtual ItemTypeEnum GetItemType() const { return eItemTable; }
   virtual HRESULT InitLoad(IStream *pstm, PinTable *ptable, int *pid, int version, HCRYPTHASH hcrypthash, HCRYPTKEY hcryptkey);
   virtual HRESULT InitPostLoad();
   virtual HRESULT InitVBA(BOOL fNew, int id, WCHAR *wzName);
   virtual ISelect *GetISelect();
   virtual void SetDefaults(bool fromMouseClick);
   virtual IScriptable *GetScriptable();
   virtual void SetDefaultPhysics(bool fromMouseClick);

   virtual PinTable *GetPTable() { return this; }
   char *GetElementName(IEditable *pedit);

   IEditable *GetElementByName(const char *name);
   void OnDelete();

   void DoLButtonDown(int x, int y, bool zoomIn = true);
   void DoLButtonUp(int x, int y);
   void DoRButtonDown(int x, int y);
   void FillCollectionContextMenu(HMENU hmenu, HMENU colSubMenu, ISelect *psel);
   void DoRButtonUp(int x, int y);
   void DoMouseMove(int x, int y);
   void DoLDoubleClick(int x, int y);
   void UseTool(int x, int y, int tool);
   void OnKeyDown(int key);

   // Transform editor window coordinates to table coordinates
   Vertex2D TransformPoint(int x, int y) const;

   void ClearMultiSel(ISelect *newSel = NULL);
   bool MultiSelIsEmpty();
   ISelect *GetSelectedItem() const { return m_vmultisel.ElementAt(0); }
   void AddMultiSel(ISelect *psel, bool fAdd, bool fUpdate = true, bool fContextClick = false);

   void BeginAutoSaveCounter();
   void EndAutoSaveCounter();
   void AutoSave();

   HRESULT TableSave();
   HRESULT SaveAs();
   virtual HRESULT ApcProject_Save();
   HRESULT Save(BOOL fSaveAs);
   HRESULT SaveToStorage(IStorage *pstg);
   HRESULT SaveInfo(IStorage* pstg, HCRYPTHASH hcrypthash);
   HRESULT SaveCustomInfo(IStorage* pstg, IStream *pstmTags, HCRYPTHASH hcrypthash);
   HRESULT WriteInfoValue(IStorage* pstg, WCHAR *wzName, char *szValue, HCRYPTHASH hcrypthash);
   HRESULT ReadInfoValue(IStorage* pstg, WCHAR *wzName, char **pszValue, HCRYPTHASH hcrypthash);
   HRESULT SaveData(IStream* pstm, HCRYPTHASH hcrypthash, HCRYPTKEY hcryptkey);
   HRESULT LoadGameFromFilename(char *szFileName);
   HRESULT LoadGameFromStorage(IStorage *pstgRoot);
   HRESULT LoadInfo(IStorage* pstg, HCRYPTHASH hcrypthash, int version);
   HRESULT LoadCustomInfo(IStorage* pstg, IStream *pstmTags, HCRYPTHASH hcrypthash, int version);
   HRESULT LoadData(IStream* pstm, int& csubobj, int& csounds, int& ctextures, int& cfonts, int& ccollection, int version, HCRYPTHASH hcrypthash, HCRYPTKEY hcryptkey);
   void ReadAccelerometerCalibration();
   virtual IEditable *GetIEditable() { return (IEditable *)this; }
   virtual void Delete() {} // Can't delete table itself
   virtual void Uncreate() {}
   virtual BOOL LoadToken(int id, BiffReader *pbr);

   virtual IDispatch *GetPrimary() { return this->GetDispatch(); }
   virtual IDispatch *GetDispatch() { return (IDispatch *)this; }
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

   void SetDefaultView();
   void GetViewRect(FRect *pfrect);

   bool IsNameUnique(WCHAR *wzName);
   void GetUniqueName(ItemTypeEnum type, WCHAR *wzUniqueName);
   void GetUniqueName(WCHAR *prefix, WCHAR *wzUniqueName);
   void GetUniqueNamePasting(int type, WCHAR *wzUniqueName);

   float GetSurfaceHeight(char *szName, float x, float y);

   void SetLoadDefaults();

   void SetDirty(SaveDirtyState sds);
   void SetNonUndoableDirty(SaveDirtyState sds);
   void CheckDirty();
   BOOL FDirty();

   BOOL FVerifySaveToClose();

   //void Play2(const LPDIRECTSOUNDBUFFER &pdsb, const int &decibelvolume, float randompitch, int pitch, PinSound * pps, PinDirectSound * pDS, float pan, float front_rear_fade, const int &flags, const VARIANT_BOOL &restart);
   //void Play(PinSoundCopy * ppsc, const int &decibelvolume, float randompitch, const LPDIRECTSOUNDBUFFER &pdsb, int pitch, PinDirectSound * pDS, float pan, float front_rear_fade, const int &flags, const VARIANT_BOOL &restart);

   HRESULT StopSound(BSTR Sound);
   void StopAllSounds();

   BOOL CheckPermissions(unsigned long flag);
   BOOL IsTableProtected();
   void ResetProtectionBlock();
   BOOL SetupProtectionBlock(unsigned char *pPassword, unsigned long flags);
   BOOL UnlockProtectionBlock(unsigned char *pPassword);

   void SwitchToLayer(int layerNumber);
   void AssignToLayer(IEditable *obj, int layerNumber);
   void AssignMultiToLayer(int layerNumber, int x, int y);
   void MergeAllLayers();
   void RestoreLayers();
   void BackupLayers();
   void DeleteFromLayer(IEditable *obj);
   void UpdateCollection(int index);
   void MoveCollectionUp(CComObject<Collection> *pcol);
   void MoveCollectionDown(CComObject<Collection> *pcol);

   int GetDetailLevel();
   float GetZPD();
   float GetMaxSeparation();
   float Get3DOffset();

   FRect3D GetBoundingBox();

   bool RenderSolid()      { return m_renderSolid; }

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
   int AddListMaterial(HWND hwndListView, Material *pmat);
   void RemoveMaterial(Material *pmat);
   void AddDbgLight( Light *plight );
   void UpdateDbgLight( void );
   void AddMaterial( Material *pmat );
   void AddDbgMaterial(Material *pmat);
   void UpdateDbgMaterial(void);
   bool IsMaterialNameUnique(char *name);
   Material* GetMaterial(char * const szName) const;
   Material* GetSurfaceMaterial(char *szName);
   Texture *GetSurfaceImage(char *szName);
   bool GetCollectionIndex(ISelect *element, int &collectionIndex, int &elementIndex);

   void LockElements();
   char m_szFileName[_MAX_PATH];
   char m_szBlueprintFileName[_MAX_PATH];
   char m_szObjFileName[_MAX_PATH];
   char m_szTitle[MAX_LINE_LENGTH];

   HWND m_hwnd;
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

   int   m_fOverridePhysics;
   float m_fOverrideGravityConstant, m_fOverrideContactFriction, m_fOverrideElasticity, m_fOverrideElasticityFalloff, m_fOverrideScatterAngle;

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
   float m_globalDifficulty;		// Table Difficulty Level

   short m_oldMousePosX;
   short m_oldMousePosY;
   _protectionData m_protectionData;

   char m_szImage[MAXTOKEN];
   char m_szPlayfieldMaterial[32];
   COLORREF m_colorbackdrop;
   bool m_ImageBackdropNightDay;

   char m_szImageColorGrade[MAXTOKEN];

   char m_szBallImage[MAXTOKEN];
   char m_szBallImageFront[MAXTOKEN];
   bool m_BallDecalMode;

   char m_szEnvImage[MAXTOKEN];

   //CComObject<Surface> *m_psur;

   Vector< IEditable > m_vedit;
   Vector< IEditable > m_layer[8];
   Vector< ISelect > m_allHitElements;

   std::vector< Texture* > m_vimage;

   int m_numMaterials;
   Vector< Material > m_materials;

   Vector< PinSound > m_vsound;

   Vector< PinFont > m_vfont;

   VectorProtected< CComObject<Collection> > m_vcollection;

   COLORREF m_rgcolorcustom[16];		// array for the choosecolor in property browser

   Vector< PinSoundCopy > m_voldsound; // copied sounds currently playing

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
   char *m_szTableName;
   char *m_szAuthor;
   char *m_szVersion;
   char *m_szReleaseDate;
   char *m_szAuthorEMail;
   char *m_szWebSite;
   char *m_szBlurb;
   char *m_szDescription;
   char *m_szRules;
   char  m_szScreenShot[MAXTOKEN];

   PinBinary *m_pbTempScreenshot; // Holds contents of screenshot image until the image asks for it

   Vector<char> m_vCustomInfoTag;
   Vector<char> m_vCustomInfoContent;

   std::vector<HANDLE> m_vAsyncHandles;

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

   int   m_useReflectionForBalls;
   float m_playfieldReflectionStrength;
   int   m_useTrailForBalls;
   float m_ballTrailStrength;
   float m_ballPlayfieldReflectionStrength;
   int   m_useAA;
   int   m_useFXAA;
   int   m_useAO;
   float m_bloom_strength;

   HWND  m_hMaterialManager;
   SearchSelectDialog m_searchSelectDlg;

   bool  m_fDirtyDraw; // Whether our background bitmap is up to date

   bool  m_activeLayers[8];
   bool  m_toggleAllLayers;
   volatile bool m_savingActive;

   bool  m_renderSolid;

   bool  m_fGrid; // Display grid or not
   bool  m_fBackdrop;
   bool  m_fRenderDecals;
   bool  m_fRenderEMReels;
   bool  m_overwriteGlobalStereo3D;
   bool  m_fReflectElementsOnPlayfield;
   bool  m_fReflectionEnabled;

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
   
   virtual LRESULT WndProc(UINT uMsg, WPARAM wParam, LPARAM lParam);

private:
   std::tr1::unordered_map<const char*, Texture*, StringHashFunctor, StringComparator> m_textureMap;      // hash table to speed up texture lookup by name
   std::tr1::unordered_map<const char*, Material*, StringHashFunctor, StringComparator> m_materialMap;    // hash table to speed up material lookup by name
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

   STDMETHOD(get_Version)(/*[out, retval]*/ int *pVal);
   STDMETHOD(get_VPBuildVersion)(/*[out, retval]*/ int *pVal);
   STDMETHOD(get_VersionMajor)(/*[out, retval]*/ int *pVal);
   STDMETHOD(get_VersionMinor)(/*[out, retval]*/ int *pVal);
   STDMETHOD(get_VersionRevision)(/*[out, retval]*/ int *pVal);

   STDMETHOD(MaterialColor)(BSTR pVal, OLE_COLOR newVal);

   void Init(PinTable *pt);

   virtual IDispatch *GetDispatch();

   virtual ISelect *GetISelect() { return NULL; }

   BOOL GetTextFileFromDirectory(char *szfilename, char *dirname, BSTR *pContents);

   BEGIN_COM_MAP(ScriptGlobalTable)
      COM_INTERFACE_ENTRY(ITableGlobal)
      COM_INTERFACE_ENTRY(IDispatch)
   END_COM_MAP()

   PinTable *m_pt;
};

#endif // !defined(AFX_PINTABLE_H__D14A2DAB_2984_4FE7_A102_D0283ECE31B4__INCLUDED_)
