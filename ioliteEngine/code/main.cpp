#include "Application/sAppExecutor.h"
#include "Game/cDX11App.h"

//! @brief entry point
int WINAPI WinMain(HINSTANCE hInst, HINSTANCE hPrevInst, LPSTR lpCmdLine, s32 nCmdShow) {
#if defined(_DEBUG)	//!< create Console
	if (!AttachConsole(ATTACH_PARENT_PROCESS)) {
		AllocConsole();
	}
	FILE *in, *out, *err;
	freopen_s(&in,  "CONIN$",  "r", stdin );
	freopen_s(&out, "CONOUT$", "w", stdout);
	freopen_s(&err, "CONOUT$", "w", stderr);
#endif

	IAppExecutor->setup(new cDX11App(), hInst, nCmdShow);
	IAppExecutor->run();
	IAppExecutor->shutdown();
	IAppExecutor->destroy();

#if defined(_DEBUG)
	fclose(err);
	fclose(out);
	fclose(in);
	FreeConsole();
#endif
	return 0;
}