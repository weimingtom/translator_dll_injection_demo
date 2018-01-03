#include <windows.h>
#include <string.h>
#include <stdio.h>

static char strDbg[255] = {0};

/*
0040D840 55                   push        ebp
0040D841 8B EC                mov         ebp,esp
0040D843 83 EC 50             sub         esp,50h
0040D846 53                   push        ebx
0040D847 56                   push        esi
0040D848 57                   push        edi

push x 4 & sub x 50h
=> 4 * 4 + 50h
=> 60h
*/

//GetTextMetricsA
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
		MOV EAX, DWORD PTR SS:[ESP + 68H] //arg2
		MOV ARG2, EAX
		MOV EAX, DWORD PTR SS:[ESP + 64H] //arg1
		MOV ARG1, EAX
		MOV EAX, DWORD PTR SS:[ESP + 60H] //EIP
		MOV Next, EAX
	}
	// 12fef8
	sprintf(strDbg, 
		"HookText, hdc:0x%0x, lptm:0x%0x,\n"
		"ESP:0x%0X, Stack args=> 0x%0X, 0x%0X, Next IP => 0x%0X", 
		hdc, lptm, 
		Stack + 0x60, ARG1, ARG2, Next);
	MessageBox(NULL, strDbg, "injectdll", MB_OK);
	return FALSE;
}

void Setup(void)
{
	HMODULE ImageBase;
	PIMAGE_NT_HEADERS PEHeader;
	PIMAGE_IMPORT_DESCRIPTOR PImport;
	FARPROC ProcAddress;
	BOOL Found;
	LPDWORD PRVA_Import;
	DWORD Old = 0;

	ImageBase = GetModuleHandle(NULL);
	PEHeader = (PIMAGE_NT_HEADERS)(((__int64)ImageBase) + ((PIMAGE_DOS_HEADER)ImageBase)->e_lfanew);
	PImport = (PIMAGE_IMPORT_DESCRIPTOR)(PEHeader->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_IMPORT].VirtualAddress + (unsigned long)ImageBase);

	ProcAddress = GetProcAddress(GetModuleHandle("gdi32.dll"), "GetTextMetricsA");
	if(ProcAddress == NULL)
	{
		MessageBox(NULL, "GetProcAddress(TextOutA)", "injectdll", MB_OK);
		return ;
	}
	Found = FALSE;
	while (PImport->Name != 0)
	{
		//MessageBox(NULL, "001", "injectdll", MB_OK);
		PRVA_Import = (LPDWORD)(PImport->FirstThunk + (unsigned long)ImageBase);
		while (*PRVA_Import != 0)
		{
			//MessageBox(NULL, "002", "injectdll", MB_OK);
			if (*((void **)PRVA_Import) == (void *)ProcAddress)
			{
				Found = TRUE;
				if(!VirtualProtect((LPVOID)PRVA_Import, 4, PAGE_READWRITE, &Old))
				{
					MessageBox(NULL, "do- VirtualProtect()", "injectdll", MB_OK);
					return ;
				}
				*((void **)PRVA_Import) = (void *)HookText;
				if(!VirtualProtect((LPVOID)PRVA_Import, 4, Old, &Old))
				{
					MessageBox(NULL, "un- VirtualProtect()", "injectdll", MB_OK);
					return ;
				}
			}
			PRVA_Import++;
		}
		PImport++;
	}
	if (Found == FALSE)
	{
		MessageBox(NULL, "No GetTextMetricsA in the import table. Wrong EXE? Wrong game? Wrong version?", "injectdll", MB_OK);
		return ;
	}
}

BOOL WINAPI DllMain(HINSTANCE hinstDLL, DWORD fdwReason, LPVOID lpReserved)
{
    switch (fdwReason) 
    {
    case DLL_PROCESS_ATTACH:
		//MessageBox(NULL, "[INJECT] Begin attach!\n", "injectdll", MB_OK);
		Setup();
        break;
	
    case DLL_THREAD_ATTACH:
        break;

    case DLL_THREAD_DETACH:
        break;

    case DLL_PROCESS_DETACH:
        break;
    }
    return TRUE;
}
