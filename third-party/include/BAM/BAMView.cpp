#include "core/stdafx.h"
#include "BAM/BAM_ViewPortSetup.h"
#include "BAM/BAM_Tracker.h"
#include "imgui/imgui.h"
#include "tinyxml2/tinyxml2.h"
#include "BAMView.h"

namespace BAMView
{
using tinyxml2::XMLDocument;
using tinyxml2::XMLPrinter;

BAM_Tracker::BAM_Tracker_Client BAM;
constexpr float degToRad = 0.01745329251f;

struct BAMGlobalSettings
{
   int rotation;
   int swapWidthHeight;
   int forceBAMView;
   bool isVisible;

   BAMGlobalSettings() { memset(this, 0, sizeof(BAMGlobalSettings)); }
} settings;

struct BAMTableSettings
{
   float scale[3];
   float translation[3];
   float angle;
   float camera[3];
   std::string tableName;

   BAMTableSettings()
   {
      scale[0] = scale[1] = scale[2] = 1.0f;
      translation[0] = translation[1] = translation[2] = 0.0f;
      angle = 0.f;
      camera[0] = 0.f;
      camera[1] = -650.f;
      camera[2] = 500.f;
      tableName.clear();
   }
} g_TableSettings, g_DefaultSettings;

XMLDocument g_settings;

static void Mat4Mul(float* const __restrict O, const float* const __restrict A, const float* const __restrict B)
{
   O[0] = A[0] * B[0] + A[1] * B[4] + A[2] * B[8] + A[3] * B[12];
   O[1] = A[0] * B[1] + A[1] * B[5] + A[2] * B[9] + A[3] * B[13];
   O[2] = A[0] * B[2] + A[1] * B[6] + A[2] * B[10] + A[3] * B[14];
   O[3] = A[0] * B[3] + A[1] * B[7] + A[2] * B[11] + A[3] * B[15];

   O[4] = A[4] * B[0] + A[5] * B[4] + A[6] * B[8] + A[7] * B[12];
   O[5] = A[4] * B[1] + A[5] * B[5] + A[6] * B[9] + A[7] * B[13];
   O[6] = A[4] * B[2] + A[5] * B[6] + A[6] * B[10] + A[7] * B[14];
   O[7] = A[4] * B[3] + A[5] * B[7] + A[6] * B[11] + A[7] * B[15];

   O[8] = A[8] * B[0] + A[9] * B[4] + A[10] * B[8] + A[11] * B[12];
   O[9] = A[8] * B[1] + A[9] * B[5] + A[10] * B[9] + A[11] * B[13];
   O[10] = A[8] * B[2] + A[9] * B[6] + A[10] * B[10] + A[11] * B[14];
   O[11] = A[8] * B[3] + A[9] * B[7] + A[10] * B[11] + A[11] * B[15];

   O[12] = A[12] * B[0] + A[13] * B[4] + A[14] * B[8] + A[15] * B[12];
   O[13] = A[12] * B[1] + A[13] * B[5] + A[14] * B[9] + A[15] * B[13];
   O[14] = A[12] * B[2] + A[13] * B[6] + A[14] * B[10] + A[15] * B[14];
   O[15] = A[12] * B[3] + A[13] * B[7] + A[14] * B[11] + A[15] * B[15];
}

static void Mat4Mul(float* const __restrict OA, const float* const __restrict B)
{
   float A[16];
   memcpy_s(A, sizeof(A), OA, sizeof(A));
   Mat4Mul(OA, A, B);
}

static void Rotate(float* const __restrict V, float a)
{
   a *= degToRad;
   const float _S = sinf(a);
   const float _C = cosf(a);
   const float R[16] = { 1, 0, 0, 0, 0, _C, -_S, 0, 0, _S, _C, 0, 0, 0, 0, 1 };
   Mat4Mul(V, R);
}

static void Scale(float* const __restrict V, float x, float y, float z)
{
   const float S[16] = { x, 0, 0, 0, 0, y, 0, 0, 0, 0, z, 0, 0, 0, 0, 1 };
   Mat4Mul(V, S);
}

static void Scale(float* const __restrict V, const float* const __restrict s) { Scale(V, s[0], s[1], s[2]); }
static void Scale(float* const __restrict V, float s) { Scale(V, s, s, s); }

static void Translate(float* const __restrict V, float x, float y, float z)
{
   const float T[16] = { 1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1, 0, x, y, z, 1 };
   Mat4Mul(V, T);
}

static void Translate(float* const __restrict V, const float* const __restrict t) { Translate(V, t[0], t[1], t[2]); }

static void ApplyRST(float* const __restrict V, float a, const float* const __restrict s, const float* const __restrict t)
{
   Rotate(V, a);
   Scale(V, s);
   Translate(V, t);
}

static void _createProjectionAndViewMatrix(float* const __restrict P, float* const __restrict V)
{
   // VPX stuffs
   const PinTable* const t = g_pplayer->m_ptable;
   int resolutionWidth = g_pplayer->m_playfieldWnd->GetWidth();
   int resolutionHeight = g_pplayer->m_playfieldWnd->GetHeight();
   int rotation = static_cast<int>(g_pplayer->m_ptable->mViewSetups[g_pplayer->m_ptable->m_BG_current_set].GetRotation(resolutionWidth, resolutionHeight) / 90.0f);
   const float tableLength = t->m_bottom;
   const float tableWidth = t->m_right;
   const float tableGlass = t->m_glassTopHeight;
   rotation = (rotation + settings.rotation) % 4;

   // Data from config file (Settings):
   float DisplaySize;
   double DisplayNativeWidth;
   double DisplayNativeHeight;
   float AboveScreen;
   float InsideScreen;

   // Data from head tracking
   float ViewerPositionX, ViewerPositionY, ViewerPositionZ;

   // Get data from BAM Tracker
   if (BAM.IsBAMTrackerPresent())
   {
      // we use Screen Width & Height as Native Resolution. Only aspect ration is important
      DisplayNativeWidth = BAM.GetScreenWidth(); // [mm]
      DisplayNativeHeight = BAM.GetScreenHeight(); // [mm]
   }
   else
   {
      DisplayNativeWidth = static_cast<double>(resolutionWidth);
      DisplayNativeHeight = static_cast<double>(resolutionHeight);
   }

   if (settings.swapWidthHeight)
   {
      std::swap(DisplayNativeWidth, DisplayNativeHeight);
   }

   if (BAM.IsBAMTrackerPresent())
   {
      double x, y, z;
      BAM.GetPosition(x, y, z);
      ViewerPositionX = (float)x;
      ViewerPositionY = (float)y;
      ViewerPositionZ = (float)z;
   }
   else
   {
      DisplayNativeWidth *= 1300.0 / DisplayNativeHeight;
      DisplayNativeHeight = 1300.0;
      ViewerPositionX = g_TableSettings.camera[0];
      ViewerPositionY = g_TableSettings.camera[1];
      ViewerPositionZ = g_TableSettings.camera[2];
   }


   const double w = DisplayNativeWidth, h = DisplayNativeHeight;
   DisplaySize = (float)(sqrt(w * w + h * h) / 25.4); // [mm] -> [inchs]

   // constant params for this project
   AboveScreen = 200.0f; // 0.2m
   InsideScreen = 2000.0f; // 2.0m

   // Data build projection matrix
   BuildProjectionMatrix(P, DisplaySize, (float)DisplayNativeWidth, (float)DisplayNativeHeight, (float)resolutionWidth, (float)resolutionHeight, 0.0f, 0.0f, (float)resolutionWidth,
      (float)resolutionHeight, ViewerPositionX, ViewerPositionY, ViewerPositionZ, -AboveScreen, InsideScreen, rotation);

   // Build View matrix from parts: Translation, Scale, Rotation
   // .. but first View Matrix has camera position
   const float VT[16] = { 1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1, 0, -ViewerPositionX, -ViewerPositionY, -ViewerPositionZ, 1 };

   // --- Scale, ... some math
   const double pixelsToMillimeters = 25.4 * DisplaySize / sqrt(DisplayNativeWidth * DisplayNativeWidth + DisplayNativeHeight * DisplayNativeHeight);
   const double pixelsToMillimetersX = pixelsToMillimeters * DisplayNativeWidth / resolutionWidth;
   const double pixelsToMillimetersY = pixelsToMillimeters * DisplayNativeHeight / resolutionHeight;
   const double ptm = rotation & 1 ? pixelsToMillimetersX : pixelsToMillimetersY;
   const double tableLengthInMillimeters = ptm * tableLength;
   const double displayLengthInMillimeters = ptm * (rotation & 1 ? pixelsToMillimeters * DisplayNativeWidth : pixelsToMillimeters * DisplayNativeHeight);

   // --- Scale world to fit in screen
   const float scale = (float)(displayLengthInMillimeters / tableLengthInMillimeters); // calc here scale

   // combine all to one matrix
   static constexpr float I[16] = { 1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1 };
   memcpy_s(V, sizeof(I), I, sizeof(I));
   Scale(V, 1, -1, 1); // mirror on Y axis
   Translate(V, -tableWidth * 0.5f, tableLength * 0.5f, -tableGlass); // center table
   Scale(V, scale); // scale to fit in screen

   // User settings
   ApplyRST(V, g_TableSettings.angle, g_TableSettings.scale, g_TableSettings.translation);

   Mat4Mul(V, VT);
}

std::wstring GetFileNameForSettingsXML()
{
   const string path = g_pvp->m_myPrefPath + "BAMViewSettings.xml";
   return MakeWString(path);
}

std::string GetTableName()
{
   const PinTable* const t = g_pplayer->m_ptable;
   auto backslash = strrchr(t->m_filename.c_str(), '\\');
   auto slash = strrchr(t->m_filename.c_str(), '/');
   auto dst = std::max(backslash, slash);
   return dst ? dst + 1 : "Unknown"s;
}

bool SaveFile(const std::wstring& path, const void* data, SIZE_T size)
{
   HANDLE hFile = CreateFileW(path.c_str(), GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);

   DWORD err = GetLastError();

   if (hFile == INVALID_HANDLE_VALUE)
      return false;

   DWORD written = 0;
   if (WriteFile(hFile, data, (DWORD)size, &written, NULL) == FALSE)
   {
      err = GetLastError();
      return false;
   }

   CloseHandle(hFile);
   return true;
}

std::string LoadFile(const std::wstring& path)
{
   HANDLE hFile;
   OVERLAPPED ol = { 0 };

   hFile = CreateFileW(path.c_str(), GENERIC_READ,
      FILE_SHARE_READ, //FILE_SHARE_READ | FILE_FLAG_OVERLAPPED,
      NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);

   DWORD err = GetLastError();

   if (hFile == INVALID_HANDLE_VALUE)
      return std::string();

   LARGE_INTEGER size;
   if (!GetFileSizeEx(hFile, &size))
   {
      err = GetLastError();
      CloseHandle(hFile);
      return std::string();
   }

   std::string out;
   out.resize(static_cast<size_t>(size.QuadPart));
   DWORD bytesRead = 0;
   ReadFile(hFile, (void*)out.c_str(), static_cast<DWORD>(size.QuadPart), &bytesRead, &ol);

   CloseHandle(hFile);
   return out;
}

void SetTableSettingsInXML(const BAMTableSettings& t)
{
   XMLDocument& doc = g_settings;
   auto ts = doc.NewElement("TableSettings");

   ts->SetAttribute("name", t.tableName.c_str());

   auto tss = doc.NewElement("Scale");
   tss->SetAttribute("x", t.scale[0]);
   tss->SetAttribute("y", t.scale[1]);
   tss->SetAttribute("z", t.scale[2]);
   ts->InsertFirstChild(tss);

   auto tst = doc.NewElement("Translation");
   tst->SetAttribute("x", t.translation[0]);
   tst->SetAttribute("y", t.translation[1]);
   tst->SetAttribute("z", t.translation[2]);
   ts->InsertEndChild(tst);

   auto tsa = doc.NewElement("Angle");
   tsa->SetAttribute("x", t.angle);
   ts->InsertEndChild(tsa);

   auto tsc = doc.NewElement("Camera");
   tsc->SetAttribute("x", t.camera[0]);
   tsc->SetAttribute("y", t.camera[1]);
   tsc->SetAttribute("z", t.camera[2]);
   ts->InsertEndChild(tsc);


   tinyxml2::XMLElement* oldts = nullptr;
   for (auto r = doc.FirstChildElement("TableSettings"); r; r = r->NextSiblingElement())
   {
      auto szName = r->Attribute("name");
      if (szName && t.tableName == szName)
      {
         oldts = r;
         break;
      }
   }

   if (oldts)
   {
      doc.InsertAfterChild(oldts, ts);
      doc.DeleteChild(oldts);
   }
   else
   {
      doc.InsertEndChild(ts);
   }
}

void SaveXML(const BAMTableSettings* const tableSettings)
{
   XMLDocument& doc = g_settings;

   const auto& g = settings;
   auto gs = doc.FirstChildElement("GlobalSettings");
   if (gs == nullptr)
   {
      gs = doc.NewElement("GlobalSettings");
      doc.InsertFirstChild(gs);
   }

   //gs->SetAttribute("MenuKey", g.menuKey);
   gs->SetAttribute("ScreenRotation", g.rotation);
   gs->SetAttribute("SwapWidthHeight", g.swapWidthHeight);
   gs->SetAttribute("ForceBAMView", g.forceBAMView);

   if (tableSettings)
      SetTableSettingsInXML(*tableSettings);

   XMLPrinter prn;
   doc.Print(&prn);

   const auto fn = GetFileNameForSettingsXML();
   SaveFile(fn, prn.CStr(), prn.CStrSize() - 1);
}

BAMTableSettings LoadXML(tinyxml2::XMLElement* ts)
{
   BAMTableSettings t;

   auto szName = ts->Attribute("name");
   if (szName)
   {
      t.tableName = szName;
   }

   auto tss = ts->FirstChildElement("Scale");
   if (tss)
   {
      t.scale[0] = tss->FloatAttribute("x", 1.0f);
      t.scale[1] = tss->FloatAttribute("y", 1.0f);
      t.scale[2] = tss->FloatAttribute("z", 1.0f);
   }

   auto tst = ts->FirstChildElement("Translation");
   if (tst)
   {
      t.translation[0] = tst->FloatAttribute("x", 0.0f);
      t.translation[1] = tst->FloatAttribute("y", 0.0f);
      t.translation[2] = tst->FloatAttribute("z", 0.0f);
   }

   auto tsa = ts->FirstChildElement("Angle");
   if (tsa)
   {
      t.angle = tsa->FloatAttribute("x", 0.0f);
   }

   auto tsc = ts->FirstChildElement("Camera");
   if (tsc)
   {
      t.camera[0] = tsc->FloatAttribute("x", 0.0f);
      t.camera[1] = tsc->FloatAttribute("y", 0.0f);
      t.camera[2] = tsc->FloatAttribute("z", 0.0f);
   }
   return t;
}

void LoadXML()
{
   XMLDocument& doc = g_settings;

   auto fn = GetFileNameForSettingsXML();
   auto xml = LoadFile(fn);

   if (xml.empty())
      return;

   if (doc.Parse(xml.c_str(), xml.size()))
      return;

   auto gs = doc.FirstChildElement("GlobalSettings");
   if (gs)
   {
      auto& g = settings;
      g.rotation = gs->IntAttribute("ScreenRotation", 0);
      g.swapWidthHeight = gs->IntAttribute("SwapWidthHeight", 0);
      g.forceBAMView = gs->IntAttribute("ForceBAMView", 0);
   }

   auto tableName = GetTableName();
   for (auto ts = doc.FirstChildElement("TableSettings"); ts; ts = ts->NextSiblingElement())
   {
      auto szName = ts->Attribute("name");
      if (szName && tableName == szName)
      {
         g_TableSettings = LoadXML(ts);
         return;
      }
      if (szName && std::string("Default") == szName)
      {
         g_DefaultSettings = LoadXML(ts);
      }
   }
   g_TableSettings = g_DefaultSettings;
   g_TableSettings.tableName = tableName;
}

// =============================================================================

void init()
{
   settings = BAMGlobalSettings();
   g_TableSettings = BAMTableSettings();

   LoadXML();
   SaveXML(nullptr);
}

void drawMenu()
{
   //ImGuiIO& io = ImGui::GetIO();
   ImGui::SetNextWindowSizeConstraints(ImVec2(350, 320), ImVec2(FLT_MAX, FLT_MAX));

   bool p_open = true;
   if (ImGui::BeginPopupModal("Headtracking Settings", &p_open))
   {
      if (ImGui::CollapsingHeader("Global Settings", ImGuiTreeNodeFlags_DefaultOpen))
      {
         ImGui::Text("Screen rotation:");
         ImGui::SameLine();
         int& r = settings.rotation;
         ImGui::RadioButton("0", &r, 0);
         ImGui::SameLine();
         ImGui::RadioButton("90", &r, 1);
         ImGui::SameLine();
         ImGui::RadioButton("180", &r, 2);
         ImGui::SameLine();
         ImGui::RadioButton("270", &r, 3);

         bool v = settings.swapWidthHeight != 0;
         ImGui::Checkbox("Swap width/height", &v);
         settings.swapWidthHeight = v ? 1 : 0;

         v = settings.forceBAMView != 0;
         ImGui::Checkbox("Force BAM View", &v);
         settings.forceBAMView = v ? 1 : 0;
      }

      ImGui::Separator();
      if (ImGui::CollapsingHeader("Table Settings", ImGuiTreeNodeFlags_DefaultOpen))
      {
         auto& t = g_TableSettings;
         ImGui::DragFloat3("Translation", t.translation);
         ImGui::DragFloat3("Scale", t.scale, 0.01f);
         ImGui::DragFloat("Angle", &t.angle, 0.05f, -180.0f, 180.0f, "%.01f");
         if (!BAM.IsBAMTrackerPresent())
         {
            ImGui::DragFloat3("Camera", t.camera);
         }
      }

      int i = static_cast<int>(ImGui::GetWindowSize().x);
      ImGui::Separator();
      if (ImGui::Button("Save as Default", ImVec2(140, 30)))
      {
         g_DefaultSettings = g_TableSettings;
         g_DefaultSettings.tableName = "Default";
         SaveXML(&g_DefaultSettings);
      }
      ImGui::SameLine();

      ImGui::Indent(std::max(150.0f, (float)i - 100.0f));
      if (ImGui::Button("Save", ImVec2(60, 30)))
      {
         SaveXML(&g_TableSettings);
      }
      ImGui::EndPopup();
   }
}

void createProjectionAndViewMatrix(float* const __restrict P, float* const __restrict V)
{
   // If BAM tracker is not running, we will not do anything.
   if (BAM.IsBAMTrackerPresent() || settings.forceBAMView)
      _createProjectionAndViewMatrix(P, V);
}
} // namespace BAMView
