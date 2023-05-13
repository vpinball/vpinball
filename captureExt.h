#pragma once
#include "typedefs3D.h"

#include <vector>
#include <string>
#include <mutex>

bool captureExternalDMD();
bool capturePUP();
void captureStop();

// Disabled for DX9 since it causes conflict with DX9
#ifdef ENABLE_SDL
#include <d3d11.h>
#include <dxgi1_2.h>

enum ecStage { ecSearching, ecFoundWaiting, ecTexture, ecFailure, ecCapturing, ecUninitialized };

class ExtCapture;

class ExtCaptureOutput
{
public:
   ExtCaptureOutput(ExtCapture* cap) { m_cap = cap; }
   ~ExtCaptureOutput() { delete[] m_MetaDataBuffer; }
   void AcquireFrame();

   IDXGIOutputDuplication* m_duplication = nullptr;
   ID3D11Device* m_d3d_device = nullptr;
   ID3D11DeviceContext* m_d3d_context = nullptr;

   unsigned char* m_srcdata = nullptr;
   int m_pitch = 0;

   ID3D11Texture2D* m_staging_tex = nullptr;

private:
   char* m_MetaDataBuffer = nullptr;
   unsigned int m_MetaDataBufferSize = 0;
   ExtCapture* m_cap;
};

typedef std::map<std::tuple<int, int>, ExtCaptureOutput *> outputmaptype;
typedef vector<class ExtCapture*> capturelisttype;

class ExtCapture
{
public:
   bool SetupCapture(const RECT& inputRect);

   void Setup(const vector<string>& windowlist);
   void SearchWindow();
   bool GetFrame();

   static void Dispose(); // Call when you have deleted all instances.

   static outputmaptype m_duplicatormap;

   ecStage m_ecStage = ecUninitialized;
   HBITMAP m_HBitmap;
   void* m_pData;
   //BaseTexture* m_texture;

private:
   friend class ExtCaptureOutput;

   static capturelisttype m_allCaptures;
   vector<string> m_searchWindows;
   int m_delay;

   IDXGIAdapter1* m_Adapter = nullptr;
   IDXGIOutput1* m_Output1 = nullptr;
   IDXGIOutput* m_Output = nullptr;
   DXGI_OUTPUT_DESC m_outputdesc;

   D3D_FEATURE_LEVEL m_d3d_feature_level; /* The selected feature level (D3D version), selected from the Feature Levels array, which is NULL here; when it's NULL the default list is used see:  https://msdn.microsoft.com/en-us/library/windows/desktop/ff476082%28v=vs.85%29.aspx ) */
   ExtCaptureOutput *m_pCapOut;

   bool m_bDirty = false;

public:
   int m_DispTop = 0, m_DispLeft = 0;
   int m_Width, m_Height = 0;
};
#endif
