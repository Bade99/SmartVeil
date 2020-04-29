#pragma once

#include <Windows.h>

#include "ThreadManager.h"
#include "OutputManager.h"

//Limit veil "fps"
#define LIMIT_UPDATE 0
#if LIMIT_UPDATE 
const double TargetFPS = 100.0;
const double TargetMs = 1000.0 / TargetFPS;//TODO(fran): do we leave this as a global?
#endif

struct WORKER_THREAD_EVENTS {
	HANDLE UnexpectedErrorEvent = nullptr;
	HANDLE ExpectedErrorEvent = nullptr;
	HANDLE TerminateThreadsEvent = nullptr;
};

struct WORKER_THREAD_INIT { //TODO(fran): probably some of this are unnecessary
	HWND output_wnd=nullptr;
	HANDLE output_wnd_ready_mutex=nullptr;//(start_mutex) //indicates when the output window is ready to show image data //TODO(fran): check if we can avoid using this
	HANDLE worker_finished_mutex = nullptr;//(thread_finished_mutex) //signaled when the worker thread is done with its work and all cleaned up
	HANDLE next_frame_mutex = nullptr;//(process_next_frame_mutex) //signaled when new frames are required
	BOOL terminate = FALSE;//(ProgramFinished) //the thread has to terminate
	WORKER_THREAD_EVENTS events;
	THREADMANAGER thread_mgr;
	OUTPUTMANAGER output_mgr;
};

/// <summary>
/// Everything related to updating the contents of the veil takes place here
/// </summary>
/// <param name="Param">The HWND of the Veil, aka the window that will darken the screen</param>
/// <returns></returns>
DWORD WINAPI WorkerThread(void* Param);

/// <summary>
/// Entry point for new duplication threads
/// </summary>
/// <param name="Param">Pointer to a THREAD_DATA structure</param>
/// <returns></returns>
DWORD WINAPI DDProc(_In_ void* Param);
