#include "ImagePresentation.h"

#include "resource.h"

#include "DYNAMIC_WAIT.h"
#include "CommonTypes.h"
#include "DisplayManager.h"
#include "DuplicationManager.h"

#include "utils.cpp"
#include "LANGUAGE_MANAGER.h"

#ifdef _DEBUG
#include "Settings.h"
#include "Common.h"
#endif

//TODO(fran): add system for letting caller know if the worker thread dies
DWORD WINAPI WorkerThread(void* Param) {
	//TODO(fran): should I use it straight instead of copying it?
	WORKER_THREAD_INIT* data = (WORKER_THREAD_INIT*)Param; //TODO(fran): check this copies everything right

	UINT OutputCount;
	RECT DeskBounds;
	INT SingleOutput= -1;

	DYNAMIC_WAIT DynamicWait;
	bool FirstTime = true;
	DUPL_RETURN Ret;
	DWORD mut_ret;
	//TODO(fran): check this mutex can be left unused
	//DWORD mut_ret = WaitForSingleObject(data.output_wnd_ready_mutex, INFINITE);
	//Assert(mut_ret == WAIT_OBJECT_0);
	//ReleaseMutex(data.output_wnd_ready_mutex);
	//Now we know the HWND is properly initialized and we can start working with it

	mut_ret = WaitForSingleObject(data->worker_finished_mutex, INFINITE);//Mutex that will be released when the thread finishes
	Assert(mut_ret == WAIT_OBJECT_0);

	//Basic update rate checking system https://stackoverflow.com/questions/1739259/how-to-use-queryperformancecounter
#ifdef _DEBUG
	double PCFreq = GetPCFrequency();
	__int64 CounterStart = 0;
	double CounterVal;
#endif

	while (WaitForSingleObject(data->next_frame_mutex, INFINITE) == WAIT_OBJECT_0) {

		if (data->terminate) {//TODO(fran): this is a bit botched
			//TODO(fran): cleanup of the other threads?
			ReleaseMutex(data->next_frame_mutex);
			ReleaseMutex(data->worker_finished_mutex);
			return 0;
		}

#ifdef _DEBUG
		StartCounter(CounterStart);
#endif

		Ret = DUPL_RETURN_SUCCESS;
		if (WaitForSingleObjectEx(data->events.UnexpectedErrorEvent, 0, FALSE) == WAIT_OBJECT_0)
		{
			// Unexpected error occurred so exit the application
			ReleaseMutex(data->next_frame_mutex);
			ReleaseMutex(data->worker_finished_mutex);
			std::wstring error_msg = RS(SCV_LANG_ERROR_UNEXPECTED);
			error_msg += L"\n";
			error_msg += RS(SCV_LANG_ERROR_CLOSING_APP);
			MessageBoxW(NULL, error_msg.c_str(), RCS(SCV_LANG_ERROR_SMARTVEIL), NULL);
			DestroyWindow(data->output_wnd); //TODO(fran): this doesnt make any sense here
			return 1;
		}
		else if (FirstTime || WaitForSingleObjectEx(data->events.ExpectedErrorEvent, 0, FALSE) == WAIT_OBJECT_0)
		{
			//All thread setup for execution runs here, I think
			if (!FirstTime)
			{
				// Terminate other threads
				SetEvent(data->events.TerminateThreadsEvent);
				data->thread_mgr.WaitForThreadTermination();
				ResetEvent(data->events.TerminateThreadsEvent);
				ResetEvent(data->events.ExpectedErrorEvent);

				// Clean up
				data->thread_mgr.Clean();
				data->output_mgr.CleanRefs();

				// As we have encountered an error due to a system transition we wait before trying again, using this dynamic wait
				// the wait periods will get progressively long to avoid wasting too much system resource if this state lasts a long time
				DynamicWait.Wait();
			}
			else
			{
				// First time through the loop so nothing to clean up
				FirstTime = false;
			}

			// Re-initialize
			Ret = data->output_mgr.InitOutput(data->output_wnd, SingleOutput, &OutputCount, &DeskBounds);
			if (Ret == DUPL_RETURN_SUCCESS)
			{
				HANDLE SharedHandle = data->output_mgr.GetSharedHandle();
				if (SharedHandle)
				{
					Ret = data->thread_mgr.Initialize(SingleOutput, OutputCount, data->events.UnexpectedErrorEvent, data->events.ExpectedErrorEvent,
														data->events.TerminateThreadsEvent, SharedHandle, &DeskBounds);
				}
				else
				{
					ProcessFailure(nullptr, RCS(SCV_LANG_ERROR_SHARED_SURF_HANDLE_GET), RCS(SCV_LANG_ERROR_SMARTVEIL), E_UNEXPECTED);
					Ret = DUPL_RETURN_ERROR_UNEXPECTED;
				}
			}

		}

		//TODO(fran): could it be that some of the flickering is produced cause we havent yet updated our current surface?
		// if that were happening then we would be presenting the one from two frames ago

		// Nothing else to do, so try to present to write out to window
		Ret = data->output_mgr.UpdateApplicationWindow(data->thread_mgr.GetPointerInfo());

#ifdef _DEBUG
		CounterVal = GetCounter(CounterStart, PCFreq);
		PostMessage(KNOWN_WINDOWS.settings, SCV_SETTINGS_UPDATE_COUNTER, (WPARAM)(float)CounterVal, 0);//this is clearly not great cause we dont know when we are gonna read this value
#endif

		// Check if for errors
		if (Ret != DUPL_RETURN_SUCCESS)
		{
			if (Ret == DUPL_RETURN_ERROR_EXPECTED)
			{
				// Some type of system transition is occurring so retry
				SetEvent(data->events.ExpectedErrorEvent);
			}
			else
			{
				// Unexpected error so exit
				ReleaseMutex(data->next_frame_mutex);
				ReleaseMutex(data->worker_finished_mutex);
				std::wstring error_msg = RS(SCV_LANG_ERROR_UNEXPECTED);
				error_msg += L"\n";
				error_msg += RS(SCV_LANG_ERROR_CLOSING_APP);
				MessageBoxW(NULL, error_msg.c_str(), RCS(SCV_LANG_ERROR_SMARTVEIL), NULL);
				DestroyWindow(data->output_wnd); //TODO(fran): this doesnt make any sense here
				return 1;
			}
		}
		ReleaseMutex(data->next_frame_mutex);

#if LIMIT_UPDATE
		if (CounterVal < TargetMs)
			Sleep(max(TargetMs - CounterVal, 0));
#endif
	}

	ReleaseMutex(data->worker_finished_mutex);
	return 0;
}

DWORD WINAPI DDProc(_In_ void* Param)
{
	// Classes
	DISPLAYMANAGER DispMgr;
	DUPLICATIONMANAGER DuplMgr;

	// D3D objects
	ID3D11Texture2D* SharedSurf = nullptr;
	IDXGIKeyedMutex* KeyMutex = nullptr;

	// Data passed in from thread creation
	THREAD_DATA* TData = reinterpret_cast<THREAD_DATA*>(Param);

	// Get desktop
	DUPL_RETURN Ret;
	HDESK CurrentDesktop = nullptr;
	CurrentDesktop = OpenInputDesktop(0, FALSE, GENERIC_ALL);
	if (!CurrentDesktop)
	{
		// We do not have access to the desktop so request a retry
		SetEvent(TData->ExpectedErrorEvent);
		Ret = DUPL_RETURN_ERROR_EXPECTED;
		goto Exit;
	}

	// Attach desktop to this thread
	bool DesktopAttached = SetThreadDesktop(CurrentDesktop) != 0;
	CloseDesktop(CurrentDesktop);
	CurrentDesktop = nullptr;
	if (!DesktopAttached)
	{
		// We do not have access to the desktop so request a retry
		Ret = DUPL_RETURN_ERROR_EXPECTED;
		goto Exit;
	}

	// New display manager
	DispMgr.InitD3D(&TData->DxRes);

	// Obtain handle to sync shared Surface
	HRESULT hr = TData->DxRes.Device->OpenSharedResource(TData->TexSharedHandle, __uuidof(ID3D11Texture2D), reinterpret_cast<void**>(&SharedSurf));
	if (FAILED(hr))
	{
		Ret = ProcessFailure(TData->DxRes.Device, RCS(SCV_LANG_ERROR_SHARED_SURF_OPEN), RCS(SCV_LANG_ERROR_SMARTVEIL), hr, SystemTransitionsExpectedErrors);
		goto Exit;
	}

	hr = SharedSurf->QueryInterface(__uuidof(IDXGIKeyedMutex), reinterpret_cast<void**>(&KeyMutex));
	if (FAILED(hr))
	{
		Ret = ProcessFailure(nullptr, RCS(SCV_LANG_ERROR_KEYEDMUTEX_GET), RCS(SCV_LANG_ERROR_SMARTVEIL), hr);
		goto Exit;
	}

	// Make duplication manager
	Ret = DuplMgr.InitDupl(TData->DxRes.Device, TData->Output);
	if (Ret != DUPL_RETURN_SUCCESS)
	{
		goto Exit;
	}

	// Get output description
	DXGI_OUTPUT_DESC DesktopDesc;
	RtlZeroMemory(&DesktopDesc, sizeof(DXGI_OUTPUT_DESC));
	DuplMgr.GetOutputDesc(&DesktopDesc);

	// Main duplication loop
	bool WaitToProcessCurrentFrame = false;
	FRAME_DATA CurrentData;

	while ((WaitForSingleObjectEx(TData->TerminateThreadsEvent, 0, FALSE) == WAIT_TIMEOUT))
	{
		if (!WaitToProcessCurrentFrame)
		{
			// Get new frame from desktop duplication
			bool TimeOut;
			Ret = DuplMgr.GetFrame(&CurrentData, &TimeOut);
			if (Ret != DUPL_RETURN_SUCCESS)
			{
				// An error occurred getting the next frame drop out of loop which
				// will check if it was expected or not
				break;
			}

			// Check for timeout
			if (TimeOut)
			{
				// No new frame at the moment
				continue;
			}
		}

		// We have a new frame so try and process it
		// Try to acquire keyed mutex in order to access shared surface
		hr = KeyMutex->AcquireSync(0, 1000);
		if (hr == static_cast<HRESULT>(WAIT_TIMEOUT))
		{
			// Can't use shared surface right now, try again later
			WaitToProcessCurrentFrame = true;
			continue;
		}
		else if (FAILED(hr))
		{
			// Generic unknown failure
			Ret = ProcessFailure(TData->DxRes.Device, RCS(SCV_LANG_ERROR_KEYEDMUTEX_ACQUIRE_UNEXPECTED), RCS(SCV_LANG_ERROR_SMARTVEIL), hr, SystemTransitionsExpectedErrors);
			DuplMgr.DoneWithFrame();
			break;
		}

		// We can now process the current frame
		WaitToProcessCurrentFrame = false;

		// Get mouse info
#if 0
		Ret = DuplMgr.GetMouse(TData->PtrInfo, &(CurrentData.FrameInfo), TData->OffsetX, TData->OffsetY);
		if (Ret != DUPL_RETURN_SUCCESS)
		{
			DuplMgr.DoneWithFrame();
			KeyMutex->ReleaseSync(1);
			break;
		}
#endif

		// Process new frame
		Ret = DispMgr.ProcessFrame(&CurrentData, SharedSurf, TData->OffsetX, TData->OffsetY, &DesktopDesc);
		if (Ret != DUPL_RETURN_SUCCESS)
		{
			DuplMgr.DoneWithFrame();
			KeyMutex->ReleaseSync(1);
			break;
		}

		// Release acquired keyed mutex
		hr = KeyMutex->ReleaseSync(1);
		if (FAILED(hr))
		{
			Ret = ProcessFailure(TData->DxRes.Device, RCS(SCV_LANG_ERROR_KEYEDMUTEX_RELEASE_UNEXPECTED), RCS(SCV_LANG_ERROR_SMARTVEIL), hr, SystemTransitionsExpectedErrors);
			DuplMgr.DoneWithFrame();
			break;
		}

		// Release frame back to desktop duplication
		Ret = DuplMgr.DoneWithFrame();
		if (Ret != DUPL_RETURN_SUCCESS)
		{
			break;
		}
	}

Exit:
	if (Ret != DUPL_RETURN_SUCCESS)
	{
		if (Ret == DUPL_RETURN_ERROR_EXPECTED)
		{
			// The system is in a transition state so request the duplication be restarted
			SetEvent(TData->ExpectedErrorEvent);
		}
		else
		{
			// Unexpected error so exit the application
			SetEvent(TData->UnexpectedErrorEvent);
		}
	}

	if (SharedSurf)
	{
		SharedSurf->Release();
		SharedSurf = nullptr;
	}

	if (KeyMutex)
	{
		KeyMutex->Release();
		KeyMutex = nullptr;
	}

	return 0;
}
