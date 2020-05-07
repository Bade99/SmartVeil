#include "CommonTypes.h"
#include <wchar.h> //swprintf_s


//bool TEST_draw = true; //TODO(fran): REMOVE

/// <summary>
/// Displays a message
/// </summary>
/// <param name="Str"></param>
/// <param name="Title"></param>
/// <param name="hr"></param>
inline void DisplayMsg(_In_ LPCWSTR Str, _In_ LPCWSTR Title, HRESULT hr)
{
	UINT flags = MB_OK | MB_TOPMOST;
	if (SUCCEEDED(hr))
	{
		MessageBoxW(nullptr, Str, Title, flags);
		return;
	}

	const UINT StringLen = (UINT)(wcslen(Str) + sizeof(" HRESULT 0x########")) + 1;
	wchar_t* OutStr = new wchar_t[StringLen];
	if (!OutStr)
	{
		return;
	}

	INT LenWritten = swprintf_s(OutStr, StringLen, L"%s HRESULT 0x%X", Str, hr);
	if (LenWritten != -1)
	{
		MessageBoxW(nullptr, OutStr, Title, flags);
	}

	delete[] OutStr;
}

_Post_satisfies_(return != DUPL_RETURN_SUCCESS)
DUPL_RETURN ProcessFailure(_In_opt_ ID3D11Device* Device, _In_ LPCWSTR Str, _In_ LPCWSTR Title, HRESULT hr, _In_opt_z_ const HRESULT* ExpectedErrors)
{
	HRESULT TranslatedHr;

	// On an error check if the DX device is lost
	if (Device)
	{
		HRESULT DeviceRemovedReason = Device->GetDeviceRemovedReason();

		switch (DeviceRemovedReason)
		{
		case DXGI_ERROR_DEVICE_REMOVED:
		case DXGI_ERROR_DEVICE_RESET:
			case static_cast<HRESULT>(E_OUTOFMEMORY) :
			{
				// Our device has been stopped due to an external event on the GPU so map them all to
				// device removed and continue processing the condition
				TranslatedHr = DXGI_ERROR_DEVICE_REMOVED;
				break;
			}

			case S_OK:
			{
				// Device is not removed so use original error
				TranslatedHr = hr;
				break;
			}

			default:
			{
				// Device is removed but not a error we want to remap
				TranslatedHr = DeviceRemovedReason;
			}
		}
	}
	else
	{
		TranslatedHr = hr;
	}

	// Check if this error was expected or not
	if (ExpectedErrors)
	{
		const HRESULT* CurrentResult = ExpectedErrors;

		while (*CurrentResult != S_OK)
		{
			if (*(CurrentResult++) == TranslatedHr)
			{
				return DUPL_RETURN_ERROR_EXPECTED;
			}
		}
	}

	// Error was not expected so display the message box
	DisplayMsg(Str, Title, TranslatedHr);

	return DUPL_RETURN_ERROR_UNEXPECTED;
}