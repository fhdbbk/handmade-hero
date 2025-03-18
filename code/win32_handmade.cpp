#include <Windows.h>

#define local_persist static
#define global_variable static
#define internal static

global_variable bool Running;

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
		OutputDebugStringA("WM_SIZE\n");
		break;
	case WM_DESTROY:
		Running = false;
		OutputDebugStringA("WM_DESTROY\n");
		break;
	case WM_CLOSE:
		// PostQuitMessage(0);
		Running = false;
		OutputDebugStringA("WM_CLOSE\n");
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
		static DWORD Operation = WHITENESS;
		PatBlt(DeviceContext, X, Y, Width, Height, Operation);
		if (Operation == WHITENESS)
		{
			Operation = BLACKNESS;
		}
		else
		{
			Operation = WHITENESS;
		}
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
			while(Running)
			{
				MSG Message;
				BOOL MessageResult = GetMessage(&Message, NULL, NULL, NULL);
				// If the function retrieves a message other than WM_QUIT, the return value is nonzero.
				if(MessageResult > 0)
				{
					TranslateMessage(&Message);
					DispatchMessage(&Message);
				}
				else
				{
					break;
				}
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