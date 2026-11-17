// VBScript language definition for ImGuiColorTextEdit (VPX vendored addition)
// case-insensitive; ' and rem comments; double-quoted strings with "" escape;
// decimal/&H/&O numbers. Word lists follow the legacy win32 codeview editor
// (src/ui/win/codeview.cpp vbsReservedWords) so both editors highlight alike.
#pragma once

#include "../TextEditor.h"
#include "../VbsLex.h"

struct TextEditorVbs {
	static TextEditor::Iterator getIdentifier(TextEditor::Iterator start, TextEditor::Iterator end) { return VbsLex::ScanIdentifier(start, end); }
	static TextEditor::Iterator getNumber(TextEditor::Iterator start, TextEditor::Iterator end) { return VbsLex::ScanNumber(start, end); }
	static bool isPunctuation(ImWchar codepoint)
	{
		switch (codepoint)
		{
		case '!':
		case '#':
		case '%':
		case '&':
		case '(':
		case ')':
		case '*':
		case '+':
		case ',':
		case '-':
		case '.':
		case '/':
		case ':':
		case '<':
		case '=':
		case '>':
		case '@':
		case '[':
		case '\\':
		case ']':
		case '^':
		case '{':
		case '|':
		case '}':
		case '~':
			return true;
		default:
			return false;
		}
	}

	// a standalone 'rem' word comments out the rest of the line
	static TextEditor::Iterator customTokenizer(TextEditor::Iterator start, TextEditor::Iterator end, TextEditor::Color& color)
	{
		TextEditor::Iterator i = VbsLex::MatchRemComment(start, end);
		if (i == start)
			return start;
		color = TextEditor::Color::comment;
		return i;
	}

	static const TextEditor::Language* getDefinition()
	{
		static bool inited = false;
		static TextEditor::Language language;
		if (!inited)
		{
			language.name = "VBScript";
			language.caseSensitive = false;

			language.singleLineComment = "'";

			language.hasDoubleQuotedStrings = true;
			language.stringEscape = 0; // doubled "" escapes, no backslash

			language.getIdentifier = getIdentifier;
			language.getNumber = getNumber;
			language.isPunctuation = isPunctuation;
			language.customTokenizer = customTokenizer;
			language.autoIndentDelta = VbsLex::SmartIndentDelta;

			language.keywords = {
				"and",
				"as",
				"byref",
				"byval",
				"call",
				"case",
				"class",
				"const",
				"continue",
				"default",
				"dim",
				"do",
				"each",
				"else",
				"elseif",
				"empty",
				"end",
				"erase",
				"error",
				"execute",
				"executeglobal",
				"exit",
				"explicit",
				"false",
				"for",
				"function",
				"get",
				"goto",
				"if",
				"in",
				"let",
				"loop",
				"me",
				"new",
				"next",
				"not",
				"nothing",
				"null",
				"on",
				"option",
				"optional",
				"or",
				"private",
				"property",
				"public",
				"randomize",
				"redim",
				"rem",
				"resume",
				"select",
				"set",
				"sub",
				"then",
				"to",
				"true",
				"type",
				"until",
				"wend",
				"while",
				"with",
			};

			language.declarations = {
				"boolean",
				"byte",
				"currency",
				"date",
				"double",
				"integer",
				"long",
				"object",
				"single",
				"string",
				"variant",
			};

			language.identifiers = {
				// VBScript builtin functions
				"abs",
				"array",
				"asc",
				"ascb",
				"ascw",
				"atn",
				"cbool",
				"cbyte",
				"ccur",
				"cdate",
				"cdbl",
				"chr",
				"chrb",
				"chrw",
				"cint",
				"clng",
				"cos",
				"createobject",
				"csng",
				"cstr",
				"dateadd",
				"datediff",
				"datepart",
				"dateserial",
				"datevalue",
				"day",
				"eval",
				"exp",
				"filter",
				"formatcurrency",
				"formatdatetime",
				"formatnumber",
				"formatpercent",
				"getlocale",
				"getobject",
				"getref",
				"hex",
				"hour",
				"inputbox",
				"instr",
				"instrb",
				"instrrev",
				"int",
				"isarray",
				"isdate",
				"isempty",
				"isnull",
				"isnumeric",
				"isobject",
				"join",
				"lbound",
				"lcase",
				"left",
				"leftb",
				"len",
				"lenb",
				"loadpicture",
				"log",
				"ltrim",
				"mid",
				"midb",
				"minute",
				"month",
				"monthname",
				"msgbox",
				"now",
				"oct",
				"replace",
				"rgb",
				"right",
				"rightb",
				"rnd",
				"round",
				"rtrim",
				"scriptengine",
				"scriptenginebuildversion",
				"scriptenginemajorversion",
				"scriptengineminorversion",
				"second",
				"setlocale",
				"sgn",
				"sin",
				"space",
				"split",
				"sqr",
				"strcomp",
				"string",
				"strreverse",
				"tan",
				"time",
				"timeserial",
				"timevalue",
				"timer",
				"trim",
				"typename",
				"ubound",
				"ucase",
				"vartype",
				"weekday",
				"weekdayname",
				"year",
				// VBScript named constants (vb*), keep in lower case
				"vbabort",
				"vbabortretryignore",
				"vbapplicationmodal",
				"vbarray",
				"vbblack",
				"vbblue",
				"vbboolean",
				"vbbyte",
				"vbcancel",
				"vbcr",
				"vbcritical",
				"vbcrlf",
				"vbcurrency",
				"vbcyan",
				"vbdataobject",
				"vbdate",
				"vbdecimal",
				"vbdefaultbutton1",
				"vbdefaultbutton2",
				"vbdefaultbutton3",
				"vbdefaultbutton4",
				"vbdouble",
				"vbempty",
				"vberror",
				"vbexclamation",
				"vbfalse",
				"vbfirstfourdays",
				"vbfirstfullweek",
				"vbfirstjan1",
				"vbformfeed",
				"vbfriday",
				"vbgeneraldate",
				"vbgreen",
				"vbignore",
				"vbinformation",
				"vbinteger",
				"vblf",
				"vblong",
				"vblongdate",
				"vblongtime",
				"vbmagenta",
				"vbmonday",
				"vbmsgboxhelpbutton",
				"vbmsgboxright",
				"vbmsgboxrtlreading",
				"vbmsgboxsetforeground",
				"vbnewline",
				"vbno",
				"vbnull",
				"vbnullchar",
				"vbnullstring",
				"vbobject",
				"vbobjecterror",
				"vbok",
				"vbokcancel",
				"vbokonly",
				"vbquestion",
				"vbred",
				"vbretry",
				"vbretrycancel",
				"vbsaturday",
				"vbshortdate",
				"vbshorttime",
				"vbsingle",
				"vbstring",
				"vbsunday",
				"vbsystemmodal",
				"vbtab",
				"vbtextcompare",
				"vbbinarycompare",
				"vbthursday",
				"vbtrue",
				"vbtuesday",
				"vbvariant",
				"vbverticaltab",
				"vbwednesday",
				"vbwhite",
				"vbyellow",
				"vbyes",
				"vbyesno",
				"vbyesnocancel",
			};

			inited = true;
		}
		return &language;
	}
};
