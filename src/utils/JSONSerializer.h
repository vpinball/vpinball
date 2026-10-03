// license:GPLv3+

#pragma once

#include <nlohmann/json.hpp>

#include "core/ieditable.h"
#include "utils/fileio.h"

// Support for the VPZ file format: a JSON based pack stored either as a folder or as a zip archive.
// The pack contains a manifest.json file, a table.json file, one json file per scene node in parts/,
// and assets (images, sounds, fonts, meshes) in their native (DCC) format, each with a sibling json
// file holding the asset metadata.
//
// Serialization reuses the IObjectReader/IObjectWriter based Save/Load code of PinTable and its parts.
// The FID (4 chars field id) to json property name mapping is context dependent: the same FID may have
// a different meaning (and therefore a different name) depending on the type of object being processed,
// so each object type has its own field map.
class JSONSerializer
{
public:
   // Node kinds are used to select the FID <-> property name map. Scene node kinds are the
   // ItemTypeEnum value of the part. Objects that do not correspond to an ItemTypeEnum use the
   // constants below.
   static constexpr int kDragPointNode = 0x100;
   static constexpr int kMaterialNode = 0x101;
   static constexpr int kRenderProbeNode = 0x102;
   static constexpr int kPinBinaryNode = 0x103;
   static constexpr int kTextureNode = 0x104;

   // Version of the VPZ pack file format (preliminary, no backward compatibility across versions)
   static constexpr int kFormatVersion = 1;

   // Write side of a pack (a folder or a zip archive). All paths are relative to the pack root
   // and use '/' separators.
   class Serializer
   {
   public:
      virtual ~Serializer() = default;
      virtual void AddTextFile(const std::filesystem::path& path, const std::string& content) = 0;
      virtual void AddBinaryFile(const std::filesystem::path& path, const std::vector<uint8_t>& data) = 0;
      // Flushes pending writes (zip footer,...); false when the pack could not be fully written
      virtual bool Finalize() { return !HasError(); }
      // True when a file could not be written
      virtual bool HasError() const { return false; }
   };

   // Read side of a pack (a folder or a zip archive).
   class Deserializer
   {
   public:
      virtual ~Deserializer() = default;
      virtual bool Exists(const std::filesystem::path& path) const = 0;
      // All files of the pack, as paths relative to the pack root
      virtual std::vector<std::filesystem::path> ListFiles() const = 0;
      virtual bool ReadBinaryFile(const std::filesystem::path& path, std::vector<uint8_t>& data) const = 0;
      bool ReadTextFile(const std::filesystem::path& path, std::string& content) const
      {
         std::vector<uint8_t> data;
         if (!ReadBinaryFile(path, data))
            return false;
         content.assign(reinterpret_cast<const char*>(data.data()), data.size());
         return true;
      }
   };

   // Creates a pack writer: a folder serializer when path is an existing directory or has no
   // extension, a zip serializer otherwise (".vpz" extension expected).
   static std::unique_ptr<Serializer> CreateWriter(const std::filesystem::path& path);
   // Creates a pack reader: a folder deserializer for a directory, a zip deserializer for a file.
   static std::unique_ptr<Deserializer> CreateReader(const std::filesystem::path& path);

   // True when the path may be a VPZ pack (a directory or a ".vpz" file)
   static bool IsPack(const std::filesystem::path& path);

   // Scene node type name <-> ItemTypeEnum ("$type" property of the json files in parts/)
   static const char* GetPartTypeName(int itemType); // nullptr if not a scene node type
   static int GetPartTypeFromName(const string& name); // -1 if unknown

   // Makes a name usable as a (portable, case insensitive) file name
   static string SanitizeFileName(const string& name);

   // FID <-> json property name translation for a given node kind (nullptr / 0 when unmapped)
   static const char* GetFieldName(int nodeKind, int fieldId);
   static int GetFieldId(int nodeKind, const string& fieldName);
   // Node kind of the sub objects for a field (defaults to the parent kind)
   static int GetSubObjectKind(int parentKind, int fieldId);
   // True when the field is a list of repeated scalar records (e.g. collection item names)
   static bool IsRepeatableField(int nodeKind, int fieldId);
};

class JSONObjectWriter final : public IObjectWriter
{
public:
   explicit JSONObjectWriter(int nodeKind, JSONSerializer::Serializer* pack = nullptr);

   // The generated json document, with its fields canonically ordered (valid once the object has been fully written)
   const nlohmann::ordered_json& Json();

   bool HasError() const override { return m_hasError; }

   void BeginObject(int objectId, bool isArray, bool isSkippable) override;
   void WriteBool(int fieldId, bool value) override;
   void WriteInt(int fieldId, int value) override;
   void WriteUInt(int fieldId, unsigned int value) override;
   void WriteFloat(int fieldId, float value) override;
   void WriteString(int fieldId, const string& value) override;
   void WriteWideString(int fieldId, const wstring& value) override;
   void WriteVector2(int fieldId, const Vertex2D& value) override;
   void WriteVector3(int fieldId, const vec3& value) override;
   void WriteVector4(int fieldId, const vec4& value) override;
   void WriteScript(int fieldId, const string& value) override;
   void WriteFontDescriptor(int fieldId, const FontDesc& value) override;
   void WriteRaw(int fieldId, const void* pvalue, int size) override;
   void EndObject() override;

private:
   struct Scope
   {
      nlohmann::ordered_json* node;
      int nodeKind;
   };

   nlohmann::ordered_json m_root;
   std::vector<Scope> m_stack;
   const int m_nodeKind;
   JSONSerializer::Serializer* const m_pack;
   bool m_hasError = false;
};

class JSONObjectReader final : public IObjectReader
{
public:
   JSONObjectReader(const nlohmann::ordered_json& node, int nodeKind, const JSONSerializer::Deserializer* pack = nullptr, int fileVersion = 1090);

   bool HasError() const override { return m_hasError; }
   int GetVersion() const override { return m_fileVersion; }

   bool AsBool() override;
   int AsInt() override;
   unsigned int AsUInt() override;
   float AsFloat() override;
   string AsString() override;
   wstring AsWideString() override;
   Vertex2D AsVector2() override;
   vec3 AsVector3() override;
   vec4 AsVector4() override;
   string AsScript(bool isScriptProtected) override;
   FontDesc AsFontDescriptor() override;
   void AsRaw(void* pvalue, int size) override;
   void AsObject(const std::function<bool(int fieldTag, IObjectReader& fieldReader)>& processField, bool isSkippable = false) override;

private:
   const nlohmann::ordered_json& m_node;
   const int m_nodeKind;
   const JSONSerializer::Deserializer* const m_pack;
   const int m_fileVersion;
   mutable bool m_hasError = false;
};
