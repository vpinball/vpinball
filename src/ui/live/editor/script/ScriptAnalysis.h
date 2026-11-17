// license:GPLv3+

#pragma once

// VBScript document model used by the live editor's script pane.
//
// Pure C++ (no ImGui and no app includes, mirrors AreaLayout's testable design): it parses
// subs, functions, classes, properties, dim/const declarations and `Set x = New C` bindings,
// tracking declaration lines, member hierarchy, signatures and doc comments. It is used for
// symbol lookup (go to definition), member completion, the symbol outliner and dynamic
// identifier highlighting.
//
// Parsing rules (improving on the win32 codeview "parse brain"):
// - ` _` line continuations join physical lines into logical lines (line numbers kept)
// - ' comments, standalone `rem` comments and "..." strings (incl. "" doubling) are skipped
// - ':' splits a logical line into statements
// - Private/Public/Default declaration prefixes are skipped
// - Class/Sub/Function/Property blocks are tracked on a stack, `End X` pops them

#include <string>
#include <string_view>
#include <vector>
#include <unordered_map>

namespace VPX::EditorUI
{

enum class SymbolKind
{
   Sub,
   Function,
   Class,
   PropGet,
   PropLet,
   PropSet,
   Dim,
   Const
};

const char *SymbolKindName(SymbolKind kind); // Short label for lists/tooltips ("Sub", "Property Get", ...)

struct Symbol
{
   std::string name; // Declaration spelling
   std::string lowerName; // Lookup key (VBScript is case insensitive)
   std::string signature; // Declaration statement, comments removed, continuations joined
   std::string comment; // Comment lines above the declaration plus the trailing ' comment
   SymbolKind kind;
   int line = 0; // Zero-based line of the declaration
   int parent = -1; // Index of the enclosing Class/Sub/... symbol, -1 at top level
   std::vector<int> children; // Indices of symbols declared inside this one
};

class ScriptModel
{
public:
   void Parse(std::string_view script); // Rebuilds all state from the script text
   void Clear()
   {
      m_symbols.clear();
      m_varClass.clear();
   }

   const std::vector<Symbol> &Symbols() const { return m_symbols; }
   std::vector<const Symbol *> TopLevel() const;

   const Symbol *Find(std::string_view name) const; // Case insensitive, top-level symbols first
   std::vector<const Symbol *> MembersOf(std::string_view className) const; // Members when name is a class
   // name -> what the name is bound to: itself for Class decls, the target of `Set name = X`
   std::string ResolveClassName(std::string_view name) const;

private:
   std::vector<Symbol> m_symbols;
   std::unordered_map<std::string, std::string> m_varClass; // variable lowerName -> bound name (`Set x = New C` / `Set x = obj`)
};

}
