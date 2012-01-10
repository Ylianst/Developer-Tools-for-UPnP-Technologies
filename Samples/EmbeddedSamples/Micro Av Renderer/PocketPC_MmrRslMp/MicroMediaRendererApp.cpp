/*   
Copyright 2006 - 2011 Intel Corporation

Licensed under the Apache License, Version 2.0 (the "License");
you may not use this file except in compliance with the License.
You may obtain a copy of the License at

   http://www.apache.org/licenses/LICENSE-2.0

Unless required by applicable law or agreed to in writing, software
distributed under the License is distributed on an "AS IS" BASIS,
WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
See the License for the specific language governing permissions and
limitations under the License.
*/

#include "stdafx.h"
#include "resource.h"
#include "initguid.h"
#include "CMicroMediaRenderer.h"
#include <commctrl.h>

// ATL bookkeeping:
CComModule _Module;
BEGIN_OBJECT_MAP(ObjectMap)
END_OBJECT_MAP()

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE, LPTSTR, int)
{
	// If the program is already running, bring its window to the foreground.
	HWND hwnd = FindWindow(NULL, TEXT("Intel Media Renderer"));	
	if (hwnd) 
	{
		SetForegroundWindow(hwnd);
		return 0;
	} 

	// Initialize COM.
	if (FAILED(CoInitializeEx(NULL, COINIT_MULTITHREADED))) 
	{
		return 0;
	}

	// Initialize ATL.
	if (FAILED(_Module.Init(ObjectMap, hInstance)))
	{
		CoUninitialize();
		return 0;
	}

	// Create the main window.
	CComMicroMediaRenderer frame;
	RECT rcPos = { 0, 26, 240, 294 };
	frame.Create(NULL, rcPos, TEXT("Intel Media Renderer"), 0, 0, 0);

	// Main message loop:
	MSG msg;
	while (GetMessage(&msg, 0, 0, 0))
	{
		TranslateMessage(&msg);
		DispatchMessage(&msg);
	}

	// Clean up.
	_Module.Term();
	CoUninitialize();
	return 0;
}
