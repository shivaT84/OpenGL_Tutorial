/**
* @file	Main.cpp
*/

#include "GLFWEW.h"
#include <iostream> 

/**
* GLFW ����̃G���[�񍐂���������.
*
* @param error �G���[�ԍ�.
* @param desc  �G���[�̓��e.
*/
void ErrorCallback(int error, const char* desc) {
	std::cerr << "ERROR: " << desc << std::endl;
}

/// �G���g���[�|�C���g
int main() {

	// �G���[�R�[���o�b�N
	glfwSetErrorCallback(ErrorCallback);

	// GLFWEW�̏�����
	GLFWEW::Window& window = GLFWEW::Window::Instance();
	if (!window.Init(800, 600, "OpenGL Tutorial")) {
		return 1;
	}

	// �`��f�o�C�X�A�Ή��o�[�W�����o��
	const GLubyte* renderer = glGetString(GL_RENDERER);
	std::cout << "Renderer: " << renderer << std::endl;
	const GLubyte* version = glGetString(GL_VERSION);
	std::cout << "Version: " << version << std::endl;

	// ���C�����[�v
	while (!window.ShouldClose()) {
		glClearColor(0.1f, 0.3f, 0.5f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		window.SwapBuffers();
	}

	// �I������
	glfwTerminate();

	return 0;
} 

/*
#include <Windows.h>
#include <GL/GL.h>
#pragma comment (lib, "opengl32.lib")

LRESULT CALLBACK WndProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam);

HGLRC oglContext;

// �G���g���[�|�C���g
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


// �E�B���h�E�v���V�[�W��
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

