#include "stdafx.h"
#include "RenderDevice.h"
#include "nvapi.h"
#include "Material.h"
#include "BasicShader.h"
#include "DMDShader.h"

#pragma comment(lib, "d3d9.lib")        // TODO: put into build system

const VertexElement VertexTexelElement[] = 
{
   { 0, 0  * sizeof(float),D3DDECLTYPE_FLOAT3,   D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_POSITION, 0 },  // pos
   { 0, 3  * sizeof(float),D3DDECLTYPE_FLOAT2,   D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_TEXCOORD, 0 },  // tex0
   D3DDECL_END()
};
VertexDeclaration* RenderDevice::m_pVertexTexelDeclaration	= NULL;

const VertexElement VertexNormalTexelElement[] = 
{
   { 0, 0  * sizeof(float),D3DDECLTYPE_FLOAT3,   D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_POSITION, 0 },  // pos
   { 0, 3  * sizeof(float),D3DDECLTYPE_FLOAT3,   D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_NORMAL,   0 },  // normal
   { 0, 6  * sizeof(float),D3DDECLTYPE_FLOAT2,   D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_TEXCOORD, 0 },  // tex0
   D3DDECL_END()
};
VertexDeclaration* RenderDevice::m_pVertexNormalTexelDeclaration	= NULL;

const VertexElement VertexNormalTexelTexelElement[] = 
{
   { 0, 0  * sizeof(float),D3DDECLTYPE_FLOAT3,   D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_POSITION, 0 },  // pos
   { 0, 3  * sizeof(float),D3DDECLTYPE_FLOAT3,   D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_NORMAL,   0 },  // normal
   { 0, 6  * sizeof(float),D3DDECLTYPE_FLOAT2,   D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_TEXCOORD, 0 },  // tex0
   { 0, 8  * sizeof(float),D3DDECLTYPE_FLOAT2,   D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_TEXCOORD, 1 },  // tex1
   D3DDECL_END()
};

VertexDeclaration* RenderDevice::m_pVertexNormalTexelTexelDeclaration = NULL;

void ReportError(const HRESULT hr, const char *file, const int line)
{
    char msg[128];
    sprintf_s(msg, 128, "Fatal error: HRESULT %x at %s:%d", hr, file, line);
    ShowError(msg);
    exit(-1);
}

D3DTexture* TextureManager::LoadTexture(MemTexture* memtex)
{
    Iter it = m_map.find(memtex);
    if (it == m_map.end())
    {
        TexInfo texinfo;
        texinfo.d3dtex = m_rd.UploadTexture(memtex, &texinfo.texWidth, &texinfo.texHeight);
        if (!texinfo.d3dtex)
            return 0;
        texinfo.dirty = false;
        m_map[memtex] = texinfo;
        return texinfo.d3dtex;
    }
    else
    {
        if (it->second.dirty)
        {
            m_rd.UpdateTexture(it->second.d3dtex, memtex);
            it->second.dirty = false;
        }
        return it->second.d3dtex;
    }
}

void TextureManager::SetDirty(MemTexture* memtex)
{
    Iter it = m_map.find(memtex);
    if (it != m_map.end())
        it->second.dirty = true;
}

void TextureManager::UnloadTexture(MemTexture* memtex)
{
    Iter it = m_map.find(memtex);
    if (it != m_map.end())
    {
        it->second.d3dtex->Release();
        m_map.erase(it);
    }
}

void TextureManager::UnloadAll()
{
    for (Iter it = m_map.begin(); it != m_map.end(); ++it)
    {
        it->second.d3dtex->Release();
    }
    m_map.clear();
}

#define CHECKD3D(s) { HRESULT hr = (s); if (FAILED(hr)) ReportError(hr, __FILE__, __LINE__); }

static unsigned int fvfToSize(DWORD fvf)
{
    switch (fvf)
    {
        case MY_D3DFVF_VERTEX:
        case MY_D3DTRANSFORMED_VERTEX:
            return sizeof(Vertex3D);
        case MY_D3DFVF_NOTEX2_VERTEX:
        case MY_D3DTRANSFORMED_NOTEX2_VERTEX:
            return sizeof(Vertex3D_NoTex2);
        case MY_D3DFVF_NOTEX_VERTEX:
            return sizeof(Vertex3D_NoTex);
		case MY_D3DFVF_TEX:
			return 5*sizeof(float);
        default:
            assert(0 && "Unknown FVF type in fvfToSize");
            return 0;
    }
}

static UINT ComputePrimitiveCount(D3DPRIMITIVETYPE type, int vertexCount)
{
    switch (type)
    {
        case D3DPT_POINTLIST:
            return vertexCount;
        case D3DPT_LINELIST:
            return vertexCount / 2;
        case D3DPT_LINESTRIP:
            return std::max(0, vertexCount - 1);
        case D3DPT_TRIANGLELIST:
            return vertexCount / 3;
        case D3DPT_TRIANGLESTRIP:
        case D3DPT_TRIANGLEFAN:
            return std::max(0, vertexCount - 2);
        default:
            return 0;
    }
}

////////////////////////////////////////////////////////////////////

void EnumerateDisplayModes(int adapter, std::vector<VideoMode>& modes)
{
    IDirect3D9 *d3d;
    d3d = Direct3DCreate9(D3D_SDK_VERSION);
    if (d3d == NULL)
    {
        ShowError("Could not create D3D9 object.");
        throw 0;
    }

    modes.clear();

    for (int j = 0; j < 2; ++j)
    {
        const D3DFORMAT fmt = (j == 0) ? D3DFMT_X8R8G8B8 : D3DFMT_R5G6B5;
        const unsigned numModes = d3d->GetAdapterModeCount(adapter, fmt);

        for (unsigned i = 0; i < numModes; ++i)
        {
            D3DDISPLAYMODE d3dmode;
            d3d->EnumAdapterModes(adapter, fmt, i, &d3dmode);

            if (d3dmode.Width >= 640)
            {
                VideoMode mode;
                mode.width = d3dmode.Width;
                mode.height = d3dmode.Height;
                mode.depth = (fmt == D3DFMT_R5G6B5) ? 16 : 32;
                mode.refreshrate = d3dmode.RefreshRate;
                modes.push_back(mode);
            }
        }
    }

    d3d->Release();
}

////////////////////////////////////////////////////////////////////

//#define MY_IDX_BUF_SIZE 8192
#define MY_IDX_BUF_SIZE 65536

RenderDevice::RenderDevice(HWND hwnd, int width, int height, bool fullscreen, int screenWidth, int screenHeight, int colordepth, int &refreshrate, int VSync, bool useAA, bool stereo3DFXAA)
    : m_texMan(*this)
{
    m_adapter = D3DADAPTER_DEFAULT;     // for now, always use the default adapter

#ifdef USE_D3D9EX
    CHECKD3D(Direct3DCreate9Ex(D3D_SDK_VERSION, &m_pD3D));
    if (m_pD3D == NULL)
    {
        throw 0;
    }
#else
    m_pD3D = Direct3DCreate9(D3D_SDK_VERSION);
    if (m_pD3D == NULL)
    {
        ShowError("Could not create D3D9 object.");
        throw 0;
    }
#endif

    D3DDEVTYPE devtype = D3DDEVTYPE_HAL;

    // Look for 'NVIDIA PerfHUD' adapter
    // If it is present, override default settings
    // This only takes effect if run under NVPerfHud, otherwise does nothing
    for (UINT adapter=0; adapter < m_pD3D->GetAdapterCount(); adapter++)
    {
        D3DADAPTER_IDENTIFIER9 Identifier;
        m_pD3D->GetAdapterIdentifier(adapter, 0, &Identifier);
        if (strstr(Identifier.Description, "PerfHUD") != 0)
        {
            m_adapter = adapter;
            devtype = D3DDEVTYPE_REF;
            break;
        }
    }

    D3DCAPS9 caps;
    m_pD3D->GetDeviceCaps(m_adapter, devtype, &caps);

	// check which parameters can be used for anisotropic filter
    m_mag_aniso = (caps.TextureFilterCaps & D3DPTFILTERCAPS_MAGFANISOTROPIC) != 0;
    m_maxaniso = caps.MaxAnisotropy;

    if(((caps.TextureCaps & D3DPTEXTURECAPS_NONPOW2CONDITIONAL) != 0) || ((caps.TextureCaps & D3DPTEXTURECAPS_POW2) != 0))
		ShowError("D3D device does only support power of 2 textures");

    // get the current display format
    D3DFORMAT format;
    if (!fullscreen)
    {
        D3DDISPLAYMODE mode;
        CHECKD3D(m_pD3D->GetAdapterDisplayMode(m_adapter, &mode));
        format = mode.Format;
		refreshrate = mode.RefreshRate;
    }
    else
    {
        format = (colordepth == 16) ? D3DFMT_R5G6B5 : D3DFMT_X8R8G8B8;
    }

	// limit vsync rate to actual refresh rate, otherwise special handling in renderloop
	if(VSync > refreshrate)
		VSync = 0;

    D3DPRESENT_PARAMETERS params;
    params.BackBufferWidth = fullscreen ? screenWidth : width;
    params.BackBufferHeight = fullscreen ? screenHeight : height;
    params.BackBufferFormat = format;
    params.BackBufferCount = 1;
    params.MultiSampleType = useAA ? D3DMULTISAMPLE_4_SAMPLES : D3DMULTISAMPLE_NONE; // D3DMULTISAMPLE_NONMASKABLE?
    params.MultiSampleQuality = 0; // if D3DMULTISAMPLE_NONMASKABLE then set to > 0
    params.SwapEffect = D3DSWAPEFFECT_DISCARD;  // FLIP ?
    params.hDeviceWindow = hwnd;
    params.Windowed = !fullscreen;
    params.EnableAutoDepthStencil = FALSE;
    params.AutoDepthStencilFormat = D3DFMT_UNKNOWN;      // ignored
    params.Flags = /*stereo3DFXAA ?*/ 0 /*: D3DPRESENTFLAG_DISCARD_DEPTHSTENCIL*/;
    params.FullScreen_RefreshRateInHz = fullscreen ? refreshrate : 0;
#ifdef USE_D3D9EX
    params.PresentationInterval = D3DPRESENT_INTERVAL_IMMEDIATE; //!! or have a special mode to force normal vsync?
#else
    params.PresentationInterval = !!VSync ? D3DPRESENT_INTERVAL_ONE : D3DPRESENT_INTERVAL_IMMEDIATE;
#endif

	// check if auto generation of mipmaps can be used, otherwise will be done via d3dx
	m_autogen_mipmap = (caps.Caps2 & D3DCAPS2_CANAUTOGENMIPMAP) != 0;
	if(m_autogen_mipmap)
		m_autogen_mipmap = (m_pD3D->CheckDeviceFormat(m_adapter, devtype, params.BackBufferFormat, D3DUSAGE_AUTOGENMIPMAP, D3DRTYPE_TEXTURE, D3DFMT_A8R8G8B8)) == D3D_OK;

		// Determine if RESZ is supported
		m_RESZ_support = (m_pD3D->CheckDeviceFormat(m_adapter, devtype, params.BackBufferFormat,
						  D3DUSAGE_RENDERTARGET, D3DRTYPE_SURFACE, ((D3DFORMAT)(MAKEFOURCC('R','E','S','Z'))))) == D3D_OK;

	// check if requested MSAA is possible
    DWORD MultiSampleQualityLevels;
    if( !SUCCEEDED(m_pD3D->CheckDeviceMultiSampleType( m_adapter, 
                                devtype, params.BackBufferFormat, 
                                params.Windowed, params.MultiSampleType, &MultiSampleQualityLevels ) ) )
    {
		ShowError("D3D device does not support this MultiSampleType");
		params.MultiSampleType = D3DMULTISAMPLE_NONE;
		params.MultiSampleQuality = 0;
    }
    else
		params.MultiSampleQuality = min(params.MultiSampleQuality, MultiSampleQualityLevels);

    const int softwareVP = GetRegIntWithDefault("Player", "SoftwareVertexProcessing", 0);
    const DWORD flags = softwareVP ? D3DCREATE_SOFTWARE_VERTEXPROCESSING : D3DCREATE_HARDWARE_VERTEXPROCESSING;

    // Create the D3D device. This optionally goes to the proper fullscreen mode.
    // It also creates the default swap chain (front and back buffer).
#ifdef USE_D3D9EX
    D3DDISPLAYMODEEX mode;
    mode.Size = sizeof(D3DDISPLAYMODEEX);
    CHECKD3D(m_pD3D->CreateDeviceEx(
               m_adapter,
               devtype,
               hwnd,
               flags /*| D3DCREATE_PUREDEVICE*/,
               &params,
               fullscreen ? &mode : NULL,
               &m_pD3DDevice));

    // Get the display mode so that we can report back the actual refresh rate.
    CHECKD3D(m_pD3DDevice->GetDisplayModeEx(m_adapter, &mode, NULL));
#else
    CHECKD3D(m_pD3D->CreateDevice(
               m_adapter,
               devtype,
               hwnd,
               flags /*| D3DCREATE_PUREDEVICE*/,
               &params,
               &m_pD3DDevice));

    // Get the display mode so that we can report back the actual refresh rate.
    D3DDISPLAYMODE mode;
    CHECKD3D(m_pD3DDevice->GetDisplayMode(m_adapter, &mode));
#endif

    refreshrate = mode.RefreshRate;

    // Retrieve a reference to the back buffer.
    CHECKD3D(m_pD3DDevice->GetBackBuffer(0, 0, D3DBACKBUFFER_TYPE_MONO, &m_pBackBuffer));

    // Set up a dynamic index buffer to cache passed indices in
    CreateIndexBuffer(MY_IDX_BUF_SIZE, D3DUSAGE_DYNAMIC, IndexBuffer::FMT_INDEX16, &m_dynIndexBuffer);

    Texture::SetRenderDevice(this);

    m_curIndexBuffer = 0;
    m_curVertexBuffer = 0;
    memset(m_curTexture, 0, 8*sizeof(m_curTexture[0]));

    // fill state caches with dummy values
    memset( renderStateCache, 0xCC, sizeof(DWORD)*RENDER_STATE_CACHE_SIZE);
    memset( textureStateCache, 0xCC, sizeof(DWORD)*8*TEXTURE_STATE_CACHE_SIZE);
    //!! memset(&materialStateCache, 0xCC, sizeof(Material));

    // initialize performance counters
    m_curDrawCalls = m_frameDrawCalls = 0;
    m_curStateChanges = m_frameStateChanges = 0;
    m_curTextureChanges = m_frameTextureChanges = 0;

    basicShader = new Shader(this);
#if _MSC_VER >= 1700
    basicShader->Load(g_basicShaderCode, sizeof(g_basicShaderCode));
#else
    basicShader->Load(basicShaderCode, sizeof(basicShaderCode));
#endif

    DMDShader = new Shader(this);
#if _MSC_VER >= 1700
    DMDShader->Load(g_dmdShaderCode, sizeof(g_dmdShaderCode));
#else
    DMDShader->Load(dmdShaderCode, sizeof(dmdShaderCode));
#endif
    // create default vertex declarations for shaders
    CreateVertexDeclaration( VertexTexelElement, &m_pVertexTexelDeclaration );
    CreateVertexDeclaration( VertexNormalTexelElement, &m_pVertexNormalTexelDeclaration );
    CreateVertexDeclaration( VertexNormalTexelTexelElement, &m_pVertexNormalTexelTexelDeclaration );
}

#include <d3dx9.h> //!! meh
#pragma comment(lib, "d3dx9.lib")        // TODO: put into build system

#define CHECKNVAPI(s) { NvAPI_Status hr = (s); if (hr != NVAPI_OK) { NvAPI_ShortString ss; NvAPI_GetErrorMessage(hr,ss); MessageBox(NULL, ss, "NVAPI", MB_OK | MB_ICONEXCLAMATION); } }
static bool NVAPIinit = false; //!! meh
static RenderTarget *src_cache = NULL; //!! meh
static D3DTexture* dest_cache = NULL;
static IDirect3DPixelShader9 *gShader = NULL; //!! meh
static const char * shader_cache = NULL;


#ifdef _DEBUG
static void CheckForD3DLeak(IDirect3DDevice9* d3d)
{
    IDirect3DSwapChain9 *swapChain;
    CHECKD3D(d3d->GetSwapChain(0, &swapChain));

    D3DPRESENT_PARAMETERS pp;
    CHECKD3D(swapChain->GetPresentParameters(&pp));
    swapChain->Release();

    // idea: device can't be reset if there are still allocated resources
    HRESULT hr = d3d->Reset(&pp);
    if (FAILED(hr))
    {
        MessageBox(0, "WARNING! Direct3D resource leak detected!", "Visual Pinball", MB_ICONWARNING);
    }
}
#endif


RenderDevice::~RenderDevice()
{
	if(src_cache != NULL)
		CHECKNVAPI(NvAPI_D3D9_UnregisterResource(src_cache)); //!! meh
	src_cache = NULL;
	if(dest_cache != NULL)
		CHECKNVAPI(NvAPI_D3D9_UnregisterResource(dest_cache)); //!! meh
	dest_cache = NULL;
	if(NVAPIinit) //!! meh
		CHECKNVAPI(NvAPI_Unload());
	NVAPIinit = false;
	if(gShader != NULL) //!! meh
		gShader->Release();
	gShader = NULL;
	shader_cache = NULL;

	//
    if (basicShader)
    {
        delete basicShader;
        basicShader=0;
    }
    if (DMDShader)
    {
        delete DMDShader;
        DMDShader=0;
    }

    m_pD3DDevice->SetStreamSource(0, NULL, 0, 0);
    m_pD3DDevice->SetIndices(NULL);

    SAFE_RELEASE(m_dynIndexBuffer);

    SAFE_RELEASE(m_pVertexTexelDeclaration);
    SAFE_RELEASE(m_pVertexNormalTexelDeclaration);
    SAFE_RELEASE(m_pVertexNormalTexelTexelDeclaration);

    m_texMan.UnloadAll();

	SAFE_RELEASE(m_pBackBuffer);

#ifdef _DEBUG
    CheckForD3DLeak(m_pD3DDevice);
#endif

    m_pD3DDevice->Release();
    m_pD3D->Release();

    /*
     * D3D sets the FPU to single precision mode when it's initialized, but doesn't
     * bother to reset the FPU when it's destroyed. This creates some precision issues
     * and messed with the old RoundToInt function, so we reset it manually here.
     */
#ifdef FP_RESET_AFTER_D3D
    _fpreset();
#endif
}

void RenderDevice::BeginScene()
{
   CHECKD3D(m_pD3DDevice->BeginScene());
}

void RenderDevice::EndScene()
{
   CHECKD3D(m_pD3DDevice->EndScene());
}

void RenderDevice::CreatePixelShader( const char* shader )
{
	if(shader != shader_cache)
	{
		if(gShader != NULL) //!! meh
		{
			gShader->Release();
			gShader = NULL;
		}
		ID3DXBuffer *tmp;
		//ID3DXBuffer *error;
		CHECKD3D(D3DXCompileShader( shader, lstrlen(shader), 0, 0, "ps_main", "ps_2_a", D3DXSHADER_OPTIMIZATION_LEVEL3|D3DXSHADER_PREFER_FLOW_CONTROL, &tmp, /*&error*/0, 0 ));
		//MessageBox(NULL,(LPCSTR)(error->GetBufferPointer()),"bla",MB_OK);
		CHECKD3D(m_pD3DDevice->CreatePixelShader( (DWORD*)tmp->GetBufferPointer(), &gShader ));
		shader_cache = shader;
	}
	CHECKD3D(m_pD3DDevice->SetPixelShader(gShader));
}

void RenderDevice::SetPixelShaderConstants(const float* constantData, const unsigned int numFloat4s)
{
    m_pD3DDevice->SetPixelShaderConstantF(0, constantData, numFloat4s);
}

static void FlushGPUCommandBuffer(IDirect3DDevice9* pd3dDevice)
{
    IDirect3DQuery9* pEventQuery;
    pd3dDevice->CreateQuery(D3DQUERYTYPE_EVENT, &pEventQuery);

    if (pEventQuery)
    {
        pEventQuery->Issue(D3DISSUE_END);
        while (S_FALSE == pEventQuery->GetData(NULL, 0, D3DGETDATA_FLUSH))
            ;
        pEventQuery->Release();
    }
}

void RenderDevice::Flip(bool vsync)
{
#ifdef USE_D3D9EX
    if(vsync)
		m_pD3DDevice->WaitForVBlank(0);
#endif
    CHECKD3D(m_pD3DDevice->Present(NULL, NULL, NULL, NULL));

    // reset performance counters
    m_frameDrawCalls = m_curDrawCalls;
    m_frameStateChanges = m_curStateChanges;
    m_frameTextureChanges = m_curTextureChanges;
    m_curDrawCalls = m_curStateChanges = m_curTextureChanges = 0;
}

RenderTarget* RenderDevice::DuplicateRenderTarget(RenderTarget* src)
{
    D3DSURFACE_DESC desc;
    src->GetDesc(&desc);
    IDirect3DSurface9 *dup;
    CHECKD3D(m_pD3DDevice->CreateRenderTarget(desc.Width, desc.Height, desc.Format,
        desc.MultiSampleType, desc.MultiSampleQuality, FALSE /* lockable */, &dup, NULL));
    return dup;
}

void RenderDevice::CopySurface(RenderTarget* dest, RenderTarget* src)
{
    //CHECKD3D(m_pD3DDevice->StretchRect(src, NULL, dest, NULL, D3DTEXF_NONE));
   HRESULT hr = m_pD3DDevice->StretchRect(src, NULL, dest, NULL, D3DTEXF_NONE);
   if( FAILED(hr) )
      ShowError("blub");

}

D3DTexture* RenderDevice::DuplicateTexture(RenderTarget* src)
{
    D3DSURFACE_DESC desc;
    src->GetDesc(&desc);
	D3DTexture* dup;
	CHECKD3D(m_pD3DDevice->CreateTexture(desc.Width, desc.Height, 1,
		     D3DUSAGE_RENDERTARGET, desc.Format, D3DPOOL_DEFAULT, &dup, NULL)); // D3DUSAGE_AUTOGENMIPMAP?
	return dup;
}

D3DTexture* RenderDevice::DuplicateDepthTexture(RenderTarget* src)
{
    D3DSURFACE_DESC desc;
    src->GetDesc(&desc);
	D3DTexture* dup;
	CHECKD3D(m_pD3DDevice->CreateTexture(desc.Width, desc.Height, 1,
		     D3DUSAGE_DEPTHSTENCIL, (D3DFORMAT)MAKEFOURCC('I','N','T','Z'), D3DPOOL_DEFAULT, &dup, NULL)); // D3DUSAGE_AUTOGENMIPMAP?
	return dup;
}

void RenderDevice::CopySurface(D3DTexture* dest, RenderTarget* src)
{
	IDirect3DSurface9 *textureSurface;
    CHECKD3D(dest->GetSurfaceLevel(0, &textureSurface));
    CHECKD3D(m_pD3DDevice->StretchRect(src, NULL, textureSurface, NULL, D3DTEXF_NONE));
    textureSurface->Release();
}

void RenderDevice::CopyDepth(D3DTexture* dest, RenderTarget* src)
{
	if(!m_RESZ_support)
	{
		if(!NVAPIinit)
		{
			 CHECKNVAPI(NvAPI_Initialize()); //!! meh
			 NVAPIinit = true;
		}
		if(src != src_cache)
		{
			if(src_cache != NULL)
				CHECKNVAPI(NvAPI_D3D9_UnregisterResource(src_cache)); //!! meh
			CHECKNVAPI(NvAPI_D3D9_RegisterResource(src)); //!! meh
			src_cache = src;
		}
		if(dest != dest_cache)
		{
			if(dest_cache != NULL)
				CHECKNVAPI(NvAPI_D3D9_UnregisterResource(dest_cache)); //!! meh
			CHECKNVAPI(NvAPI_D3D9_RegisterResource(dest)); //!! meh
			dest_cache = dest;
		}

		//CHECKNVAPI(NvAPI_D3D9_AliasSurfaceAsTexture(m_pD3DDevice,src,dest,0));
		CHECKNVAPI(NvAPI_D3D9_StretchRectEx(m_pD3DDevice, src, NULL, dest, NULL, D3DTEXF_NONE));
	}
	else
	{
#define RESZ_CODE 0x7fa05000
		IDirect3DSurface9 *pINTZDSTSurface;
		dest->GetSurfaceLevel(0, &pINTZDSTSurface);
		// Bind depth buffer
		m_pD3DDevice->SetDepthStencilSurface(pINTZDSTSurface);

		m_pD3DDevice->BeginScene();

		m_pD3DDevice->SetVertexShader(NULL);
		m_pD3DDevice->SetPixelShader(NULL);
		m_pD3DDevice->SetFVF(D3DFVF_XYZ);

		// Bind depth stencil texture to texture sampler 0
		m_pD3DDevice->SetTexture(0, dest);

		// Perform a dummy draw call to ensure texture sampler 0 is set before the resolve is triggered
		// Vertex declaration and shaders may need to me adjusted to ensure no debug
		// error message is produced
		m_pD3DDevice->SetRenderState(D3DRS_ZENABLE, FALSE);
		m_pD3DDevice->SetRenderState(D3DRS_ZWRITEENABLE, FALSE);
		m_pD3DDevice->SetRenderState(D3DRS_COLORWRITEENABLE, 0);
		D3DXVECTOR3 vDummyPoint(0.0f, 0.0f, 0.0f);
		m_pD3DDevice->DrawPrimitiveUP(D3DPT_POINTLIST, 1, vDummyPoint, sizeof(D3DXVECTOR3));
		m_pD3DDevice->SetRenderState(D3DRS_ZWRITEENABLE, TRUE);
		m_pD3DDevice->SetRenderState(D3DRS_ZENABLE, TRUE);
		m_pD3DDevice->SetRenderState(D3DRS_COLORWRITEENABLE, 0x0F);

		// Trigger the depth buffer resolve; after this call texture sampler 0
		// will contain the contents of the resolve operation
		m_pD3DDevice->SetRenderState(D3DRS_POINTSIZE, RESZ_CODE);

		// This hack to fix resz hack, has been found by Maksym Bezus!!!
		// Without this line resz will be resolved only for first frame
		m_pD3DDevice->SetRenderState(D3DRS_POINTSIZE, 0); // TROLOLO!!!

		m_pD3DDevice->EndScene();

		pINTZDSTSurface->Release();
	}
}

D3DTexture* RenderDevice::CreateSystemTexture(MemTexture* surf)
{
    IDirect3DTexture9 *sysTex;

    int texwidth = surf->width();
    int texheight = surf->height();

    CHECKD3D(m_pD3DDevice->CreateTexture(texwidth, texheight, m_autogen_mipmap ? 1 : 0, 0, D3DFMT_A8R8G8B8,
                D3DPOOL_SYSTEMMEM, &sysTex, NULL));

    // copy data into system memory texture
    D3DLOCKED_RECT locked;
    CHECKD3D(sysTex->LockRect(0, &locked, NULL, 0));
    BYTE *pdest = (BYTE*)locked.pBits;
    for (int y = 0; y < surf->height(); ++y)
    {
        memcpy(pdest + y*locked.Pitch, surf->data() + y*surf->pitch(), 4 * surf->width());
    }
    CHECKD3D(sysTex->UnlockRect(0));

	if(!m_autogen_mipmap)
		CHECKD3D(D3DXFilterTexture(sysTex,NULL,D3DX_DEFAULT,D3DX_DEFAULT));

    return sysTex;
}

D3DTexture* RenderDevice::UploadTexture(MemTexture* surf, int *pTexWidth, int *pTexHeight)
{
    IDirect3DTexture9 *sysTex, *tex;

    int texwidth = surf->width();
    int texheight = surf->height();

    if (pTexWidth) *pTexWidth = texwidth;
    if (pTexHeight) *pTexHeight = texheight;

    sysTex = CreateSystemTexture(surf);

	CHECKD3D(m_pD3DDevice->CreateTexture(texwidth, texheight, m_autogen_mipmap ? 0 : sysTex->GetLevelCount(), m_autogen_mipmap ? D3DUSAGE_AUTOGENMIPMAP : 0, D3DFMT_A8R8G8B8,
                D3DPOOL_DEFAULT, &tex, NULL));

    CHECKD3D(m_pD3DDevice->UpdateTexture(sysTex, tex));
	CHECKD3D(sysTex->Release());
	
	if(m_autogen_mipmap)
	    tex->GenerateMipSubLevels(); // tell driver that now is a good time to generate mipmaps
    
    return tex;
}

void RenderDevice::UpdateTexture(D3DTexture* tex, MemTexture* surf)
{
    IDirect3DTexture9* sysTex = CreateSystemTexture(surf);
    CHECKD3D(m_pD3DDevice->UpdateTexture(sysTex, tex));
    CHECKD3D(sysTex->Release());
}

void RenderDevice::SetTexture(DWORD texUnit, D3DTexture* tex )
{
    if (texUnit >= 8 || tex != m_curTexture[texUnit])
    {
        CHECKD3D(m_pD3DDevice->SetTexture(texUnit, tex));
        if (texUnit < 8)
            m_curTexture[texUnit] = tex;

        m_curTextureChanges++;
    }
}

void RenderDevice::SetTextureFilter(DWORD texUnit, DWORD mode)
{
	if((mode == TEXTURE_MODE_TRILINEAR) && m_force_aniso)
		mode = TEXTURE_MODE_ANISOTROPIC;

	switch ( mode )
	{
	default:
	case TEXTURE_MODE_POINT:
		// Don't filter textures, no mipmapping.
		CHECKD3D(m_pD3DDevice->SetSamplerState(texUnit, D3DSAMP_MAGFILTER, D3DTEXF_POINT));
		CHECKD3D(m_pD3DDevice->SetSamplerState(texUnit, D3DSAMP_MINFILTER, D3DTEXF_POINT));
		CHECKD3D(m_pD3DDevice->SetSamplerState(texUnit, D3DSAMP_MIPFILTER, D3DTEXF_NONE));
		break;

	case TEXTURE_MODE_BILINEAR:
		// Interpolate in 2x2 texels, no mipmapping.
		CHECKD3D(m_pD3DDevice->SetSamplerState(texUnit, D3DSAMP_MAGFILTER, D3DTEXF_LINEAR));
		CHECKD3D(m_pD3DDevice->SetSamplerState(texUnit, D3DSAMP_MINFILTER, D3DTEXF_LINEAR));
		CHECKD3D(m_pD3DDevice->SetSamplerState(texUnit, D3DSAMP_MIPFILTER, D3DTEXF_NONE));
		break;

	case TEXTURE_MODE_TRILINEAR:
		// Filter textures on 2 mip levels (interpolate in 2x2 texels). And filter between the 2 mip levels.
		CHECKD3D(m_pD3DDevice->SetSamplerState(texUnit, D3DSAMP_MAGFILTER, D3DTEXF_LINEAR));
		CHECKD3D(m_pD3DDevice->SetSamplerState(texUnit, D3DSAMP_MINFILTER, D3DTEXF_LINEAR));
		CHECKD3D(m_pD3DDevice->SetSamplerState(texUnit, D3DSAMP_MIPFILTER, D3DTEXF_LINEAR));
		break;

	case TEXTURE_MODE_ANISOTROPIC:
		// Full HQ anisotropic Filter. Should lead to driver doing whatever it thinks is best.
		CHECKD3D(m_pD3DDevice->SetSamplerState(texUnit, D3DSAMP_MAGFILTER, m_mag_aniso ? D3DTEXF_ANISOTROPIC : D3DTEXF_LINEAR));
		CHECKD3D(m_pD3DDevice->SetSamplerState(texUnit, D3DSAMP_MINFILTER, D3DTEXF_ANISOTROPIC));
		CHECKD3D(m_pD3DDevice->SetSamplerState(texUnit, D3DSAMP_MIPFILTER, D3DTEXF_LINEAR));
		CHECKD3D(m_pD3DDevice->SetSamplerState(texUnit, D3DSAMP_MAXANISOTROPY, min(m_maxaniso,(DWORD)16)));
		break;
	}
}

void RenderDevice::SetTextureStageState( DWORD p1, D3DTEXTURESTAGESTATETYPE p2, DWORD p3)
{
    if( (unsigned int)p2 < TEXTURE_STATE_CACHE_SIZE && p1 < 8)
    {
        if(textureStateCache[p1][p2] == p3)
        {
            // texture stage state hasn't changed since last call of this function -> do nothing here
            return;
        }
        textureStateCache[p1][p2] = p3;
    }
    CHECKD3D(m_pD3DDevice->SetTextureStageState(p1, p2, p3));

   m_curStateChanges++;
}

void RenderDevice::SetRenderTarget( RenderTarget* surf)
{
    CHECKD3D(m_pD3DDevice->SetRenderTarget(0, surf));
}

void RenderDevice::SetZBuffer( RenderTarget* surf)
{
    CHECKD3D(m_pD3DDevice->SetDepthStencilSurface(surf));
}

void RenderDevice::SetRenderState( const RenderStates p1, const DWORD p2 )
{
   if ( (unsigned int)p1 < RENDER_STATE_CACHE_SIZE)
   {
      if( renderStateCache[p1]==p2 )
      {
         // this render state is already set -> don't do anything then
         return;
      }
      renderStateCache[p1]=p2;
   }
   CHECKD3D(m_pD3DDevice->SetRenderState((D3DRENDERSTATETYPE)p1, p2));

   m_curStateChanges++;
}

void RenderDevice::SetTextureAddressMode(DWORD texUnit, TextureAddressMode mode)
{
    CHECKD3D(m_pD3DDevice->SetSamplerState(texUnit, D3DSAMP_ADDRESSU, mode));
    CHECKD3D(m_pD3DDevice->SetSamplerState(texUnit, D3DSAMP_ADDRESSV, mode));
}

void RenderDevice::CreateVertexBuffer( unsigned int vertexCount, DWORD usage, DWORD fvf, VertexBuffer **vBuffer )
{
    // NB: We always specify WRITEONLY since MSDN states,
    // "Buffers created with D3DPOOL_DEFAULT that do not specify D3DUSAGE_WRITEONLY may suffer a severe performance penalty."
    // This means we cannot read from vertex buffers, but I don't think we need to.
    CHECKD3D(m_pD3DDevice->CreateVertexBuffer(vertexCount * fvfToSize(fvf), D3DUSAGE_WRITEONLY | usage, fvf,
                D3DPOOL_DEFAULT, (IDirect3DVertexBuffer9**)vBuffer, NULL));
}

void RenderDevice::CreateIndexBuffer(unsigned int numIndices, DWORD usage, IndexBuffer::Format format, IndexBuffer **idxBuffer)
{
    // NB: We always specify WRITEONLY since MSDN states,
    // "Buffers created with D3DPOOL_DEFAULT that do not specify D3DUSAGE_WRITEONLY may suffer a severe performance penalty."
    const unsigned idxSize = (format == IndexBuffer::FMT_INDEX16) ? 2 : 4;
    CHECKD3D(m_pD3DDevice->CreateIndexBuffer(idxSize * numIndices, usage | D3DUSAGE_WRITEONLY, (D3DFORMAT)format,
                D3DPOOL_DEFAULT, (IDirect3DIndexBuffer9**)idxBuffer, NULL));
}

IndexBuffer* RenderDevice::CreateAndFillIndexBuffer(unsigned int numIndices, const WORD * indices)
{
    IndexBuffer* ib;
    CreateIndexBuffer(numIndices, 0, IndexBuffer::FMT_INDEX16, &ib);

    void* buf;
    ib->lock(0, 0, &buf, 0);
    memcpy(buf, indices, numIndices * sizeof(indices[0]));
    ib->unlock();

    return ib;
}

IndexBuffer* RenderDevice::CreateAndFillIndexBuffer(unsigned int numIndices, const unsigned int * indices)
{
    IndexBuffer* ib;
    CreateIndexBuffer(numIndices, 0, IndexBuffer::FMT_INDEX32, &ib);

    void* buf;
    ib->lock(0, 0, &buf, 0);
    memcpy(buf, indices, numIndices * sizeof(indices[0]));
    ib->unlock();

    return ib;
}

IndexBuffer* RenderDevice::CreateAndFillIndexBuffer(const std::vector<WORD>& indices)
{
    return CreateAndFillIndexBuffer(indices.size(), &indices[0]);
}

IndexBuffer* RenderDevice::CreateAndFillIndexBuffer(const std::vector<unsigned int>& indices)
{
    return CreateAndFillIndexBuffer(indices.size(), &indices[0]);
}


RenderTarget* RenderDevice::AttachZBufferTo(RenderTarget* surf)
{
    D3DSURFACE_DESC desc;
    surf->GetDesc(&desc);

    IDirect3DSurface9 *pZBuf;
    CHECKD3D(m_pD3DDevice->CreateDepthStencilSurface(desc.Width, desc.Height, D3DFMT_D16 /*D3DFMT_D24X8*/,
            desc.MultiSampleType, desc.MultiSampleQuality, FALSE, &pZBuf, NULL));

    return pZBuf;
}

void RenderDevice::DrawPrimitive(D3DPRIMITIVETYPE type, DWORD fvf, const void* vertices, DWORD vertexCount)
{
    m_pD3DDevice->SetFVF(fvf);
    CHECKD3D(m_pD3DDevice->DrawPrimitiveUP(type, ComputePrimitiveCount(type, vertexCount), vertices, fvfToSize(fvf)));
    m_curVertexBuffer = 0;      // DrawPrimitiveUP sets the VB to NULL

   m_curDrawCalls++;
}

void RenderDevice::DrawIndexedPrimitive(D3DPRIMITIVETYPE type, DWORD fvf, const void* vertices, DWORD vertexCount, const WORD* indices, DWORD indexCount)
{
    m_pD3DDevice->SetFVF(fvf);
    CHECKD3D(m_pD3DDevice->DrawIndexedPrimitiveUP(type, 0, vertexCount, ComputePrimitiveCount(type, indexCount),
                indices, D3DFMT_INDEX16, vertices, fvfToSize(fvf)));
    m_curVertexBuffer = 0;      // DrawIndexedPrimitiveUP sets the VB to NULL
    m_curIndexBuffer = 0;       // DrawIndexedPrimitiveUP sets the IB to NULL

   m_curDrawCalls++;
}

void RenderDevice::DrawPrimitiveVB(D3DPRIMITIVETYPE type, VertexBuffer* vb, DWORD startVertex, DWORD vertexCount)
{
    D3DVERTEXBUFFER_DESC desc;
    vb->GetDesc(&desc);     // let's hope this is not very slow

    const unsigned int vsize = fvfToSize(desc.FVF);

    CHECKD3D(m_pD3DDevice->SetFVF(desc.FVF));

    if (m_curVertexBuffer != vb)
    {
        CHECKD3D(m_pD3DDevice->SetStreamSource(0, vb, 0, vsize));
        m_curVertexBuffer = vb;
    }

    CHECKD3D(m_pD3DDevice->DrawPrimitive(type, startVertex, ComputePrimitiveCount(type, vertexCount)));

   m_curDrawCalls++;
}

void RenderDevice::DrawIndexedPrimitiveVB( D3DPRIMITIVETYPE type, VertexBuffer* vb, DWORD startVertex, DWORD vertexCount, const WORD* indices, DWORD indexCount)
{
    if (indexCount > MY_IDX_BUF_SIZE)
    {
        ShowError("Call to DrawIndexedPrimitiveVB has too many indices. Use an index buffer.");
        return;
    }

    // copy the indices to the dynamic index buffer
    WORD *buffer;
    m_dynIndexBuffer->lock(0, indexCount * sizeof(WORD), (void**)&buffer, IndexBuffer::DISCARD);
    memcpy(buffer, indices, indexCount * sizeof(WORD));
    m_dynIndexBuffer->unlock();

    DrawIndexedPrimitiveVB(type, vb, startVertex, vertexCount, m_dynIndexBuffer, 0, indexCount);
}

void RenderDevice::DrawIndexedPrimitiveVB( D3DPRIMITIVETYPE type, VertexBuffer* vb, DWORD startVertex, DWORD vertexCount, IndexBuffer* ib, DWORD startIndex, DWORD indexCount)
{
    // get VB description (for FVF)
    D3DVERTEXBUFFER_DESC desc;
    vb->GetDesc(&desc);     // let's hope this is not very slow

    const unsigned int vsize = fvfToSize(desc.FVF);

    // bind the vertex and index buffers
    CHECKD3D(m_pD3DDevice->SetFVF(desc.FVF));

    if (m_curVertexBuffer != vb)
    {
        CHECKD3D(m_pD3DDevice->SetStreamSource(0, vb, 0, vsize));
        m_curVertexBuffer = vb;
    }

    if (m_curIndexBuffer != ib)
    {
        CHECKD3D(m_pD3DDevice->SetIndices(ib));
        m_curIndexBuffer = ib;
    }

    // render
    CHECKD3D(m_pD3DDevice->DrawIndexedPrimitive(type, startVertex, 0, vertexCount, startIndex, ComputePrimitiveCount(type, indexCount)));

   m_curDrawCalls++;
}

void RenderDevice::SetTransform( TransformStateType p1, D3DMATRIX* p2)
{
   CHECKD3D(m_pD3DDevice->SetTransform((D3DTRANSFORMSTATETYPE)p1, p2));
}

void RenderDevice::GetTransform( TransformStateType p1, D3DMATRIX* p2)
{
   CHECKD3D(m_pD3DDevice->GetTransform((D3DTRANSFORMSTATETYPE)p1, p2));
}

void RenderDevice::Clear(DWORD numRects, D3DRECT* rects, DWORD flags, D3DCOLOR color, D3DVALUE z, DWORD stencil)
{
   m_pD3DDevice->Clear(numRects, rects, flags, color, z, stencil);
}

void RenderDevice::SetViewport( ViewPort* p1)
{
   m_pD3DDevice->SetViewport((D3DVIEWPORT9*)p1);
}

void RenderDevice::GetViewport( ViewPort* p1)
{
   m_pD3DDevice->GetViewport((D3DVIEWPORT9*)p1);
}

Shader::Shader(RenderDevice *renderDevice)
{
    m_renderDevice = renderDevice;
    m_shader=0;
}

Shader::~Shader()
{
    if( m_shader )
    {
        this->Unload();
    }
}

// loads an HLSL effect file
// if fromFile is true the shaderName should point to the full filename (with path) to the .fx file
// if fromFile is false the shaderName should be the resource name not the IDC_XX_YY value. Search vpinball_eng.rc for ".fx" to see an example
bool Shader::Load( const BYTE* shaderCodeName, UINT codeSize )
{
    LPD3DXBUFFER pBufferErrors;
    DWORD dwShaderFlags = D3DXSHADER_PARTIALPRECISION;
    HRESULT hr;
/*
    if ( fromFile )
    {
            dwShaderFlags = D3DXSHADER_DEBUG|D3DXSHADER_SKIPOPTIMIZATION;
            hr = D3DXCreateEffectFromFile(	m_renderDevice->GetCoreDevice(),		// pDevice
            shaderName,			// pSrcFile
            NULL,				// pDefines
            NULL,				// pInclude
            dwShaderFlags,		// Flags
            NULL,				// pPool
            &m_shader,			// ppEffect
            &pBufferErrors);	// ppCompilationErrors
    }
    else
    {
        hr = D3DXCreateEffectFromResource(	m_renderDevice->GetCoreDevice(),		// pDevice
            NULL,			
            shaderName,         // resource name
            NULL,				// pDefines
            NULL,				// pInclude
            dwShaderFlags,		// Flags
            NULL,				// pPool
            &m_shader,			// ppEffect
            &pBufferErrors);	// ppCompilationErrors

    }
*/
    hr=D3DXCreateEffect( m_renderDevice->GetCoreDevice(), shaderCodeName, codeSize,NULL,NULL,dwShaderFlags,NULL, &m_shader, &pBufferErrors);
    if(FAILED(hr) )
    {
        if ( pBufferErrors )
        {
            LPVOID pCompileErrors = pBufferErrors->GetBufferPointer(); 
            MessageBox(NULL, (const char*)pCompileErrors, "Compile Error", MB_OK|MB_ICONEXCLAMATION);
        }
        return false;
    }
    return true;
}

void Shader::Unload()
{
    if( m_shader )
    {
        m_shader->Release();
        m_shader=0;
    }
}

void Shader::Begin( unsigned int pass )
{
   unsigned int cPasses;
   m_shader->Begin(&cPasses,0);
   m_shader->BeginPass(pass);  

}

void Shader::End()
{
   m_shader->EndPass();  
   m_shader->End();  

}

void Shader::SetTexture( D3DXHANDLE texelName, Texture *texel)
{
   if ( texel->m_pdsBufferColorKey )
      m_shader->SetTexture(texelName,m_renderDevice->m_texMan.LoadTexture(texel->m_pdsBufferColorKey));
   else if (texel->m_pdsBuffer )
      m_shader->SetTexture(texelName,m_renderDevice->m_texMan.LoadTexture(texel->m_pdsBuffer));
}

void Shader::SetTexture( D3DXHANDLE texelName, D3DTexture *texel)
{
   m_shader->SetTexture(texelName,texel);
}

void Shader::SetMaterial( const Material * const mat, 
                          D3DXVECTOR4 cBase, 
                          D3DXVECTOR4 cGlossy, 
                          D3DXVECTOR4 cClearcoat,
                          float fWrapLighting, float fRoughness, float fEdge, float fOpacity,
                          bool bIsMetal, bool bOpacityActive )
{
    if( mat )
    {
		fWrapLighting = mat->m_fWrapLighting;
		fRoughness = mat->m_fRoughness;
		fEdge = mat->m_fEdge;
		fOpacity = mat->m_fOpacity;
		cBase = convertColor(mat->m_cBase);
		cGlossy = convertColor(mat->m_cGlossy);
		cClearcoat = convertColor(mat->m_cClearcoat);
		bIsMetal = mat->m_bIsMetal;
		bOpacityActive = mat->m_bOpacityActive;
    }

    m_shader->SetFloat("fWrapLighting",fWrapLighting);
    m_shader->SetFloat("fRoughness",exp2f(10.0f * fRoughness + 1.0f)); // map from 0..1 to 2..2048
    m_shader->SetFloat("fEdge",fEdge);
    m_shader->SetVector("cBase",&cBase);
    m_shader->SetVector("cGlossy",&cGlossy);
    m_shader->SetVector("cClearcoat",&cClearcoat);
	m_shader->SetBool("bIsMetal", bIsMetal);
    m_shader->SetFloat("fmaterialAlpha", bOpacityActive ? fOpacity : 1.0f);
    if(bOpacityActive /*&& (fOpacity < 1.0f)*/)
		g_pplayer->m_pin3d.EnableAlphaBlend(1,false);
    else
		g_pplayer->m_pin3d.DisableAlphaBlend();
}
