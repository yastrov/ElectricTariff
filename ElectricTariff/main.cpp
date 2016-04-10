/*
Author: Yuri Astrov
License : The MIT License
*/

#include <windows.h>
#include <cstdlib>
#include <string>
#ifdef _DEBUG
	#include <sstream>
#endif
#include <wchar.h>
#include <Strsafe.h>
#include "resource.h"
#include <Intsafe.h>

// Global variables
#pragma region include and define
//#define MANUAL_MENU_CREATE
// The main window class name.
static const std::wstring szWindowClass = L"ElectricTariff";

// The string that appears in the application's title bar.
static wchar_t szTitle[] = L"WinApp";

HINSTANCE hInst;
HWND _hWnd;
NOTIFYICONDATAW nid = { 0 };

#define WM_TRAYICON WM_USER+1
//For other defines see resource.h
#pragma endregion

#pragma region define icon
static HICON GreenIcon = nullptr;
static HICON RedIcon = nullptr;
static HICON YellowIcon = nullptr;
std::wstring LoadStringFromResource(
	__in UINT stringID,
	__in_opt HINSTANCE instance);
#pragma endregion

#pragma region define notify icon and tray
HMENU  hPopupMenu = nullptr; // Popup Menu descriptor
void initNotifyIcon();
void createPopupMenu();

void updateIconWithHourInformation(bool flag = false);
void showNotifyInformation(const wchar_t *title, const wchar_t * message, HICON icon);
void showNotifyInformation(const wchar_t *title, HICON icon);

void showNotifyInformation(const std::wstring &title, const std::wstring & message, HICON icon);
void showNotifyInformation(const std::wstring &title, HICON icon);
void showNotifyInformation(const wchar_t *title, HICON icon);
LRESULT trayMenuProc(const HWND &hWnd, const UINT &message, const  WPARAM &wParam, const LPARAM &lParam);
#pragma endregion

#pragma region define timer
constexpr UINT Timer_Interval = 60 * 60 * 1000; //1000 = sec, 1000*60 = min. Main timer interval, once a hour.
const UINT Timer_Interval_delta = 1000; //Offset for Main timer start.

void startTimer();
void startSupportTimer();
LRESULT timerProc(const HWND &hWnd, const UINT &message, const  WPARAM &wParam, const LPARAM &lParam);
#pragma endregion

#pragma region define registry
void AddToAutoRun(const wchar_t *name, const wchar_t *path);
void AddToAutoRun()
{
	wchar_t ourPath[MAX_PATH];
	const DWORD result = ::GetModuleFileNameW(
		nullptr,    // retrieve path of current process .EXE
		ourPath,
		MAX_PATH
		);
	if (result == 0)
	{
		// Error
		const DWORD error = ::GetLastError();
		MessageBoxW(NULL,
			LoadStringFromResource(IDS_STRING_ERR_CantGetPathToExe, hInst).c_str(),
			szWindowClass.c_str(),
			MB_ICONWARNING);
		return;
	}
	AddToAutoRun(szWindowClass.c_str(), ourPath);
}
void RemoveFromAutorun(const wchar_t *name);
void RemoveFromAutorun() 
{
	RemoveFromAutorun(szWindowClass.c_str());
}
#pragma endregion

#pragma region MAIN
// This function from WWW.
std::wstring LoadStringFromResource(
	__in UINT stringID,
	__in_opt HINSTANCE instance = NULL)
{
	wchar_t * pBuf = nullptr;

	const int len = LoadStringW(
		instance,
		stringID,
		reinterpret_cast< wchar_t * >(&pBuf),
		0);

	if (len)
		return std::wstring(pBuf, len);
	else
		return std::wstring();
}

void MsgBoxFromHRESULT(LONG result)
{
	wchar_t buf[MAX_PATH] = { 0 };
	FormatMessageW(FORMAT_MESSAGE_FROM_SYSTEM, L"", result, NULL, buf, MAX_PATH, nullptr);
	MessageBoxW(NULL, buf,
		szWindowClass.c_str(),
		MB_OK | MB_ICONWARNING);
}

void MsgBoxLastError()
{
	wchar_t buf[MAX_PATH];
	FormatMessageW(FORMAT_MESSAGE_FROM_SYSTEM, nullptr, GetLastError(),
		MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), buf, MAX_PATH, nullptr);
	MessageBoxW(NULL, buf,
		szWindowClass.c_str(),
		MB_OK | MB_ICONWARNING);
}

LRESULT WmCommandProc(const HWND &hWnd, const UINT &message, const  WPARAM &wParam, const LPARAM &lParam);

// Forward declarations of functions included in this code module:
LRESULT CALLBACK WndProc(HWND, UINT, WPARAM, LPARAM);

int WINAPI WinMain(HINSTANCE hInstance,
	HINSTANCE hPrevInstance,
	char *lpCmdLine,
	int nCmdShow)
{
	#pragma region init window
	WNDCLASSEXW wcex;

	wcex.cbSize = sizeof(WNDCLASSEXW);
	wcex.style = CS_HREDRAW | CS_VREDRAW;
	wcex.lpfnWndProc = WndProc;
	wcex.cbClsExtra = 0;
	wcex.cbWndExtra = 0;
	wcex.hInstance = hInstance;
	wcex.hIcon = LoadIconW(hInstance, MAKEINTRESOURCEW(IDI_MYICON)); //In old IDI_APPLICATION
	wcex.hCursor = LoadCursorW(NULL, IDC_ARROW);
	wcex.hbrBackground = reinterpret_cast<HBRUSH>(COLOR_WINDOW + 1);
	wcex.lpszMenuName = nullptr;
	wcex.lpszClassName = szWindowClass.c_str();
	wcex.hIconSm = LoadIconW(hInstance, MAKEINTRESOURCEW(IDI_MYICON)); //In old IDI_APPLICATION

	if (!RegisterClassExW(&wcex))
	{
		MessageBoxW(NULL,
			LoadStringFromResource(IDS_STRING_ERR_RegisterClass, NULL).c_str(),
			szWindowClass.c_str(),
			MB_ICONWARNING);
		return 1;
	}

	hInst = hInstance; // Store instance handle in our global variable
					   // The parameters to CreateWindow explained:
					   // szWindowClass: the name of the application
					   // szTitle: the text that appears in the title bar
					   // WS_OVERLAPPEDWINDOW: the type of window to create
					   // CW_USEDEFAULT, CW_USEDEFAULT: initial position (x, y)
					   // 500, 100: initial size (width, length)
					   // NULL: the parent of this window
					   // NULL: this application does not have a menu bar
					   // hInstance: the first parameter from WinMain
					   // NULL: not used in this application
	HWND hWnd = CreateWindowW(
		szWindowClass.c_str(),
		szTitle,
		WS_OVERLAPPEDWINDOW,
		CW_USEDEFAULT, CW_USEDEFAULT,
		500, 100,
		NULL,
		NULL,
		hInstance,
		NULL
		);

	if (!hWnd)
	{
		MessageBoxW(NULL,
			L"Call to CreateWindow failed!",
			LoadStringFromResource(IDS_STRING_ERR_CallCreateWindow, hInst).c_str(),
			MB_ICONWARNING);

		return 1;
	}
	_hWnd = hWnd;

	// The parameters to ShowWindow explained:
	// hWnd: the value returned from CreateWindow
	// nCmdShow: the fourth parameter from WinMain
	ShowWindow(hWnd,
		nCmdShow);
	UpdateWindow(hWnd);
	#pragma endregion

	#pragma region OUR code
	ShowWindow(hWnd, SW_HIDE);
	#pragma region Load icons
	GreenIcon = LoadIconW(hInst, MAKEINTRESOURCEW(IDI_GREENICON));
	if (GreenIcon == nullptr)
	{
		MessageBoxW(NULL, LoadStringFromResource(IDS_STRING_ERR_CantLoadIcon, hInst).c_str(), szWindowClass.c_str(), MB_OK | MB_ICONERROR);
		return 1;
	}
	RedIcon = LoadIconW(hInst, MAKEINTRESOURCEW(IDI_REDICON));
	if (RedIcon == nullptr)
	{
		MessageBoxW(NULL, LoadStringFromResource(IDS_STRING_ERR_CantLoadIcon, hInst).c_str(), szWindowClass.c_str(), MB_OK | MB_ICONERROR);
		return 1;
	}
	YellowIcon = LoadIconW(hInst, MAKEINTRESOURCEW(IDI_YELLOWICON));
	if (YellowIcon == nullptr)
	{
		MessageBoxW(NULL, LoadStringFromResource(IDS_STRING_ERR_CantLoadIcon, hInst).c_str(), szWindowClass.c_str(), MB_OK | MB_ICONERROR);
		return 1;
	}
	#pragma endregion
	initNotifyIcon();
	updateIconWithHourInformation();
	startSupportTimer();
	#pragma endregion

	#pragma region message loop
	MSG msg;
	while (GetMessageW(&msg, NULL, 0, 0))
	{
		TranslateMessage(&msg);
		DispatchMessageW(&msg);
	}
	#pragma endregion
	DestroyMenu(hPopupMenu);
	return static_cast<int>(msg.wParam);
}

//
//  FUNCTION: WndProc(HWND, UINT, WPARAM, LPARAM)
//
//  PURPOSE:  Processes messages for the main window.
//
//  WM_PAINT    - Paint the main window
//  WM_DESTROY  - post a quit message and return
//  WM_CREATE
//  WM_COMMAND  - Process any menu
//  WM_TRAYICON - Our command from Tray Icon
//  WM_TIMER    - Timer
//  WM_CLOSE    - Close window
//
//
LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	switch(message)
	{
	case WM_PAINT: {
		PAINTSTRUCT ps{ 0 };
		HDC hdc;
		std::wstring greeting = LoadStringFromResource(IDS_STRING_Greeting, hInst);
		hdc = BeginPaint(hWnd, &ps);
		int r = 0;
		SizeTToInt(greeting.length(), &r);
		TextOutW(hdc,
			5, 5,
			greeting.c_str(), r);

		EndPaint(hWnd, &ps);}
		break;
	case WM_DESTROY:
		KillTimer(_hWnd, IDT_TIMER_MAIN);
		KillTimer(_hWnd, IDT_TIMER_SUPPORT);
		Shell_NotifyIconW(NIM_DELETE, &nid);
		PostQuitMessage(0);
		break;
	case WM_CONTEXTMENU:
	case WM_CREATE:
		createPopupMenu();
		break;
	case WM_CLOSE:
		if (MessageBoxW(NULL, LoadStringFromResource(IDS_STRING_ExitQuestion, hInst).c_str(), szWindowClass.c_str(), MB_OKCANCEL) == IDOK)
		{
			DestroyWindow(_hWnd);
		}
		break;
	case WM_TRAYICON:
		return trayMenuProc(hWnd, message, wParam, lParam);
		break;
	case WM_COMMAND:
		// Process Menu
		return WmCommandProc(hWnd, message, wParam, lParam);
		break;
	case WM_TIMER:
		return timerProc(hWnd, message, wParam, lParam);
		break;
	default:
		return DefWindowProcW(hWnd, message, wParam, lParam);
		break;
	}
	return 0;
}

/* WM_COMMAND:
Message Source  wParam (high word)  wParam (low word)               lParam
Menu            0                   Menu identifier (IDM_*)         0
Accelerator     1                   Accelerator identifier (IDM_*)  0
Control         Control-defined     Control identifier              Handle to the
				notification code                                   control window
*/
LRESULT WmCommandProc(const HWND &hWnd, const UINT &message, const  WPARAM &wParam, const LPARAM &lParam)
{
	const WORD &wNotifyCode = HIWORD(wParam);	// код уведомления
	const WORD &wID = LOWORD(wParam);  // идентификатор элемента меню, управления или
									   // клавиши ускорителя
	const HWND hwndCtl = (HWND)lParam;  // дескриптор элемента управления 
	switch (wNotifyCode)
	{
	case 0: // Menu
	{
		// Check LOWORD(wParam) here - our Menu ID
		switch (wID) {
		case ID_EXIT:
			if (MessageBoxW(NULL, LoadStringFromResource(IDS_STRING_ExitQuestion, hInst).c_str(),
				szWindowClass.c_str(),
				MB_YESNO | MB_ICONQUESTION) == IDYES) {
				DestroyWindow(_hWnd);
				//Also we can call: SendMessage(hwnd, WM_DESTROY, 0, 0);
			}
			break;
		case ID_AUTOSTART_ADD:
			AddToAutoRun();
			break;
		case ID_AUTOSTART_REMOVE:
			RemoveFromAutorun();
			break;
		default:
			return DefWindowProcW(hWnd, message, wParam, lParam);
			break;
		}
	}
	break;
	default:
		return DefWindowProcW(hWnd, message, wParam, lParam);
		break;
	}
	return 0;
}
#pragma endregion

#pragma region TIMER
void startTimer() {
	SetTimer(_hWnd, IDT_TIMER_MAIN, Timer_Interval, nullptr);
}

void startSupportTimer(){
	SYSTEMTIME lt = { 0 };
	GetLocalTime(&lt);
	const WORD delta_time = 60 - lt.wMinute;
	if (delta_time > 1) {
		const UINT Timer_Interval1 = 1000 * 60 * static_cast<UINT>(delta_time)+ Timer_Interval_delta;
		SetTimer(_hWnd, IDT_TIMER_SUPPORT, Timer_Interval1, nullptr);
	}
	else startTimer();
}

LRESULT timerProc(const HWND &hWnd, const UINT &message, const  WPARAM &wParam, const LPARAM &lParam)
{
	switch (wParam) {
	case IDT_TIMER_MAIN:
		break;
	case IDT_TIMER_SUPPORT:
		KillTimer(_hWnd, IDT_TIMER_SUPPORT);
		startTimer();
		break;
	case ID_TEST_ABOUT:
		MessageBoxW(NULL, LoadStringFromResource(IDS_STRING_About, hInst).c_str(),
			szWindowClass.c_str(), MB_OK | MB_ICONINFORMATION);
		break;
	default:
		return DefWindowProcW(hWnd, message, wParam, lParam);
		break;
	}
	updateIconWithHourInformation();
	return 0;
}
#pragma endregion

#pragma region REGISTRY functions
void AddToAutoRun(const wchar_t *name, const wchar_t *path)
{
	wchar_t buf[MAX_PATH] = { 0 };
	HKEY key = nullptr;
	LSTATUS status = RegOpenKeyExW(HKEY_CURRENT_USER, L"SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Run", 0, KEY_ALL_ACCESS | KEY_WOW64_64KEY, &key);
	if (status != ERROR_SUCCESS) {
		MsgBoxFromHRESULT(status);
		return;
	}
	swprintf_s(buf, MAX_PATH, L"\"%s\"", path);
	int size = 0;
	SizeTToInt(wcslen(buf)*sizeof(wchar_t) + sizeof(wchar_t), &size);
	status = RegSetValueExW(key, name, 0, REG_SZ, reinterpret_cast<LPBYTE>(buf), size);
	// Next command create subfolder for "Run".
	//HKEY k;
	//status = RegCreateKeyExW(key, name, 0, 0 ,REG_SZ, KEY_ALL_ACCESS | KEY_WOW64_64KEY,
	//	NULL,
	//	&k,
	//	NULL
	//	);
	//status = RegSetValueExW(k, name, 0, REG_SZ, reinterpret_cast<LPBYTE>(buf), size);
	//RegCloseKey(k);
	if (status != ERROR_SUCCESS)
	{
		MsgBoxFromHRESULT(status);
	}

	RegCloseKey(key);
}

void RemoveFromAutorun(const wchar_t *name)
{
	if (name == nullptr) return;
	HKEY hkey = nullptr;
	LSTATUS result = RegOpenKeyExW(HKEY_CURRENT_USER, L"SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Run",
		0, KEY_ALL_ACCESS | KEY_WOW64_64KEY, &hkey);
	if (result != ERROR_SUCCESS) {
		MsgBoxFromHRESULT(result);
		return;
	}
	//In next command we really find subfolder for Run:
	//result = RegDeleteKeyExW(hkey, name, KEY_WOW64_64KEY, 0);
	result = RegDeleteValueW(hkey, name);
	if (result != ERROR_SUCCESS) {
		MsgBoxFromHRESULT(result);
	}
	RegCloseKey(hkey);
}
#pragma endregion

#pragma region NOTIFY TRAY ICON
void initNotifyIcon() {
	nid.cbSize = sizeof(NOTIFYICONDATA);
	nid.hWnd = _hWnd;
	nid.uID = ID_TRAY; // It conflict with GUID
	nid.uFlags = NIF_ICON | NIF_MESSAGE | NIF_TIP | NIF_INFO;
	//nid.uFlags = NIF_ICON | NIF_TIP | NIF_GUID  | NIF_SHOWTIP;
	nid.uVersion = NOTIFYICON_VERSION_4;
	nid.hIcon = LoadIconW(hInst, MAKEINTRESOURCE(IDI_MYICON));
	nid.uCallbackMessage = WM_TRAYICON;
	if (nid.hIcon == nullptr)
	{
		MessageBoxW(NULL, LoadStringFromResource(IDS_STRING_ERR_CantLoadIcon, hInst).c_str(),
			szWindowClass.c_str(), MB_OK | MB_ICONWARNING);
		return;
	}
	wcscpy_s(nid.szTip, 64, szWindowClass.c_str());
	// {821A88A2-C946-4BBD-B19D-654A020A09F8}
	static const GUID myGUID =
	{ 0x821a88a2, 0xc946, 0x4bbd,{ 0xb1, 0x9d, 0x65, 0x4a, 0x2, 0xa, 0x9, 0xf8 } };

	nid.guidItem = myGUID;
	Shell_NotifyIconW(NIM_ADD, &nid);
}

void createPopupMenu() {
#ifdef MANUAL_MENU_CREATE
	hPopupMenu = CreatePopupMenu();
	AppendMenuW(hPopupMenu, MF_STRING, ID_AUTOSTART_REMOVE, L"Убрать из автозагрузки");
	AppendMenuW(hPopupMenu, MF_STRING, ID_AUTOSTART_ADD, L"Добавить в автозагрузку");
	AppendMenuW(hPopupMenu, MF_SEPARATOR, NULL, L"");
	AppendMenuW(hPopupMenu, MF_STRING, ID_EXIT, L"Выйти из программы");
#else
	hPopupMenu = LoadMenuW(hInst, MAKEINTRESOURCEW(IDR_POPUPMENU));
	if (!hPopupMenu) {
		MsgBoxLastError();
	}
#endif
}

LRESULT trayMenuProc(const HWND &hWnd, const UINT &message, const WPARAM &wParam, const LPARAM &lParam) {
	switch (wParam)
	{
	case ID_TRAY:
		break;
	default:
		break;
	}
	switch (lParam)
	{
	case WM_LBUTTONUP:
		updateIconWithHourInformation(true);
		break;
	case WM_RBUTTONDOWN:
	{
		POINT curPoint;
		GetCursorPos(&curPoint);

		// should SetForegroundWindow according
		// to original poster so the popup shows on top
		SetForegroundWindow(_hWnd);

		// TrackPopupMenu blocks the app until TrackPopupMenu returns
		const UINT clicked = TrackPopupMenu(
			#ifdef MANUAL_MENU_CREATE
			hPopupMenu,
			#else
			// Fix Bug if menu loded from resources.
			GetSubMenu(hPopupMenu, 0),
			#endif
			TPM_RETURNCMD | TPM_NONOTIFY, // don't send me WM_COMMAND messages about this window, instead return the identifier of the clicked menu item
			curPoint.x,
			curPoint.y,
			0,
			_hWnd,
			nullptr
			);
		switch (clicked) {
		case ID_EXIT:
			if (MessageBoxW(NULL, LoadStringFromResource(IDS_STRING_ExitQuestion, hInst).c_str(),
				szWindowClass.c_str(),
				MB_YESNO | MB_ICONQUESTION) == IDYES) {
				DestroyWindow(_hWnd);
				//Also we can call: SendMessage(hwnd, WM_DESTROY, 0, 0);
			}
			break;
		case ID_AUTOSTART_ADD:
			AddToAutoRun();
			break;
		case ID_AUTOSTART_REMOVE:
			RemoveFromAutorun();
			break;
		case ID_TEST_ABOUT:
			MessageBoxW(NULL, LoadStringFromResource(IDS_STRING_About, hInst).c_str(),
				szWindowClass.c_str(), MB_OK | MB_ICONINFORMATION);
			break;
		default:
			break;
		}
		break;
	}
	default:
		return DefWindowProcW(hWnd, message, wParam, lParam);
		break;
	}
	return 0;
}

void showNotifyInformation(const wchar_t * title, const wchar_t * message, HICON icon) {
	if (wcslen(title) < 64) {
		wcscpy_s(nid.szTip, title);
	}
	if (wcslen(message) < 256) {
		wcscpy_s(nid.szInfo, message);
	}
	if (szWindowClass.length() < 64) {
		wcscpy_s(nid.szInfoTitle, 64, szWindowClass.c_str());
	}
	nid.hIcon = icon;
	nid.hBalloonIcon = icon;
	Shell_NotifyIconW(NIM_MODIFY, &nid);
}

void showNotifyInformation(const wchar_t * title, HICON icon) {
	if (wcslen(title) < 64) {
		wcscpy_s(nid.szTip, title);
	}
	nid.hIcon = icon;
	nid.hBalloonIcon = icon;
	Shell_NotifyIconW(NIM_MODIFY, &nid);
}

void showNotifyInformation(const std::wstring &title, const std::wstring & message, HICON icon) {
	if (title.length() < 64) {
		wcscpy_s(nid.szTip, title.c_str());
	}
	if (message.length() < 256) {
		wcscpy_s(nid.szInfo, message.c_str());
	}
	if (szWindowClass.length() < 64) {
		wcscpy_s(nid.szInfoTitle, szWindowClass.c_str());
	}
	nid.hIcon = icon;
	nid.hBalloonIcon = icon;
	Shell_NotifyIconW(NIM_MODIFY, &nid);
}
void showNotifyInformation(const std::wstring &title, HICON icon) {
	if (title.length() < 64) {
		wcscpy_s(nid.szTip, title.c_str());
	}
	if (szWindowClass.length() < 64) {
		wcscpy_s(nid.szInfoTitle, 64, szWindowClass.c_str());
	}
	nid.hIcon = icon;
	nid.hBalloonIcon = icon;
	Shell_NotifyIconW(NIM_MODIFY, &nid);
}

//
//  FUNCTION: updateIconWithHourInformation(bool flag)
//
//  PURPOSE:  Check current time and show notification and icon.
//
//  flag  - if true, baloon message show always, else - if tarif changes.
//
void updateIconWithHourInformation(bool flag)
{
	SYSTEMTIME lt = { 0 };
	GetLocalTime(&lt);
	const WORD &hour = lt.wHour;
#ifdef _DEBUG
	std::wostringstream stringStream;
	if (flag) {
		stringStream << L"[Manually] ";
	}
	else stringStream << L"[Timer] ";
	stringStream << L"Request time and update message in: "
				<< L"Hour "<<lt.wHour << L"Min " << lt.wMinute
		<< L"Sec " << lt.wSecond << L"Msec " << lt.wMilliseconds;
	OutputDebugStringW(stringStream.str().c_str());
#endif

	static std::wstring LowTariffTitle = LoadStringFromResource(IDS_STRING_LowTariffTitle, hInst);
	static std::wstring LowTariffMessage = LoadStringFromResource(IDS_STRING_LowTariffMessage, hInst);
	static std::wstring MediumTariffTitle = LoadStringFromResource(IDS_STRING_MediumTariffTitle, hInst);
	static std::wstring MediumTariffMessage = LoadStringFromResource(IDS_STRING_MediumTariffMessage, hInst);
	static std::wstring HighTariffTitle = LoadStringFromResource(IDS_STRING_HighTariffTitle, hInst);
	static std::wstring HighTariffMessage = LoadStringFromResource(IDS_STRING_HighTariffMessage, hInst);

	if ((hour >= 7 && hour < 10) || (hour >= 17 && hour < 21)) {
		if (hour == 7 || hour == 17 || flag)
			showNotifyInformation(HighTariffTitle.c_str(), HighTariffMessage.c_str(), RedIcon);
		else showNotifyInformation(HighTariffTitle.c_str(), RedIcon);
		return;
	}
	if ((hour >= 10 && hour < 17) || (hour >= 21 && hour < 23)) {
		if (hour == 10 || hour == 21 || flag)
			showNotifyInformation(MediumTariffTitle.c_str(), MediumTariffMessage.c_str(), YellowIcon);
		else showNotifyInformation(MediumTariffTitle.c_str(), YellowIcon);
		return;
	}
	if (hour == 10 || hour == 21 || flag)
		showNotifyInformation(LowTariffTitle.c_str(), LowTariffMessage.c_str(), GreenIcon);
	else showNotifyInformation(LowTariffTitle.c_str(), GreenIcon);
}
#pragma endregion