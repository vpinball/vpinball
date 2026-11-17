// license:GPLv3+

#pragma once

// Script-visible API knowledge for the live editor's script pane:
// - the builtin VPX script libraries (core.vbs plus the files the table script pulls
//   in through GetTextFile("x.vbs") / ExecuteGlobal), parsed with the same ScriptModel
// - the script-visible name and script interface of each table part
//   ("Bumper1" -> "IBumper"), used to resolve member-completion contexts
//
// Pure data, no ImGui. The PinTable is only read, never modified.

#include <string>
#include <string_view>
#include <unordered_map>
#include <vector>

#include "ScriptAnalysis.h"

class PinTable;

namespace VPX::EditorUI
{

class ScriptApi
{
public:
   ScriptApi() = default;

   // Rebuilds the table part index and (lazily, once) parses the builtin libraries.
   void Refresh(const PinTable *table, std::string_view scriptText = {});

   // Model of the parsed builtin libraries (core.vbs + files included by the script)
   const ScriptModel &CoreModel() const { return m_coreModel; }

   // Global-namespace symbols for completion and coloring: top-level symbols of the
   // builtin libraries and the table part names (parts carry their interface in
   // `signature`, e.g. "IBumper")
   const std::vector<Symbol> &Globals() const { return m_globals; }

   // Script name -> interface name ("Bumper1" -> "IBumper", "Debug" -> "IVPDebug"), "" when unknown
   std::string InterfaceOf(std::string_view name) const;

   // Members of a named class/interface: script classes come from the table script's
   // own model, core classes from the builtin libraries; COM interface members land in
   // a later phase (vpinball_script_api.inc)
   std::vector<const Symbol *> MembersOf(std::string_view name, const ScriptModel &scriptModel) const;

   // Files included by a script through GetTextFile("x.vbs") (comments/strings aware)
   static std::vector<std::string> ScriptIncludes(std::string_view script);

private:
   void EnsureCoreParsed();

   ScriptModel m_coreModel;
   bool m_coreParsed = false;
   std::vector<Symbol> m_globals; // core top-level symbols + part names
   std::unordered_map<std::string, std::string> m_partIface; // script lowerName -> interface name
   std::string m_parsedIncludes; // include scan input (reparse on change)
};

}
