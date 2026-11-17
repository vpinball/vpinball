// Shared VBScript lexical helpers, used by the TextEditor VBScript language
// definition (languages/TextEditorVbs.h) and by the live editor's script model
// (src/ui/live/editor/script/ScriptAnalysis.h). Free functions templated on a
// forward iterator whose value is a character/codepoint (char, ImWchar): both
// plain string iterators and TextEditor::Iterator qualify.
#pragma once

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
