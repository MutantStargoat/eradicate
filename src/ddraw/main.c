#include <stdio.h>
#include <windows.h>
#include <ddraw.h>
#include "gfx.h"
#include "timer.h"
#include "game.h"

#define WCNAME	"eradicatewin"

static LRESULT CALLBACK handle_msg(HWND win, unsigned int msg, WPARAM wparam, LPARAM lparam);
static int regwinclass(HINSTANCE hinst);

void msgbox(const char *msg);

HWND win;

int have_joy;
unsigned int joy_bnstate, joy_bndiff, joy_bnpress;

static unsigned int num_pressed;
static unsigned char keystate[256];

static unsigned long start_time;
static unsigned int modkeys;

static int quit;


int WINAPI WinMain(HINSTANCE hinst, HINSTANCE hprev, char *cmdline, int showcmd)
{
	int vmidx;
	char *fake_argv[] = {"game.exe", 0};

#ifndef NDEBUG
	AllocConsole();
	freopen("CONIN$", "r", stdin);
	freopen("CONOUT$", "w", stdout);
	freopen("CONOUT$", "w", stderr);
#endif

	if(regwinclass(hinst) == -1) {
		return 1;
	}

	if(!(win = CreateWindowEx(0, WCNAME, "eradicate", WS_POPUP, 10, 10, 320, 240, 0, 0, hinst, 0))) {
		msgbox("failed to create window");
		return 1;
	}
	ShowWindow(win, 1);

	if(au_init() == -1) {
		return 1;
	}

	if(init_video() == -1) {
		return 1;
	}

	if((vmidx = match_video_mode(640, 480, 16)) == -1) {
		return 1;
	}
	if(!set_video_mode(vmidx, 1)) {
		msgbox("failed to set requested video mode 640x480 16bpp");
		return 1;
	}

	time_msec = 0;
	if(init(1, fake_argv) == -1) {
		return 1;
	}
	reset_timer();

	for(;;) {
		MSG msg;

		while(PeekMessage(&msg, 0, 0, 0, PM_REMOVE)) {
			TranslateMessage(&msg);
			DispatchMessage(&msg);

			if(quit) goto end;
		}

		draw();
	}

end:
	cleanup_video();
	DestroyWindow(win);
	UnregisterClass(WCNAME, hinst);
	return 0;
}

void game_quit(void)
{
	quit = 1;
}

/* joystick */
int joy_detect(void)
{
	return 0;
}

void joy_update(void)
{
}

int kb_isdown(int key)
{
	switch(key) {
	case KB_ANY:
		return num_pressed;

	case KB_ALT:
		return keystate[KB_LALT] + keystate[KB_RALT];

	case KB_CTRL:
		return keystate[KB_LCTRL] + keystate[KB_RCTRL];
	}

	if(isalpha(key)) {
		key = tolower(key);
	}
	return keystate[key];
}

/* timer */
void init_timer(int res_hz)
{
}

void reset_timer(void)
{
	start_time = timeGetTime();
}

unsigned long get_msec(void)
{
	return timeGetTime() - start_time;
}

static LRESULT CALLBACK handle_msg(HWND win, unsigned int msg, WPARAM wparam, LPARAM lparam)
{
	switch(msg) {
	case WM_CLOSE:
		PostQuitMessage(0);
		break;

	case WM_QUIT:
		quit = 1;
		break;

	default:
		return DefWindowProc(win, msg, wparam, lparam);
	}

	return 0;
}

static int regwinclass(HINSTANCE hinst)
{
	WNDCLASSEX wc = {0};

	wc.cbSize = sizeof wc;
	wc.lpszClassName = WCNAME;
	wc.hInstance = hinst;
	wc.style = CS_HREDRAW | CS_VREDRAW | CS_OWNDC;
	wc.lpfnWndProc = handle_msg;
	wc.hIcon = wc.hIconSm = LoadIcon(0, IDI_APPLICATION);
	wc.hCursor = LoadCursor(0, IDC_ARROW);
	wc.hbrBackground = (HBRUSH)GetStockObject(BLACK_BRUSH);

	if(!RegisterClassEx(&wc)) {
		msgbox("Failed to register window class");
		return -1;
	}
	return 0;
}

void msgbox(const char *msg)
{
	MessageBox(0, msg, "error", MB_OK);
}
