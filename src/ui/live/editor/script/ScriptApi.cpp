// license:GPLv3+

#include "core/stdafx.h"
#include "ScriptApi.h"

#include "core/VPApp.h"
#include "core/ieditable.h"
#include "parts/pintable.h"
#include "textedit/VbsLex.h"

#include <fstream>

namespace VPX::EditorUI
{

// generated per-interface member/event tables
#include "core/generated/vpinball_script_api.inc"

namespace
{

   std::string Lower(const std::string_view s)
   {
      std::string r(s);
      for (char &c : r)
         c = static_cast<char>(VbsLex::ToLower(c));
      return r;
   }

   // Part item types -> script COM interface names (see src/core/vpinball.idl)
   const char *InterfaceOfType(const ItemTypeEnum type)
   {
      switch (type)
      {
      case eItemSurface: return "IWall";
      case eItemFlipper: return "IFlipper";
      case eItemTimer: return "ITimer";
      case eItemPlunger: return "IPlunger";
      case eItemTextbox: return "ITextbox";
      case eItemBumper: return "IBumper";
      case eItemTrigger: return "ITrigger";
      case eItemLight: return "ILight";
      case eItemKicker: return "IKicker";
      case eItemDecal: return "IDecal";
      case eItemGate: return "IGate";
      case eItemSpinner: return "ISpinner";
      case eItemRamp: return "IRamp";
      case eItemTable: return "ITableGlobal";
      case eItemCollection: return "ICollection";
      case eItemDispReel: return "IDispReel";
      case eItemLightSeq: return "ILightSeq";
      case eItemPrimitive: return "IPrimitive";
      case eItemFlasher: return "IFlasher";
      case eItemRubber: return "IRubber";
      case eItemHitTarget: return "IHitTarget";
      case eItemBall: return "IBall";
      case eItemPartGroup: return "IPartGroup";
      default: return nullptr;
      }
   }

   bool ReadTextFile(const std::filesystem::path &path, std::string &out)
   {
      std::ifstream file(path, std::ios::binary | std::ios::ate);
      if (!file)
         return false;
      const std::streamsize size = file.tellg();
      file.seekg(0, std::ios::beg);
      out.resize(static_cast<size_t>(size));
      return size <= 0 || file.read(out.data(), size).good();
   }

} // namespace

std::vector<std::string> ScriptApi::ScriptIncludes(const std::string_view script)
{
   // Look for GetTextFile("name.vbs") at statement level: cut comments first so
   // commented-out includes are skipped, then match the call followed by a string
   std::vector<std::string> files;
   size_t pos = 0;
   while (pos <= script.size())
   {
      const size_t eol = script.find('\n', pos);
      std::string_view line = script.substr(pos, (eol == std::string_view::npos ? script.size() : eol) - pos);
      if (!line.empty() && line.back() == '\r')
         line.remove_suffix(1);
      pos = eol == std::string_view::npos ? script.size() + 1 : eol + 1;

      // cut the comment off the line (' outside strings)
      bool inStr = false;
      size_t codeEnd = line.size();
      for (size_t i = 0; i < line.size(); ++i)
      {
         const char c = line[i];
         if (inStr)
         {
            if (c == '"' && i + 1 < line.size() && line[i + 1] == '"')
               ++i;
            else if (c == '"')
               inStr = false;
         }
         else if (c == '"')
            inStr = true;
         else if (c == '\'')
         {
            codeEnd = i;
            break;
         }
      }

      // find `GetTextFile` word occurrences, then a quoted file name
      for (size_t i = 0; i < codeEnd;)
      {
         const char c = line[i];
         if (!VbsLex::IsIdentStart(c))
         {
            ++i;
            continue;
         }
         size_t j = i + 1;
         while (j < codeEnd && VbsLex::IsIdentChar(line[j]))
            ++j;
         const std::string_view word = line.substr(i, j - i);
         if (Lower(word) == "gettextfile")
         {
            // optional whitespace, '(', whitespace, then a "file" literal
            size_t k = j;
            while (k < codeEnd && (line[k] == ' ' || line[k] == '\t' || line[k] == '('))
               ++k;
            if (k < codeEnd && line[k] == '"')
            {
               const size_t nameEnd = line.find('"', k + 1);
               if (nameEnd != std::string_view::npos && nameEnd <= codeEnd)
               {
                  std::string file(line.substr(k + 1, nameEnd - k - 1));
                  if (!file.empty())
                     files.push_back(std::move(file));
                  i = nameEnd + 1;
                  continue;
               }
            }
         }
         i = j;
      }
   }
   return files;
}

void ScriptApi::EnsureCoreParsed()
{
   if (m_coreParsed)
      return;
   m_coreParsed = true;

   // core.vbs is the VPX script library always available to table scripts; files the
   // script includes via GetTextFile land in the same model (symbol line numbers are
   // relative to the concatenated text, only used for completion/hover, not navigation)
   std::string core;
   if (!ReadTextFile(g_app->m_fileLocator.GetAppPath(FileLocator::AppSubFolder::Scripts, "core.vbs"), core))
      return;
   std::string text = core;
   for (const std::string &include : ScriptIncludes(core))
   {
      std::string extra;
      if (ReadTextFile(g_app->m_fileLocator.GetAppPath(FileLocator::AppSubFolder::Scripts, include), extra))
         text += '\n' + extra;
   }
   m_coreModel.Parse(text);
   m_globals.clear();
   for (const Symbol *s : m_coreModel.TopLevel())
      m_globals.push_back(*s);
}

void ScriptApi::Refresh(const PinTable *table, const std::string_view scriptText)
{
   EnsureCoreParsed();

   // files the table script itself includes on top of core.vbs' own includes are
   // re-parsed when the include set changes (kept cheap: only on Refresh calls)
   std::string extraFiles;
   for (const std::string &include : ScriptIncludes(scriptText))
   {
      extraFiles += include;
      extraFiles += ';';
   }
   if (extraFiles != m_parsedIncludes)
   {
      m_parsedIncludes = extraFiles;
      // rebuild the core model with the script's own includes appended
      std::string core;
      if (ReadTextFile(g_app->m_fileLocator.GetAppPath(FileLocator::AppSubFolder::Scripts, "core.vbs"), core))
      {
         std::string text = core;
         for (const std::string &include : ScriptIncludes(scriptText))
         {
            std::string extra;
            if (ReadTextFile(g_app->m_fileLocator.GetAppPath(FileLocator::AppSubFolder::Scripts, include), extra))
               text += '\n' + extra;
         }
         for (const std::string &include : ScriptIncludes(core))
         {
            std::string extra;
            if (ReadTextFile(g_app->m_fileLocator.GetAppPath(FileLocator::AppSubFolder::Scripts, include), extra))
               text += '\n' + extra;
         }
         m_coreModel.Parse(text);
      }
   }

   // rebuild globals: core symbols + part names (part name -> script interface)
   m_partIface.clear();
   m_globals.clear();
   for (const Symbol *s : m_coreModel.TopLevel())
      m_globals.push_back(*s);
   const auto addGlobal = [&](std::string name, const char *iface)
   {
      Symbol sym;
      sym.name = std::move(name);
      sym.lowerName = Lower(sym.name);
      sym.signature = iface;
      sym.kind = SymbolKind::Dim;
      m_partIface[sym.lowerName] = iface;
      m_globals.push_back(std::move(sym));
   };
   addGlobal("Debug", "IVPDebug");
   addGlobal("Me", "ITableGlobal");
   if (table != nullptr)
   {
      // the table's own script name ("Table1"), used for "Table1_Init" style handlers
      if (const IScriptable *scriptable = table->GetIScriptable(); scriptable != nullptr && !scriptable->m_name.empty())
         addGlobal(scriptable->m_name, "ITable");
      for (const IEditable *part : table->GetParts())
         if (const char *iface = InterfaceOfType(part->GetItemType()))
            addGlobal(part->GetName(), iface);
      for (const IScriptable *part : table->GetCollections())
         addGlobal(part->get_Name(), "ICollection");
   }
}

std::string ScriptApi::InterfaceOf(const std::string_view name) const
{
   const auto it = m_partIface.find(Lower(name));
   return it == m_partIface.end() ? std::string() : it->second;
}

const ScriptApiInterface *ScriptApi::FindInterface(const std::string_view name)
{
   const std::string lower = Lower(name);
   for (const ScriptApiInterface *iface = s_scriptApiInterfaces; iface->name != nullptr; ++iface)
      if (lower == Lower(iface->name))
         return iface;
   return nullptr;
}

const ScriptApiInterface *ScriptApi::InterfaceOfName(const std::string_view name) const
{
   const std::string iface = InterfaceOf(name);
   return iface.empty() ? nullptr : FindInterface(iface);
}

std::vector<const Symbol *> ScriptApi::MembersOf(const std::string_view name, const ScriptModel &scriptModel) const
{
   // resolve the name a few hops: script classes win over builtin library classes
   // of the same name, `Set x = ...` bindings chain to their target, and whatever
   // remains is tried as a COM part name -> script interface
   const ScriptApiInterface *iface = nullptr;
   std::string current(name);
   for (int hops = 0; hops < 8; ++hops)
   {
      std::vector<const Symbol *> members = scriptModel.MembersOf(current);
      if (members.empty())
         members = m_coreModel.MembersOf(current);
      if (!members.empty())
         return members;

      iface = InterfaceOfName(current);
      std::string next = scriptModel.ResolveClassName(current);
      if (next.empty())
         next = m_coreModel.ResolveClassName(current);
      if (iface != nullptr || next.empty() || next == current)
         break;
      current = std::move(next);
   }
   if (iface == nullptr)
      return {};

   m_tmpMembers.clear();
   m_tmpMembers.reserve(static_cast<size_t>(iface->memberCount));
   for (int i = 0; i < iface->memberCount; ++i)
   {
      const ScriptApiEntry &entry = iface->members[i];
      Symbol sym;
      sym.name = entry.name;
      sym.lowerName = Lower(entry.name);
      sym.signature = entry.signature;
      if (entry.flags & kScriptApiMethod)
         sym.kind = std::string_view(entry.signature).starts_with("Function") ? SymbolKind::Function : SymbolKind::Sub;
      else if (entry.flags & kScriptApiConst)
         sym.kind = SymbolKind::Const;
      else
         sym.kind = SymbolKind::PropGet;
      m_tmpMembers.push_back(std::move(sym));
   }
   std::vector<const Symbol *> members;
   members.reserve(m_tmpMembers.size());
   for (const Symbol &sym : m_tmpMembers)
      members.push_back(&sym);
   return members;
}

}
