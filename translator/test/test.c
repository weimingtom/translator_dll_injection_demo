#include <windows.h>
#include <stdio.h>
#include <string.h>

int main()
{
	TEXTMETRIC tm;
	HDC hdc;
	char str[255] = {0};
	hdc = CreateDCA(TEXT("DISPLAY"),NULL,NULL,NULL);
	sprintf(str, "0x%0X, 0x%0X", hdc, &tm);
	MessageBox(NULL, str, "test", MB_OK);
	GetTextMetricsA(hdc, &tm);
	return 0;
}
