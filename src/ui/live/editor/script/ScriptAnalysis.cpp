// license:GPLv3+

#include "core/stdafx.h"
#include "ScriptAnalysis.h"

#include "textedit/VbsLex.h"

#include <optional>

namespace VPX::EditorUI
{

const char *SymbolKindName(const SymbolKind kind)
{
   switch (kind)
   {
   case SymbolKind::Sub: return "Sub";
   case SymbolKind::Function: return "Function";
   case SymbolKind::Class: return "Class";
   case SymbolKind::PropGet: return "Property Get";
   case SymbolKind::PropLet: return "Property Let";
   case SymbolKind::PropSet: return "Property Set";
   case SymbolKind::Dim: return "Dim";
   case SymbolKind::Const: return "Const";
   }
   return "?";
}

namespace
{

   std::string Lower(const std::string_view s)
   {
      std::string r(s);
      for (char &c : r)
         c = static_cast<char>(VbsLex::ToLower(c));
      return r;
   }

   std::string_view Trim(const std::string_view s)
   {
      size_t b = 0, e = s.size();
      while (b < e && isspace(static_cast<unsigned char>(s[b])))
         ++b;
      while (e > b && isspace(static_cast<unsigned char>(s[e - 1])))
         --e;
      return s.substr(b, e - b);
   }

   // Offset of the comment start in one physical line (' outside strings, or a
   // standalone rem word), or npos when the line has no comment.
   size_t FindCommentStart(const std::string_view line)
   {
      const char *const s = line.data();
      const char *const e = s + line.size();
      bool inStr = false;
      for (const char *p = s; p < e; ++p)
      {
         const char c = *p;
         if (inStr)
         {
            if (c == '"')
            {
               if (p + 1 < e && p[1] == '"')
                  ++p; // "" escape stays inside the string
               else
                  inStr = false;
            }
         }
         else if (c == '"')
            inStr = true;
         else if (c == '\'')
            return static_cast<size_t>(p - s);
         else if (VbsLex::IsIdentStart(c))
         {
            const char *const idEnd = VbsLex::ScanIdentifier(p, e);
            if (VbsLex::MatchWord(p, e, "rem") == idEnd)
               return static_cast<size_t>(p - s);
            p = idEnd - 1;
         }
      }
      return std::string_view::npos;
   }

   // Blank string literal contents (and their quotes) to space, in place.
   void BlankStrings(std::string &text)
   {
      bool inStr = false;
      for (size_t i = 0; i < text.size(); ++i)
      {
         const char c = text[i];
         if (inStr)
         {
            if (c == '"' && i + 1 < text.size() && text[i + 1] == '"')
               text[i++] = ' '; // "" escape
            else
               inStr = c != '"';
            text[i] = ' ';
         }
         else if (c == '"')
         {
            inStr = true;
            text[i] = ' ';
         }
      }
   }

   struct PhysLine
   {
      std::string_view raw;
      std::string code; // comments removed, strings blanked (positions preserved)
      std::string clean; // comments removed, strings kept (positions preserved)
      std::string comment; // comment text without the leading ' or rem
      bool commentOnly = false; // nothing but a comment on this line
      bool continues = false; // code ends with a ` _` continuation
   };

   PhysLine AnalyzeLine(const std::string_view raw)
   {
      PhysLine line;
      line.raw = raw;
      const size_t commentAt = FindCommentStart(raw);
      const std::string_view body = raw.substr(0, commentAt == std::string_view::npos ? raw.size() : commentAt);
      line.clean.assign(body);
      line.code.assign(body);
      BlankStrings(line.code);
      if (commentAt != std::string_view::npos)
      {
         std::string_view c = raw.substr(commentAt);
         if (!c.empty() && c.front() == '\'')
            c.remove_prefix(1);
         else if (Lower(c.substr(0, 3)) == "rem")
            c.remove_prefix(3);
         line.comment.assign(Trim(c));
      }
      line.commentOnly = Trim(line.code).empty() && !line.comment.empty();
      line.continues = !line.clean.empty() && line.clean.back() == '_';
      return line;
   }

   // Collapse a multi-line signature: ` _` continuations and newlines become spaces.
   std::string JoinSignature(const std::string_view text)
   {
      std::string sig;
      sig.reserve(text.size());
      for (size_t i = 0; i < text.size(); ++i)
      {
         const char c = text[i];
         if (c == '_' && (i + 1 == text.size() || isspace(static_cast<unsigned char>(text[i + 1]))))
         {
            // continuation marker: skip it plus the following whitespace/newline
            size_t j = i + 1;
            bool newline = false;
            while (j < text.size() && isspace(static_cast<unsigned char>(text[j])))
            {
               newline = newline || text[j] == '\n';
               ++j;
            }
            if (newline)
            {
               i = j - 1;
               if (!sig.empty() && sig.back() != ' ')
                  sig += ' ';
               continue;
            }
         }
         sig += c == '\n' ? ' ' : c;
      }
      return std::string(Trim(sig));
   }

   struct Word
   {
      std::string_view text; // points into the statement's code buffer
      size_t start; // offset in the statement
   };

   // Identifiers of a statement slice of blanked code, in order.
   std::vector<Word> ScanWords(const std::string_view code)
   {
      std::vector<Word> words;
      const char *const s = code.data();
      const char *const e = s + code.size();
      for (const char *p = s; p < e;)
      {
         if (!VbsLex::IsIdentStart(*p))
         {
            ++p;
            continue;
         }
         const char *const idEnd = VbsLex::ScanIdentifier(p, e);
         words.push_back({ std::string_view(p, static_cast<size_t>(idEnd - p)), static_cast<size_t>(p - s) });
         p = idEnd;
      }
      return words;
   }

} // namespace

void ScriptModel::Parse(const std::string_view script)
{
   m_symbols.clear();
   m_varClass.clear();

   // Split into physical lines, each pre-analyzed for code / comment / continuation
   std::vector<PhysLine> lines;
   size_t pos = 0;
   while (pos <= script.size())
   {
      const size_t eol = script.find('\n', pos);
      const size_t len = (eol == std::string_view::npos ? script.size() : eol) - pos;
      std::string_view raw = script.substr(pos, len);
      if (!raw.empty() && raw.back() == '\r')
         raw.remove_suffix(1);
      lines.push_back(AnalyzeLine(raw));
      if (eol == std::string_view::npos)
         break;
      pos = eol + 1;
   }

   std::vector<int> stack; // indices of open Sub/Function/Class/Property symbols

   // Block structure check: the open blocks of any kind (keyword: sub, if, for, select,...), innermost last
   m_diagnostics.clear();
   struct OpenBlock
   {
      std::string kw;
      ScriptDiagnostic at; // The opening statement
   };
   std::vector<OpenBlock> blocks;
   const auto blockName = [](const std::string &kw) -> std::string
   {
      if (kw == "select")
         return "Select Case";
      std::string name = kw;
      name[0] = static_cast<char>(toupper(static_cast<unsigned char>(name[0])));
      return name;
   };
   const auto reportUnclosed = [&](const OpenBlock &block)
   {
      ScriptDiagnostic d = block.at;
      d.message = '\'' + blockName(block.kw) + "' is not closed";
      m_diagnostics.push_back(std::move(d));
   };

   const auto addSymbol = [&](const SymbolKind kind, const std::string_view name, const std::string_view signature, const int line, const std::string_view ownComment)
   {
      Symbol sym;
      sym.name = std::string(Trim(name));
      sym.lowerName = Lower(sym.name);
      sym.signature = std::string(Trim(signature));
      sym.kind = kind;
      sym.line = line;
      sym.parent = stack.empty() ? -1 : stack.back();
      sym.comment = std::string(Trim(ownComment));
      const int index = static_cast<int>(m_symbols.size());
      m_symbols.push_back(std::move(sym));
      if (sym.parent >= 0)
         m_symbols[static_cast<size_t>(sym.parent)].children.push_back(index);
      return index;
   };

   // Group physical lines into logical lines (joined on ` _` continuations)
   for (size_t seg0 = 0; seg0 < lines.size();)
   {
      size_t segN = seg0;
      while (segN < lines.size() && lines[segN].continues)
         ++segN;
      // logical line spans [seg0, segN]

      // Build the joined code and clean texts (offsets between them stay aligned)
      std::string code, clean;
      std::vector<size_t> segOff; // start offset of each segment in the joined buffers
      for (size_t i = seg0; i <= segN && i < lines.size(); ++i)
      {
         segOff.push_back(code.size());
         if (!code.empty())
         {
            code += '\n';
            clean += '\n';
            segOff.back() = code.size();
         }
         code += lines[i].code;
         clean += lines[i].clean;
      }
      const size_t segEnd = segN < lines.size() ? segN : lines.size() - 1;

      // Comment text of a statement is the comment of its starting segment plus the comment-only lines directly above
      // it, at most the last few of them (a large commented out block above a declaration is not its documentation)
      const auto commentFor = [&](const size_t stmtOff)
      {
         size_t seg = 0;
         for (size_t i = 0; i < segOff.size(); ++i)
            if (segOff[i] <= stmtOff)
               seg = i;
         constexpr size_t maxDocLines = 16;
         size_t first = seg0 + seg;
         while (first > 0 && seg0 + seg - first < maxDocLines && lines[first - 1].commentOnly)
            --first;
         std::string docs;
         for (size_t l = first; l < seg0 + seg; ++l)
         {
            docs += lines[l].comment;
            docs += '\n';
         }
         return docs + lines[seg0 + seg].comment;
      };

      // Split the joined code into statements on ':' (strings are already blanked)
      size_t stmtStart = 0;
      bool singleLineIf = false; // The rest of the logical line is the body of a single line If: not checked
      const auto runStatement = [&](const size_t a, const size_t b)
      {
         const std::string_view stmtCode = Trim(std::string_view(code).substr(a, b - a));
         if (stmtCode.empty())
            return;
         const size_t off = static_cast<size_t>(stmtCode.data() - code.data());
         const std::vector<Word> words = ScanWords(stmtCode);
         if (words.empty())
            return;
         const int line0 = static_cast<int>(seg0 +
            [&]
            {
               size_t seg = 0;
               for (size_t i = 0; i < segOff.size(); ++i)
                  if (segOff[i] <= off)
                     seg = i;
               return seg;
            }());
         const std::string signature = JoinSignature(std::string_view(clean).substr(off, stmtCode.size()));
         // Only declarations keep a comment: computed on their first use, not for every statement
         std::optional<std::string> commentCache;
         const auto comment = [&]() -> const std::string & { return commentCache ? *commentCache : commentCache.emplace(commentFor(off)); };

         auto wordAt = [&](const size_t i) { return i < words.size() ? Lower(words[i].text) : std::string(); };
         size_t w = 0;
         while (wordAt(w) == "private" || wordAt(w) == "public" || wordAt(w) == "default")
            ++w; // declaration modifiers

         const std::string kw = wordAt(w);

         // Block structure: openers push, closers pop their block, reporting the blocks left open in between
         if (!singleLineIf)
         {
            // Position of the statement keyword(s) words[w..lastWord], on the physical line of the keyword
            const auto at = [&](const size_t lastWord, std::string message)
            {
               const size_t kwOff = off + words[w].start;
               size_t seg = 0;
               for (size_t i = 0; i < segOff.size(); ++i)
                  if (segOff[i] <= kwOff)
                     seg = i;
               ScriptDiagnostic d;
               d.line = static_cast<int>(seg0 + seg);
               d.column = static_cast<int>(kwOff - segOff[seg]);
               d.length = static_cast<int>(words[lastWord].start + words[lastWord].text.size() - words[w].start);
               d.message = std::move(message);
               return d;
            };
            const std::string kw1 = wordAt(w + 1);
            std::string opens, closes;
            size_t lastWord = w;
            if (kw == "sub" || kw == "function" || kw == "class" || kw == "for" || kw == "do" || kw == "while" || kw == "with")
               opens = kw;
            else if ((kw == "property" && (kw1 == "get" || kw1 == "let" || kw1 == "set")) || (kw == "select" && kw1 == "case"))
            {
               opens = kw;
               lastWord = w + 1;
            }
            else if (kw == "if")
            {
               // Only `If x Then` ending its logical line opens a block. `If x Then y`, `If x Then : y` and even `If x Then :`
               // are single line Ifs (VBScript rejects an `End If` on a later line, accepts one on the same line)
               if (Lower(words.back().text) == "then" && b == code.size())
                  opens = kw;
               else
                  singleLineIf = true;
            }
            else if (kw == "end" && (kw1 == "sub" || kw1 == "function" || kw1 == "class" || kw1 == "property" || kw1 == "if" || kw1 == "select" || kw1 == "with"))
            {
               closes = kw1;
               lastWord = w + 1;
            }
            else if (kw == "next" || kw == "loop" || kw == "wend")
               closes = kw == "next" ? "for" : kw == "loop" ? "do" : "while";
            else if ((kw == "else" || kw == "elseif") && (blocks.empty() || blocks.back().kw != "if"))
               m_diagnostics.push_back(at(w, std::string(kw == "else" ? "'Else'" : "'ElseIf'") + " outside of an 'If' block"));
            else if (kw == "case" && (blocks.empty() || blocks.back().kw != "select"))
               m_diagnostics.push_back(at(w, "'Case' outside of a 'Select Case' block"));

            if (!opens.empty())
            {
               // A Sub/Function/Property can only be in a Class, and a Class at top level: the blocks still open were not closed
               if (opens == "sub" || opens == "function" || opens == "property" || opens == "class")
                  while (!blocks.empty() && (opens == "class" || blocks.back().kw != "class"))
                  {
                     reportUnclosed(blocks.back());
                     blocks.pop_back();
                  }
               blocks.push_back({ opens, at(lastWord, {}) });
            }
            else if (!closes.empty())
            {
               if (std::ranges::none_of(blocks, [&closes](const OpenBlock &block) { return block.kw == closes; }))
               {
                  const std::string closer = kw == "end" ? "End " + blockName(closes) : blockName(kw);
                  m_diagnostics.push_back(at(lastWord, '\'' + closer + "' without '" + blockName(closes) + '\''));
               }
               else
               {
                  // The blocks opened after the closed one were not closed
                  while (blocks.back().kw != closes)
                  {
                     reportUnclosed(blocks.back());
                     blocks.pop_back();
                  }
                  blocks.pop_back();
               }
            }
         }

         const auto openBlock = [&](const SymbolKind kind, const size_t nameWord)
         {
            if (nameWord >= words.size())
               return;
            const int index = addSymbol(kind, words[nameWord].text, signature, line0, comment());
            stack.push_back(index);
         };

         // declaration list: ident[(...)] [= value] separated by commas — value-side
         // identifiers (after a top-level '=') are skipped by tracking paren depth
         const auto addDecls = [&](const SymbolKind kind, const std::string_view list)
         {
            for (const Word ident : ScanWords(list))
            {
               const size_t p = static_cast<size_t>(ident.text.data() - list.data());
               size_t depth = 0;
               bool value = false;
               for (size_t q = 0; q < p; ++q)
               {
                  const char c = list[q];
                  if (c == '(' || c == '[')
                     ++depth;
                  else if (c == ')' || c == ']')
                     depth = depth > 0 ? depth - 1 : 0;
                  else if (c == '=' && depth == 0)
                     value = true;
                  else if (c == ',' && depth == 0)
                     value = false;
               }
               if (!value)
                  addSymbol(kind, ident.text, signature, line0, comment());
            }
         };

         if (kw == "sub" || kw == "function")
            openBlock(kw == "sub" ? SymbolKind::Sub : SymbolKind::Function, w + 1);
         else if (kw == "class")
            openBlock(SymbolKind::Class, w + 1);
         else if (kw == "property")
         {
            const std::string pk = wordAt(w + 1);
            const SymbolKind kind = pk == "get" ? SymbolKind::PropGet : pk == "let" ? SymbolKind::PropLet : pk == "set" ? SymbolKind::PropSet : SymbolKind::Dim;
            if (kind != SymbolKind::Dim)
               openBlock(kind, w + 2);
         }
         else if (kw == "end")
         {
            const std::string ek = wordAt(w + 1);
            const SymbolKind want = ek == "sub" ? SymbolKind::Sub
               : ek == "function"               ? SymbolKind::Function
               : ek == "class"                  ? SymbolKind::Class
               : ek == "property"               ? SymbolKind::PropGet // any property kind pops
                                                : SymbolKind::Dim;
            if (want != SymbolKind::Dim && !stack.empty())
            {
               const SymbolKind top = m_symbols[static_cast<size_t>(stack.back())].kind;
               const bool isProp = top == SymbolKind::PropGet || top == SymbolKind::PropLet || top == SymbolKind::PropSet;
               if (top == want || (ek == "property" && isProp))
                  stack.pop_back();
            }
         }
         else if (kw == "dim" || kw == "const")
            addDecls(kw == "dim" ? SymbolKind::Dim : SymbolKind::Const, stmtCode.substr(words[w].start + words[w].text.size()));
         else if (kw == "set")
         {
            // Set x = New Class / Set x = part -> remember the variable's target so
            // member completion can resolve x's members (class members or interface)
            if (words.size() == w + 4 && wordAt(w + 2) == "new")
               m_varClass[Lower(words[w + 1].text)] = Lower(words[w + 3].text);
            else if (words.size() == w + 3)
               m_varClass[Lower(words[w + 1].text)] = Lower(words[w + 2].text);
         }
         else if (w < words.size() && !stack.empty() && m_symbols[static_cast<size_t>(stack.back())].kind == SymbolKind::Class)
         {
            // inside a class, an unrecognized declaration-shaped statement is a field:
            // `Private x`, `Public x, y` (Dim is implicit; modifiers already skipped)
            addDecls(SymbolKind::Dim, stmtCode.substr(words[w].start));
         }
      };

      for (size_t i = 0; i <= code.size(); ++i)
      {
         if (i == code.size() || code[i] == ':')
         {
            runStatement(stmtStart, i);
            stmtStart = i + 1;
         }
      }

      seg0 = segN + 1;
   }

   for (const OpenBlock &block : blocks)
      reportUnclosed(block);
   std::ranges::stable_sort(m_diagnostics, {}, [](const ScriptDiagnostic &d) { return d.line; });
}

std::vector<const Symbol *> ScriptModel::TopLevel() const
{
   std::vector<const Symbol *> out;
   for (const Symbol &s : m_symbols)
      if (s.parent < 0)
         out.push_back(&s);
   return out;
}

const Symbol *ScriptModel::Find(const std::string_view name) const
{
   const std::string lower = Lower(name);
   for (const Symbol &s : m_symbols)
      if (s.parent < 0 && s.lowerName == lower)
         return &s;
   for (const Symbol &s : m_symbols)
      if (s.lowerName == lower)
         return &s;
   return nullptr;
}

std::vector<const Symbol *> ScriptModel::MembersOf(const std::string_view className) const
{
   std::vector<const Symbol *> out;
   const std::string lower = Lower(className);
   for (const Symbol &s : m_symbols)
      if (s.kind == SymbolKind::Class && s.lowerName == lower)
         for (const int child : s.children)
            out.push_back(&m_symbols[static_cast<size_t>(child)]);
   return out;
}

std::string ScriptModel::ResolveClassName(const std::string_view name) const
{
   const std::string lower = Lower(name);
   for (const Symbol &s : m_symbols)
      if (s.kind == SymbolKind::Class && s.lowerName == lower)
         return s.lowerName;
   const auto it = m_varClass.find(lower);
   return it != m_varClass.end() ? it->second : std::string();
}

} // namespace VPX::EditorUI
