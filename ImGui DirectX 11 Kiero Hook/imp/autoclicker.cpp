#include <Windows.h>
#include "autoclicker.h"

void autoclicker::click(HWND minecraftHWND)
{
	PostMessage(minecraftHWND, WM_LBUTTONDOWN, 1, 0);
	PostMessage(minecraftHWND, WM_LBUTTONUP, 0, 0);
}