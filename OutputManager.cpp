
#include "OutputManager.h"

#include "PSVeil.h"

#include "utils.cpp"

//#include <stdio.h>
//#include "warning.h"

using namespace DirectX;

//TODO(fran): i need the default vertex and pixel shader for copying to the backbuffer !!!

//
// Constructor NULLs out all pointers & sets appropriate var vals
//
OUTPUTMANAGER::OUTPUTMANAGER() :
								 //m_SwapChain(nullptr), 
                                 m_Device(nullptr),
                                 //m_Factory(nullptr),
                                 m_DeviceContext(nullptr),
                                 m_RTV(nullptr),
                                 m_SamplerLinear(nullptr),
                                 //m_BlendState(nullptr),
                                 m_VertexShader(nullptr),
                                 m_PixelShader(nullptr),
                                 m_InputLayout(nullptr),
                                 m_SharedSurf(nullptr),
                                 m_KeyMutex(nullptr),
                                 m_WindowHandle(nullptr),
                                 m_NeedsResize(false),
                                 //m_OcclusionCookie(0),
								 Threshold(.7f),
								 Opacity(.1f),
								 ThresholdOpacityBuffer(nullptr)
								 //needsClearing(false)
{
	manual_Backbuffer[0] = nullptr;
	manual_Backbuffer[1] = nullptr;
}

//
// Destructor which calls CleanRefs to release all references and memory.
//
OUTPUTMANAGER::~OUTPUTMANAGER()
{
    CleanRefs();
}

//
// Indicates that window has been resized.
//
void OUTPUTMANAGER::WindowResize()
{
    m_NeedsResize = true;
}

void OUTPUTMANAGER::SetThreshold(float threshold)
{
	Assert(threshold >= 0 || threshold <= 1);
	Threshold = threshold;
}

void OUTPUTMANAGER::SetOpacity(float opacity)
{
	Assert(opacity >= 0 || opacity <= 1);
	Opacity = opacity;
}

//void OUTPUTMANAGER::RestartTextures()
//{
//	needsClearing = true;
//}

//
// Initialize all state
//
DUPL_RETURN OUTPUTMANAGER::InitOutput(HWND Window, INT SingleOutput, _Out_ UINT* OutCount, _Out_ RECT* DeskBounds)
{
    HRESULT hr;

    // Store window handle
    m_WindowHandle = Window;

    // Driver types supported
    D3D_DRIVER_TYPE DriverTypes[] =
    {
        D3D_DRIVER_TYPE_HARDWARE,
        D3D_DRIVER_TYPE_WARP,
        D3D_DRIVER_TYPE_REFERENCE,
    };
    UINT NumDriverTypes = ARRAYSIZE(DriverTypes);

    // Feature levels supported
    D3D_FEATURE_LEVEL FeatureLevels[] =
    {
        D3D_FEATURE_LEVEL_11_0,
        D3D_FEATURE_LEVEL_10_1,
        D3D_FEATURE_LEVEL_10_0,
        D3D_FEATURE_LEVEL_9_1
    };
    UINT NumFeatureLevels = ARRAYSIZE(FeatureLevels);
    D3D_FEATURE_LEVEL FeatureLevel;

    // Create device
    for (UINT DriverTypeIndex = 0; DriverTypeIndex < NumDriverTypes; ++DriverTypeIndex)
    {
        hr = D3D11CreateDevice(nullptr, DriverTypes[DriverTypeIndex], nullptr, 
			0
			#ifdef _DX_DEBUG_LAYER 
			| D3D11_CREATE_DEVICE_DEBUG
			#endif
			, FeatureLevels, NumFeatureLevels,
        D3D11_SDK_VERSION, &m_Device, &FeatureLevel, &m_DeviceContext);
        if (SUCCEEDED(hr))
        {
            // Device creation succeeded, no need to loop anymore
            break;
        }
    }
    if (FAILED(hr))
    {
        return ProcessFailure(m_Device, L"Device creation in OUTPUTMANAGER failed", L"Error", hr, SystemTransitionsExpectedErrors);
    }
#ifdef _DX_DEBUG_LAYER
	const char m_DeviceName[] = "OUTMGR_m_Device";
	m_Device->SetPrivateData(WKPDID_D3DDebugObjectName, sizeof(m_DeviceName) - 1, m_DeviceName);
#endif

    // Get DXGI factory
    //IDXGIDevice* DxgiDevice = nullptr;
    //hr = m_Device->QueryInterface(__uuidof(IDXGIDevice), reinterpret_cast<void**>(&DxgiDevice));
    //if (FAILED(hr))
    //{
    //    return ProcessFailure(nullptr, L"Failed to QI for DXGI Device", L"Error", hr, nullptr);
    //}

    //IDXGIAdapter* DxgiAdapter = nullptr;
    //hr = DxgiDevice->GetParent(__uuidof(IDXGIAdapter), reinterpret_cast<void**>(&DxgiAdapter));
    //DxgiDevice->Release();
    //DxgiDevice = nullptr;
    //if (FAILED(hr))
    //{
    //    return ProcessFailure(m_Device, L"Failed to get parent DXGI Adapter", L"Error", hr, SystemTransitionsExpectedErrors);
    //}

    //hr = DxgiAdapter->GetParent(__uuidof(IDXGIFactory2), reinterpret_cast<void**>(&m_Factory));
    //DxgiAdapter->Release();
    //DxgiAdapter = nullptr;
    //if (FAILED(hr))
    //{
    //    return ProcessFailure(m_Device, L"Failed to get parent DXGI Factory", L"Error", hr, SystemTransitionsExpectedErrors);
    //}

    // Register for occlusion status windows message
#if 0
	hr = m_Factory->RegisterOcclusionStatusWindow(Window, OCCLUSION_STATUS_MSG, &m_OcclusionCookie);
    if (FAILED(hr))
    {
        return ProcessFailure(m_Device, L"Failed to register for occlusion message", L"Error", hr, SystemTransitionsExpectedErrors);
    }
#endif

    // Get window size
    RECT WindowRect;
    GetClientRect(m_WindowHandle, &WindowRect);
    UINT Width = WindowRect.right - WindowRect.left;
    UINT Height = WindowRect.bottom - WindowRect.top;

    // Create swapchain for window
#if 0
    DXGI_SWAP_CHAIN_DESC1 SwapChainDesc;
    RtlZeroMemory(&SwapChainDesc, sizeof(SwapChainDesc));

    SwapChainDesc.SwapEffect = DXGI_SWAP_EFFECT_FLIP_SEQUENTIAL;
    SwapChainDesc.BufferCount = 2;
    SwapChainDesc.Width = Width;
    SwapChainDesc.Height = Height;
    SwapChainDesc.Format = DXGI_FORMAT_B8G8R8A8_UNORM;
    SwapChainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
    SwapChainDesc.SampleDesc.Count = 1;
    SwapChainDesc.SampleDesc.Quality = 0;
	SwapChainDesc.Flags = DXGI_SWAP_CHAIN_FLAG_GDI_COMPATIBLE;
    hr = m_Factory->CreateSwapChainForHwnd(m_Device, Window, &SwapChainDesc, nullptr, nullptr, &m_SwapChain);
    if (FAILED(hr))
    {
        return ProcessFailure(m_Device, L"Failed to create window swapchain", L"Error", hr, SystemTransitionsExpectedErrors);
    }
#else
	D3D11_TEXTURE2D_DESC textureDesc;
	textureDesc.ArraySize = 1;
	textureDesc.BindFlags = D3D11_BIND_RENDER_TARGET | D3D11_BIND_SHADER_RESOURCE;
	textureDesc.CPUAccessFlags = 0;
	textureDesc.Format = DXGI_FORMAT_B8G8R8A8_UNORM;
	textureDesc.Height = Height;
	textureDesc.MipLevels = 1;
	textureDesc.MiscFlags = D3D11_RESOURCE_MISC_GDI_COMPATIBLE;
	textureDesc.SampleDesc.Count = 1;
	textureDesc.SampleDesc.Quality = 0;
	textureDesc.Usage = D3D11_USAGE_DEFAULT;
	textureDesc.Width = Width;

	hr = m_Device->CreateTexture2D(&textureDesc, nullptr, &manual_Backbuffer[0]);
	if (FAILED(hr)) { return ProcessFailure(nullptr, L"Failed to create the frontbuffer texture for the output", L"Error", hr); }

#ifdef _DX_DEBUG_LAYER
	const char manual_BackbufferName0[] = "OUTMGR_manual_Backbuffer0";
	manual_Backbuffer[0]->SetPrivateData(WKPDID_D3DDebugObjectName, sizeof(manual_BackbufferName0) - 1, manual_BackbufferName0);
#endif

	hr = m_Device->CreateTexture2D(&textureDesc, nullptr, &manual_Backbuffer[1]);
	if (FAILED(hr)) { return ProcessFailure(nullptr, L"Failed to create the backbuffer texture for the output", L"Error", hr); }

#ifdef _DX_DEBUG_LAYER
	const char manual_BackbufferName1[] = "OUTMGR_manual_Backbuffer1";
	manual_Backbuffer[1]->SetPrivateData(WKPDID_D3DDebugObjectName, sizeof(manual_BackbufferName1) - 1, manual_BackbufferName1);
#endif

#endif

    // Disable the ALT-ENTER shortcut for entering full-screen mode
#if 0
    hr = m_Factory->MakeWindowAssociation(Window, DXGI_MWA_NO_ALT_ENTER);
    if (FAILED(hr))
    {
        return ProcessFailure(m_Device, L"Failed to make window association", L"Error", hr, SystemTransitionsExpectedErrors);
    }
#endif

    // Create shared texture
    DUPL_RETURN Return = CreateSharedSurf(SingleOutput, OutCount, DeskBounds);
    if (Return != DUPL_RETURN_SUCCESS)
    {
        return Return;
    }

    // Make new render target view
    Return = MakeRTV();
    if (Return != DUPL_RETURN_SUCCESS)
    {
        return Return;
    }

    // Set view port
    SetViewPort(Width, Height);

    // Create the sample state
    D3D11_SAMPLER_DESC SampDesc;
    RtlZeroMemory(&SampDesc, sizeof(SampDesc));
    SampDesc.Filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR;
    SampDesc.AddressU = D3D11_TEXTURE_ADDRESS_CLAMP;
    SampDesc.AddressV = D3D11_TEXTURE_ADDRESS_CLAMP;
    SampDesc.AddressW = D3D11_TEXTURE_ADDRESS_CLAMP;
    SampDesc.ComparisonFunc = D3D11_COMPARISON_NEVER;
    SampDesc.MinLOD = 0;
    SampDesc.MaxLOD = D3D11_FLOAT32_MAX;
    hr = m_Device->CreateSamplerState(&SampDesc, &m_SamplerLinear);
    if (FAILED(hr))
    {
        return ProcessFailure(m_Device, L"Failed to create sampler state in OUTPUTMANAGER", L"Error", hr, SystemTransitionsExpectedErrors);
    }

	float temp_buffer[4] = { Threshold,Opacity,0,0 };

	D3D11_BUFFER_DESC ThresholdDesc; 
	ThresholdDesc.Usage = D3D11_USAGE_DEFAULT;
	ThresholdDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	ThresholdDesc.ByteWidth = sizeof(float) * 4;
	ThresholdDesc.CPUAccessFlags = 0;
	ThresholdDesc.MiscFlags = 0;
	ThresholdDesc.StructureByteStride = 0;
	D3D11_SUBRESOURCE_DATA ThresholdData;
	ThresholdData.pSysMem = &temp_buffer;
	ThresholdData.SysMemPitch = 0;
	ThresholdData.SysMemSlicePitch = 0;
	hr = m_Device->CreateBuffer(&ThresholdDesc, &ThresholdData, &ThresholdOpacityBuffer);
	if (FAILED(hr)) { return ProcessFailure(nullptr, L"Failed to load threshold and opacity data to the GPU", L"Error", hr); }
#ifdef _DX_DEBUG_LAYER
	const char ThresholdOpacityBufferName[] = "OUT_MGR_ThresholdOpacityBuffer";
	ThresholdOpacityBuffer->SetPrivateData(WKPDID_D3DDebugObjectName, sizeof(ThresholdOpacityBufferName) - 1, ThresholdOpacityBufferName);
#endif
    // Create the blend state
#if 0
    D3D11_BLEND_DESC BlendStateDesc; 
	//TODO(fran): de esta forma tengo "acceso" al pixel que ya está en la textura, osea al veil anterior, me puedo beneficiar de esto
	//            para no tener que crear otro texture con el veil viejo y asignarlo como shader resource ????
    BlendStateDesc.AlphaToCoverageEnable = FALSE;
    BlendStateDesc.IndependentBlendEnable = FALSE;
    BlendStateDesc.RenderTarget[0].BlendEnable = TRUE;
    BlendStateDesc.RenderTarget[0].SrcBlend = D3D11_BLEND_SRC_ALPHA;
    BlendStateDesc.RenderTarget[0].DestBlend = D3D11_BLEND_INV_SRC_ALPHA;
    BlendStateDesc.RenderTarget[0].BlendOp = D3D11_BLEND_OP_ADD;
    BlendStateDesc.RenderTarget[0].SrcBlendAlpha = D3D11_BLEND_ONE;
    BlendStateDesc.RenderTarget[0].DestBlendAlpha = D3D11_BLEND_ZERO;
    BlendStateDesc.RenderTarget[0].BlendOpAlpha = D3D11_BLEND_OP_ADD;
    BlendStateDesc.RenderTarget[0].RenderTargetWriteMask = D3D11_COLOR_WRITE_ENABLE_ALL;
    hr = m_Device->CreateBlendState(&BlendStateDesc, &m_BlendState);
    if (FAILED(hr))
    {
        return ProcessFailure(m_Device, L"Failed to create blend state in OUTPUTMANAGER", L"Error", hr, SystemTransitionsExpectedErrors);
    }
#endif

    // Initialize shaders
    Return = InitShaders();

    
#if 0
    if (Return != DUPL_RETURN_SUCCESS)
    {
        return Return;
    }
    GetWindowRect(m_WindowHandle, &WindowRect);
	MoveWindow(m_WindowHandle, WindowRect.left, WindowRect.top, (DeskBounds->right - DeskBounds->left) / 2, (DeskBounds->bottom - DeskBounds->top) / 2, TRUE);
#endif
	

    return Return;
}

//
// Recreate shared texture
//
DUPL_RETURN OUTPUTMANAGER::CreateSharedSurf(INT SingleOutput, _Out_ UINT* OutCount, _Out_ RECT* DeskBounds)
{
    HRESULT hr;

    // Get DXGI resources
    IDXGIDevice* DxgiDevice = nullptr;
    hr = m_Device->QueryInterface(__uuidof(IDXGIDevice), reinterpret_cast<void**>(&DxgiDevice));
    if (FAILED(hr))
    {
        return ProcessFailure(nullptr, L"Failed to QI for DXGI Device", L"Error", hr);
    }

    IDXGIAdapter* DxgiAdapter = nullptr;
    hr = DxgiDevice->GetParent(__uuidof(IDXGIAdapter), reinterpret_cast<void**>(&DxgiAdapter));
    DxgiDevice->Release();
    DxgiDevice = nullptr;
    if (FAILED(hr))
    {
        return ProcessFailure(m_Device, L"Failed to get parent DXGI Adapter", L"Error", hr, SystemTransitionsExpectedErrors);
    }

    // Set initial values so that we always catch the right coordinates
    DeskBounds->left = INT_MAX;
    DeskBounds->right = INT_MIN;
    DeskBounds->top = INT_MAX;
    DeskBounds->bottom = INT_MIN;

    IDXGIOutput* DxgiOutput = nullptr;

    // Figure out right dimensions for full size desktop texture and # of outputs to duplicate
    UINT OutputCount;
    if (SingleOutput < 0)
    {
        hr = S_OK;
        for (OutputCount = 0; SUCCEEDED(hr); ++OutputCount)
        {
            if (DxgiOutput)
            {
                DxgiOutput->Release();
                DxgiOutput = nullptr;
            }
            hr = DxgiAdapter->EnumOutputs(OutputCount, &DxgiOutput);
            if (DxgiOutput && (hr != DXGI_ERROR_NOT_FOUND))
            {
                DXGI_OUTPUT_DESC DesktopDesc;
                DxgiOutput->GetDesc(&DesktopDesc);

                DeskBounds->left = min(DesktopDesc.DesktopCoordinates.left, DeskBounds->left);
                DeskBounds->top = min(DesktopDesc.DesktopCoordinates.top, DeskBounds->top);
                DeskBounds->right = max(DesktopDesc.DesktopCoordinates.right, DeskBounds->right);
                DeskBounds->bottom = max(DesktopDesc.DesktopCoordinates.bottom, DeskBounds->bottom);
            }
        }

        --OutputCount;
    }
    else
    {
        hr = DxgiAdapter->EnumOutputs(SingleOutput, &DxgiOutput);
        if (FAILED(hr))
        {
            DxgiAdapter->Release();
            DxgiAdapter = nullptr;
            return ProcessFailure(m_Device, L"Output specified to be duplicated does not exist", L"Error", hr);
        }
        DXGI_OUTPUT_DESC DesktopDesc;
        DxgiOutput->GetDesc(&DesktopDesc);
        *DeskBounds = DesktopDesc.DesktopCoordinates;

        DxgiOutput->Release();
        DxgiOutput = nullptr;

        OutputCount = 1;
    }

    DxgiAdapter->Release();
    DxgiAdapter = nullptr;

    // Set passed in output count variable
    *OutCount = OutputCount;

    if (OutputCount == 0)
    {
        // We could not find any outputs, the system must be in a transition so return expected error
        // so we will attempt to recreate
        return DUPL_RETURN_ERROR_EXPECTED;
    }

    // Create shared texture for all duplication threads to draw into
    D3D11_TEXTURE2D_DESC DeskTexD;
    RtlZeroMemory(&DeskTexD, sizeof(D3D11_TEXTURE2D_DESC));
    DeskTexD.Width = DeskBounds->right - DeskBounds->left;
    DeskTexD.Height = DeskBounds->bottom - DeskBounds->top;
    DeskTexD.MipLevels = 1;
    DeskTexD.ArraySize = 1;
    DeskTexD.Format = DXGI_FORMAT_B8G8R8A8_UNORM;
    DeskTexD.SampleDesc.Count = 1;
    DeskTexD.Usage = D3D11_USAGE_DEFAULT;
    DeskTexD.BindFlags = D3D11_BIND_RENDER_TARGET | D3D11_BIND_SHADER_RESOURCE;
    DeskTexD.CPUAccessFlags = 0;
    DeskTexD.MiscFlags = D3D11_RESOURCE_MISC_SHARED_KEYEDMUTEX;

    hr = m_Device->CreateTexture2D(&DeskTexD, nullptr, &m_SharedSurf);
    if (FAILED(hr))
    {
        if (OutputCount != 1)
        {
            // If we are duplicating the complete desktop we try to create a single texture to hold the
            // complete desktop image and blit updates from the per output DDA interface.  The GPU can
            // always support a texture size of the maximum resolution of any single output but there is no
            // guarantee that it can support a texture size of the desktop.
            // The sample only use this large texture to display the desktop image in a single window using DX
            // we could revert back to using GDI to update the window in this failure case.
			//TODO(fran): do we support what this message says?
            return ProcessFailure(m_Device, L"Failed to create DirectX shared texture, we are attempting to create a texture the size of the complete desktop and this may be larger than the maximum texture size of your GPU.  Please try again using the -output command line parameter to duplicate only 1 monitor or configure your computer to a single monitor configuration", L"Error", hr, SystemTransitionsExpectedErrors);
        }
        else
        {
            return ProcessFailure(m_Device, L"Failed to create shared texture", L"Error", hr, SystemTransitionsExpectedErrors);
        }
    }
#ifdef _DX_DEBUG_LAYER
	const char m_SharedSurfName[] = "OUTMGR_m_SharedSurf";
	m_SharedSurf->SetPrivateData(WKPDID_D3DDebugObjectName, sizeof(m_SharedSurfName) - 1, m_SharedSurfName);
#endif

    // Get keyed mutex
    hr = m_SharedSurf->QueryInterface(__uuidof(IDXGIKeyedMutex), reinterpret_cast<void**>(&m_KeyMutex));
    if (FAILED(hr))
    {
        return ProcessFailure(m_Device, L"Failed to query for keyed mutex in OUTPUTMANAGER", L"Error", hr);
    }

    return DUPL_RETURN_SUCCESS;
}

//
// Present to the application window
//
DUPL_RETURN OUTPUTMANAGER::UpdateApplicationWindow(/*_In_ PTR_INFO* PointerInfo*/) //TODO(fran): unnecessary parameter, can we take something from it or we just remove it?
{

	HRESULT hr;
	DUPL_RETURN Ret;
    // In a typical desktop duplication application there would be an application running on one system collecting the desktop images
    // and another application running on a different system that receives the desktop images via a network and display the image. This
    // sample contains both these aspects into a single application.
    // This routine is the part of the sample that displays the desktop image onto the display

	//TODO(fran): should I put this inside DrawFrame??
//	if (needsClearing) {
//		needsClearing = false;
//#if 1 //TODO(fran): there is some problem here
//		Ret = ResetSecondaryTexture();
//		if (Ret != DUPL_RETURN_SUCCESS) {
//			return Ret;
//		}
//#endif
//	}

	MakeRTV();

    // Try and acquire sync on common display buffer
    hr = m_KeyMutex->AcquireSync(1, 100);
    if (hr == static_cast<HRESULT>(WAIT_TIMEOUT))
    {
        // Another thread has the keyed mutex so try again later
        return DUPL_RETURN_SUCCESS;
    }
    else if (FAILED(hr))
    {
        return ProcessFailure(m_Device, L"Failed to acquire Keyed mutex in OUTPUTMANAGER", L"Error", hr, SystemTransitionsExpectedErrors);
    }



    // Got mutex, so draw
    Ret = DrawFrame();
#if 0
    if (Ret == DUPL_RETURN_SUCCESS)
        // We have keyed mutex so we can access the mouse info
        if (PointerInfo->Visible)
            // Draw mouse into texture
            Ret = DrawMouse(PointerInfo);
#endif



    // Release keyed mutex
    hr = m_KeyMutex->ReleaseSync(0);
    if (FAILED(hr))
        return ProcessFailure(m_Device, L"Failed to Release Keyed mutex in OUTPUTMANAGER", L"Error", hr, SystemTransitionsExpectedErrors);


    // Present to window if all worked
	if (Ret != DUPL_RETURN_SUCCESS) return Ret;
    // Present to window
#if 0
    hr = m_SwapChain->Present(1, 0);
#endif

	IDXGISurface1* outputSurf = nullptr;
	hr = manual_Backbuffer[0]->QueryInterface(__uuidof(IDXGISurface1), reinterpret_cast<void**>(&outputSurf));
	if (FAILED(hr)) { 
		ProcessFailure(nullptr, L"Failed to obtain the surface from the backbuffer", L"Error", hr);
	}

	HDC outputDC;
	hr = outputSurf->GetDC(FALSE, &outputDC);
	if (FAILED(hr)) { 
		ProcessFailure(nullptr, L"Failed to obtain the DC from the surface of the backbuffer", L"Error", hr);
	}


#if 1
	HDC hdcScreen = GetDC(NULL); //TODO(fran): why can I use any dc????
#else
	HDC hdcScreen = GetDC(m_WindowHandle);
#endif

	RECT rect;
	GetClientRect(m_WindowHandle, &rect);
	POINT origin;
	origin.x = rect.left;
	origin.y = rect.top;
	SIZE size;
	size.cx = rect.right;
	size.cy = rect.bottom;

	POINT source_origin;
	source_origin.x = 0;
	source_origin.y = 0;

	BLENDFUNCTION blend = { 0 };
	blend.BlendOp = AC_SRC_OVER;
	blend.SourceConstantAlpha = 255; //only want to use per pixel alpha
	blend.AlphaFormat = AC_SRC_ALPHA; // bitmap has alpha channel (per pixel alpha)

	//TODO?(fran): I think that now the app never sleeps, cause I update the window, therefore there was a window update and a new acquired frame, and so on and on, the loop never stops
	// POSSIBLE SOLUTION: I could update every other frame

#if 1
	//if (TEST_draw) {
		BOOL res = UpdateLayeredWindow(m_WindowHandle, hdcScreen, &origin, &size, outputDC, &source_origin, NULL,
			&blend, ULW_ALPHA);
		if (res == 0) {
			return ProcessFailure(nullptr, L"Failed to update the veil window with the new frame", L"Error", E_UNEXPECTED);
		}
	//}
#endif

#if 1
	ReleaseDC(NULL, hdcScreen);
#else
	ReleaseDC(m_WindowHandle, hdcScreen);
#endif

	RECT norect;
	RtlZeroMemory(&norect, sizeof(norect));
	hr = outputSurf->ReleaseDC(&norect);
	//TODO(fran): should I release outputSurf?
	if (FAILED(hr)) { 
		return ProcessFailure(nullptr, L"Failed to release the DC of the surface of the backbuffer", L"Error", hr);
	}

#if 0
    if (FAILED(hr))
    {
        return ProcessFailure(m_Device, L"Failed to present", L"Error", hr, SystemTransitionsExpectedErrors);
    }
#endif

	SwitchBuffers();

    return Ret;
}

//
// Returns shared handle
//
HANDLE OUTPUTMANAGER::GetSharedHandle()
{
    HANDLE Hnd = nullptr;

    // QI IDXGIResource interface to synchronized shared surface.
    IDXGIResource* DXGIResource = nullptr;
    HRESULT hr = m_SharedSurf->QueryInterface(__uuidof(IDXGIResource), reinterpret_cast<void**>(&DXGIResource));
    if (SUCCEEDED(hr))
    {
        // Obtain handle to IDXGIResource object.
        DXGIResource->GetSharedHandle(&Hnd);
        DXGIResource->Release();
        DXGIResource = nullptr;
    }

    return Hnd;
}

//
// Draw frame into backbuffer
//
DUPL_RETURN OUTPUTMANAGER::DrawFrame()
{
    HRESULT hr;

    // If window was resized, resize swapchain
    if (m_NeedsResize)
    {
        DUPL_RETURN Ret = ResizeSwapChain();
        if (Ret != DUPL_RETURN_SUCCESS)
        {
            return Ret;
        }
        m_NeedsResize = false;
    }

    // Vertices for drawing whole texture
    VERTEX Vertices[NUMVERTICES] =
    {
        {XMFLOAT3(-1.0f, -1.0f, 0), XMFLOAT2(0.0f, 1.0f)},
        {XMFLOAT3(-1.0f, 1.0f, 0), XMFLOAT2(0.0f, 0.0f)},
        {XMFLOAT3(1.0f, -1.0f, 0), XMFLOAT2(1.0f, 1.0f)},
        {XMFLOAT3(1.0f, -1.0f, 0), XMFLOAT2(1.0f, 1.0f)},
        {XMFLOAT3(-1.0f, 1.0f, 0), XMFLOAT2(0.0f, 0.0f)},
        {XMFLOAT3(1.0f, 1.0f, 0), XMFLOAT2(1.0f, 0.0f)},
    };

    D3D11_TEXTURE2D_DESC FrameDesc;
    m_SharedSurf->GetDesc(&FrameDesc);

    D3D11_SHADER_RESOURCE_VIEW_DESC ShaderDesc;
    ShaderDesc.Format = FrameDesc.Format;
    ShaderDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
    ShaderDesc.Texture2D.MostDetailedMip = FrameDesc.MipLevels - 1;
    ShaderDesc.Texture2D.MipLevels = FrameDesc.MipLevels;

    // Create new shader resource view
    ID3D11ShaderResourceView* ShaderResource = nullptr;
    hr = m_Device->CreateShaderResourceView(m_SharedSurf, &ShaderDesc, &ShaderResource);
    if (FAILED(hr))
    {
        return ProcessFailure(m_Device, L"Failed to create shader resource when drawing a frame", L"Error", hr, SystemTransitionsExpectedErrors);
    }

	D3D11_TEXTURE2D_DESC PreviousVeilDesc;
	manual_Backbuffer[1]->GetDesc(&PreviousVeilDesc);

	D3D11_SHADER_RESOURCE_VIEW_DESC PreviousVeilShaderDesc;
	PreviousVeilShaderDesc.Format = PreviousVeilDesc.Format;
	PreviousVeilShaderDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
	PreviousVeilShaderDesc.Texture2D.MostDetailedMip = PreviousVeilDesc.MipLevels - 1;
	PreviousVeilShaderDesc.Texture2D.MipLevels = PreviousVeilDesc.MipLevels;

	ID3D11ShaderResourceView* PreviousVeilShaderResource = nullptr;
	hr = m_Device->CreateShaderResourceView(manual_Backbuffer[1], &PreviousVeilShaderDesc, &PreviousVeilShaderResource);
	if (FAILED(hr))
	{
		return ProcessFailure(m_Device, L"Failed to create shader resource when drawing a frame", L"Error", hr, SystemTransitionsExpectedErrors);
	}

    // Set resources
    UINT Stride = sizeof(VERTEX);
    UINT Offset = 0;
#if 0
    FLOAT blendFactor[4] = {0.f, 0.f, 0.f, 0.f};
    m_DeviceContext->OMSetBlendState(nullptr, blendFactor, 0xffffffff);
	// I think the ffffffff represents what part of each pixel to use, in this case every bit is set to 1
	// so we will use the whole 8 bits of a r g b
    m_DeviceContext->OMSetRenderTargets(1, &m_RTV, nullptr);
#endif
    m_DeviceContext->VSSetShader(m_VertexShader, nullptr, 0);
    m_DeviceContext->PSSetShader(m_PixelShader, nullptr, 0);
    m_DeviceContext->PSSetShaderResources(0, 1, &ShaderResource);
	m_DeviceContext->PSSetShaderResources(1, 1, &PreviousVeilShaderResource);
    m_DeviceContext->PSSetSamplers(0, 1, &m_SamplerLinear);
    m_DeviceContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

    D3D11_BUFFER_DESC BufferDesc;
    RtlZeroMemory(&BufferDesc, sizeof(BufferDesc));
    BufferDesc.Usage = D3D11_USAGE_DEFAULT;
    BufferDesc.ByteWidth = sizeof(VERTEX) * NUMVERTICES;
    BufferDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
    BufferDesc.CPUAccessFlags = 0;
    D3D11_SUBRESOURCE_DATA InitData;
    RtlZeroMemory(&InitData, sizeof(InitData));
    InitData.pSysMem = Vertices;

    ID3D11Buffer* VertexBuffer = nullptr;

    // Create vertex buffer
    hr = m_Device->CreateBuffer(&BufferDesc, &InitData, &VertexBuffer);
    if (FAILED(hr))
    {
        ShaderResource->Release();
        ShaderResource = nullptr;
		PreviousVeilShaderResource->Release();
		PreviousVeilShaderResource = nullptr;
        return ProcessFailure(m_Device, L"Failed to create vertex buffer when drawing a frame", L"Error", hr, SystemTransitionsExpectedErrors);
    }
    m_DeviceContext->IASetVertexBuffers(0, 1, &VertexBuffer, &Stride, &Offset);

	//Source Row Pitch = [size of one element in bytes] * [number of elements in one row]
	//Source Depth Pitch = [Source Row Pitch] * [number of rows(height)]
	float temp_buffer[4] = { Threshold,Opacity,0,0 };
	m_DeviceContext->UpdateSubresource(ThresholdOpacityBuffer, 0, 0, &temp_buffer, sizeof(float), sizeof(float));

	m_DeviceContext->PSSetConstantBuffers(0,1,&ThresholdOpacityBuffer);

    // Draw textured quad onto render target
    m_DeviceContext->Draw(NUMVERTICES, 0);

    VertexBuffer->Release();
    VertexBuffer = nullptr;

    // Release shader resource
    ShaderResource->Release();
    ShaderResource = nullptr;

	PreviousVeilShaderResource->Release();
	PreviousVeilShaderResource = nullptr;

    return DUPL_RETURN_SUCCESS;
}

//
// Initialize shaders for drawing to screen
//
DUPL_RETURN OUTPUTMANAGER::InitShaders()
{
    HRESULT hr;

    UINT Size = ARRAYSIZE(g_VS);
    hr = m_Device->CreateVertexShader(g_VS, Size, nullptr, &m_VertexShader);
    if (FAILED(hr))
    {
        return ProcessFailure(m_Device, L"Failed to create vertex shader in OUTPUTMANAGER", L"Error", hr, SystemTransitionsExpectedErrors);
    }

    D3D11_INPUT_ELEMENT_DESC Layout[] =
    {
        {"POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0},
        {"TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 12, D3D11_INPUT_PER_VERTEX_DATA, 0}
    };
    UINT NumElements = ARRAYSIZE(Layout);
    hr = m_Device->CreateInputLayout(Layout, NumElements, g_VS, Size, &m_InputLayout);
    if (FAILED(hr))
    {
        return ProcessFailure(m_Device, L"Failed to create input layout in OUTPUTMANAGER", L"Error", hr, SystemTransitionsExpectedErrors);
    }
    m_DeviceContext->IASetInputLayout(m_InputLayout);

    Size = ARRAYSIZE(g_PSV);
    hr = m_Device->CreatePixelShader(g_PSV, Size, nullptr, &m_PixelShader);
    if (FAILED(hr))
    {
        return ProcessFailure(m_Device, L"Failed to create pixel shader in OUTPUTMANAGER", L"Error", hr, SystemTransitionsExpectedErrors);
    }

    return DUPL_RETURN_SUCCESS;
}

//
// Reset render target view
//
DUPL_RETURN OUTPUTMANAGER::MakeRTV()
{
	HRESULT hr;
    // Get backbuffer
#if 0
    ID3D11Texture2D* BackBuffer = nullptr;
    hr = m_SwapChain->GetBuffer(0, __uuidof(ID3D11Texture2D), reinterpret_cast<void**>(&BackBuffer));
    if (FAILED(hr))
    {
        return ProcessFailure(m_Device, L"Failed to get backbuffer for making render target view in OUTPUTMANAGER", L"Error", hr, SystemTransitionsExpectedErrors);
    }
#else
	ID3D11Texture2D* BackBuffer = manual_Backbuffer[0];
#endif

	if (m_RTV) {
		m_RTV->Release();
	}

    // Create a render target view
    hr = m_Device->CreateRenderTargetView(BackBuffer, nullptr, &m_RTV);
    //BackBuffer->Release();

    if (FAILED(hr))
    {
        return ProcessFailure(m_Device, L"Failed to create render target view in OUTPUTMANAGER", L"Error", hr, SystemTransitionsExpectedErrors);
    }

    // Set new render target
	ID3D11ShaderResourceView *const pSRV[1] = { NULL };
	m_DeviceContext->PSSetShaderResources(0, 1, pSRV);
	m_DeviceContext->PSSetShaderResources(1, 1, pSRV);
    m_DeviceContext->OMSetRenderTargets(1, &m_RTV, nullptr);

    return DUPL_RETURN_SUCCESS;
}

//
// Set new viewport
//
void OUTPUTMANAGER::SetViewPort(UINT Width, UINT Height)
{
    D3D11_VIEWPORT VP;
    VP.Width = static_cast<FLOAT>(Width);
    VP.Height = static_cast<FLOAT>(Height);
    VP.MinDepth = 0.0f;
    VP.MaxDepth = 1.0f;
    VP.TopLeftX = 0;
    VP.TopLeftY = 0;
    m_DeviceContext->RSSetViewports(1, &VP);
}

//
// Resize swapchain
//
DUPL_RETURN OUTPUTMANAGER::ResizeSwapChain()
{
    if (m_RTV)
    {
        m_RTV->Release();
        m_RTV = nullptr;
    }

    RECT WindowRect;
    GetClientRect(m_WindowHandle, &WindowRect);
    UINT Width = WindowRect.right - WindowRect.left;
    UINT Height = WindowRect.bottom - WindowRect.top;

    // Resize swapchain
#if 0
    DXGI_SWAP_CHAIN_DESC SwapChainDesc;
    m_SwapChain->GetDesc(&SwapChainDesc);
    HRESULT hr = m_SwapChain->ResizeBuffers(SwapChainDesc.BufferCount, Width, Height, SwapChainDesc.BufferDesc.Format, SwapChainDesc.Flags);
    if (FAILED(hr))
    {
        return ProcessFailure(m_Device, L"Failed to resize swapchain buffers in OUTPUTMANAGER", L"Error", hr, SystemTransitionsExpectedErrors);
    }
#else
	if (manual_Backbuffer[0]) {
		manual_Backbuffer[0]->Release();
		manual_Backbuffer[0] = nullptr;
	}
	if (manual_Backbuffer[1]) {
		manual_Backbuffer[1]->Release();
		manual_Backbuffer[1] = nullptr;
	}
	D3D11_TEXTURE2D_DESC RenderTargetDesc;
	RenderTargetDesc.ArraySize = 1;
	RenderTargetDesc.BindFlags = D3D11_BIND_RENDER_TARGET | D3D11_BIND_SHADER_RESOURCE;
	RenderTargetDesc.CPUAccessFlags = 0;
	RenderTargetDesc.Format = DXGI_FORMAT_B8G8R8A8_UNORM;
	RenderTargetDesc.Height = Height;
	RenderTargetDesc.MipLevels = 1;
	RenderTargetDesc.MiscFlags = D3D11_RESOURCE_MISC_GDI_COMPATIBLE;
	RenderTargetDesc.SampleDesc.Count = 1;
	RenderTargetDesc.SampleDesc.Quality = 0;
	RenderTargetDesc.Usage = D3D11_USAGE_DEFAULT;
	RenderTargetDesc.Width = Width;

	HRESULT hr = m_Device->CreateTexture2D(&RenderTargetDesc, nullptr, &manual_Backbuffer[0]);
	if (FAILED(hr)) { 
		return ProcessFailure(nullptr, L"Failed to create the frontbuffer texture for the output", L"Error", hr);
	}
	hr = m_Device->CreateTexture2D(&RenderTargetDesc, nullptr, &manual_Backbuffer[1]);
	if (FAILED(hr)) { 
		return ProcessFailure(nullptr, L"Failed to create the backbuffer texture for the output", L"Error", hr);
	}
#endif

    // Make new render target view
    DUPL_RETURN Ret = MakeRTV();
    if (Ret != DUPL_RETURN_SUCCESS)
    {
        return Ret;
    }

    // Set new viewport
    SetViewPort(Width, Height);

    return Ret;
}

DUPL_RETURN OUTPUTMANAGER::ResetSecondaryTexture() 
//INFO: this function needs to set the render target to the secondary texture to clear it, therefore the real render target must be set AFTER this
{
	ID3D11RenderTargetView* tempRTV;

	// Create a temporal render target view
	HRESULT hr = m_Device->CreateRenderTargetView(manual_Backbuffer[1], nullptr, &tempRTV);
	//manual_Backbuffer[1]->Release();
	//manual_Backbuffer[1] = nullptr;

	if (FAILED(hr)) { 
		return ProcessFailure(nullptr, L"Failed to create render target from the backbuffer", L"Error", hr);
	}

	// Set the render target
	m_DeviceContext->OMSetRenderTargets(1, &tempRTV, nullptr);

	//Clear the texture
	float clearColor[4] = { 0,0,0,0 };
	m_DeviceContext->ClearRenderTargetView(tempRTV, clearColor);
	
	//Leave everything clean for the real render target
	m_DeviceContext->OMSetRenderTargets(0, nullptr, nullptr);
	tempRTV->Release();

	return DUPL_RETURN_SUCCESS;
}

void OUTPUTMANAGER::SwitchBuffers()
{
	//TODO(fran): there is still some problems with switching buffers and cleaning them
	ID3D11Texture2D* tex = manual_Backbuffer[0];
	manual_Backbuffer[0] = manual_Backbuffer[1];
	manual_Backbuffer[1] = tex;
	manual_Backbuffer[1]->Release();
}

//
// Releases all references
//
void OUTPUTMANAGER::CleanRefs()
{
	if (ThresholdOpacityBuffer) { //INFO IMPORTANT(fran): ThresholdOpacityBuffer was keeping a ref to the Device that created it, so when we release it we are also lower its device's ref count
		ThresholdOpacityBuffer->Release();
		ThresholdOpacityBuffer = nullptr;
	}
	if (manual_Backbuffer[0]) {
		manual_Backbuffer[0]->Release();
		manual_Backbuffer[0] = nullptr;
	}
	if (manual_Backbuffer[1]) {
		manual_Backbuffer[1]->Release();
		manual_Backbuffer[1] = nullptr;
	}
	if (m_VertexShader)
    {
        m_VertexShader->Release();
        m_VertexShader = nullptr;
    }

    if (m_PixelShader)
    {
        m_PixelShader->Release();
        m_PixelShader = nullptr;
    }

    if (m_InputLayout)
    {
        m_InputLayout->Release();
        m_InputLayout = nullptr;
    }

    if (m_RTV)
    {
        m_RTV->Release();
        m_RTV = nullptr;
    }

    if (m_SamplerLinear)
    {
        m_SamplerLinear->Release();
        m_SamplerLinear = nullptr;
    }
#if 0
    if (m_BlendState)
    {
        m_BlendState->Release();
        m_BlendState = nullptr;
    }
#endif
    if (m_DeviceContext)
    {
        m_DeviceContext->Release();
        m_DeviceContext = nullptr;
    }
#ifndef _DX_DEBUG_LAYER
    if (m_Device)
    {
        m_Device->Release();
        m_Device = nullptr;
    }
#endif
#if 0
    if (m_SwapChain)
    {
        m_SwapChain->Release();
        m_SwapChain = nullptr;
    }
#endif

    if (m_SharedSurf)
    {
        m_SharedSurf->Release();
        m_SharedSurf = nullptr;
    }

    if (m_KeyMutex)
    {
        m_KeyMutex->Release();
        m_KeyMutex = nullptr;
    }

    //if (m_Factory)
    //{
    //    //if (m_OcclusionCookie)
    //    //{
    //    //    m_Factory->UnregisterOcclusionStatus(m_OcclusionCookie);
    //    //    m_OcclusionCookie = 0;
    //    //}
    //    m_Factory->Release();
    //    m_Factory = nullptr;
    //}
#ifdef _DX_DEBUG_LAYER
	//IDXGIDebug* m_d3dDebug;
	ID3D11Debug* d;
	m_Device->QueryInterface(__uuidof(ID3D11Debug), reinterpret_cast<void**>(&d));
	d->ReportLiveDeviceObjects(D3D11_RLDO_SUMMARY | D3D11_RLDO_DETAIL);
#endif
}

//
// Draw mouse provided in buffer to backbuffer
//
/*
DUPL_RETURN OUTPUTMANAGER::DrawMouse(_In_ PTR_INFO* PtrInfo)
{
	// Vars to be used
	ID3D11Texture2D* MouseTex = nullptr;
	ID3D11ShaderResourceView* ShaderRes = nullptr;
	ID3D11Buffer* VertexBufferMouse = nullptr;
	D3D11_SUBRESOURCE_DATA InitData;
	D3D11_TEXTURE2D_DESC Desc;
	D3D11_SHADER_RESOURCE_VIEW_DESC SDesc;

	// Position will be changed based on mouse position
	VERTEX Vertices[NUMVERTICES] =
	{
		{XMFLOAT3(-1.0f, -1.0f, 0), XMFLOAT2(0.0f, 1.0f)},
		{XMFLOAT3(-1.0f, 1.0f, 0), XMFLOAT2(0.0f, 0.0f)},
		{XMFLOAT3(1.0f, -1.0f, 0), XMFLOAT2(1.0f, 1.0f)},
		{XMFLOAT3(1.0f, -1.0f, 0), XMFLOAT2(1.0f, 1.0f)},
		{XMFLOAT3(-1.0f, 1.0f, 0), XMFLOAT2(0.0f, 0.0f)},
		{XMFLOAT3(1.0f, 1.0f, 0), XMFLOAT2(1.0f, 0.0f)},
	};

	D3D11_TEXTURE2D_DESC FullDesc;
	m_SharedSurf->GetDesc(&FullDesc);
	INT DesktopWidth = FullDesc.Width;
	INT DesktopHeight = FullDesc.Height;

	// Center of desktop dimensions
	INT CenterX = (DesktopWidth / 2);
	INT CenterY = (DesktopHeight / 2);

	// Clipping adjusted coordinates / dimensions
	INT PtrWidth = 0;
	INT PtrHeight = 0;
	INT PtrLeft = 0;
	INT PtrTop = 0;

	// Buffer used if necessary (in case of monochrome or masked pointer)
	BYTE* InitBuffer = nullptr;

	// Used for copying pixels
	D3D11_BOX Box;
	Box.front = 0;
	Box.back = 1;

	Desc.MipLevels = 1;
	Desc.ArraySize = 1;
	Desc.Format = DXGI_FORMAT_B8G8R8A8_UNORM;
	Desc.SampleDesc.Count = 1;
	Desc.SampleDesc.Quality = 0;
	Desc.Usage = D3D11_USAGE_DEFAULT;
	Desc.BindFlags = D3D11_BIND_SHADER_RESOURCE;
	Desc.CPUAccessFlags = 0;
	Desc.MiscFlags = 0;

	// Set shader resource properties
	SDesc.Format = Desc.Format;
	SDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
	SDesc.Texture2D.MostDetailedMip = Desc.MipLevels - 1;
	SDesc.Texture2D.MipLevels = Desc.MipLevels;

	switch (PtrInfo->ShapeInfo.Type)
	{
		case DXGI_OUTDUPL_POINTER_SHAPE_TYPE_COLOR:
		{
			PtrLeft = PtrInfo->Position.x;
			PtrTop = PtrInfo->Position.y;

			PtrWidth = static_cast<INT>(PtrInfo->ShapeInfo.Width);
			PtrHeight = static_cast<INT>(PtrInfo->ShapeInfo.Height);

			break;
		}

		case DXGI_OUTDUPL_POINTER_SHAPE_TYPE_MONOCHROME:
		{
			ProcessMonoMask(true, PtrInfo, &PtrWidth, &PtrHeight, &PtrLeft, &PtrTop, &InitBuffer, &Box);
			break;
		}

		case DXGI_OUTDUPL_POINTER_SHAPE_TYPE_MASKED_COLOR:
		{
			ProcessMonoMask(false, PtrInfo, &PtrWidth, &PtrHeight, &PtrLeft, &PtrTop, &InitBuffer, &Box);
			break;
		}

		default:
			break;
	}

	// VERTEX creation
	Vertices[0].Pos.x = (PtrLeft - CenterX) / (FLOAT)CenterX;
	Vertices[0].Pos.y = -1 * ((PtrTop + PtrHeight) - CenterY) / (FLOAT)CenterY;
	Vertices[1].Pos.x = (PtrLeft - CenterX) / (FLOAT)CenterX;
	Vertices[1].Pos.y = -1 * (PtrTop - CenterY) / (FLOAT)CenterY;
	Vertices[2].Pos.x = ((PtrLeft + PtrWidth) - CenterX) / (FLOAT)CenterX;
	Vertices[2].Pos.y = -1 * ((PtrTop + PtrHeight) - CenterY) / (FLOAT)CenterY;
	Vertices[3].Pos.x = Vertices[2].Pos.x;
	Vertices[3].Pos.y = Vertices[2].Pos.y;
	Vertices[4].Pos.x = Vertices[1].Pos.x;
	Vertices[4].Pos.y = Vertices[1].Pos.y;
	Vertices[5].Pos.x = ((PtrLeft + PtrWidth) - CenterX) / (FLOAT)CenterX;
	Vertices[5].Pos.y = -1 * (PtrTop - CenterY) / (FLOAT)CenterY;

	// Set texture properties
	Desc.Width = PtrWidth;
	Desc.Height = PtrHeight;

	// Set up init data
	InitData.pSysMem = (PtrInfo->ShapeInfo.Type == DXGI_OUTDUPL_POINTER_SHAPE_TYPE_COLOR) ? PtrInfo->PtrShapeBuffer : InitBuffer;
	InitData.SysMemPitch = (PtrInfo->ShapeInfo.Type == DXGI_OUTDUPL_POINTER_SHAPE_TYPE_COLOR) ? PtrInfo->ShapeInfo.Pitch : PtrWidth * BPP;
	InitData.SysMemSlicePitch = 0;

	// Create mouseshape as texture
	HRESULT hr = m_Device->CreateTexture2D(&Desc, &InitData, &MouseTex);
	if (FAILED(hr))
	{
		return ProcessFailure(m_Device, L"Failed to create mouse pointer texture", L"Error", hr, SystemTransitionsExpectedErrors);
	}

	// Create shader resource from texture
	hr = m_Device->CreateShaderResourceView(MouseTex, &SDesc, &ShaderRes);
	if (FAILED(hr))
	{
		MouseTex->Release();
		MouseTex = nullptr;
		return ProcessFailure(m_Device, L"Failed to create shader resource from mouse pointer texture", L"Error", hr, SystemTransitionsExpectedErrors);
	}

	D3D11_BUFFER_DESC BDesc;
	ZeroMemory(&BDesc, sizeof(D3D11_BUFFER_DESC));
	BDesc.Usage = D3D11_USAGE_DEFAULT;
	BDesc.ByteWidth = sizeof(VERTEX) * NUMVERTICES;
	BDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	BDesc.CPUAccessFlags = 0;

	ZeroMemory(&InitData, sizeof(D3D11_SUBRESOURCE_DATA));
	InitData.pSysMem = Vertices;

	// Create vertex buffer
	hr = m_Device->CreateBuffer(&BDesc, &InitData, &VertexBufferMouse);
	if (FAILED(hr))
	{
		ShaderRes->Release();
		ShaderRes = nullptr;
		MouseTex->Release();
		MouseTex = nullptr;
		return ProcessFailure(m_Device, L"Failed to create mouse pointer vertex buffer in OutputManager", L"Error", hr, SystemTransitionsExpectedErrors);
	}

	// Set resources
	FLOAT BlendFactor[4] = {0.f, 0.f, 0.f, 0.f};
	UINT Stride = sizeof(VERTEX);
	UINT Offset = 0;
	m_DeviceContext->IASetVertexBuffers(0, 1, &VertexBufferMouse, &Stride, &Offset);
	m_DeviceContext->OMSetBlendState(m_BlendState, BlendFactor, 0xFFFFFFFF);
	m_DeviceContext->OMSetRenderTargets(1, &m_RTV, nullptr);
	m_DeviceContext->VSSetShader(m_VertexShader, nullptr, 0);
	m_DeviceContext->PSSetShader(m_PixelShader, nullptr, 0);
	m_DeviceContext->PSSetShaderResources(0, 1, &ShaderRes);
	m_DeviceContext->PSSetSamplers(0, 1, &m_SamplerLinear);

	// Draw
	m_DeviceContext->Draw(NUMVERTICES, 0);

	// Clean
	if (VertexBufferMouse)
	{
		VertexBufferMouse->Release();
		VertexBufferMouse = nullptr;
	}
	if (ShaderRes)
	{
		ShaderRes->Release();
		ShaderRes = nullptr;
	}
	if (MouseTex)
	{
		MouseTex->Release();
		MouseTex = nullptr;
	}
	if (InitBuffer)
	{
		delete [] InitBuffer;
		InitBuffer = nullptr;
	}

	return DUPL_RETURN_SUCCESS;
}
*/

/*
//
// Process both masked and monochrome pointers
//
DUPL_RETURN OUTPUTMANAGER::ProcessMonoMask(bool IsMono, _Inout_ PTR_INFO* PtrInfo, _Out_ INT* PtrWidth, _Out_ INT* PtrHeight, _Out_ INT* PtrLeft, _Out_ INT* PtrTop, _Outptr_result_bytebuffer_(*PtrHeight * *PtrWidth * BPP) BYTE** InitBuffer, _Out_ D3D11_BOX* Box)
{
	// Desktop dimensions
	D3D11_TEXTURE2D_DESC FullDesc;
	m_SharedSurf->GetDesc(&FullDesc);
	INT DesktopWidth = FullDesc.Width;
	INT DesktopHeight = FullDesc.Height;

	// Pointer position
	INT GivenLeft = PtrInfo->Position.x;
	INT GivenTop = PtrInfo->Position.y;

	// Figure out if any adjustment is needed for out of bound positions
	if (GivenLeft < 0)
	{
		*PtrWidth = GivenLeft + static_cast<INT>(PtrInfo->ShapeInfo.Width);
	}
	else if ((GivenLeft + static_cast<INT>(PtrInfo->ShapeInfo.Width)) > DesktopWidth)
	{
		*PtrWidth = DesktopWidth - GivenLeft;
	}
	else
	{
		*PtrWidth = static_cast<INT>(PtrInfo->ShapeInfo.Width);
	}

	if (IsMono)
	{
		PtrInfo->ShapeInfo.Height = PtrInfo->ShapeInfo.Height / 2;
	}

	if (GivenTop < 0)
	{
		*PtrHeight = GivenTop + static_cast<INT>(PtrInfo->ShapeInfo.Height);
	}
	else if ((GivenTop + static_cast<INT>(PtrInfo->ShapeInfo.Height)) > DesktopHeight)
	{
		*PtrHeight = DesktopHeight - GivenTop;
	}
	else
	{
		*PtrHeight = static_cast<INT>(PtrInfo->ShapeInfo.Height);
	}

	if (IsMono)
	{
		PtrInfo->ShapeInfo.Height = PtrInfo->ShapeInfo.Height * 2;
	}

	*PtrLeft = (GivenLeft < 0) ? 0 : GivenLeft;
	*PtrTop = (GivenTop < 0) ? 0 : GivenTop;

	// Staging buffer/texture
	D3D11_TEXTURE2D_DESC CopyBufferDesc;
	CopyBufferDesc.Width = *PtrWidth;
	CopyBufferDesc.Height = *PtrHeight;
	CopyBufferDesc.MipLevels = 1;
	CopyBufferDesc.ArraySize = 1;
	CopyBufferDesc.Format = DXGI_FORMAT_B8G8R8A8_UNORM;
	CopyBufferDesc.SampleDesc.Count = 1;
	CopyBufferDesc.SampleDesc.Quality = 0;
	CopyBufferDesc.Usage = D3D11_USAGE_STAGING;
	CopyBufferDesc.BindFlags = 0;
	CopyBufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_READ;
	CopyBufferDesc.MiscFlags = 0;

	ID3D11Texture2D* CopyBuffer = nullptr;
	HRESULT hr = m_Device->CreateTexture2D(&CopyBufferDesc, nullptr, &CopyBuffer);
	if (FAILED(hr))
	{
		return ProcessFailure(m_Device, L"Failed creating staging texture for pointer", L"Error", hr, SystemTransitionsExpectedErrors);
	}

	// Copy needed part of desktop image
	Box->left = *PtrLeft;
	Box->top = *PtrTop;
	Box->right = *PtrLeft + *PtrWidth;
	Box->bottom = *PtrTop + *PtrHeight;
	m_DeviceContext->CopySubresourceRegion(CopyBuffer, 0, 0, 0, 0, m_SharedSurf, 0, Box);

	// QI for IDXGISurface
	IDXGISurface* CopySurface = nullptr;
	hr = CopyBuffer->QueryInterface(__uuidof(IDXGISurface), (void **)&CopySurface);
	CopyBuffer->Release();
	CopyBuffer = nullptr;
	if (FAILED(hr))
	{
		return ProcessFailure(nullptr, L"Failed to QI staging texture into IDXGISurface for pointer", L"Error", hr, SystemTransitionsExpectedErrors);
	}

	// Map pixels
	DXGI_MAPPED_RECT MappedSurface;
	hr = CopySurface->Map(&MappedSurface, DXGI_MAP_READ);
	if (FAILED(hr))
	{
		CopySurface->Release();
		CopySurface = nullptr;
		return ProcessFailure(m_Device, L"Failed to map surface for pointer", L"Error", hr, SystemTransitionsExpectedErrors);
	}

	// New mouseshape buffer
	*InitBuffer = new (std::nothrow) BYTE[*PtrWidth * *PtrHeight * BPP];
	if (!(*InitBuffer))
	{
		return ProcessFailure(nullptr, L"Failed to allocate memory for new mouse shape buffer.", L"Error", E_OUTOFMEMORY);
	}

	UINT* InitBuffer32 = reinterpret_cast<UINT*>(*InitBuffer);
	UINT* Desktop32 = reinterpret_cast<UINT*>(MappedSurface.pBits);
	UINT  DesktopPitchInPixels = MappedSurface.Pitch / sizeof(UINT);

	// What to skip (pixel offset)
	UINT SkipX = (GivenLeft < 0) ? (-1 * GivenLeft) : (0);
	UINT SkipY = (GivenTop < 0) ? (-1 * GivenTop) : (0);

	if (IsMono)
	{
		for (INT Row = 0; Row < *PtrHeight; ++Row)
		{
			// Set mask
			BYTE Mask = 0x80;
			Mask = Mask >> (SkipX % 8);
			for (INT Col = 0; Col < *PtrWidth; ++Col)
			{
				// Get masks using appropriate offsets
				BYTE AndMask = PtrInfo->PtrShapeBuffer[((Col + SkipX) / 8) + ((Row + SkipY) * (PtrInfo->ShapeInfo.Pitch))] & Mask;
				BYTE XorMask = PtrInfo->PtrShapeBuffer[((Col + SkipX) / 8) + ((Row + SkipY + (PtrInfo->ShapeInfo.Height / 2)) * (PtrInfo->ShapeInfo.Pitch))] & Mask;
				UINT AndMask32 = (AndMask) ? 0xFFFFFFFF : 0xFF000000;
				UINT XorMask32 = (XorMask) ? 0x00FFFFFF : 0x00000000;

				// Set new pixel
				InitBuffer32[(Row * *PtrWidth) + Col] = (Desktop32[(Row * DesktopPitchInPixels) + Col] & AndMask32) ^ XorMask32;

				// Adjust mask
				if (Mask == 0x01)
				{
					Mask = 0x80;
				}
				else
				{
					Mask = Mask >> 1;
				}
			}
		}
	}
	else
	{
		UINT* Buffer32 = reinterpret_cast<UINT*>(PtrInfo->PtrShapeBuffer);

		// Iterate through pixels
		for (INT Row = 0; Row < *PtrHeight; ++Row)
		{
			for (INT Col = 0; Col < *PtrWidth; ++Col)
			{
				// Set up mask
				UINT MaskVal = 0xFF000000 & Buffer32[(Col + SkipX) + ((Row + SkipY) * (PtrInfo->ShapeInfo.Pitch / sizeof(UINT)))];
				if (MaskVal)
				{
					// Mask was 0xFF
					InitBuffer32[(Row * *PtrWidth) + Col] = (Desktop32[(Row * DesktopPitchInPixels) + Col] ^ Buffer32[(Col + SkipX) + ((Row + SkipY) * (PtrInfo->ShapeInfo.Pitch / sizeof(UINT)))]) | 0xFF000000;
				}
				else
				{
					// Mask was 0x00
					InitBuffer32[(Row * *PtrWidth) + Col] = Buffer32[(Col + SkipX) + ((Row + SkipY) * (PtrInfo->ShapeInfo.Pitch / sizeof(UINT)))] | 0xFF000000;
				}
			}
		}
	}

	// Done with resource
	hr = CopySurface->Unmap();
	CopySurface->Release();
	CopySurface = nullptr;
	if (FAILED(hr))
	{
		return ProcessFailure(m_Device, L"Failed to unmap surface for pointer", L"Error", hr, SystemTransitionsExpectedErrors);
	}

	return DUPL_RETURN_SUCCESS;
}
*/