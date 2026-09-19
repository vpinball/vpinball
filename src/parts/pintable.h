// license:GPLv3+

// interface for the PinTable class.

#pragma once


#include "parts/Collection.h"
#include "parts/pinbinary.h"
#include "renderer/RenderProbe.h"
#include "renderer/ViewSetup.h"
#include "unordered_dense.h"
#include "utils/eventproxy.h"
#include "utils/fileio.h"
#include "utils/hash.h"

#include "pole/pole.h"

#include <atomic>

#ifdef __STANDALONE__
#include <iostream>
class Light;
#endif



#define MIN_ZOOM 0.126f // purposely make them offset from powers to 2 to account for roundoff error
#define MAX_ZOOM 63.9f

#define DISABLE_SCRIPT_EDITING 0x00000002 // cannot open script windows (stops editing and viewing)
#define DISABLE_EVERYTHING 0x80000000 // everything is off limits (including future locks)

struct LightSource
{
   COLORREF emission;
   Vertex3Ds pos;
};

struct WhereUsedInfo
{
   string searchObjectName;      // Source object to search for (images, materials etc)
   string whereUsedObjectname;   // Table object (Bumpers, Lights, Ramps, Walls etc) found to be using the source object
   string whereUsedPropertyName; // Property name where used (If searching for images this could be 'Image', 'Side Image' etc.  If search for materials this could be 'Material', 'Cap Material, 'Base Material' etc.
};

namespace VPX
{
class Sound;
};
class Texture;
class Material;
class Collection;
class Flipper;

class VPXFileFeedback;
namespace VPX::InGameUI { class InGameUIItem; }

class PinTable : public CComObjectRootEx<CComSingleThreadModel>,
                 public IDispatchImpl<ITable, &IID_ITable, &LIBID_VPinballLib>,
                 public IConnectionPointContainerImpl<PinTable>,
                 public EventProxy<PinTable, &DIID_ITableEvents>,
                 // IProvideClassInfo provides an ITypeInfo for the whole coclass
                 // allowing VBScript to get the set of events to sync to.
                 // VBA does not need this interface for some reason
                 public IProvideClassInfo2Impl<&CLSID_Table, &DIID_ITableEvents, &LIBID_VPinballLib>,
                 public IScriptable,
                 public IEditable,
                 public IPerPropertyBrowsing // Ability to fill in dropdown in property browser
{
public:
#ifdef __STANDALONE__
   STDMETHOD(GetIDsOfNames)(REFIID /*riid*/, LPOLESTR* rgszNames, UINT cNames, LCID lcid,DISPID* rgDispId);
   STDMETHOD(Invoke)(DISPID dispIdMember, REFIID /*riid*/, LCID lcid, WORD wFlags, DISPPARAMS* pDispParams, VARIANT* pVarResult, EXCEPINFO* pExcepInfo, UINT* puArgErr);
   STDMETHOD(GetDocumentation)(MEMBERID index, BSTR *pBstrName, BSTR *pBstrDocString, DWORD *pdwHelpContext, BSTR *pBstrHelpFile);
   HRESULT FireDispID(const DISPID dispid, DISPPARAMS * const pdispparams) final;
#endif
   STDMETHOD(get_BallFrontDecal)(/*[out, retval]*/ BSTR *pVal);
   STDMETHOD(put_BallFrontDecal)(/*[in]*/ BSTR newVal);
   // deprecated
   STDMETHOD(get_YieldTime)(/*[out, retval]*/ LONG *pVal);
   // deprecated
   STDMETHOD(put_YieldTime)(/*[in]*/ LONG newVal);
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
   STDMETHOD(get_Image)(/*[out, retval]*/ BSTR *pVal);
   STDMETHOD(put_Image)(/*[in]*/ BSTR newVal);

   STDMETHOD(PlaySound)(BSTR soundName, int loopcount, float volume, float pan, float randompitch, int pitch, VARIANT_BOOL usesame, VARIANT_BOOL restart, float front_rear_fade);
   STDMETHOD(StopSound)(BSTR soundName);
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

   struct TableOption
   {
      const VPX::Properties::PropertyRegistry::PropId id;
      const float displayScale;
      const string format;
      float value;
   };
   const vector<TableOption>& GetOptions() const;
   void SetOptionLiveValue(VPX::Properties::PropertyRegistry::PropId id, float value); // Live value (not persisted unlike the script API put_Option which directly persist the option value)
   STDMETHOD(get_Option)(BSTR optionName, float minValue, float maxValue, float step, float defaultValue, int unit, /*[optional][in]*/ VARIANT values, /*[out, retval]*/ float *param);
   STDMETHOD(put_Option)(BSTR optionName, float minValue, float maxValue, float step, float defaultValue, int unit, /*[optional][in]*/ VARIANT values, /*[in]*/ float val);

   /////////////////////////////////////////////

   PinTable();
   ~PinTable() override;

public:
   void RemoveInvalidReferences();

   void SetDirtyDraw();

   bool GetDecalsEnabled()  const { return m_renderDecals; }  // Enable backdrop image, decals and lights on backdrop
   bool GetEMReelsEnabled() const { return m_renderEMReels; } // Enable dispreel on backdrop

   void ImportBackdropPOV(const std::filesystem::path &filename, const bool toUserSettings);
   void ExportBackdropPOV(const std::filesystem::path &filename) const;

   static std::array<string, 18> VPPelementNames; // names of the fields in a .vpp file
   void ImportVPP(const std::filesystem::path &filename);
   void ExportVPP(const std::filesystem::path &filename, Flipper *const flipper);

   enum class OptionEventType { Initialized, Changed, Reseted, EndOfEdit };
   void FireOptionEvent(OptionEventType event);

   VPX::Sound *ImportSound(const std::filesystem::path &filename);
   void ReImportSound(VPX::Sound *const pps, const std::filesystem::path &filename);
   bool ExportSound(VPX::Sound *const pps, const std::filesystem::path &filename);
   void RemoveSound(VPX::Sound *const pps);
   Texture* ImportImage(const std::filesystem::path &filename, const string &imageName);
   void RemoveImage(Texture *const ppi);

   Texture *GetImage(const string &szName) const;
   bool GetImageLink(const Texture *const ppi) const;
   PinBinary *GetImageLinkBinary(const int id);
   Light *GetLight(const string &szName) const;
   RenderProbe *GetRenderProbe(const string &szName) const;

   void ShowWhereImagesUsed(vector<WhereUsedInfo> &);
   void ShowWhereImageUsed(vector<WhereUsedInfo> &,Texture *const ppi);
   void ShowWhereMaterialsUsed(vector<WhereUsedInfo> &);
   void ShowWhereMaterialUsed(vector<WhereUsedInfo> &, Material *const ppi);

   void ParseScript(const string &script, vector<string> &functions, vector<string> &identifiers, const std::function<void(const string &, int)>& onDuplicate) const;
   string AuditTable(bool log) const;

   void AddFont(PinFont *const ppf);
   void RemoveFont(PinFont *const ppf);
   const vector<PinFont *> &GetFontList() const { return m_vfont; }

   // Expected by CodeViewer
   void SetDirtyScript(SaveDirtyState sds);

   void ExportMesh(ObjLoader &loader) final;

#pragma region IEditable
public:
   PinTable *GetPTable() final { return this; }
   const PinTable *GetPTable() const final { return this; }
   IHitable *GetIHitable() final { return nullptr; }
   const IHitable *GetIHitable() const final { return nullptr; }
   IRenderable *GetIRenderable() final { return nullptr; }
   const IRenderable *GetIRenderable() const final { return nullptr; }
   IScriptable *GetIScriptable() final { return static_cast<IScriptable *>(this); }
   const IScriptable *GetIScriptable() const final { return static_cast<const IScriptable *>(this); }
   IFireEvents *GetIFireEvents() final { return nullptr; }
   ItemTypeEnum GetItemType() const final { return eItemTable; }
   static inline constexpr ItemTypeEnum ItemType = eItemTable;
   void SetDefaults(const bool fromMouseClick) final { }
   void SetDefaultPhysics(const bool fromMouseClick) final;
   void WriteRegDefaults() final { }
   EventProxyBase *GetEventProxyBase() final { return static_cast<EventProxyBase *>(this); }
   void Save(IObjectWriter &writer, const bool saveForUndo) final;
   void ClearForOverwrite() final;
   void Load(IObjectReader &reader) final;
   PinTable *CopyForPlay() const final; // Not exactly const as we add a reference, and copied table keeps a non const reference on this
   Vertex2D GetCenter() const final { return { 0.f, 0.f }; }
   void FlipY(const Vertex2D &pvCenter) final { }
   void FlipX(const Vertex2D &pvCenter) final { }
   void Rotate(const float ang, const Vertex2D &pvCenter, const bool useElementCenter) final { }
   void Scale(const float scalex, const float scaley, const Vertex2D &pvCenter, const bool useElementCenter) final { }
   void Translate(const Vertex2D &offset) final { }
#pragma endregion

#pragma region IScriptable
   IDispatch *GetIDispatch() final { return static_cast<IDispatch *>(this); }
   const IDispatch *GetIDispatch() const final { return static_cast<const IDispatch *>(this); }
#pragma endregion

#pragma region IPerPropertyBrowsing
   // FIXME Remove as this is unmaintained deadcode (no internal property page, no will for VPX to offer a data model for an external COM editor)
   STDMETHOD(GetDisplayString)(DISPID dispID, BSTR *pbstr) { return ResultFromScode(E_NOTIMPL); }
   STDMETHOD(MapPropertyToPage)(DISPID dispID, CLSID *pclsid) { return ResultFromScode(E_NOTIMPL); }
   STDMETHOD(GetPredefinedStrings)(DISPID dispID, CALPOLESTR *pcaStringsOut, CADWORD *pcaCookiesOut);
   STDMETHOD(GetPredefinedValue)(DISPID dispID, DWORD dwCookie, VARIANT *pVarOut);
   STDMETHOD(GetPredefinedStrings)(DISPID dispID, CALPOLESTR *pcaStringsOut, CADWORD *pcaCookiesOut, IEditable *piedit);
   STDMETHOD(GetPredefinedValue)(DISPID dispID, DWORD dwCookie, VARIANT *pVarOut, IEditable *piedit);
#pragma endregion

   HRESULT Save(VPXFileFeedback &feedback);
   HRESULT SaveToStorage(InMemStructuredStorage *pstg, VPXFileFeedback &feedback);
   HRESULT LoadGameFromFilename(const std::filesystem::path &filename, VPXFileFeedback &feedback);
   void LoadScriptOverride(const std::filesystem::path& scriptPath);

private:
   HRESULT SaveInfo(InMemStructuredStorage *pstg, HCRYPTHASH hcrypthash);
   HRESULT SaveCustomInfo(InMemStructuredStorage *pstg, InMemStream *pstmTags, HCRYPTHASH hcrypthash);
   static HRESULT WriteInfoValue(InMemStructuredStorage *pstg, const string &name, const string &szValue, HCRYPTHASH hcrypthash);
   static void ReadInfoValue(POLE::Storage &storage, const string &wzName, string &output, HCRYPTHASH hcrypthash);
   void LoadInfo(POLE::Storage &storage, HCRYPTHASH hcrypthash, int version);
   void LoadCustomInfo(POLE::Storage &storage, HCRYPTHASH hcrypthash, int version);

public:
   void Uncreate(IEditable *pie);
   void Undelete(IEditable *pie);

   void ReorderParts(bool isDrawingOrder);

#pragma region Scene Parts
public:
   const vector<IEditable *> &GetParts() const { return m_vedit; }
   bool HasPart(IEditable *part) const { return std::ranges::find(m_vedit, part) != m_vedit.end(); }
   void AddPart(IEditable *part);
   void RemovePart(IEditable *part);
   void RenamePart(IEditable *part, const wstring& newName);
   void MovePartToFront(IEditable *part);
   void MovePartToBack(IEditable *part);
   IEditable *GetElementByName(const char *const name) const;

private:
   vector<IEditable *> m_vedit;
#pragma endregion


#pragma region Unique name ids
public:
   bool IsNameUnique(const wstring &wzName) const;
   void GetUniqueName(const ItemTypeEnum type, wstring &wzUniqueName) const;
   wstring GetUniqueName(const wstring &wzRoot) const;

private:
   ankerl::unordered_dense::set<wstring> m_scriptableNames;
#pragma endregion


public:
   float GetSurfaceHeight(const string &name, float x, float y) const;

   void SetLoadDefaults();

   void SetDirty(SaveDirtyState sds);
   void SetNonUndoableDirty(SaveDirtyState sds);
   void CheckDirty();
   bool FDirty() const;

   VPX::Sound *GetSound(const string &name) const;

   int GetDetailLevel() const { return m_settings.GetPlayer_AlphaRampAccuracy(); } // used for rubber, ramp and ball

   FRect3D GetBoundingBox() const;
   void ComputeNearFarPlane(const Matrix3D &matWorldView, const float scale, float &zNear, float &zFar) const;
   static void ComputeNearFarPlane(const vector<Vertex3Ds> &bounds, const Matrix3D &matWorldView, const float scale, float &zNear, float &zFar);

   bool RenderSolid() const { return m_renderSolid; }

   static void InvokeBallBallCollisionCallback(const class HitBall *b1, const class HitBall *b2, float hitVelocity);

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

#pragma region Material
public:
   void RemoveMaterial(Material *const pmat);
   void AddMaterial(Material *const pmat);
   bool IsMaterialNameUnique(const string &name) const;
   Material *GetMaterial(const string &name) const;
   Material *GetSurfaceMaterial(const wstring &name) const;
   Texture *GetSurfaceImage(const wstring &name) const;
   bool IsDummyMaterial(const Material *const mat) const { return mat == m_dummyMaterial.get(); }

private:
   std::unique_ptr<Material> m_dummyMaterial;
#pragma endregion


#pragma region Collection
public:
   void AddCollection(CComObject<Collection> *collection);
   void RemoveCollection(CComObject<Collection> *collection);
   void RenameCollection(Collection *collection, const wstring &newName);
   void MoveCollectionUp(CComObject<Collection> *pcol);
   void MoveCollectionDown(CComObject<Collection> *pcol);
   const vector<CComObject<Collection> *> &GetCollections() const { return m_vcollection; }
   bool GetCollectionIndex(const IEditable *const element, int &collectionIndex, int &elementIndex);
   void ToggleCollectionMembership(const int colIndex, const vector<IEditable *> &selection);
   const wstring &GetCollectionNameByElement(const IEditable *const element) const;

private:
   vector<CComObject<Collection> *> m_vcollection;
#pragma endregion


public:
   std::filesystem::path m_filename;
   string m_title;

   // Flag that disables all table edition. Lock toggles are counted to identify version changes in a table (for example to guarantee untouched table for tournament)
   bool IsLocked() const { return (m_tablelocked & 1) != 0; }
   void ToggleLock() { m_tablelocked++; }

   bool TournamentModePossible() const { return IsLocked() && !FDirty() && m_external_script_name.empty(); }

   // Override automatic ini path (used for commandline override)
   void SetSettingsFileName(const std::filesystem::path &path)
   {
      m_iniFileName = FileExists(path) ? path : std::filesystem::path();
      m_settings.SetIniPath(GetSettingsFileName());
      m_settings.Load(false);
   }

   // Get the ini file name to use for this table (either overridden or derived from table or folder name)
   std::filesystem::path GetSettingsFileName() const
   {
      // Overriden externally (on command line)
      if (!m_iniFileName.empty() && FileExists(m_iniFileName))
         return m_iniFileName;

      // File not yet saved => No table ini file available
      if (!FileExists(m_filename))
         return std::filesystem::path();

      // Table ini file alongside table file, name matching table filename
      std::filesystem::path tableIni = m_filename;
      tableIni.replace_extension(".ini");
      if (FileExists(tableIni))
         return tableIni;

      // Table ini file alongside table file, name matching folder name
      const auto folder = m_filename.parent_path();
      auto fn = folder.filename();
      fn += ".ini"sv;
      std::filesystem::path folderIni = folder / fn;
      folderIni = find_case_insensitive_file_path(folderIni);
      if (!folderIni.empty())
         return folderIni;

      // No existing file: defaults to ini file alongside table file, name matching table filename
      return tableIni;
   }

   Settings m_settings; // Settings for this table (apply overrides above application settings)

   PinTable * m_liveBaseTable = nullptr; // Defined when this table is a live shallow copy of another table
   template <class T> T *GetLiveFromStartup(T *obj)
   {
      const auto it = m_startupToLive.find(obj);
      return it != m_startupToLive.end() ? static_cast<T *>(it->second) : nullptr;
   }
   template <class T> T *GetStartupFromLive(T *obj)
   {
      const auto it = m_liveToStartup.find(obj);
      return it != m_liveToStartup.end() ? static_cast<T *>(it->second) : nullptr;
   }

   // FIXME circular dependency with PinTableWnd, needed while splitting Win32 editor from core parts, but must be removed afterward
   class PinTableWnd *m_tableEditor = nullptr;

private:
   std::filesystem::path m_iniFileName;

   ankerl::unordered_dense::map<void *, void *> m_startupToLive; // For live table, maps back and forth to startup table editable parts, materials,...
   ankerl::unordered_dense::map<void *, void *> m_liveToStartup;

public:
   float m_left = 0.f; // always zero for now
   float m_top = 0.f; // always zero for now
   float m_right = 0.f;
   float m_bottom = 0.f;

   Vertex2D EvaluateGlassHeight() const;
   float m_glassBottomHeight = 210.f; // Height of glass above playfield at bottom of playfield
   float m_glassTopHeight = 210.f; // Height of glass above playfield at top of playfield

   float m_defaultBulbIntensityScaleOnBall = 1.f;

   // View mode selection
   ViewSetupID GetViewMode() const { return m_viewMode; }
   bool IsFSSEnabled() const;
   void EnableFSS(const bool enable);
   ViewSetupID GetViewSetupOverride() const { return m_viewModeOverride; }
   void SetViewSetupOverride(const ViewSetupID v) { m_viewModeOverride = v; UpdateCurrentBGSet(); }
   ViewSetup& GetViewSetup() { return mViewSetups[GetViewMode()]; }
   const ViewSetup& GetViewSetup() const { return mViewSetups[GetViewMode()]; }
   ViewSetup mViewSetups[NUM_BG_SETS];
   string m_BG_image[NUM_BG_SETS];
private:
   void UpdateCurrentBGSet();
   bool m_isFSSViewModeEnabled = false; // Flag telling if this table supports Full Single Screen POV (defaults is to use it in desktop mode if available)
   ViewSetupID m_viewMode; // Cache of the active view setup ID (depends on table but also on application settings and user overriding it)
   ViewSetupID m_viewModeOverride = BG_INVALID; // Allow to easily override the POV for testing (not persisted)

public:

   float m_angletiltMax;
   float m_angletiltMin;

   int m_overridePhysics = 0;
   float m_fOverrideGravityConstant;
   float m_fOverrideContactFriction;
   float m_fOverrideElasticity;
   float m_fOverrideElasticityFalloff;
   float m_fOverrideScatterAngle;
   float m_fOverrideMinSlope;
   float m_fOverrideMaxSlope;

   bool m_overridePhysicsFlipper = false;

   unsigned int m_PhysicsMaxLoops;

   float m_Gravity;
   float m_friction;
   float m_elasticity;
   float m_elasticityFalloff;
   float m_scatter;
   float m_defaultScatter = 0.f;

   float m_nudgeTime = 5.f;
   Vertex2D m_tblNudgeRead;
   float m_tblNudgeReadTilt = 0.f;
   Vertex2D m_tblNudgePlumb;

   uint32_t m_tblAutoStart; // msecs before trying an autostart if doing once-only method .. 0 is automethod
   uint32_t m_tblAutoStartRetry; // msecs before retrying to autostart.
   bool m_tblAutoStartEnabled;

   bool m_script_protected = false; // To be able to decrypt old tables with protected script

   float m_difficulty = 0.2f; // table difficulty Level
   float m_globalDifficulty;  // global difficulty, i.e. table difficulty optionally overriden by settings

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

   vector<Texture *> m_vimage;
   vector<Texture *> m_vliveimage;
   const vector<Texture *> &GetImageList() const { return m_vimage; }

   int m_numMaterials = 0;
   vector<Material *> m_materials;
   const vector<Material *> &GetMaterialList() const { return m_materials; }

   vector<VPX::Sound *> m_vsound;

   vector<RenderProbe *> m_vrenderprobe;
   void RemoveRenderProbe(RenderProbe *pb) { std::erase(m_vrenderprobe, pb); }
   RenderProbe *NewRenderProbe() { auto pb = new RenderProbe(); m_vrenderprobe.push_back(pb); return pb; }
   const vector<RenderProbe *> &GetRenderProbeList() const { return m_vrenderprobe; }
   vector<RenderProbe *> GetRenderProbeList(RenderProbe::ProbeType type) const
   {
      std::vector<RenderProbe *> list;
      std::ranges::copy_if(m_vrenderprobe.begin(), m_vrenderprobe.end(), std::back_inserter(list), [type](const RenderProbe *rp) { return rp->GetType() == type; });
      return list;
   }

   COLORREF m_rgcolorcustom[16]; // array for the choosecolor in property browser

   float m_TableSoundVolume;
   float m_TableMusicVolume;

   FRect m_rcDragRect; // Multi-select

   string m_original_table_script; // Script defined in the loaded file
   std::filesystem::path m_external_script_name; // if defined, file that override internal script
   string m_script_text; // Actual script (either a copy of the original or the one loaded from the override file)

   CComObject<class ScriptGlobalTable> *m_psgt; // Object to expose to script for global functions

   SaveDirtyState m_sdsDirtyProp = eSaveClean;
   SaveDirtyState m_sdsDirtyScript = eSaveClean;
   SaveDirtyState m_sdsNonUndoableDirty = eSaveClean;
   SaveDirtyState m_sdsCurrentDirtyState = eSaveClean;

   // Table info
   string m_tableName;
   string m_author;
   string m_version;
   string m_releaseDate;
   string m_authorEMail;
   string m_webSite;
   string m_blurb;
   string m_description;
   string m_rules;
   string m_screenShot;
   string m_dateSaved;
   unsigned int m_numTimesSaved = 0;

   vector<string> m_vCustomInfoTag;
   vector<string> m_vCustomInfoContent;

   LightSource m_Light[MAX_LIGHT_SOURCES];
   COLORREF m_lightAmbient;
   float m_lightHeight;
   float m_lightRange;
   float m_lightEmissionScale;
   float m_envEmissionScale;
   float m_globalEmissionScale = 1.f;
   float m_AOScale;
   float m_SSRScale;
   float m_groundToLockbarHeight = CMTOVPU(91.f); // Height of lockbar from ground (corresponding to cab model for VR)
   Matrix3D GetDefaultPlayfieldToCabMatrix() const
   {
      //const float baseSlope = lerp(table->m_angletiltMin, table->m_angletiltMax, table->m_difficulty);
      //const Matrix3D cabinetSlope = Matrix3D::MatrixRotateX(ANGTORAD(table->GetPlayfieldSlope() - baseSlope));
      return Matrix3D::MatrixTranslate(0.f, 0.f, -(m_groundToLockbarHeight - m_glassBottomHeight));
   }

   float m_playfieldReflectionStrength; // default (implicit) playfield reflection strength (0 to disable playfield reflection)

   float m_ballPlayfieldReflectionStrength;
   bool m_enableAO;
   bool m_enableSSR;
   float m_bloom_strength;

   volatile std::atomic<bool> m_savingActive = false;

   bool m_renderSolid = true;
   bool m_renderDecals = true;
   bool m_renderEMReels = true;
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
   float GetTableWidth() const;
   void SetTableWidth(const float value);
   float GetHeight() const;
   void SetHeight(const float value);

   float GetPlayfieldSlope() const;
   float GetPlayfieldOverridenSlope() const;

   void SetNotesText(const string &text)
   {
      m_notesText = text;
      SetDirtyDraw();
   }
   const string& GetNotesText() const { return m_notesText; }

   ToneMapper GetToneMapper() const { return m_toneMapper; }
   void SetToneMapper(const ToneMapper& tm) { m_toneMapper = tm; }
   float GetExposure() const { return m_exposure; }
   void SetExposure(const float exposure) { m_exposure = exposure; }

   void SetupLookUpTables(bool isPlaying);

   // Win32 editor state which is persisted in the table file
   Vertex2D m_winEditorViewOffset;
   float m_winEditorZoom = 1.f;
   bool m_winEditorGrid = true;
   bool m_winEditorBackdrop = true;

private:
   unsigned int m_tablelocked = 0;

   string m_notesText;
   ankerl::unordered_dense::map<string, Texture *, StringHashFunctor, StringComparator> m_textureMap; // hash table to speed up texture lookup by name
   ankerl::unordered_dense::map<string, Material *, StringHashFunctor, StringComparator> m_materialMap; // hash table to speed up material lookup by name
   ankerl::unordered_dense::map<string, Light *, StringHashFunctor, StringComparator> m_lightMap; // hash table to speed up light lookup by name
   ankerl::unordered_dense::map<string, RenderProbe *, StringHashFunctor, StringComparator> m_renderprobeMap; // hash table to speed up renderprobe lookup by name

   PinBinary *m_pbTempScreenshot = nullptr; // Holds contents of screenshot image until the image asks for it
   int m_loadTemp[5] = { 0, 0, 0, 0, 0 }; // Used to temporarily store the number of elements loaded for each type (subobjects, sounds, textures, fonts, collections) during loading phase

   vector<PinFont *> m_vfont;

   ankerl::unordered_dense::set<std::string> m_loggedSoundErrors;

   ToneMapper m_toneMapper = ToneMapper::TM_AGX;
   float m_exposure = 1.f;

   std::optional<VPX::Properties::PropertyRegistry::PropId> RegisterOption(BSTR optionName, float minValue, float maxValue, float step, float defaultValue, int unit, /*[optional][in]*/ VARIANT values);
   vector<TableOption> m_tableOptions;
};
