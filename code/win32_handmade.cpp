#include <Windows.h>

#define local_persist static
#define global_variable static
#define internal static

global_variable bool Running;
global_variable BITMAPINFO BitmapInfo;
global_variable void *BitmapMemory;
global_variable int BitmapWidth;
global_variable int BitmapHeight;
global_variable int BytesPerPixel = 4;

internal void RenderGradient(int XOffset, int YOffset)
{
	int Pitch = BitmapWidth * BytesPerPixel;
	UINT8 *Row = (UINT8 *)BitmapMemory;
	for(int Y = 0; Y < BitmapHeight; Y++)
	{
		// UINT8 *Pixel = (UINT8 *)Row;
		UINT32 *Pixel = (UINT32 *)Row;
		for(int X = 0; X < BitmapWidth; X++)
		{
			// Pixel in memory: BB GG RR xx
			// *Pixel = (UINT8)(X + XOffset);
			// Pixel++;
			// *Pixel = (UINT8)(Y + YOffset);
			// Pixel++;
			// *Pixel = 0;
			// Pixel++;
			// *Pixel = 0;
			// Pixel++;
			UINT8 Blue = (X + XOffset);
			UINT8 Green = (Y + YOffset);
			*Pixel++ = ((Green << 8) | Blue);
		}
		Row += Pitch;
	}
}

// Resize Device Independent Bitmap section 
internal void Win32ResizeDIBSection(int Width, int Height)
{
	if(BitmapMemory)
	{
		VirtualFree(BitmapMemory, 0, MEM_RELEASE);
	}

	BitmapWidth = Width;
	BitmapHeight = Height;	

	BitmapInfo.bmiHeader.biSize = sizeof(BitmapInfo.bmiHeader);
	BitmapInfo.bmiHeader.biWidth = BitmapWidth;
	BitmapInfo.bmiHeader.biHeight = -BitmapHeight;      // negative for top-down DIB
	BitmapInfo.bmiHeader.biPlanes = 1;
	BitmapInfo.bmiHeader.biBitCount = 32;
	BitmapInfo.bmiHeader.biCompression = BI_RGB;
	BitmapInfo.bmiHeader.biSizeImage = 0;
	BitmapInfo.bmiHeader.biXPelsPerMeter = 0;
	BitmapInfo.bmiHeader.biYPelsPerMeter = 0;
	BitmapInfo.bmiHeader.biClrUsed = 0;
	BitmapInfo.bmiHeader.biClrImportant = 0;

	int BitmapMemorySize = (BitmapWidth * BitmapHeight) * BytesPerPixel;
	BitmapMemory = VirtualAlloc(NULL, BitmapMemorySize, MEM_COMMIT, PAGE_READWRITE);

	// RenderGradient(0, 0);
}

internal void Win32UpdateWindow(HDC DeviceContext, RECT *WindowRect, int X, int Y, int Width, int Height)
{
	int WindowWidth = WindowRect->right - WindowRect->left;
	int WindowHeight = WindowRect->bottom - WindowRect->top;
	StretchDIBits(DeviceContext, 
		0, 0, BitmapWidth, BitmapHeight,
		0, 0, WindowWidth, WindowHeight,
		BitmapMemory,
		&BitmapInfo,
		DIB_RGB_COLORS, SRCCOPY
	  );

}

LRESULT CALLBACK MainWindowCallback(
	HWND Window,
	UINT Message,
	WPARAM wParam,
	LPARAM lParam)
{
	LRESULT Result = 0;
	switch (Message) 
	{
	case WM_SIZE:
	{
		RECT ClientRect;
		GetClientRect(Window, &ClientRect);
		int Width = ClientRect.right - ClientRect.left;
		int Height = ClientRect.bottom - ClientRect.top;
		Win32ResizeDIBSection(Width, Height);
		OutputDebugStringA("WM_SIZE\n");
	}
		break;
	case WM_DESTROY:
	{
		Running = false;
		OutputDebugStringA("WM_DESTROY\n");
	}
		break;
	case WM_CLOSE:
	{
		// PostQuitMessage(0);
		Running = false;
		OutputDebugStringA("WM_CLOSE\n");
	}
		break;
	case WM_ACTIVATEAPP:
		OutputDebugStringA("WM_ACTIVATEAPP\n");
		break;
	case WM_PAINT:
	{
		PAINTSTRUCT Paint;
		HDC DeviceContext = BeginPaint(Window, &Paint);
		int X = Paint.rcPaint.left;
		int Y = Paint.rcPaint.top;
		int Height = Paint.rcPaint.bottom - Paint.rcPaint.top;
		int Width = Paint.rcPaint.right - Paint.rcPaint.left;

		RECT ClientRect;
		GetClientRect(Window, &ClientRect);

		Win32UpdateWindow(DeviceContext, &ClientRect, X, Y, Width, Height);
		EndPaint(Window, &Paint);
	}
		break;
	default:
		//OutputDebugStringA("default\n");
		// For all the messages we don't handle, let the default window handle them.
		Result = DefWindowProc(Window, Message, wParam, lParam);
		break;
	}
	return Result;
}

int CALLBACK WinMain(
	HINSTANCE Instance,
	HINSTANCE PrevInstance,
	LPSTR     CommandLine,
	int       ShowCode
)
{
	// link user32.lib for using this function
	// MessageBox(NULL, (LPCTSTR)"This is a window for handmade", (LPCTSTR)"Handmade", MB_OK | MB_ICONINFORMATION);
	WNDCLASSA windowClass = {};
	windowClass.style = CS_OWNDC | CS_HREDRAW | CS_VREDRAW;
	windowClass.lpfnWndProc = MainWindowCallback;	
	windowClass.hInstance = Instance;
	windowClass.lpszClassName = "HandMadeHeroWindowClass";

	if (RegisterClassA(&windowClass))
	{
		HWND windowHandle = CreateWindowExA(NULL, windowClass.lpszClassName, "Handmade Hero", 
											WS_OVERLAPPEDWINDOW | WS_VISIBLE, CW_USEDEFAULT, CW_USEDEFAULT,
											CW_USEDEFAULT, CW_USEDEFAULT, NULL, NULL, Instance, NULL);
		if(windowHandle)
		{
			Running = true;
			int XOffset = 0;
			int YOffset = 0;
			while(Running)
			{
				MSG Message;
				while(PeekMessage(&Message, 0, 0, 0, PM_REMOVE))
				{
					if(Message.message == WM_QUIT)
					{
						Running = false;
					}
					TranslateMessage(&Message);
					DispatchMessage(&Message);
				}

				RenderGradient(XOffset, YOffset);
				HDC DeviceContext = GetDC(windowHandle);
				RECT ClientRect;
				GetClientRect(windowHandle, &ClientRect);
				int WindowWidth = ClientRect.right - ClientRect.left;
				int WindowHeight = ClientRect.bottom - ClientRect.top;
				Win32UpdateWindow(DeviceContext, &ClientRect, 0, 0, WindowWidth, WindowHeight);
				ReleaseDC(windowHandle, DeviceContext);
				XOffset++;
			}
		}
		else
		{
			// TODO: Logging
		}
	}
	else
	{
		// TODO: Logging
	}
	return 0;
}