#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif

#include "Windows.h"
#include "stdio.h"

HHOOK hook;

LRESULT CALLBACK LowLevelMouseProc(_In_ int nCode, _In_ WPARAM wParam, _In_ LPARAM lParam) {
	MSLLHOOKSTRUCT *data = (MSLLHOOKSTRUCT *)lParam;
	LRESULT res;
	POINT p;
	p.x = data->pt.x;
	p.y = data->pt.y;
	res = CallNextHookEx(hook, nCode, wParam, lParam);
	switch (wParam)
	{
		case WM_LBUTTONDOWN:
		{
			printf("DLL: M DOWN (%ld,%ld)\n", p.x, p.y);
			break;
		}
		case WM_LBUTTONUP:
		{
			printf("DLL: M UP (%ld,%ld)\n", p.x, p.y);
			break;
		}
		// case WM_MOUSEMOVE:
		// {
		// 	printf("M MOVE (%ld,%ld)\n", p.x, p.y);
		// 	break;
		// }
		
		default:
			break;
	}
	return res;
}

__declspec(dllexport) BOOL install() {
	hook = SetWindowsHookExA(WH_MOUSE_LL, LowLevelMouseProc, GetModuleHandleA(NULL), 0);
	return hook != NULL;
}

__declspec(dllexport) BOOL uninstall() {
	HHOOK hook_ = hook;
	BOOL unhooked = UnhookWindowsHookEx(hook_);
	if (unhooked) {
		hook = NULL;
	}
	return unhooked;
}
