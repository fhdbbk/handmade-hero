#include <Windows.h>

#define local_persist static
#define global_variable static
#define internal static

global_variable bool Running;
// global_variable BITMAPINFO BitmapInfo;
// global_variable void *BitmapMemory;
// global_variable int BitmapWidth;
// global_variable int BitmapHeight;
// global_variable int BytesPerPixel = 4;

struct win32_offscreen_buffer
{
	BITMAPINFO Info;
	void *Memory;
	int Width;
	int Height;
	int Pitch;
	int BytesPerPixel;
};

global_variable win32_offscreen_buffer GlobalBackBuffer;

struct win32_window_dimension
{
	int Width;
	int Height;
};

win32_window_dimension Win32GetWindowDimension(HWND Window)
{
	win32_window_dimension Result;
	RECT ClientRect;
	GetClientRect(Window, &ClientRect);
	Result.Width = ClientRect.right - ClientRect.left;
	Result.Height = ClientRect.bottom - ClientRect.top;
	return Result;
}

internal void RenderGradient(win32_offscreen_buffer Buffer, int XOffset, int YOffset)
{
	UINT8 *Row = (UINT8 *)Buffer.Memory;
	for(int Y = 0; Y < Buffer.Height; Y++)
	{
		// UINT8 *Pixel = (UINT8 *)Row;
		UINT32 *Pixel = (UINT32 *)Row;
		for(int X = 0; X < Buffer.Width; X++)
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
		Row += Buffer.Pitch;
	}
}

// Resize Device Independent Bitmap section 
internal void Win32ResizeDIBSection(win32_offscreen_buffer *Buffer, int Width, int Height)
{
	if(Buffer->Memory)
	{
		VirtualFree(Buffer->Memory, 0, MEM_RELEASE);
	}

	Buffer->Width = Width;
	Buffer->Height = Height;
	Buffer->BytesPerPixel = 4;	

	Buffer->Info.bmiHeader.biSize = sizeof(Buffer->Info.bmiHeader);
	Buffer->Info.bmiHeader.biWidth = Buffer->Width;
	Buffer->Info.bmiHeader.biHeight = -Buffer->Height;      // negative for top-down DIB
	Buffer->Info.bmiHeader.biPlanes = 1;
	Buffer->Info.bmiHeader.biBitCount = 32;
	Buffer->Info.bmiHeader.biCompression = BI_RGB;
	Buffer->Info.bmiHeader.biSizeImage = 0;
	Buffer->Info.bmiHeader.biXPelsPerMeter = 0;
	Buffer->Info.bmiHeader.biYPelsPerMeter = 0;
	Buffer->Info.bmiHeader.biClrUsed = 0;
	Buffer->Info.bmiHeader.biClrImportant = 0;

	int BitmapMemorySize = (Buffer->Width * Buffer->Height) * Buffer->BytesPerPixel;
	Buffer->Memory = VirtualAlloc(NULL, BitmapMemorySize, MEM_COMMIT, PAGE_READWRITE);
	Buffer->Pitch = Width * Buffer->BytesPerPixel;

	// RenderGradient(0, 0);
}

internal void Win32UpdateWindow(win32_offscreen_buffer Buffer, HDC DeviceContext, int WindowWidth, int WindowHeight)
{
	StretchDIBits(DeviceContext, 
		0, 0, Buffer.Width, Buffer.Height,
		0, 0, WindowWidth, WindowHeight,
		Buffer.Memory,
		&Buffer.Info,
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
		win32_window_dimension Dimension = Win32GetWindowDimension(Window);
		Win32ResizeDIBSection(&GlobalBackBuffer, Dimension.Width, Dimension.Height);
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

		win32_window_dimension Dimension = Win32GetWindowDimension(Window);

		Win32UpdateWindow(GlobalBackBuffer, DeviceContext, Dimension.Width, Dimension.Height);
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
	windowClass.style = CS_HREDRAW | CS_VREDRAW;
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

				RenderGradient(GlobalBackBuffer, XOffset, YOffset);
				HDC DeviceContext = GetDC(windowHandle);
				win32_window_dimension Dimension = Win32GetWindowDimension(windowHandle);

				Win32UpdateWindow(GlobalBackBuffer, DeviceContext, Dimension.Width, Dimension.Height);
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