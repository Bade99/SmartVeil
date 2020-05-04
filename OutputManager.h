
#pragma once

#include "CommonTypes.h"

//
// Handles the task of drawing into a window.
// Has the functionality to draw the mouse given a mouse shape buffer and position
//
class OUTPUTMANAGER
{
    public:
        OUTPUTMANAGER();
        ~OUTPUTMANAGER();
        DUPL_RETURN InitOutput(HWND Window, INT SingleOutput, _Out_ UINT* OutCount, _Out_ RECT* DeskBounds);
        DUPL_RETURN UpdateApplicationWindow(_In_ PTR_INFO* PointerInfo);
        void CleanRefs();
        HANDLE GetSharedHandle();
        void WindowResize();
		void SetThreshold(float threshold); // Receives a value normalized between 0 and 1
		void SetOpacity(float opacity); // Receives a value normalized between 0 and 1
		void RestartTextures();
    private:
    // Methods
        DUPL_RETURN ProcessMonoMask(bool IsMono, _Inout_ PTR_INFO* PtrInfo, _Out_ INT* PtrWidth, _Out_ INT* PtrHeight, _Out_ INT* PtrLeft, _Out_ INT* PtrTop, _Outptr_result_bytebuffer_(*PtrHeight * *PtrWidth * BPP) BYTE** InitBuffer, _Out_ D3D11_BOX* Box);
        DUPL_RETURN MakeRTV();
        void SetViewPort(UINT Width, UINT Height);
        DUPL_RETURN InitShaders();
        DUPL_RETURN InitGeometry();
        DUPL_RETURN CreateSharedSurf(INT SingleOutput, _Out_ UINT* OutCount, _Out_ RECT* DeskBounds);
        DUPL_RETURN DrawFrame();
        DUPL_RETURN DrawMouse(_In_ PTR_INFO* PtrInfo);
        DUPL_RETURN ResizeSwapChain();
		DUPL_RETURN ResetSecondaryTexture();

		void SwitchBuffers();
    // Vars
		ID3D11Texture2D* manual_Backbuffer[2]; //the one at position 0 will always be the output and the other will be the previous output
		ID3D11Device* m_Device;
#if 0
        IDXGISwapChain1* m_SwapChain;
#endif
        IDXGIFactory2* m_Factory;
        ID3D11DeviceContext* m_DeviceContext;
        ID3D11RenderTargetView* m_RTV;
        ID3D11SamplerState* m_SamplerLinear;
#if 0
        ID3D11BlendState* m_BlendState;
#endif
        ID3D11VertexShader* m_VertexShader;
        ID3D11PixelShader* m_PixelShader;
        ID3D11InputLayout* m_InputLayout;
        ID3D11Texture2D* m_SharedSurf;
        IDXGIKeyedMutex* m_KeyMutex;
        HWND m_WindowHandle;
        bool m_NeedsResize;
        DWORD m_OcclusionCookie;
		float Threshold;
		float Opacity;
		ID3D11Buffer* ThresholdOpacityBuffer;
		bool needsClearing;
};
