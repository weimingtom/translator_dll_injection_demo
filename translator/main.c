#include <windows.h>
#include <stdio.h>
#include <string.h>

//#define DLL_FILE_NAME "translation.dll"
#define DLL_FILE_NAME "injectdll.dll"

#define EXE_FILE_NAME "test.exe"
#define TITLE_NAME "translator"

static char dbg[1000] = {0};

int FileExists(const char *filename)
{
	WIN32_FIND_DATA FindFileData;
	HANDLE hFind;
	hFind = FindFirstFile(filename, &FindFileData);
	if (hFind == INVALID_HANDLE_VALUE) 
	{
		return 0;
	}
	else
	{
		FindClose(hFind);
		if (!(FindFileData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY))
		{
			return 1;
		}
	}
	return 0;
}

int main(int argc, char **argv)
{
	char CDPath[255] = {0};
	char DLLPath[255] = {0};
	char EXEPath[255] = {0};
	char *pos;
    STARTUPINFO si;
    PROCESS_INFORMATION pi;
	HANDLE hProcess;
	LPVOID PLib;
	SIZE_T I;
	FARPROC LL;
	HANDLE hT;

	sprintf(dbg, "[DEBUG] argv[0] : %s\n", argv[0]);	
	OutputDebugString(dbg);

	pos = strrchr(argv[0], '\\');
	strncpy(CDPath, argv[0], pos - argv[0]);

	sprintf(dbg, "[DEBUG] CDPath : %s\n", CDPath);	
	OutputDebugString(dbg);

	sprintf(DLLPath, "%s\\%s", CDPath, DLL_FILE_NAME);

	sprintf(dbg, "[DEBUG] DLLPath : %s\n", DLLPath);	
	OutputDebugString(dbg);

	if (!FileExists(DLLPath))
	{
		sprintf(dbg, "[ERROR] DLLPath %s is not existed!\n", DLLPath);	
		OutputDebugString(dbg);
		return 1;
	}

	sprintf(EXEPath, "%s\\%s", CDPath, EXE_FILE_NAME);

	sprintf(dbg, "[DEBUG] EXEPath : %s\n", EXEPath);	
	OutputDebugString(dbg);

	if (!FileExists(EXEPath))
	{
		sprintf(dbg, "[ERROR] EXEPath %s is not existed!\n", EXEPath);	
		OutputDebugString(dbg);
		return 2;
	}

    ZeroMemory(&si, sizeof(si));
    si.cb = sizeof(si);
    ZeroMemory(&pi, sizeof(pi));

    if(!CreateProcess(EXEPath, "", NULL, NULL, FALSE,
		CREATE_SUSPENDED, NULL, CDPath, &si, &pi))
    {
        MessageBox(NULL, "Cannot create a process", TITLE_NAME, MB_OK);
        return 3;
    }

	hProcess = OpenProcess(PROCESS_CREATE_THREAD | PROCESS_VM_OPERATION | PROCESS_VM_WRITE, 
		FALSE, pi.dwProcessId);
	if (hProcess == NULL)
	{
		MessageBox(NULL, "Cannot open a process", TITLE_NAME, MB_OK);
		return 4;
	}
	
	PLib = VirtualAllocEx(hProcess, NULL, sizeof(DLLPath), MEM_COMMIT, PAGE_READWRITE);
	if (PLib == NULL)
	{
		MessageBox(NULL, "Cannot call VirtualAllocEx", TITLE_NAME, MB_OK);
		return 5;
	}
	if (!WriteProcessMemory(hProcess, PLib, DLLPath, sizeof(DLLPath), &I))
	{
		MessageBox(NULL, "Cannot write to the process", TITLE_NAME, MB_OK);
		return 6;
	}
	LL = GetProcAddress(GetModuleHandle("kernel32.dll"), "LoadLibraryA");
	if (LL == NULL)
	{
		MessageBox(NULL, "Cannot GetProcAddress(LoadLibraryA)", TITLE_NAME, MB_OK);
		return 6;
	}
	hT = CreateRemoteThread(hProcess, NULL, 0, (LPTHREAD_START_ROUTINE)LL, PLib, 0, &I);
	if (hT == NULL)
	{
		MessageBox(NULL, "Cannot create a thread", TITLE_NAME, MB_OK);
		return 6;
	}

    WaitForSingleObject(hT, INFINITE);

	if (ResumeThread(pi.hThread) == (DWORD)(-1))
	{
		MessageBox(NULL, "Cannot run the thread", TITLE_NAME, MB_OK);
		return 7;
	}

	VirtualFreeEx(hProcess, PLib, sizeof(DLLPath), MEM_DECOMMIT);
	CloseHandle(hT);
    CloseHandle(hProcess);

	return 0;
}
