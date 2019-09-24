#include "sWindow.h"

//! static member
SINGLETON_INSTANCE(sWindow);

//! @brief コンストラクタ
sWindow::sWindow() : mTitle(""), mWidth(800), mHeight(600), mHandle(), mIsClose(false){

}
//! @brief デストラクタ
sWindow::~sWindow() {

}
//! @brief コモンコールバック
LRESULT CALLBACK sWindow::WndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam) {
	sWindow* window = (sWindow*)(GetWindowLongPtr(hWnd, GWLP_USERDATA));
	if (window == nullptr) {
		if (msg == WM_CREATE) {
			window = (sWindow*)(((LPCREATESTRUCT)lParam)->lpCreateParams);
		}
		if (window != nullptr) {
			SetWindowLongPtr(hWnd, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(window));
		}
	}
	if (window != nullptr) {
		return window->ClassWndProc(hWnd, msg, wParam, lParam);
	}
	return DefWindowProc(hWnd, msg, wParam, lParam);
}
//! @brief クラス専用のコールバック
LRESULT CALLBACK sWindow::ClassWndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam) {
	switch (msg) {
	case WM_CLOSE:
		DestroyWindow(hWnd);
		break;
	case WM_DESTROY:
		PostQuitMessage(0);
		break;
	default:
		return DefWindowProc(hWnd, msg, wParam, lParam);
	}
	return 0;
}
//! @brief 初期化処理
void sWindow::setup(const char* title, u32 width, u32 height, HINSTANCE hInst, s32 nCmdShow) {
	const char* className = "iolite Engine";
	UINT windowStyle = WS_OVERLAPPEDWINDOW & (~WS_MAXIMIZEBOX);
	mTitle  = title;
	mWidth  = width;
	mHeight = height;

	WNDCLASSEX wc;
	wc.cbSize        = sizeof(WNDCLASSEX);
	wc.style         = 0;
	wc.lpfnWndProc   = WndProc;
	wc.cbClsExtra    = 0;
	wc.cbWndExtra    = 0;
	wc.hInstance     = hInst;
	wc.hIcon         = LoadIcon(NULL, IDI_APPLICATION);
	wc.hCursor       = LoadCursor(NULL, IDC_ARROW);
	wc.hbrBackground = (HBRUSH)(COLOR_WINDOW+1);
	wc.lpszMenuName  = NULL;
	wc.lpszClassName = className;
	wc.hIconSm       = LoadIcon(NULL, IDI_APPLICATION);
	RegisterClassEx(&wc);

	RECT rc = { 0, 0, (LONG)mWidth, (LONG)mHeight };
	AdjustWindowRect( &rc, windowStyle, FALSE );

	mHandle = CreateWindowEx(WS_EX_CLIENTEDGE, className, mTitle, windowStyle, CW_USEDEFAULT, CW_USEDEFAULT, mWidth, mHeight, NULL, NULL, hInst, this);
	ShowWindow(mHandle, nCmdShow);
	UpdateWindow(mHandle);
}
//! @brief 終了チェック
bool sWindow::shouldClose() {
	return mIsClose;
}
//! @brief Windows処理チェック
void sWindow::waitEvents() {
	MSG msg;
	if (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE) > 0) {
		if (msg.message == WM_QUIT) {
			mIsClose = true;
		}
		TranslateMessage(&msg);
		DispatchMessage(&msg);
	}
}
//! @brief アプリの終了リクエスト
void sWindow::closeRequest() {
	CloseWindow(mHandle);
}
//! @brief ウィンドウハンドル取得
HWND sWindow::getHandle() {
	return mHandle;
}
//! @brief クライアントサイズ取得
void sWindow::getClientRect(UINT& width, UINT& height) {
	RECT rc;
	GetClientRect(getHandle(), &rc);
	width  = rc.right - rc.left;
	height = rc.bottom - rc.top;
}