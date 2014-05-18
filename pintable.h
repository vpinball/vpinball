// PinTable.h: interface for the PinTable class.
//
//////////////////////////////////////////////////////////////////////
#pragma once
#if !defined(AFX_PINTABLE_H__D14A2DAB_2984_4FE7_A102_D0283ECE31B4__INCLUDED_)
#define AFX_PINTABLE_H__D14A2DAB_2984_4FE7_A102_D0283ECE31B4__INCLUDED_

#include <unordered_map>
#include "hash.h"

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

enum LightType {
	LIGHT_DIRECTIONAL = 0,
	LIGHT_SPOT = 1,
	LIGHT_POINT = 2
};

struct LightSource {
	COLORREF ambient;
	COLORREF diffuse;
	COLORREF specular;
	Vertex3Ds pos;
	Vertex3Ds dir;
	LightType type;
	bool enabled;
};

typedef struct {
	long			fileversion;
	long			size;
	unsigned char	paraphrase[PROT_CIPHER_LENGTH];
	unsigned long	flags;
	int				keyversion;
	int				spare1;
	int				spare2;
} _protectionData;

class PinTable :
	public CComObjectRootEx<CComSingleThreadModel>,
	public IDispatchImpl<ITable, &IID_ITable, &LIBID_VPinballLib>,
#ifdef VBA
	public CApcProject<PinTable>,
	public CApcProjectItem<PinTable>,
#endif
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
	STDMETHOD(get_BallBackDecal)(/*[out, retval]*/ BSTR *pVal);
	STDMETHOD(put_BallBackDecal)(/*[in]*/ BSTR newVal);
	STDMETHOD(get_BallFrontDecal)(/*[out, retval]*/ BSTR *pVal);
	STDMETHOD(put_BallFrontDecal)(/*[in]*/ BSTR newVal);
	STDMETHOD(get_YieldTime)(/*[out, retval]*/ long *pVal);
	STDMETHOD(put_YieldTime)(/*[in]*/ long newVal);
	STDMETHOD(get_BallImage)(/*[out, retval]*/ BSTR *pVal);
	STDMETHOD(put_BallImage)(/*[in]*/ BSTR newVal);
	
	STDMETHOD(get_Gravity)(/*[out, retval]*/ float *pVal);
	STDMETHOD(put_Gravity)(/*[in]*/ float newVal);
	STDMETHOD(get_HardFriction)(/*[out, retval]*/ float *pVal);
	STDMETHOD(put_HardFriction)(/*[in]*/ float newVal);
	STDMETHOD(get_HardScatter)(/*[out, retval]*/ float *pVal);
	STDMETHOD(put_HardScatter)(/*[in]*/ float newVal);
	STDMETHOD(get_MaxBallSpeed)(/*[out, retval]*/ float *pVal);
	STDMETHOD(put_MaxBallSpeed)(/*[in]*/ float newVal);
	STDMETHOD(get_DampingFriction)(/*[out, retval]*/ float *pVal);
	STDMETHOD(put_DampingFriction)(/*[in]*/ float newVal);
	STDMETHOD(get_PlungerNormalize)(/*[out, retval]*/ int *pVal);
	STDMETHOD(put_PlungerNormalize)(/*[in]*/ int newVal);

	STDMETHOD(get_PlungerFilter)(/*[out, retval]*/ VARIANT_BOOL *pVal);
	STDMETHOD(put_PlungerFilter)(/*[in]*/ VARIANT_BOOL newVal);

	STDMETHOD(get_PhysicsLoopTime)(/*[out, retval]*/ int *pVal);
	STDMETHOD(put_PhysicsLoopTime)(/*[in]*/ int newVal);

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
	STDMETHOD(get_Rotation)(/*[out, retval]*/ float *pVal);
	STDMETHOD(put_Rotation)(/*[in]*/ float newVal);
	STDMETHOD(get_SlopeMax)(/*[out, retval]*/ float *pVal);
	STDMETHOD(put_SlopeMax)(/*[in]*/ float newVal);
	STDMETHOD(get_SlopeMin)(/*[out, retval]*/ float *pVal);
	STDMETHOD(put_SlopeMin)(/*[in]*/ float newVal);
	STDMETHOD(get_BackdropImage)(/*[out, retval]*/ BSTR *pVal);
	STDMETHOD(put_BackdropImage)(/*[in]*/ BSTR newVal);
	STDMETHOD(get_BackdropColor)(/*[out, retval]*/ OLE_COLOR *pVal);
	STDMETHOD(put_BackdropColor)(/*[in]*/ OLE_COLOR newVal);
	STDMETHOD(get_PlayfieldColor)(/*[out, retval]*/ OLE_COLOR *pVal);
	STDMETHOD(put_PlayfieldColor)(/*[in]*/ OLE_COLOR newVal);

	STDMETHOD(get_Light0Ambient)(/*[out, retval]*/ OLE_COLOR *pVal);
	STDMETHOD(put_Light0Ambient)(/*[in]*/ OLE_COLOR newVal);
	STDMETHOD(get_Light0Diffuse)(/*[out, retval]*/ OLE_COLOR *pVal);
	STDMETHOD(put_Light0Diffuse)(/*[in]*/ OLE_COLOR newVal);
	STDMETHOD(get_Light0Specular)(/*[out, retval]*/ OLE_COLOR *pVal);
	STDMETHOD(put_Light0Specular)(/*[in]*/ OLE_COLOR newVal);
	STDMETHOD(get_Light0PX)(/*[out, retval]*/ float *pVal);
	STDMETHOD(put_Light0PX)(/*[in]*/ float newVal);
	STDMETHOD(get_Light0PY)(/*[out, retval]*/ float *pVal);
	STDMETHOD(put_Light0PY)(/*[in]*/ float newVal);
	STDMETHOD(get_Light0PZ)(/*[out, retval]*/ float *pVal);
	STDMETHOD(put_Light0PZ)(/*[in]*/ float newVal);
	STDMETHOD(get_Light0DX)(/*[out, retval]*/ float *pVal);
	STDMETHOD(put_Light0DX)(/*[in]*/ float newVal);
	STDMETHOD(get_Light0DY)(/*[out, retval]*/ float *pVal);
	STDMETHOD(put_Light0DY)(/*[in]*/ float newVal);
	STDMETHOD(get_Light0DZ)(/*[out, retval]*/ float *pVal);
	STDMETHOD(put_Light0DZ)(/*[in]*/ float newVal);
	STDMETHOD(get_Light0Type)(/*[out, retval]*/ int *pVal);
	STDMETHOD(put_Light0Type)(/*[in]*/ int newVal);
	STDMETHOD(get_Light1Ambient)(/*[out, retval]*/ OLE_COLOR *pVal);
	STDMETHOD(put_Light1Ambient)(/*[in]*/ OLE_COLOR newVal);
	STDMETHOD(get_Light1Diffuse)(/*[out, retval]*/ OLE_COLOR *pVal);
	STDMETHOD(put_Light1Diffuse)(/*[in]*/ OLE_COLOR newVal);
	STDMETHOD(get_Light1Specular)(/*[out, retval]*/ OLE_COLOR *pVal);
	STDMETHOD(put_Light1Specular)(/*[in]*/ OLE_COLOR newVal);
	STDMETHOD(get_Light1PX)(/*[out, retval]*/ float *pVal);
	STDMETHOD(put_Light1PX)(/*[in]*/ float newVal);
	STDMETHOD(get_Light1PY)(/*[out, retval]*/ float *pVal);
	STDMETHOD(put_Light1PY)(/*[in]*/ float newVal);
	STDMETHOD(get_Light1PZ)(/*[out, retval]*/ float *pVal);
	STDMETHOD(put_Light1PZ)(/*[in]*/ float newVal);
	STDMETHOD(get_Light1DX)(/*[out, retval]*/ float *pVal);
	STDMETHOD(put_Light1DX)(/*[in]*/ float newVal);
	STDMETHOD(get_Light1DY)(/*[out, retval]*/ float *pVal);
	STDMETHOD(put_Light1DY)(/*[in]*/ float newVal);
	STDMETHOD(get_Light1DZ)(/*[out, retval]*/ float *pVal);
	STDMETHOD(put_Light1DZ)(/*[in]*/ float newVal);
	STDMETHOD(get_Light1Type)(/*[out, retval]*/ int *pVal);
	STDMETHOD(put_Light1Type)(/*[in]*/ int newVal);
    STDMETHOD(get_NormalizeNormals)(/*[out, retval]*/ int *pVal);
    STDMETHOD(put_NormalizeNormals)(/*[in]*/ int newVal);
    STDMETHOD(get_BallReflection)(/*[out, retval]*/ int *pVal);
    STDMETHOD(put_BallReflection)(/*[in]*/ int newVal);
    STDMETHOD(get_ReflectionStrength)(/*[out, retval]*/ int *pVal);
    STDMETHOD(put_ReflectionStrength)(/*[in]*/ int newVal);
    STDMETHOD(get_ShadowX)(/*[out, retval]*/ float *pVal);
    STDMETHOD(put_ShadowX)(/*[in]*/ float newVal);
    STDMETHOD(get_ShadowY)(/*[out, retval]*/ float *pVal);
    STDMETHOD(put_ShadowY)(/*[in]*/ float newVal);
	STDMETHOD(get_BallTrail)(/*[out, retval]*/ int *pVal);
    STDMETHOD(put_BallTrail)(/*[in]*/ int newVal);
    STDMETHOD(get_TrailStrength)(/*[out, retval]*/ int *pVal);
    STDMETHOD(put_TrailStrength)(/*[in]*/ int newVal);
    
	STDMETHOD(get_Height)(/*[out, retval]*/ float *pVal);
	STDMETHOD(put_Height)(/*[in]*/ float newVal);
	STDMETHOD(get_Width)(/*[out, retval]*/ float *pVal);
	STDMETHOD(put_Width)(/*[in]*/ float newVal);
	STDMETHOD(get_GlassHeight)(/*[out, retval]*/ float *pVal);
	STDMETHOD(put_GlassHeight)(/*[in]*/ float newVal);
	STDMETHOD(get_TableHeight)(/*[out, retval]*/ float *pVal);
	STDMETHOD(put_TableHeight)(/*[in]*/ float newVal);
	STDMETHOD(Nudge)(float Angle, float Force);
	STDMETHOD(get_DisplayBackdrop)(/*[out, retval]*/ VARIANT_BOOL *pVal);
	STDMETHOD(put_DisplayBackdrop)(/*[in]*/ VARIANT_BOOL newVal);
	STDMETHOD(get_DisplayGrid)(/*[out, retval]*/ VARIANT_BOOL *pVal);
	STDMETHOD(put_DisplayGrid)(/*[in]*/ VARIANT_BOOL newVal);
	STDMETHOD(get_Image)(/*[out, retval]*/ BSTR *pVal);
	STDMETHOD(put_Image)(/*[in]*/ BSTR newVal);

	STDMETHOD(PlaySound)(BSTR bstr, int loopcount, float volume, float pan, float randompitch, int pitch, VARIANT_BOOL usesame, VARIANT_BOOL restart);
	STDMETHOD(FireKnocker)(/*[in]*/ int Count);
	STDMETHOD(QuitPlayer)(/*[in]*/ int CloseType);
	STDMETHOD(StartShake)(/*[in]*/ void);
	STDMETHOD(StopShake)(/*[in]*/ void);

	STDMETHOD(get_FieldOfView)(/*[out, retval]*/ float *pVal);
	STDMETHOD(put_FieldOfView)(/*[in]*/ float newVal);
	STDMETHOD(get_Inclination)(/*[out, retval]*/ float *pVal);
	STDMETHOD(put_Inclination)(/*[in]*/ float newVal);
	STDMETHOD(get_Layback)(/*[out, retval]*/ float *pVal);
	STDMETHOD(put_Layback)(/*[in]*/ float newVal);
	STDMETHOD(get_Name)(/*[out, retval]*/ BSTR *pVal);
	STDMETHOD(put_Name)(/*[in]*/ BSTR newVal);
    STDMETHOD(get_EnableAntialiasing)(/*[out, retval]*/ int *pVal);
    STDMETHOD(put_EnableAntialiasing)(/*[in]*/ int newVal);
    STDMETHOD(get_EnableFXAA)(/*[out, retval]*/ int *pVal);
    STDMETHOD(put_EnableFXAA)(/*[in]*/ int newVal);
    STDMETHOD(get_RenderShadows)(/*[out, retval]*/ VARIANT_BOOL *pVal);
    STDMETHOD(put_RenderShadows)(/*[in]*/ VARIANT_BOOL newVal);
    
	STDMETHOD(get_OverridePhysics)(/*[out, retval]*/ long *pVal);
    STDMETHOD(put_OverridePhysics)(/*[in]*/ long newVal);

	STDMETHOD(ImportPhysics)();
	STDMETHOD(ExportPhysics)();

	STDMETHOD(get_MaxSeparation)(/*[out, retval]*/ float *pVal);
	STDMETHOD(put_MaxSeparation)(/*[in]*/ float newVal);
	STDMETHOD(get_ZPD)(/*[out, retval]*/ float *pVal);
	STDMETHOD(put_ZPD)(/*[in]*/ float newVal);

	STDMETHOD(get_EnableDecals)(/*[out, retval]*/ VARIANT_BOOL *pVal);
	STDMETHOD(put_EnableDecals)(/*[in]*/ VARIANT_BOOL newVal);
	STDMETHOD(get_EnableEMReels)(/*[out, retval]*/ VARIANT_BOOL *pVal);
	STDMETHOD(put_EnableEMReels)(/*[in]*/ VARIANT_BOOL newVal);

	/////////////////////////////////////////////
	STDMETHOD(get_Accelerometer)(/*[out, retval]*/ VARIANT_BOOL *pVal);
	STDMETHOD(put_Accelerometer)(/*[in]*/ VARIANT_BOOL newVal);	
	STDMETHOD(get_AccelNormalMount)(/*[out, retval]*/ VARIANT_BOOL *pVal);
	STDMETHOD(put_AccelNormalMount)(/*[in]*/ VARIANT_BOOL newVal);
	STDMETHOD(get_AccelerometerAngle)(/*[out, retval]*/ float *pVal); 
	STDMETHOD(put_AccelerometerAngle)(/*[in]*/ float newVal);
	STDMETHOD(get_AccelerometerAmp)(/*[out, retval]*/ float *pVal);
	STDMETHOD(put_AccelerometerAmp)(/*[in]*/ float newVal);
	STDMETHOD(get_AccelerometerAmpX)(/*[out, retval]*/ float *pVal);
	STDMETHOD(put_AccelerometerAmpX)(/*[in]*/ float newVal);
	STDMETHOD(get_AccelerometerAmpY)(/*[out, retval]*/ float *pVal);
	STDMETHOD(put_AccelerometerAmpY)(/*[in]*/ float newVal);
	STDMETHOD(get_AccelerManualAmp)(/*[out, retval]*/ float *pVal);
	STDMETHOD(put_AccelerManualAmp)(/*[in]*/ float newVal);
	STDMETHOD(get_GlobalDifficulty)(/*[out, retval]*/ float *pVal);
	STDMETHOD(put_GlobalDifficulty)(/*[in]*/ float newVal);

	//!! deprecated
	STDMETHOD(get_HardwareRender)(/*[out, retval]*/ VARIANT_BOOL *pVal);
	STDMETHOD(put_HardwareRender)(/*[in]*/ VARIANT_BOOL newVal);
	STDMETHOD(get_AlternateRender)(/*[out, retval]*/ VARIANT_BOOL *pVal);
	STDMETHOD(put_AlternateRender)(/*[in]*/ VARIANT_BOOL newVal);

	STDMETHOD(get_DeadSlider)(/*[out, retval]*/  int *pVal);
	STDMETHOD(put_DeadSlider)(/*[in]*/ int newVal);
	STDMETHOD(get_DeadZone)(/*[out, retval]*/  int *pVal);
	STDMETHOD(put_DeadZone)(/*[in]*/ int newVal);
	STDMETHOD(get_JoltAmount)(/*[out, retval]*/  int *pVal);
	STDMETHOD(put_JoltAmount)(/*[in]*/ int newVal);
	STDMETHOD(get_TiltAmount)(/*[out, retval]*/ int *pVal);
	STDMETHOD(put_TiltAmount)(/*[in]*/ int newVal);
	STDMETHOD(get_JoltTriggerTime)(/*[out, retval]*/ int *pVal);
	STDMETHOD(put_JoltTriggerTime)(/*[in]*/ int newVal);
	STDMETHOD(get_TiltTriggerTime)(/*[out, retval]*/ int *pVal);
	STDMETHOD(put_TiltTriggerTime)(/*[in]*/ int newVal);

	STDMETHOD(get_TableSoundVolume)(/*[out, retval]*/ int *pVal);
	STDMETHOD(put_TableSoundVolume)(/*[in]*/ int newVal);
	STDMETHOD(get_TableMusicVolume)(/*[out, retval]*/ int *pVal);
	STDMETHOD(put_TableMusicVolume)(/*[in]*/ int newVal);

	STDMETHOD(get_TableAdaptiveVSync)(/*[out, retval]*/ int *pVal);
	STDMETHOD(put_TableAdaptiveVSync)(/*[in]*/ int newVal);

	STDMETHOD(get_AlphaRampAccuracy)(/*[out, retval]*/ int *pVal);
    STDMETHOD(put_AlphaRampAccuracy)(/*[in]*/ int newVal);

    STDMETHOD(get_GlobalAlphaAcc)(/*[out, retval]*/ VARIANT_BOOL *pVal);
    STDMETHOD(put_GlobalAlphaAcc)(/*[in]*/ VARIANT_BOOL newVal);

    STDMETHOD(get_GlobalStereo3D)(/*[out, retval]*/ VARIANT_BOOL *pVal);
    STDMETHOD(put_GlobalStereo3D)(/*[in]*/ VARIANT_BOOL newVal);

    STDMETHOD(Version)(/*[out, retval]*/ int *pVal);

	/////////////////////////////////////////////

	PinTable();
	virtual ~PinTable();

	void Init(VPinball *pvp);
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

	void Copy();
	void Paste(BOOL fAtLocation, int x, int y);

	void ExportBlueprint();

	//void FireVoidEvent(int dispid);
	void FireKeyEvent(int dispid, int keycode);

	void Play();
	void StopPlaying();

	void ImportSound(HWND hwndListView, char *filename, BOOL fPlay);
	void ReImportSound(HWND hwndListView, PinSound *pps, char *filename, BOOL fPlay);
	bool ExportSound(HWND hwndListView, PinSound *pps,char *filename);
	void ListSounds(HWND hwndListView);
	int AddListSound(HWND hwndListView, PinSound *pps);
	void RemoveSound(PinSound *pps);
	HRESULT SaveSoundToStream(PinSound *pps, IStream *pstm);
	HRESULT LoadSoundFromStream(IStream *pstm);
	void ClearOldSounds();
	bool ExportImage(HWND hwndListView, Texture *ppi, char *filename);
	void ImportImage(HWND hwndListView, char *filename);
	void ReImportImage(HWND hwndListView, Texture *ppi, char *filename);
	void ListImages(HWND hwndListView);
	int AddListImage(HWND hwndListView, Texture *ppi);
	void RemoveImage(Texture *ppi);
	HRESULT LoadImageFromStream(IStream *pstm, int version);
	Texture *GetImage(char *szName);
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
	BOOL FMutilSelLocked();

	virtual void SelectItem(IScriptable *piscript);
	virtual void DoCodeViewCommand(int command);
	virtual void SetDirtyScript(SaveDirtyState sds);

	// Multi-object manipulation
	virtual void GetCenter(Vertex2D * const pv) const;
	virtual void PutCenter(const Vertex2D * const pv);
	virtual void FlipY(Vertex2D * const pvCenter);
	virtual void FlipX(Vertex2D * const pvCenter);
	virtual void Rotate(float ang, Vertex2D *pvCenter);
	virtual void Scale(float scalex, float scaley, Vertex2D *pvCenter);
	virtual void Translate(Vertex2D *pvOffset);

	// IEditable (mostly bogus for now)
	virtual void PreRender(Sur * const psur);
	virtual ItemTypeEnum GetItemType();
	virtual HRESULT InitLoad(IStream *pstm, PinTable *ptable, int *pid, int version, HCRYPTHASH hcrypthash, HCRYPTKEY hcryptkey);
	virtual HRESULT InitPostLoad();
	virtual HRESULT InitVBA(BOOL fNew, int id, WCHAR *wzName);
	virtual ISelect *GetISelect();
	virtual void SetDefaults(bool fromMouseClick);
	virtual IScriptable *GetScriptable();

	virtual PinTable *GetPTable() {return this;}
    char *GetElementName( IEditable *pedit );

	void OnDelete();

	void DoLButtonDown(int x,int y);
	void DoLButtonUp(int x,int y);
	void DoRButtonDown(int x,int y);
	void DoRButtonUp(int x,int y);
	void DoMouseMove(int x,int y);
	void DoLDoubleClick(int x, int y);
	void UseTool(int x,int y,int tool);
	void OnKeyDown(int key);

	void TransformPoint(int x, int y, Vertex2D *pv);

	void AddMultiSel(ISelect *psel, BOOL fAdd, BOOL fUpdate);
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
	HRESULT CreateIEditableFromType(int type, IEditable **piedit);
	virtual IEditable *GetIEditable() {return (IEditable *)this;}
	virtual void Delete() {} // Can't delete table itself
	virtual void Uncreate() {}
	virtual BOOL LoadToken(int id, BiffReader *pbr);

	virtual IDispatch *GetPrimary() {return this->GetDispatch();}
	virtual IDispatch *GetDispatch() {return (IDispatch *)this;}
	virtual IFireEvents *GetIFireEvents() {return (IFireEvents *)this;}
	virtual IDebugCommands *GetDebugCommands() {return NULL;}

	void SetZoom(float zoom);
	void SetMyScrollInfo();

	void BackupForPlay();
	void RestoreBackup();

	void BeginUndo();
	void EndUndo();
	void Undo();

	void Uncreate(IEditable *pie);
	void Undelete(IEditable *pie);

	STDMETHOD(GetDisplayString)(DISPID dispID, BSTR *pbstr) {return hrNotImplemented;}
	STDMETHOD(MapPropertyToPage)(DISPID dispID, CLSID *pclsid) {return hrNotImplemented;}
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
	void GetUniqueName(int type, WCHAR *wzUniqueName);
	void GetUniqueNamePasting(int type, WCHAR *wzUniqueName);

	float GetSurfaceHeight(char *szName, float x, float y);

	void SetLoadDefaults();

	void SetDirty(SaveDirtyState sds);
	void SetNonUndoableDirty(SaveDirtyState sds);
	void CheckDirty();
	BOOL FDirty();

	BOOL FVerifySaveToClose();

	HRESULT StopSound(BSTR Sound);

	BOOL CheckPermissions(unsigned long flag);
	BOOL IsTableProtected();
	void ResetProtectionBlock();
	BOOL SetupProtectionBlock(unsigned char *pPassword, unsigned long flags);
	BOOL UnlockProtectionBlock(unsigned char *pPassword);

    void SwitchToLayer(int layerNumber );
    void AssignToLayer(IEditable *obj, int layerNumber );
    void AssignMultiToLayer( int layerNumber, int x, int y );
    void MergeAllLayers();
    void RestoreLayers();
    void BackupLayers();
    void DeleteFromLayer( IEditable *obj );
    void AddToCollection(int index);
    void MoveCollectionUp(CComObject<Collection> *pcol );
    void MoveCollectionDown(CComObject<Collection> *pcol );

    int GetAlphaRampsAccuracy();
    float GetZPD();
    float GetMaxSeparation();

    FRect3D GetBoundingBox();

    bool RenderSolid()      { return m_renderSolid; }

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

	char m_szFileName[_MAX_PATH];
	char m_szBlueprintFileName[_MAX_PATH];
	char m_szTitle[1024];

#ifdef VBA
	IStorage* m_pStg;
#endif

	HWND m_hwnd;
	VPinball *m_pvp;
	float m_offsetx;
	float m_offsety;
	float m_zoom;
	//ISelect *m_pselcur;
	Vector<ISelect> m_vmultisel;
	
	float m_left; // always zero for now
	float m_top; // always zero for now
	float m_right;
	float m_bottom;

	float m_glassheight;
	float m_tableheight;

	BOOL m_fRenderDecals;
	BOOL m_fRenderEMReels;
	float m_rotation;
	float m_inclination;
	float m_layback;
	float m_FOV;
	
	float m_maxSeparation;
	float m_globalMaxSeparation;
	float m_ZPD;
	float m_globalZPD;
    BOOL m_overwriteGlobalStereo3D;

	float m_xlatex;
	float m_xlatey;
    float m_xlatez;
	float m_scalex;
	float m_scaley;
	float m_angletiltMax;
	float m_angletiltMin;

	int m_fOverridePhysics;
    float m_fOverrideGravityConstant;

	float m_Gravity;
	float m_hardFriction;
	float m_hardScatter;
	float m_maxBallSpeed;
	float m_dampingFriction;
	int m_plungerNormalize;
	BOOL m_plungerFilter;
	unsigned int m_PhysicsMaxLoops;

	BOOL m_tblAccelerometer;		// true if electronic accelerometer enabled
	BOOL m_tblAccelNormalMount;		// true is Normal Mounting (Left Hand Coordinates)
    BOOL m_tblAutoStartEnabled;
    BOOL m_tblMirrorEnabled;		// Mirror tables left to right.  This is activated by a cheat during table selection.
	float m_tblAccelAngle;			// 0 degrees rotated counterclockwise (GUI is lefthand coordinates)
	float m_tblAccelAmp;			// Accelerometer gain 
	float m_tblAccelAmpX;			// Accelerometer gain X axis
	float m_tblAccelAmpY;			// Accelerometer gain Y axis
	float m_tblAccelManualAmp;		// manual input gain, generally from joysticks
    U32 m_tblAutoStart;             // msecs before trying an autostart if doing once-only method .. 0 is automethod
    U32 m_tblAutoStartRetry;        // msecs before retrying to autostart.
    float m_tblVolmod;              // volume modulation for doing audio balancing
    U32 m_tblExitConfirm;           // msecs for esc button to be pressed to exit completely
	float m_globalDifficulty;		// Table Difficulty Level

#ifdef ULTRAPIN
    U32 m_timeout;                  // 0 means no timeout, otherwise in units of msecs when the table timeout should occur
#endif

	int m_jolt_amount;
	int m_tilt_amount;
	int m_DeadZ;
	int m_jolt_trigger_time;
	int m_tilt_trigger_time;

    short m_oldMousePosX;
    short m_oldMousePosY;
	BOOL m_fRenderShadows;
	_protectionData	m_protectionData;

	char m_szImage[MAXTOKEN];
	char m_szImageBackdrop[MAXTOKEN];
	COLORREF m_colorplayfield;
	COLORREF m_colorbackdrop;

	char m_szBallImage[MAXTOKEN];
	char m_szBallImageFront[MAXTOKEN];
	char m_szBallImageBack[MAXTOKEN];

	//CComObject<Surface> *m_psur;

	Vector< IEditable > m_vedit;
    Vector< IEditable > m_layer[8];
    Vector< ISelect > m_allHitElements;
      
	Vector< Texture > m_vimage;

	Vector< PinSound > m_vsound;

	Vector< PinFont > m_vfont;

	Vector< CComObject<Collection> > m_vcollection;

	COLORREF m_rgcolorcustom[16];		// array for the choosecolor in property browser

	Vector< PinSoundCopy > m_voldsound; // copied sounds currently playing

	float m_TableSoundVolume;
	float m_TableMusicVolume;

	int m_TableAdaptiveVSync;

	BOOL m_fGrid; // Display grid or not
	BOOL m_fBackdrop;

	FRect m_rcDragRect; // Multi-select

	HBITMAP m_hbmOffScreen; // Buffer for drawing the editor window
	BOOL m_fDirtyDraw; // Whether our background bitmap is up to date

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

	VectorInt<HANDLE> m_vAsyncHandles;

    int m_globalAlphaRampsAccuracy;
    int m_userAlphaRampsAccuracy;
    BOOL m_overwriteGlobalAlphaRampsAccuracy;

	LightSource m_Light[MAX_LIGHT_SOURCES];
	BOOL m_NormalizeNormals;
    int m_useReflectionForBalls;
    int m_ballReflectionStrength;
    int m_useTrailForBalls;
    int m_ballTrailStrength;
    int m_useAA;
    int m_useFXAA;

    float m_zScale;

	float m_shadowDirX;
    float m_shadowDirY;
	bool m_Shake;		// Is the "Earthshaker" effect active.  This will affect nudge (ball physics) and the render.

	bool m_activeLayers[8];
    bool m_toggleAllLayers;   
    bool m_savingActive;

    bool m_renderSolid;

private:
    std::tr1::unordered_map<const char*, Texture*, StringHashFunctor, StringComparator> m_textureMap;      // hash table to speed up texture lookup by name
};

#endif // !defined(AFX_PINTABLE_H__D14A2DAB_2984_4FE7_A102_D0283ECE31B4__INCLUDED_)
