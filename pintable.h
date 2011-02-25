// PinTable.h: interface for the PinTable class.
//
//////////////////////////////////////////////////////////////////////
#pragma once

#if !defined(AFX_PINTABLE_H__D14A2DAB_2984_4FE7_A102_D0283ECE31B4__INCLUDED_)
#define AFX_PINTABLE_H__D14A2DAB_2984_4FE7_A102_D0283ECE31B4__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#define MIN_ZOOM 0.126f // purposely make them offset from powers to 2 to account for roundoff error
#define MAX_ZOOM 63.9f
int DeadZ;

#define MAX_TEXTURE_SIZE 4096

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
	public IDispatchImpl<ITable, &IID_ITable, &LIBID_VBATESTLib>,
#ifdef VBA
	public CApcProject<PinTable>,
	public CApcProjectItem<PinTable>,
#endif
	public IConnectionPointContainerImpl<PinTable>,
	public EventProxy<PinTable, &DIID_ITableEvents>,
	// IProvideClassInfo provides an ITypeInfo for the whole coclass
	// allowing VBScript to get the set of events to sync to.
	// VBA does not need this interface for some reason
	public IProvideClassInfo2Impl<&CLSID_Table, &DIID_ITableEvents, &LIBID_VBATESTLib>,
	public ISelect,
	//public IUndo,
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
	STDMETHOD(get_Xlatex)(/*[out, retval]*/ float *pVal);
	STDMETHOD(put_Xlatex)(/*[in]*/ float newVal);
	STDMETHOD(get_Xlatey)(/*[out, retval]*/ float *pVal);
	STDMETHOD(put_Xlatey)(/*[in]*/ float newVal);
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

	STDMETHOD(PlaySound)(BSTR bstr, int loopcount, float volume);
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
	STDMETHOD(get_RenderShadows)(/*[out, retval]*/ VARIANT_BOOL *pVal);
	STDMETHOD(put_RenderShadows)(/*[in]*/ VARIANT_BOOL newVal);

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
	STDMETHOD(get_AccelerManualAmp)(/*[out, retval]*/ float *pVal);
	STDMETHOD(put_AccelerManualAmp)(/*[in]*/ float newVal);
	STDMETHOD(get_GlobalDifficulty)(/*[out, retval]*/ float *pVal);
	STDMETHOD(put_GlobalDifficulty)(/*[in]*/ float newVal);
	STDMETHOD(get_TableCaching)(/*[out, retval]*/ VARIANT_BOOL *pVal);
	STDMETHOD(put_TableCaching)(/*[in]*/ VARIANT_BOOL newVal);

	STDMETHOD(get_HardwareRender)(/*[out, retval]*/ VARIANT_BOOL *pVal);
	STDMETHOD(put_HardwareRender)(/*[in]*/ VARIANT_BOOL newVal);
	STDMETHOD(get_AlternateRender)(/*[out, retval]*/ VARIANT_BOOL *pVal);
	STDMETHOD(put_AlternateRender)(/*[in]*/ VARIANT_BOOL newVal);
	STDMETHOD(get_UseD3DBlit)(/*[out, retval]*/ VARIANT_BOOL *pVal);
	STDMETHOD(put_UseD3DBlit)(/*[in]*/ VARIANT_BOOL newVal);
	/////////////////////////////////////////////

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
	
	/////////////////////////////////////////////
	PinTable();
	virtual ~PinTable();

	void Init(VPinball *pvp);
	void InitPostLoad(VPinball *pvp);

	virtual HRESULT GetTypeName(BSTR *pVal);
	//virtual int GetDialogID();
	virtual void GetDialogPanes(Vector<PropertyPane> *pvproppane);

	void CreateTableWindow();
	void SetCaption(char *szCaption);

	HRESULT InitVBA();
	void CloseVBA();

	void Render(Sur * psur);
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
	//void RemoveListSound(HWND hwndListView, PinSound *pps);
	void RemoveSound(PinSound *pps);
	HRESULT SaveSoundToStream(PinSound *pps, IStream *pstm);
	HRESULT LoadSoundFromStream(IStream *pstm);
	void ClearOldSounds();
	bool ExportImage(HWND hwndListView, PinImage *ppi, char *filename);
	void ImportImage(HWND hwndListView, char *filename);
	void ReImportImage(HWND hwndListView, PinImage *ppi, char *filename);
	void ListImages(HWND hwndListView);
	int AddListImage(HWND hwndListView, PinImage *ppi);
	//void RemoveListSound(HWND hwndListView, PinSound *pps);
	void RemoveImage(PinImage *ppi);
	//HRESULT SaveImageToStream(PinImage *ppi, IStream *pstm);
	HRESULT LoadImageFromStream(IStream *pstm, int version);
	PinImage *GetImage(char *szName);
	void GetTVTU(PinImage *ppi, float *pmaxtu, float *pmaxtv);
	void CreateGDIBackdrop();
	int GetImageLink(PinImage *ppi);
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
	//void DoCommandToObject(int x, int y, int command, ISelect *psel);

	virtual void SelectItem(IScriptable *piscript);
	virtual void DoCodeViewCommand(int command);
	virtual void SetDirtyScript(SaveDirtyState sds);
	//virtual void UnsetDirtyScript();

	// Multi-object manipulation
	virtual void GetCenter(Vertex2D *pv);
	virtual void PutCenter(Vertex2D *pv);
	virtual void FlipY(Vertex2D * const pvCenter);
	virtual void FlipX(Vertex2D * const pvCenter);
	virtual void Rotate(float ang, Vertex2D *pvCenter);
	virtual void Scale(float scalex, float scaley, Vertex2D *pvCenter);
	virtual void Translate(Vertex2D *pvOffset);

	// IEditable (mostly bogus for now)
	virtual void PreRender(Sur *psur);
	virtual ItemTypeEnum GetItemType();
	virtual HRESULT InitLoad(IStream *pstm, PinTable *ptable, int *pid, int version, HCRYPTHASH hcrypthash, HCRYPTKEY hcryptkey);
	virtual HRESULT InitPostLoad();
	virtual HRESULT InitVBA(BOOL fNew, int id, WCHAR *wzName);
	virtual ISelect *GetISelect();
	virtual void SetDefaults(bool fromMouseClick);
	virtual IScriptable *GetScriptable();

	virtual PinTable *GetPTable() {return this;}

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

	STDMETHOD(get_GridSize)(float *pgs);
	STDMETHOD(put_GridSize)(float gs);

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
	void GetUniqueName(int type, WCHAR *wzOriginalName, WCHAR *wzUniqueName);

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
	float m_gridsize;
	float m_offsetx;
	float m_offsety;
	float m_zoom;
	//ISelect *m_pselcur;
	Vector<ISelect> m_vmultisel;
	
	bool  m_Shake;		// Is the "Earthshaker" effect active.  This will affect nudge (ball physics) and the render.

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
	float m_xlatex;
	float m_xlatey;
	float m_scalex;
	float m_scaley;
	float m_angletiltMax;
	float m_angletiltMin;

	float m_Gravity;
	float m_hardFriction;
	float m_hardScatter;
	float m_maxBallSpeed;
	float m_dampingFriction;
	int m_plungerNormalize;
	BOOL m_plungerFilter;
	int m_PhysicsLoopTime;

    static int m_tblNumStartBalls;	// The number of balls that are on the table at startup minus the current player ball.
	static int NumStartBalls();		// Returns the number of start balls.
	BOOL m_tblAccelerometer;		// true if electronic accelerometer enabled
	BOOL m_tblAccelNormalMount;		// true is Normal Mounting (Left Hand Coordinates)
    BOOL m_tblAutoStartEnabled;
    BOOL m_tblMirrorEnabled;		// Mirror tables left to right.  This is activated by a cheat during table selection.
	float m_tblAccelAngle;			// 0 degrees rotated counterclockwise (GUI is lefthand coordinates)
	float m_tblAccelAmp;			// Accelerometer gain 
	float m_tblAccelManualAmp;		// manual input gain, generally from joysticks
    float m_tblAutoStart;           // seconds before trying an autostart if doing once-only method .. 0 is automethod
    float m_tblAutoStartRetry;      // seconds before retrying to autostart.
    float m_tblVolmod;              // volume modulation for doing audio balancing
    float m_tblExitConfirm;         // seconds before trying an autostart if doing once-only method .. 0 is automethod

	float m_globalDifficulty;		// Table Difficulty Level
    float m_timeout;                // 0 means no timeout, otherwise in units of seconds when the table timeout should occur
	BOOL m_TableCaching;			// Table Render Cacheing
	BOOL m_CacheEnabled;			// Table Render Cacheing

    // 1.0f means constantly tilting (an invalid/degenerative state)
    // 0.0f means will never tilt
    F32 m_tiltsens;  // 0.0f to 1.0f

    // NOTE: m_nudgesens is not strictly needed
    // as is duplicate data as in the joystick calibration
    // but it is here for completeness
    F32 m_nudgesens; // 0.0f to 1.0f

    int m_units_coin1;				// The number of units added on an event from coin switch 1.
    int m_units_coin2;				// The number of units added on an event from coin switch 2.
    int m_units_bonus;				// The number of units required for a bonus credit.
    int m_units_credit;				// The number of units for a credit.
    int m_custom_coins;
	
	int m_jolt_amount;
	int m_tilt_amount;
	int DeadZ;
	int m_jolt_trigger_time;
	int m_tilt_trigger_time;

	BOOL	m_fRenderShadows;
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

	Vector< PinImage > m_vimage;

	Vector< PinSound > m_vsound;

	Vector< PinFont > m_vfont;

	Vector< CComObject<Collection> > m_vcollection;

	COLORREF rgcolorcustom[16];		// array for the choosecolor in property browser

	Vector< PinSoundCopy > m_voldsound; // copied sounds currently playing

	//HBITMAP m_hbmBackdrop;

	BOOL m_fGrid; // Display grid or not
	BOOL m_fBackdrop;

	FRect m_rcDragRect; // Multi-select

	HBITMAP m_hbmOffScreen; // Buffer for drawing the editor window
	BOOL m_fDirtyDraw; // Whether our background bitmap is up to date

	//UndoRecord* m_pstgBackup; // back up of object data during play, for restoring original state afterwards

	PinUndo m_undo;

	CComObject<CodeViewer> *m_pcv;

	CComObject<ScriptGlobalTable> *m_psgt; // Object to expose to script for global functions

	int m_suffixcount[eItemTypeCount]; // For finding unique item names when adding an item

	SaveDirtyState m_sdsDirtyProp;
	SaveDirtyState m_sdsDirtyScript;
	SaveDirtyState m_sdsNonUndoableDirty;
	SaveDirtyState m_sdsCurrentDirtyState;

	//BOOL m_fScriptDirtyBeforePlay;

	//temp storage for resolved name
	WCHAR mp_resolvedName[MAXNAMEBUFFER];

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
	char m_szScreenShot[MAXTOKEN];

	PinBinary *m_pbTempScreenshot; // Holds contents of screenshot image until the image asks for it
	//PinBinary m_pbScreenSound;

	Vector<char> m_vCustomInfoTag;
	Vector<char> m_vCustomInfoContent;

	VectorInt<HANDLE> m_vAsyncHandles;

	int m_alphaRampsAccuracy;
};

#endif // !defined(AFX_PINTABLE_H__D14A2DAB_2984_4FE7_A102_D0283ECE31B4__INCLUDED_)
