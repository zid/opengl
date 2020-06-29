#include <windows.h>
#include <stdio.h>
#include <stdlib.h>
#include <stddef.h>
#include "gl.h"
#include "engine.h"

#define WIDTH 1600
#define HEIGHT 900

#define WGL_SAMPLE_BUFFERS_ARB 0x2041
#define WGL_SAMPLES_ARB        0x2042
#define WGL_DRAW_TO_WINDOW_ARB 0x2001
#define WGL_ACCELERATION_ARB   0x2003
#define WGL_FULL_ACCELERATION_ARB         0x2027
#define WGL_SUPPORT_OPENGL_ARB            0x2010
#define WGL_PIXEL_TYPE_ARB                0x2013
#define WGL_COLOR_BITS_ARB                0x2014
#define WGL_ALPHA_BITS_ARB                0x201B
#define WGL_DEPTH_BITS_ARB                0x2022
#define WGL_TYPE_RGBA_ARB                 0x202B
#define WGL_CONTEXT_MAJOR_VERSION_ARB     0x2091
#define WGL_CONTEXT_MINOR_VERSION_ARB     0x2092
#define WGL_CONTEXT_PROFILE_MASK_ARB      0x9126
#define WGL_CONTEXT_CORE_PROFILE_BIT_ARB  0x00000001

static void gl_context_init(HWND hwnd)
{
	HDC h;
	HGLRC tgl, gl;
	HWND fake_window;
	unsigned int nformat;
	int pf;

	WNDCLASS wc = {
		.style = CS_OWNDC,
		.lpfnWndProc = DefWindowProcA,
		.hInstance = GetModuleHandle(0),
		.lpszClassName = "dummy"
	};

	PIXELFORMATDESCRIPTOR pfd = {
		sizeof(PIXELFORMATDESCRIPTOR),
		1,
		PFD_DRAW_TO_WINDOW | PFD_SUPPORT_OPENGL | PFD_DOUBLEBUFFER,
		PFD_TYPE_RGBA,
		32,
		0, 0, 0,
		0, 0, 0,
		0, 0,
		0, 0, 0, 0, 0,
		24, 0, 0, 0, 0, 0, 0, 0
	};

	const int attrib[] = {
		WGL_CONTEXT_MAJOR_VERSION_ARB, 4,
		WGL_CONTEXT_MINOR_VERSION_ARB, 5,
		WGL_CONTEXT_PROFILE_MASK_ARB, WGL_CONTEXT_CORE_PROFILE_BIT_ARB,
		0, 0
	};

	int pixel_format[] =
	{
		WGL_DRAW_TO_WINDOW_ARB, 1,
		WGL_ACCELERATION_ARB, WGL_FULL_ACCELERATION_ARB,
		WGL_SUPPORT_OPENGL_ARB, 1,
		WGL_COLOR_BITS_ARB, 32,
		WGL_DEPTH_BITS_ARB, 24,
		WGL_ALPHA_BITS_ARB, 8,
		WGL_PIXEL_TYPE_ARB, WGL_TYPE_RGBA_ARB,
		WGL_SAMPLE_BUFFERS_ARB, 1,
		WGL_SAMPLES_ARB, 16,
		0, 0
	};

	/* Create a fake window */
	RegisterClass(&wc);
	fake_window = CreateWindowEx(0, wc.lpszClassName, "dummy window", 0, 0, 0, 0, 0, 0, 0, wc.hInstance, 0);
	
	/* Create a fake DC from fake window */
	h = GetDC(fake_window);
	nformat = ChoosePixelFormat(h, &pfd);

	/* Set a similar pixel format to what we want, and create a fake opengl context */
	SetPixelFormat(h, nformat, &pfd);
	tgl = wglCreateContext(h);
	wglMakeCurrent(h, tgl);

	/* Using the fake context, get the function pointers for the extended functions */
	wglCreateContextAttribsARB = (void *)wglGetProcAddress("wglCreateContextAttribsARB");
	wglChoosePixelFormatARB = (void *)wglGetProcAddress("wglChoosePixelFormatARB");

	/* Destroy the fake window/dc/context */
	wglMakeCurrent(h, 0);
	wglDeleteContext(tgl);
	ReleaseDC(fake_window, h);
	DestroyWindow(fake_window);
	UnregisterClass(wc.lpszClassName, wc.hInstance);

	/* Now the DC of the real window */
	h = GetDC(hwnd);

	/* Set the real pixel format */
	wglChoosePixelFormatARB(h, pixel_format, 0, 1, &pf, &nformat);
	if(!nformat)
	{
		fprintf(stderr, "Fatal: Pixel format unsupported");
		exit(0);
	}
	
	DescribePixelFormat(h, pf, sizeof pfd, &pfd);
	if(!SetPixelFormat(h, pf, &pfd))
	{
		fprintf(stderr, "Fatal: Could not set pixel format");
		exit(0);
	}

	/* Create the real context */
	gl = wglCreateContextAttribsARB(h, 0, attrib);
	if(!gl)
	{
		fprintf(stderr, "Fatal: Couldn't create OpenGL context");
		exit(0);
	}

	wglMakeCurrent(NULL, NULL);
	wglDeleteContext(tgl);
	wglMakeCurrent(h, gl);
	ReleaseDC(hwnd, h);
}

LRESULT APIENTRY mainproc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	switch(uMsg)
	{
		case WM_KEYDOWN:
		if(wParam == VK_F5)
			engine_reload();
		if(wParam != VK_ESCAPE)
				break;
		case WM_QUIT:
		case WM_DESTROY:
			PostQuitMessage(0);
		break;
	}

	return DefWindowProc(hwnd, uMsg, wParam, lParam);
}

#ifdef NOCONSOLE
int APIENTRY WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance,
	LPSTR lpCmdLine, int nCmdShow)
#else
int main(void)
#endif
{
	WNDCLASSEX wcx;
	MSG msg;
	HWND hwnd;
	HDC hdc;
	RECT r;

#ifdef NOCONSOLE
	(void)hPrevInstance;
	(void)nCmdShow;
	(void)lpCmdLine;
#endif

	wcx.cbSize        = sizeof(WNDCLASSEX);
	wcx.style         = 0;
	wcx.lpfnWndProc   = mainproc;
	wcx.cbClsExtra    = 0;
	wcx.cbWndExtra    = 0;
	wcx.hInstance     = 0;
	wcx.hIcon         = NULL;
	wcx.hCursor       = LoadCursor(NULL, IDC_ARROW);
	wcx.hbrBackground = GetSysColorBrush(COLOR_3DLIGHT);
	wcx.lpszMenuName  = NULL;
	wcx.lpszClassName =  "gl";
	wcx.hIconSm       = NULL;

	if(!RegisterClassEx(&wcx))
		return 0;

	r.top = 0;
	r.left = 0;
	r.right = WIDTH;
	r.bottom = HEIGHT;

	AdjustWindowRectEx(&r, WS_SYSMENU | WS_CLIPCHILDREN, 0, WS_EX_WINDOWEDGE);

	hwnd = CreateWindowEx(
		WS_EX_WINDOWEDGE, "gl", "gl",
		WS_CLIPCHILDREN | WS_SYSMENU,
		CW_USEDEFAULT, CW_USEDEFAULT, r.right, r.bottom,
		NULL, NULL, 0 /* hInstance */, NULL
	);

	gl_context_init(hwnd);
	gl_init_procs();
	engine_init();

	ShowWindow(hwnd, SW_SHOW);
	hdc = GetWindowDC(hwnd);

	while(1)
	{
		if (PeekMessage(&msg, 0, 0, 0, PM_REMOVE))
		{
			if(msg.message == WM_QUIT)
				break;
			TranslateMessage(&msg);
			DispatchMessage(&msg);
			continue;
		}

		engine();

		SwapBuffers(hdc);
	}

	return msg.wParam;
}
