#include "ImagePresentation.h"

#include "resource.h"

#include "DYNAMIC_WAIT.h"
#include "CommonTypes.h"
#include "DisplayManager.h"
#include "DuplicationManager.h"
#include "ThreadManager.h"

#include "utils.cpp"
#include "LANGUAGE_MANAGER.h"

#ifdef _DEBUG
#include "Settings.h"
#include "Common.h"
#endif

//TODOs:
//add system for letting caller know if the worker thread dies
//gpu usage spikes to 100% for one second when the thread closed


DWORD WINAPI WorkerThread(void* Param) {
	//TODO(fran): should I use it straight instead of copying it?
	WORKER_THREAD_INIT* data = (WORKER_THREAD_INIT*)Param; //TODO(fran): check this copies everything right

	THREADMANAGER thread_mgr;

	//Error events
	bool UnexpectedErrorEvent = false;
	bool ExpectedErrorEvent = false;
	bool TerminateThreadsEvent = false;

	UINT OutputCount;
	RECT DeskBounds;
	INT SingleOutput= -1;

	DYNAMIC_WAIT DynamicWait;
	bool FirstTime = true;
	DUPL_RETURN Ret;

	const DWORD mut_ret = WaitForSingleObject(data->worker_finished_mutex, INFINITE);//Mutex that will be released when the thread finishes
	Assert(mut_ret == WAIT_OBJECT_0);

	//Basic update rate checking system https://stackoverflow.com/questions/1739259/how-to-use-queryperformancecounter
#ifdef _DEBUG
	double PCFreq = GetPCFrequency();
	__int64 CounterStart = 0;
	double CounterVal;
#endif
	while (!data->terminate) {

#ifdef _DEBUG
		StartCounter(CounterStart);
#endif
		if (UnexpectedErrorEvent) // Unexpected error occurred so exit the application
		{
			TerminateThreadsEvent = true;
			thread_mgr.WaitForThreadTermination();
			//data->thread_mgr.Clean();
			data->output_mgr.CleanRefs();
			ReleaseMutex(data->worker_finished_mutex);
			ShowError(RS(SCV_LANG_ERROR_UNEXPECTED) + L"\n" + RS(SCV_LANG_ERROR_CLOSING_APP), RS(SCV_LANG_ERROR_SMARTVEIL));
			return 1;
		}
		else if (FirstTime || ExpectedErrorEvent) //On first time through the loop initialization happens, otherwise on error we clean up
		{
			if (!FirstTime) //Cleanup on expected error
			{
				// Terminate other threads
				TerminateThreadsEvent=true; //TODO(fran): this is the only place this gets set -> pointless to have mutex capability
				thread_mgr.WaitForThreadTermination(); //also all the other threads are killed, so NO races

				TerminateThreadsEvent=false; //This is the only place that resets this two -> pointless to have mutex capability,
				ExpectedErrorEvent = false;

				thread_mgr.Clean();
				data->output_mgr.CleanRefs();

				// As we have encountered an error due to a system transition we wait before trying again, using this dynamic wait
				// the wait periods will get progressively long to avoid wasting too much system resource if this state lasts a long time
				DynamicWait.Wait();
			}
			else
			{
				FirstTime = false; // First time through the loop so nothing to clean up
			}

			// Re-initialize
			Ret = data->output_mgr.InitOutput(data->output_wnd, SingleOutput, &OutputCount, &DeskBounds);
			if (Ret == DUPL_RETURN_SUCCESS)
			{
				HANDLE SharedHandle = data->output_mgr.GetSharedHandle();
				if (SharedHandle)
					thread_mgr.Initialize(SingleOutput, OutputCount, &UnexpectedErrorEvent, &ExpectedErrorEvent,
														&TerminateThreadsEvent, SharedHandle, &DeskBounds);
				else
					ProcessFailure(nullptr, RCS(SCV_LANG_ERROR_SHARED_SURF_HANDLE_GET), RCS(SCV_LANG_ERROR_SMARTVEIL), E_UNEXPECTED);
			}
		}

		//TODO(fran): could it be that some of the flickering is produced cause we havent yet updated our current surface?
		// if that were happening then we would be presenting the one from two frames ago

		// Draw to our window (veil)
		Ret = data->output_mgr.UpdateApplicationWindow();//data->thread_mgr.GetPointerInfo());

#ifdef _DEBUG
		CounterVal = GetCounter(CounterStart, PCFreq);
		PostMessage(KNOWN_WINDOWS.settings, SCV_SETTINGS_UPDATE_COUNTER, (WPARAM)(float)CounterVal, 0);
#endif

		if (Ret != DUPL_RETURN_SUCCESS) // Check for errors
		{
			if (Ret == DUPL_RETURN_ERROR_EXPECTED)
			{
				ExpectedErrorEvent=true; // Some type of system transition is occurring so retry
				//OPTIMIZE(fran): we can probably join this with parts of the if from above, reducing at least one if
			}
			else
			{
				// Unexpected error so exit
				TerminateThreadsEvent = true;
				thread_mgr.WaitForThreadTermination();
				//data->thread_mgr.Clean();
				data->output_mgr.CleanRefs();
				ReleaseMutex(data->worker_finished_mutex);
				ShowError(RS(SCV_LANG_ERROR_UNEXPECTED) + L"\n" + RS(SCV_LANG_ERROR_CLOSING_APP), RS(SCV_LANG_ERROR_SMARTVEIL)); //TODO(fran): closing app part
				return 1;
			}
		}

#if LIMIT_UPDATE
		if (CounterVal < TargetMs)
			Sleep(max(TargetMs - CounterVal, 0));
#endif
	}

	//Cleanup before exiting
	TerminateThreadsEvent = true;
	thread_mgr.WaitForThreadTermination(); //TODO(fran): I dont know if all this that I put on every function exit are really necessary
	//data->thread_mgr.Clean();
	data->output_mgr.CleanRefs();
	ReleaseMutex(data->worker_finished_mutex);

	//INFO(fran): go to CommonTypes.h to define-undef (turn on-off) directx debugging (_DX_DEBUG_LAYER)
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
		*TData->ExpectedErrorEvent=true;
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

	while (!*TData->TerminateThreadsEvent)
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
		hr = KeyMutex->AcquireSync(0, 1000);//TODO(fran): I'd put 100
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
#ifdef _DEBUG
		//INFO(fran): mouse movement alone does NOT increase any of this counts
		/*char buf[50];
		sprintf(buf, "Dirties: %u\t Moves: %u\n", CurrentData.DirtyCount, CurrentData.MoveCount);
		OutputDebugStringA(buf);*/
		//if (CurrentData.DirtyCount == 1) //TODO(fran): this dont work, we need more info
		//	TEST_draw = false;
		//else TEST_draw = true;
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
			*TData->ExpectedErrorEvent=true;
		}
		else
		{
			// Unexpected error so exit the application
			*TData->UnexpectedErrorEvent=true;
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
