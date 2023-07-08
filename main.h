
#pragma once
////////////////////
//プロトタイプ宣言
////////////////////
#include<Windows.h>

// WM_CLIPBORADUPDATE内の処理
bool OnClipBoardUpdate();

// WM_COMMAND内の処理
bool OnCommand(HWND, WPARAM, LPARAM);

// WM_CREATE内の処理
bool OnCreate(HWND, WPARAM, LPARAM);

// 子ウィンドウの作成
bool CreateChildWindow(HWND, WPARAM, LPARAM);

// WM_CTLCOLORSTATIC内の処理
long OnCtlColorStatic(WPARAM, LPARAM);

// メモリデバイスコンテキスト描画の事前準備
bool GetWindowMemDC(HWND);

// メモリデバイスコンテキストへの描画
bool PaintToMemDC(HWND);

// フォントの定義
bool OnFont(HWND);

BOOL DesktopCenterWindow(HWND);
//
LPVOID FindDIBBits(LPVOID);
//
int DIBNumColors(LPVOID);

bool PaintImages(HDC);

bool SaveClipBoardImage(HANDLE);

bool UpadateClipDisp(HWND);