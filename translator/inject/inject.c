#include <windows.h>
#include <stdio.h>
#include <string.h>

static char strDbg[255] = {0};

BOOL WINAPI HookText(HDC hdc, LPTEXTMETRICA lptm)
{
	// 12fe98(temp) 12fe9C(no temp)
	//DWORD temp = 1;
	DWORD Stack = 0;
	DWORD ARG1 = 0;
	PWCHAR ARG2 = 0;
	PWORD Next = 0;
	// + 4CH + 4
	__asm {
		MOV Stack, ESP
		MOV EAX, DWORD PTR SS:[ESP + 68H]
		MOV ARG2, EAX
		MOV EAX, DWORD PTR SS:[ESP + 64H]
		MOV ARG1, EAX
		MOV EAX, DWORD PTR SS:[ESP + 60H]
		MOV Next, EAX
	}
	// 12fef8
	sprintf(strDbg, 
		"HookText, hdc:0x%0x, lptm:0x%0x,\n"
		"ESP:0x%0X, Stack => 0x%0X, 0x%0X, Next => 0x%0X", 
		hdc, lptm, 
		Stack + 0x60, ARG1, ARG2, Next);
	MessageBox(NULL, strDbg, "injectdll", MB_OK);
	return FALSE;
}

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow)
{
	MessageBox(NULL, "[INJECT] hello, world!\n", "inject", MB_OK);
	return 0;
}
