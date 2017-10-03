/**
* @file	Main.cpp
*/

#include "GLFWEW.h"
#include <iostream> 

/**
* GLFW からのエラー報告を処理する.
*
* @param error エラー番号.
* @param desc  エラーの内容.
*/
void ErrorCallback(int error, const char* desc) {
	std::cerr << "ERROR: " << desc << std::endl;
}

/// エントリーポイント
int main() {

	// エラーコールバック
	glfwSetErrorCallback(ErrorCallback);

	// GLFWEWの初期化
	GLFWEW::Window& window = GLFWEW::Window::Instance();
	if (!window.Init(800, 600, "OpenGL Tutorial")) {
		return 1;
	}

	// 描画デバイス、対応バージョン出力
	const GLubyte* renderer = glGetString(GL_RENDERER);
	std::cout << "Renderer: " << renderer << std::endl;
	const GLubyte* version = glGetString(GL_VERSION);
	std::cout << "Version: " << version << std::endl;

	// メインループ
	while (!window.ShouldClose()) {
		glClearColor(0.1f, 0.3f, 0.5f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		window.SwapBuffers();
	}

	// 終了処理
	glfwTerminate();

	return 0;
} 

/*
#include <Windows.h>
#include <GL/GL.h>
#pragma comment (lib, "opengl32.lib")

LRESULT CALLBACK WndProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam);

HGLRC oglContext;

// エントリーポイント
int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nShowCmd) {

	WNDCLASS wc = {};
	wc.lpfnWndProc = WndProc;
	wc.hInstance = hInstance;
	wc.hbrBackground = reinterpret_cast<HBRUSH>(COLOR_BACKGROUND);
	wc.lpszClassName = L"OpenGLVersionCheck";
	wc.style = CS_OWNDC;
	if (!RegisterClass(&wc)) {
		return 1;
	}
	CreateWindow(
		wc.lpszClassName, L"OpenGL Version Check", WS_OVERLAPPEDWINDOW | WS_VISIBLE,
		0, 0, 640, 480, 0, 0, hInstance, 0
	);

	MSG msg;
	while (GetMessage(&msg, nullptr, 0, 0) > 0) {
		DispatchMessage(&msg);
	}
	return 0;
}


// ウィンドウプロシージャ
LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam) {
	switch (message) {
	case WM_CREATE: {
		PIXELFORMATDESCRIPTOR pfd = {};
		pfd.nSize = sizeof(PIXELFORMATDESCRIPTOR);
		pfd.nVersion = 1;
		pfd.dwFlags = PFD_DRAW_TO_WINDOW | PFD_SUPPORT_OPENGL | PFD_DOUBLEBUFFER;
		pfd.iPixelType = PFD_TYPE_RGBA;
		pfd.cColorBits = 32;
		pfd.cDepthBits = 24;
		pfd.cStencilBits = 8;
		pfd.iLayerType = PFD_MAIN_PLANE;

		HDC hDC = GetDC(hWnd);
		int pixelFormat = ChoosePixelFormat(hDC, &pfd);
		SetPixelFormat(hDC, pixelFormat, &pfd);
		oglContext = wglCreateContext(hDC);
		wglMakeCurrent(hDC, oglContext);
		ReleaseDC(hWnd, hDC);
		MessageBoxA(0, reinterpret_cast<const char*>(glGetString(GL_VERSION)), "OPENGL VERSION", 0);
		break;
	}
	case WM_DESTROY:
		wglDeleteContext(oglContext);
		PostQuitMessage(0);
		break;
	default:
		return DefWindowProc(hWnd, message, wParam, lParam);
	}
	return 0;
}
*/

