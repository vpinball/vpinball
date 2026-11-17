// Shared VBScript lexical helpers, used by the TextEditor VBScript language
// definition (languages/TextEditorVbs.h) and by the live editor's script model
// (src/ui/live/editor/script/ScriptAnalysis.h). Free functions templated on a
// forward iterator whose value is a character/codepoint (char, ImWchar): both
// plain string iterators and TextEditor::Iterator qualify.
#pragma once

#include <string>
#include <string_view>
#include <vector>

namespace VbsLex
{

template<typename C> constexpr bool IsIdentStart(C c) { return (c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z') || (c == '_'); }
template<typename C> constexpr bool IsIdentChar(C c) { return IsIdentStart(c) || (c >= '0' && c <= '9'); }
template<typename C> constexpr bool IsDigit(C c) { return (c >= '0') && (c <= '9'); }
template<typename C> constexpr bool IsOctDigit(C c) { return (c >= '0') && (c <= '7'); }
template<typename C> constexpr bool IsHexDigit(C c) { return IsDigit(c) || (c >= 'a' && c <= 'f') || (c >= 'A' && c <= 'F'); }
template<typename C> constexpr C ToLower(C c) { return (c >= 'A' && c <= 'Z') ? static_cast<C>(c - 'A' + 'a') : c; }

// Identifier: [A-Za-z_][A-Za-z0-9_]*. Returns the end of the token, or `start`
// when no identifier begins there.
template<typename It> It ScanIdentifier(It start, It end)
{
   if (start == end || !IsIdentStart(*start))
      return start;
   It i = start;
   ++i;
   while (i != end && IsIdentChar(*i))
      ++i;
   return i;
}

// Number: decimal with optional fraction/exponent, &H hex, &O octal, bare &
// octal. Returns the end of the token, or `start` when no number begins there.
template<typename It> It ScanNumber(It start, It end)
{
   if (start == end)
      return start;
   It i = start;
   if (*i == '&')
   {
      ++i;
      if (i == end)
         return start;
      const auto c = ToLower(*i);
      if (c == 'h' || c == 'o')
      {
         ++i;
         const It first = i;
         while (i != end && (c == 'h' ? IsHexDigit(*i) : IsOctDigit(*i)))
            ++i;
         return i == first ? start : i;
      }
      // bare &octal: i already sits on the first potential digit
      const It first = i;
      while (i != end && IsOctDigit(*i))
         ++i;
      return i == first ? start : i;
   }
   bool any = false;
   while (i != end && IsDigit(*i))
   {
      ++i;
      any = true;
   }
   if (i != end && *i == '.')
   {
      ++i;
      while (i != end && IsDigit(*i))
      {
         ++i;
         any = true;
      }
   }
   if (!any)
      return start;
   if (i != end && ToLower(*i) == 'e')
   {
      It j = i;
      ++j;
      if (j != end && (*j == '+' || *j == '-'))
         ++j;
      const It firstDigit = j;
      while (j != end && IsDigit(*j))
         ++j;
      if (j != firstDigit)
         i = j;
   }
   return i;
}

// Whole-word, case-insensitive keyword test: `it` must sit on an identifier
// start in [begin, end); `word` is lowercase ASCII. Returns the identifier end
// when it spells exactly `word`, `it` otherwise.
template<typename It> It MatchWord(It it, It end, const char *word)
{
   It identEnd = ScanIdentifier(it, end);
   if (identEnd == it)
      return it;
   It i = it;
   const char *w = word;
   for (; i != identEnd && *w != 0; ++i, ++w)
      if (ToLower(*i) != *w)
         return it;
   return (i == identEnd && *w == 0) ? identEnd : it;
}

// A standalone 'rem' word opens a line comment (rest of the line). Returns the
// line end when `it` begins a rem comment, `it` otherwise.
template<typename It> It MatchRemComment(It it, It end)
{
   return MatchWord(it, end, "rem") != it ? end : it;
}

inline void StripCommentsAndStrings(const char *begin, const char *end, char *out); // defined below

// Collect the code words of a statement (identifiers only, everything else
// skipped): `code` is a statement text with comments and strings already
// blanked (see StripCommentsAndStrings). Words are lowercased.
inline std::vector<std::string> CodeWords(const std::string_view code)
{
   std::vector<std::string> words;
   for (size_t i = 0; i < code.size();)
   {
      if (!IsIdentStart(code[i]))
      {
         ++i;
         continue;
      }
      const size_t start = i;
      while (i < code.size() && IsIdentChar(code[i]))
         ++i;
      std::string word;
      word.reserve(i - start);
      for (size_t j = start; j < i; ++j)
         word += static_cast<char>(ToLower(code[j]));
      words.push_back(std::move(word));
   }
   return words;
}

// Block-aware indent delta for auto-indent (see TextEditor::Language::autoIndentDelta):
// `code` is the raw code before the cursor on the line where Enter was pressed;
// comments and strings are blanked internally. Returns +1 when the statement
// opens a block, -1 when it closes one, 0 otherwise.
inline int SmartIndentDelta(const std::string_view code)
{
   std::string stripped(code.size(), ' ');
   StripCommentsAndStrings(code.data(), code.data() + code.size(), stripped.data());
   const std::vector<std::string> words = CodeWords(stripped);
   if (words.empty())
      return 0;

   size_t w = 0;
   // declaration modifiers before Sub/Function/Property/Class
   while (w < words.size() && (words[w] == "public" || words[w] == "private" || words[w] == "default"))
      ++w;
   if (w == words.size())
      return 0;
   const std::string &first = words[w];

   // block closers: `End Sub/If/...`, `Next`, `Loop`, `Wend`
   if (first == "next" || first == "loop" || first == "wend")
      return -1;
   if (first == "end")
   {
      if (w + 1 < words.size())
      {
         const std::string &what = words[w + 1];
         if (what == "sub" || what == "function" || what == "if" || what == "select" || what == "class" || what == "property" || what == "with")
            return -1;
      }
      return 0; // bare `End` ends the script, not a block
   }

   // block openers
   if (first == "sub" || first == "function" || first == "class" || first == "for" || first == "do" || first == "while" || first == "with" || first == "else" || first == "elseif"
      || first == "case")
      return 1;
   if (first == "property" && w + 1 < words.size() && (words[w + 1] == "get" || words[w + 1] == "let" || words[w + 1] == "set"))
      return 1;
   // `Select Case x` opens; `If ... Then` only opens when `then` is the last word
   // (single-line `If x Then y` and `If x Then : y` have code after it)
   if (first == "select")
      return w + 1 < words.size() && words[w + 1] == "case" ? 1 : 0;
   if (first == "if")
      return words.back() == "then" ? 1 : 0;
   return 0;
}

// Companion of SmartIndentDelta for re-indentation: how the statement's own
// line indents relative to the enclosing block level. Returns -1 when the
// statement backs one level out of the block (hard closers `End X`, `Next`,
// `Loop`, `Wend`, and mid-block statements `Else`, `ElseIf`, `Case`), 0
// otherwise. Combined with SmartIndentDelta as:
//   lineIndent = level + SelfDelta;  level += SelfDelta + Delta
inline int SmartIndentSelfDelta(const std::string_view code)
{
   std::string stripped(code.size(), ' ');
   StripCommentsAndStrings(code.data(), code.data() + code.size(), stripped.data());
   const std::vector<std::string> words = CodeWords(stripped);
   size_t w = 0;
   while (w < words.size() && (words[w] == "public" || words[w] == "private" || words[w] == "default"))
      ++w;
   if (w == words.size())
      return 0;
   const std::string &first = words[w];
   if (first == "next" || first == "loop" || first == "wend" || first == "else" || first == "elseif" || first == "case")
      return -1;
   if (first == "end" && w + 1 < words.size())
   {
      const std::string &what = words[w + 1];
      if (what == "sub" || what == "function" || what == "if" || what == "select" || what == "class" || what == "property" || what == "with")
         return -1;
   }
   return 0;
}

// The statement text of a physical line with comments (' and rem) and string
// literals removed: characters outside code are blanked to space so positions
// are preserved. `in`/`out` are char-based (UTF-8 is safe: VBS code is ASCII).
inline void StripCommentsAndStrings(const char *begin, const char *end, char *out)
{
   const char *s = begin;
   char *o = out;
   while (s < end)
   {
      const char c = *s;
      if (c == '\'')
         break; // ' comment: rest of line
      if (c == '"')
      {
         *o++ = ' ';
         ++s;
         while (s < end)
         {
            if (*s == '"')
            {
               *o++ = ' ';
               ++s;
               if (s < end && *s == '"')
               { // "" escape: still inside the string
                  *o++ = ' ';
                  ++s;
               }
               else
                  break;
            }
            else
            {
               *o++ = ' ';
               ++s;
            }
         }
         continue;
      }
      // standalone rem word: comment to end of line
      if (MatchRemComment(s, end) != s)
         break;
      *o++ = c;
      ++s;
   }
   while (s++ < end)
      *o++ = ' ';
}
}
