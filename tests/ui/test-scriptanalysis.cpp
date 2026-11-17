// license:GPLv3+

#include "core/stdafx.h"

#include "ui/live/editor/script/ScriptAnalysis.h"

#include "textedit/VbsLex.h"

#include "doctest.h"

using VPX::EditorUI::ScriptModel;
using VPX::EditorUI::Symbol;
using VPX::EditorUI::SymbolKind;

namespace
{

const Symbol *FindKind(const ScriptModel &m, const char *name, const SymbolKind kind)
{
   for (const Symbol &s : m.Symbols())
      if (s.lowerName == name && s.kind == kind)
         return &s;
   return nullptr;
}

TEST_CASE("ScriptModel: subs, functions and line numbers")
{
   ScriptModel m;
   m.Parse("Option Explicit\n"
           "\n"
           "Sub Init()\n"
           "End Sub\n"
           "\n"
           "Function Add(a, b)\n"
           "   Add = a + b\n"
           "End Function\n");

   REQUIRE(m.Symbols().size() == 2);
   const Symbol *init = m.Find("init");
   REQUIRE(init != nullptr);
   CHECK(init->kind == SymbolKind::Sub);
   CHECK(init->name == "Init");
   CHECK(init->line == 2);
   CHECK(init->signature == "Sub Init()");
   const Symbol *add = m.Find("ADD");
   REQUIRE(add != nullptr);
   CHECK(add->kind == SymbolKind::Function);
   CHECK(add->signature == "Function Add(a, b)");
   CHECK(m.TopLevel().size() == 2);
}

TEST_CASE("ScriptModel: colon separated statements and modifiers")
{
   ScriptModel m;
   m.Parse("Public Sub A() : End Sub\n"
           "Private Const K = 1 : Dim x, y(10), z\n"
           "Default Property Get P() : End Property\n");

   CHECK(m.Find("a") != nullptr);
   CHECK(m.Find("k") != nullptr);
   CHECK(m.Find("x") != nullptr);
   CHECK(m.Find("y") != nullptr);
   CHECK(m.Find("z") != nullptr);
   const Symbol *p = FindKind(m, "p", SymbolKind::PropGet);
   REQUIRE(p != nullptr);
   CHECK(p->signature == "Default Property Get P()");
}

TEST_CASE("ScriptModel: line continuations join into one declaration")
{
   ScriptModel m;
   m.Parse("Sub Load(ByVal name, _\n"
           "         ByRef count)\n"
           "End Sub\n");

   const Symbol *load = m.Find("load");
   REQUIRE(load != nullptr);
   CHECK(load->line == 0);
   CHECK(load->signature == "Sub Load(ByVal name, ByRef count)");
}

TEST_CASE("ScriptModel: comments and strings are skipped")
{
   ScriptModel m;
   m.Parse("' Sub InComment()\n"
           "Rem Function InRem()\n"
           "Dim s : s = \"Sub InString()\"\n"
           "Dim t : t = \"don't stop \"\"now\"\"\" ' Sub Fake()\n"
           "Sub Real() ' the real one\n"
           "End Sub\n");

   CHECK(m.Find("incomment") == nullptr);
   CHECK(m.Find("inrem") == nullptr);
   CHECK(m.Find("instring") == nullptr);
   CHECK(m.Find("fake") == nullptr);
   const Symbol *real = m.Find("real");
   REQUIRE(real != nullptr);
   CHECK(real->comment == "the real one");
   CHECK(m.Find("s") != nullptr);
   CHECK(m.Find("t") != nullptr);
}

TEST_CASE("ScriptModel: classes and member lookup")
{
   ScriptModel m;
   m.Parse("Class Point\n"
           "   Public X, Y\n"
           "   Private m_tag\n"
           "   Public Sub Move(dx, dy)\n"
           "      Dim tmp\n"
           "   End Sub\n"
           "   Public Property Get Name()\n"
           "   End Property\n"
           "End Class\n"
           "Dim origin\n");

   const Symbol *cls = m.Find("point");
   REQUIRE(cls != nullptr);
   CHECK(cls->kind == SymbolKind::Class);
   CHECK(cls->children.size() == 5); // X, Y, m_tag, Move, Name

   const auto members = m.MembersOf("POINT");
   std::vector<std::string> names;
   for (const Symbol *s : members)
      names.push_back(s->name);
   CHECK(std::find(names.begin(), names.end(), "X") != names.end());
   CHECK(std::find(names.begin(), names.end(), "Y") != names.end());
   CHECK(std::find(names.begin(), names.end(), "m_tag") != names.end());
   CHECK(std::find(names.begin(), names.end(), "Move") != names.end());
   CHECK(std::find(names.begin(), names.end(), "Name") != names.end());
   // Move's local Dim is not a class member
   CHECK(std::find(names.begin(), names.end(), "tmp") == names.end());
   // but it is still a known symbol inside the Sub
   const Symbol *tmp = m.Find("tmp");
   REQUIRE(tmp != nullptr);
   const Symbol *move = FindKind(m, "move", SymbolKind::Sub);
   REQUIRE(move != nullptr);
   CHECK(tmp->parent == static_cast<int>(move - m.Symbols().data()));
}

TEST_CASE("ScriptModel: Set x = New C resolves a variable's class")
{
   ScriptModel m;
   m.Parse("Class Ball\n"
           "   Public Sub Roll()\n"
           "   End Sub\n"
           "End Class\n"
           "Dim b\n"
           "Set b = New Ball\n");

   CHECK(m.ResolveClassName("B") == "ball");
   CHECK(m.ResolveClassName("Ball") == "ball");
   CHECK(m.ResolveClassName("nope").empty());
   const auto members = m.MembersOf(m.ResolveClassName("b"));
   REQUIRE(members.size() == 1);
   CHECK(members[0]->name == "Roll");
}

TEST_CASE("ScriptModel: doc comment block above a declaration")
{
   ScriptModel m;
   m.Parse("' Draws the playfield.\n"
           "' Called every frame.\n"
           "Sub Draw()\n"
           "End Sub\n");

   const Symbol *draw = m.Find("draw");
   REQUIRE(draw != nullptr);
   CHECK(draw->comment == "Draws the playfield.\nCalled every frame.");
}

TEST_CASE("ScriptModel: malformed input stays safe")
{
   ScriptModel m;
   m.Parse("Sub\nEnd Sub\nClass\nProperty Get\nEnd Property\nDim\nConst\n' trailing comment only");
   // Sub/Class/Property without a name produce nothing, same for bare Dim/Const
   CHECK(m.Symbols().empty());
   m.Parse("");
   CHECK(m.Symbols().empty());
}

TEST_CASE("VbsLex: SmartIndentDelta block awareness")
{
   using VbsLex::SmartIndentDelta;

   // block openers indent the following line
   CHECK(SmartIndentDelta("Sub Foo()") == 1);
   CHECK(SmartIndentDelta("Private Function Foo()") == 1);
   CHECK(SmartIndentDelta("Public Property Get X()") == 1);
   CHECK(SmartIndentDelta("Class Foo") == 1);
   CHECK(SmartIndentDelta("If x Then") == 1);
   CHECK(SmartIndentDelta("If x Then ' comment") == 1);
   CHECK(SmartIndentDelta("Else") == 1);
   CHECK(SmartIndentDelta("ElseIf x Then") == 1);
   CHECK(SmartIndentDelta("For i = 1 To 10") == 1);
   CHECK(SmartIndentDelta("For Each f In c") == 1);
   CHECK(SmartIndentDelta("Do While x") == 1);
   CHECK(SmartIndentDelta("While x") == 1);
   CHECK(SmartIndentDelta("Select Case a") == 1);
   CHECK(SmartIndentDelta("Case 1, 2") == 1);
   CHECK(SmartIndentDelta("Case Else") == 1);
   CHECK(SmartIndentDelta("With x") == 1);

   // block closers dedent the following line
   CHECK(SmartIndentDelta("End Sub") == -1);
   CHECK(SmartIndentDelta("End If") == -1);
   CHECK(SmartIndentDelta("End Select") == -1);
   CHECK(SmartIndentDelta("End Class") == -1);
   CHECK(SmartIndentDelta("Next") == -1);
   CHECK(SmartIndentDelta("Loop") == -1);
   CHECK(SmartIndentDelta("Wend") == -1);

   // everything else keeps the same indent
   CHECK(SmartIndentDelta("x = 1") == 0);
   CHECK(SmartIndentDelta("If x Then y") == 0); // single-line If
   CHECK(SmartIndentDelta("If x Then : y") == 0);
   CHECK(SmartIndentDelta("End") == 0); // bare End ends the script
   CHECK(SmartIndentDelta("") == 0);
   CHECK(SmartIndentDelta("' just a comment") == 0);
   CHECK(SmartIndentDelta("x = \"End Sub\"") == 0); // inside a string, not a closer
}
}
