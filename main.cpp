//////
//1.ウィンドウ
//////
#include<windows.h>
#include<tchar.h>
#include<string>
#include<vector>
#include<gdiplus.h>
#include<gdipluscolor.h>

// 自作ヘッダファイルのロード
#include"main.h"
#include"window.h"
#include "resource.h"
#include"file.h"
#include"VersionInfo.h"

//////////////////////////////
// ビジュアルスタイル有効化
/////////////////////////////
#pragma comment(linker,"/manifestdependency:\"type='win32' \
  name='Microsoft.Windows.Common-Controls' \
  version='6.0.0.0' \
  processorArchitecture='*' \
  publicKeyToken='6595b64144ccf1df' \
  language='*'\"") 

#pragma comment (lib,"Gdiplus.lib")


using namespace std;
using namespace Gdiplus;

LRESULT CALLBACK windowfunc(HWND, UINT, WPARAM, LPARAM);
static const TCHAR szWinName[] = _T("ClipBoard History");

/////////////////////////////
// ウィンドウの背景色の定義
/////////////////////////////
const HBRUSH BackGround_clear = CreateSolidBrush(RGB(255, 255, 255));
const HBRUSH BackGround_dark = CreateSolidBrush(RGB(0, 0, 0));

/////////////////////////////
// RECT構造体変数の初期化
/////////////////////////////
RECT Winsize = { 0,0,1100,750 };

// フォント用
HFONT hFont1, hFont2, hFont3, hFontBig, hFontSmall;

// WM_PAINT
HDC hMemDC;
HDC hdcMem_paintImages;
HBITMAP hBitmap_paintImages;
HBITMAP hBitmapOld_paintImages;
HGDIOBJ hOld;

// ベクター
vector<string> clipboardHistory;
vector<HBITMAP> clipboardImageHistory;

ULONG_PTR gdiplusToken;

// Bitmap
HBITMAP hBitmap{};

// ブラシ
HPEN hBluePen = CreatePen(PS_SOLID, 4, RGB(0, 203, 152));
HGDIOBJ hOldPen{};

//WinMain関数
int WINAPI WinMain(_In_ HINSTANCE hThisInst, _In_opt_ HINSTANCE hPrevInst, _In_ LPSTR lpszArgs, _In_ int nWinMode)
{
	HWND		hWnd{};
	MSG			msg;
	WNDCLASSEX	wcl{};
	HACCEL haccel{};

	// アプリケーションの二重起動防止
	HANDLE hMutex{};
	if (hMutex = CreateMutex(NULL, TRUE, _T("uniqueApplicationName"))) {
		if (GetLastError() == ERROR_ALREADY_EXISTS) {
			// アプリが実行中である場合は終了する。
			MessageBox(NULL, _T("既に起動しています。"), _T("エラー"), MB_OK);
			CloseHandle(hMutex);
			return 0;
		}
	}
	

	//ウィンドウクラスの定義
	wcl.cbSize = sizeof(WNDCLASSEX);				//WNDCLASSEX構造体のサイズ
	wcl.style = 0;									//ウィンドウクラススタイル
	wcl.lpfnWndProc = windowfunc;					//ウィンドウ関数
	wcl.cbClsExtra = 0;								//ウィンドウクラスのエキストラ
	wcl.cbWndExtra = 0;								//ウィンドウインスタンスのエキストラ
	wcl.hInstance = hThisInst;						//このプログラムのインスタンスへのハンドル
	wcl.hIcon = LoadIcon(NULL, IDI_APPLICATION);	//アイコンへのハンドル
	wcl.hCursor = LoadCursor(NULL, IDC_ARROW);		//カーソルへのハンドル
	wcl.hbrBackground = (HBRUSH)COLOR_WINDOW;		//背景ブラシへのハンドル
	wcl.lpszMenuName = MAKEINTRESOURCE(IDR_MENU1);						//メニュー
	wcl.lpszClassName = szWinName;					//ウィンドウクラス名
	wcl.hIconSm = LoadIcon(NULL, IDI_WINLOGO);		//スモールアイコンへのハンドル

	//ウィンドウクラスの登録
	if (!RegisterClassEx(&wcl)) {
		return(0);
	}

	//ウィンドウの生成
	hWnd = CreateWindow(
		szWinName,				//ウィンドウクラス名
		szWinName,				//ウィンドウ名
		WS_OVERLAPPED | WS_CAPTION | WS_SYSMENU | WS_MINIMIZEBOX, //ウィンドウスタイル
		CW_USEDEFAULT,			//x座標
		CW_USEDEFAULT,			//y座標
		Winsize.right,			//幅
		Winsize.bottom,			//高さ
		HWND_DESKTOP,			//親ウィンドウへのハンドル
		NULL,					//メニューへのハンドル
		hThisInst,				//このプログラムのインスタンスへのハンドル
		NULL					//追加引数
	);

	// ドロッグ＆ドロップファイルへの対応
	DragAcceptFiles(hWnd, TRUE);

	// クリップボードの変更を検知するリスナーを登録
	if (!AddClipboardFormatListener(hWnd)) {
		MessageBox(hWnd, _T("クリップボード形式リスナーの登録ができませんでした。\r\nOSが動作保証対象外である可能性があります。"), _T("Error"), MB_OK);
	}

	//ウィンドウの表示
	ShowWindow(hWnd, nWinMode);
	UpdateWindow(hWnd);
	
	// アプリのアイコンを変更
	HICON hIcon;
	hIcon = (HICON)LoadImage(hThisInst, MAKEINTRESOURCE(IDI_ICON1), IMAGE_ICON, 48, 48, 0);
	SendMessage(hWnd, WM_SETICON, ICON_SMALL, (LPARAM)hIcon);

	//キーボードアクセラレータのロード
	haccel = LoadAccelerators(hThisInst, MAKEINTRESOURCE(IDR_ACCELERATOR1));

	//メッセージループの生成
	while (GetMessage(&msg, NULL, 0, 0)) {
		if (!TranslateAccelerator(hWnd, haccel, &msg)) {
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}
	}
	// ミューテックスを閉じる
	if (hMutex) {
		CloseHandle(hMutex);
	}
	
	return((int)msg.wParam);
}

//ウィンドウ関数
LRESULT CALLBACK windowfunc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	switch (uMsg) {
	case WM_DESTROY:
		// 後始末
		// リソースの解放
		DeleteDC(hdcMem_paintImages);
		// ブラシの削除
		SelectObject(hMemDC, hOldPen);
		DeleteObject(hBluePen);
		// クリップボード形式リスナーの解除
		RemoveClipboardFormatListener(hWnd);
		DeleteObject(hFont1);
		DeleteObject(hFont2);
		DeleteObject(hFont3);
		DeleteObject(hFontBig);
		DeleteObject(hFontSmall);
		SelectObject(hMemDC, hOld);
		DeleteDC(hMemDC);
		PostQuitMessage(0);

		return(0);
	case WM_CREATE:
		OnCreate(hWnd, wParam, lParam);
		break;
	case WM_COMMAND:
		OnCommand(hWnd, wParam, lParam);
		break;
	case WM_PAINT:
	{
		PAINTSTRUCT ps;
		HDC hdc = BeginPaint(hWnd, &ps);

		// メモリDCからDCにコピー
		BitBlt(ps.hdc,
			ps.rcPaint.left, ps.rcPaint.top,
			ps.rcPaint.right - ps.rcPaint.left, ps.rcPaint.bottom - ps.rcPaint.top,
			hMemDC,
			ps.rcPaint.left, ps.rcPaint.top,
			SRCCOPY);

		// クリップボードの画像をディスプレイにコピー
	/*	if (!clipboardImageHistory.empty()) {
			PaintImages(hdc);
		}else{
			MessageBox(hWnd, _T("clipboardImageHistory is empty..."), NULL, MB_OK);
		}*/
		
		EndPaint(hWnd, &ps);
		return(0);
	}
		break;

	case WM_DROPFILES:
	{
		// ウィンドウにファイルがドロップされた
		HDROP hDrop = (HDROP)wParam;
		TCHAR IpszFilePath[MAX_PATH] = { 0 };
		UINT uiFileCount = DragQueryFile(hDrop, 0xFFFFFFFF, NULL, 0);
		for (size_t i = 0; i < uiFileCount; i++)
		{
			DragQueryFile(hDrop, i, IpszFilePath, MAX_PATH);
			string FilePath = IpszFilePath;
			// クリップボードにパス名をコピー
			clipboardHistory.push_back(FilePath);
		}
		DragFinish(hDrop);
		// ウィンドウの表示内容を更新
		UpadateClipDisp(hWnd);
	}

		break;
	case WM_CLIPBOARDUPDATE:
		// クリップボードが変更された
		OnClipBoardUpdate();
		// エディットボックスの更新
		UpadateClipDisp(hWnd);
		break;
	case WM_CTLCOLORSTATIC:
		return (OnCtlColorStatic(wParam, lParam));
	case WM_ERASEBKGND:
		// 何も処理しない(画面のちらつき防止）
		return 1;
		break;

	case WM_CLOSE:
		if (MessageBox(hWnd, _T("終了しますか？"), _T("終了確認"), MB_YESNO | MB_ICONQUESTION) == IDYES) {
			DestroyWindow(hWnd);
		}
		break;
	default:
		return(DefWindowProc(hWnd, uMsg, wParam, lParam));
	}
	return(0);
}

// WM_CLIPBOARDUPDATE内の処理
bool OnClipBoardUpdate() {
	
	HANDLE hClip{};

	if (OpenClipboard(NULL))
	{
		if (IsClipboardFormatAvailable(CF_TEXT)) {
			hClip = GetClipboardData(CF_TEXT);
			if (hClip) {
				// クリップビードにテキストがコピーされた場合
				char* cText = static_cast<char*>(GlobalLock(hClip));
				if (cText)
				{
					string str = cText;
					clipboardHistory.push_back(str);
					GlobalUnlock(hClip);
				}
			}
		}
		//else if (IsClipboardFormatAvailable(CF_DIB) ){
		//	// DIB形式の場合
		//	hClip = GetClipboardData(CF_DIB);
		//	if (hClip) {
		//		SaveClipBoardImage(hClip);
		//	}
		//}
		//else if (IsClipboardFormatAvailable(CF_BITMAP)) {
		//	// BITMAP形式の場合
		//	HBITMAP hBitmap_local = (HBITMAP)GetClipboardData(CF_BITMAP);
		//	if (hBitmap_local) {
		//		clipboardImageHistory.push_back(hBitmap);
		//	}
		//	
		//}
		//else
		//{
		//		MessageBox(NULL, _T("コピー元の形式が不明です。（ソースコード要確認）"), NULL, MB_OK);
		//}
		//
	}
	// クリップボードのクローズ
	CloseClipboard();

	return true;
}

// WM_COMMAND内の処理
bool OnCommand(HWND hWnd, WPARAM wParam, LPARAM lParam) {
	switch (LOWORD(wParam)){
	case ID_BUTTON_CLEAR:
		if (MessageBox(hWnd, _T("クリップボードの履歴を全て消去します。\r\n続行しますか？"), _T("消去確認"), MB_YESNO | MB_ICONQUESTION) == IDYES) {
			if (!clipboardHistory.empty()) {
				clipboardHistory.clear();
			}else {
				MessageBox(hWnd, _T("クリップボードが空です。"), NULL, MB_OK);
			}
			
			SetWindowText(GetDlgItem(hWnd, ID_EDIT_CLIPBORD_1), TEXTEDITBOX);
		}
		break;
	case ID_MENU_CLIP:
		UpadateClipDisp(hWnd);
		break;
		
	case ID_BUTTON_EDIT_CLEAR:
		if (HIWORD(wParam) == BN_CLICKED) {
			// エディットボックス内の文字列を消去
			SetWindowText(GetDlgItem(hWnd, ID_EDIT_CLIPBORD_1), _T(""));
		}
		break;
	case ID_EXIT:
		if (MessageBox(hWnd, _T("終了しますか？"), _T("終了確認"), MB_YESNO | MB_ICONQUESTION) == IDYES) {
			DestroyWindow(hWnd);
		}
		break;
	case ID_MENU_COPY_NEW:
		// 最新のコピー内容のみを削除
		if (!clipboardHistory.empty()) {
			// クリップボードが空でない場合
			clipboardHistory.pop_back();
			MessageBox(hWnd, _T("最後の要素を削除しました。"), _T("削除完了"), MB_OK);
			UpadateClipDisp(hWnd);
		}else{
			MessageBox(hWnd, _T("クリップボードは空です。"), _T("エラー"), MB_OK);
		}
		break;
	case ID_MENU_HELP:
	{
		string temp2 = _T("使用可能なOS: Windows Vista以降");
		string temp3 = _T("使い方: メニューバーにある「更新」をクリックするか、キーボード上で「Ctrl + V」と打つこと　　　 でクリップボードの履歴を表示させることができます。\r\n　　　 表示させた後、内容を編集することも可能です。\r\n\r\n");
		string temp4 = _T("履歴の消去ボタン: ボタンを押すと、アプリ上で保存済みのクリップボードの履歴が全て　　　　　　　  消去されます。復元することはできません\r\n\r\n");
		string temp5 = _T("現在の表示内容をクリア: 表示内容を消去します。クリップボードの履歴は消去\r\n　　　　　　　　 　　されません。\r\n\r\n");
		string all =  temp3 + temp4 + temp5 +  temp2;
		MessageBox(hWnd, all.c_str(), _T("ヘルプ"), MB_OK);
	}
		break;
	case ID_MENU_USE:
	{
		string temp1 = _T("どんなアプリ: クリップボードへのコピー履歴を表示する簡素なWindows専用\r\n　　　　　アプリケーションです。\r\n\r\n");
		string temp2 = _T("アプリの良いところ: 起動後にコピーした内容を全て確認することが可能です。誤って\r\n　　　　　　 　クリップボードにコピーした内容を上書きしてしまった場合でも過去の\r\n　　　　　　 　コピー内容を見ることができます。\r\n　　　　　　 　Windows標準アプリよりも大きな画面で見れるため、\r\n　　　　　　 　一目でコピー履歴を確認できる点がこのアプリの優位性です。");
		
		string all = temp1 + temp2;
		MessageBox(hWnd, all.c_str(), _T("Hi"), MB_OK);
	}
	break;
	case ID_MENU_FILE:
		// 名前を付けて保存用のダイアログボックス
		SelectFile(hWnd);
		file_open(hWnd);

		break;
	case ID_BIG:
		//MessageBox(hWnd, _T("Comming Soon !"), _T("Sorry"), MB_OK);
		SendMessage(GetDlgItem(hWnd, ID_EDIT_CLIPBORD_1), WM_SETFONT, (WPARAM)hFontBig, TRUE);
		break;
	case ID_MEDIUM:
		SendMessage(GetDlgItem(hWnd, ID_EDIT_CLIPBORD_1), WM_SETFONT, (WPARAM)hFont2, TRUE);
		break;
	case ID_SMALL:
		SendMessage(GetDlgItem(hWnd, ID_EDIT_CLIPBORD_1), WM_SETFONT, (WPARAM)hFontSmall,TRUE);
		break;
	case ID_MENU_VERSIONINFO:
		SendVersionInfoToMessageBox(hWnd);
		break;
	default:
		break;
	}
	return true;
}

// WM_CREATE内の処理
bool OnCreate(HWND hWnd, WPARAM wParam, LPARAM lParam) {
	// GDI+の初期化
	GdiplusStartupInput gdiplusStartupInput;
	GdiplusStartup(&gdiplusToken, &gdiplusStartupInput, NULL);
	// 子ウィンドウの作成
	CreateChildWindow(hWnd, wParam, lParam);
	// メモリデバイスコンテキストの取得
	GetWindowMemDC(hWnd);
	// メモリデバイスコンテキストへの描画
	PaintToMemDC(hWnd);
	// フォントの適用
	OnFont(hWnd);
	// デスクトップを中央に配置
	DesktopCenterWindow(hWnd);
	// 実行ファイルのパス名の取得
	TCHAR lpfileName[MAX_PATH];
	GetModuleFileName(NULL, lpfileName, sizeof(lpfileName));
	// ウィンドウタイトルの変更
	SetWindowText(hWnd, lpfileName);
	return true;
}

// メモリデバイスコンテキスト描画の事前準備
bool GetWindowMemDC(HWND hWnd) {

	HDC hDC;
	HBITMAP hBitmap;

	// デバイスコンテキストの取得
	hDC = GetDC(hWnd);
	// メモリデバイスコンテキストの取得
	hMemDC = CreateCompatibleDC(hDC);
	// ビットマップハンドルの取得
	hBitmap = CreateCompatibleBitmap(hDC, Winsize.right, Winsize.bottom);
	// デバイスコンテキストの解放
	ReleaseDC(hWnd, hDC);
	// メモリDCにビットマップを割りつけ
	hOld = SelectObject(hMemDC, hBitmap);
	// ビットマップの削除
	DeleteObject(hBitmap);

	return true;
}

// メモリデバイスコンテキストへの描画
bool PaintToMemDC(HWND hWnd) {
	// 背景の塗りつぶし
	FillRect(hMemDC, &Winsize, BackGround_clear);
	// エディットボックスの枠
	RECT recthEditClip = { Winsize.left + 30 - 2, 90 - 2, (Winsize.right - 50) + 2, Winsize.bottom - Winsize.top - 90 + 2 };
	
	hOldPen = SelectObject(hMemDC, hBluePen);
	
	// 描画開始位置を設定
	MoveToEx(hMemDC, recthEditClip.left, recthEditClip.top, NULL);

	// 左上から右上への線
	LineTo(hMemDC, recthEditClip.right, recthEditClip.top);

	// 右上から右下への線
	LineTo(hMemDC, recthEditClip.right, recthEditClip.bottom);

	// 右下から左下への線
	LineTo(hMemDC, recthEditClip.left, recthEditClip.bottom);

	// 左下から左上への線
	LineTo(hMemDC, recthEditClip.left, recthEditClip.top);

	return true;
}

// 子ウィンドウの作成
bool CreateChildWindow(HWND hWnd, WPARAM wParam, LPARAM lParam) {
	
	HWND hStaticClipBord = CreateWindowEx(
			WS_EX_WINDOWEDGE,
			_T("STATIC"),
			_T("クリップボード履歴"),
			WS_VISIBLE | WS_CHILD | SS_LEFT,
			Winsize.left + 30, 30, 300, 50,
			hWnd, (HMENU)ID_STATIC_CLIPBORD, ((LPCREATESTRUCT)(lParam))->hInstance, NULL);

	HWND hEditClip1 = CreateWindow(_T("EDIT"), 
		TEXTEDITBOX,
		WS_CHILD | WS_VISIBLE | WS_VSCROLL | ES_LEFT | ES_MULTILINE |ES_AUTOVSCROLL,// |  WS_BORDER,
		Winsize.left + 30, 90, (Winsize.right - 80), Winsize.bottom - Winsize.top - 180,
		hWnd, (HMENU)ID_EDIT_CLIPBORD_1,
		((LPCREATESTRUCT)(lParam))->hInstance, NULL);

	HWND hButtonClear = CreateWindowEx(
		WS_EX_WINDOWEDGE,
		_T("BUTTON"),
		_T("履歴の消去"),
		WS_VISIBLE | WS_CHILD | BS_PUSHBUTTON,
		Winsize.right - 270, 30, 200, 50,
		hWnd, (HMENU)ID_BUTTON_CLEAR, ((LPCREATESTRUCT)(lParam))->hInstance, NULL);

	HWND hButtonEditClear = CreateWindowEx(
		WS_EX_WINDOWEDGE,
		_T("BUTTON"),
		_T("現在の表示内容をクリア"),
		WS_VISIBLE | WS_CHILD | BS_PUSHBUTTON,
		Winsize.right - 600, 30, 300, 50,
		hWnd, (HMENU)ID_BUTTON_EDIT_CLEAR, ((LPCREATESTRUCT)(lParam))->hInstance, NULL);

	return true;
}

// WM_CTLCOLORSTATIC内の処理
long OnCtlColorStatic(WPARAM wParam, LPARAM lParam) {
	int i = GetWindowLong((HWND)lParam, GWL_ID);
	if (i == 0) {
		return -1;
	}
	else if(i == ID_STATIC_CLIPBORD){
		// 色の変更用コードを記述
		SetBkMode((HDC)wParam, TRANSPARENT);
		SetTextColor((HDC)wParam, RGB(20, 20, 20));
		return (long)CreateSolidBrush(RGB(255, 255, 255));
	}
	return true;
}

// フォント設定
bool OnFont(HWND hWnd) {
	// 新しくHFONT型変数を追加する場合は、WM_DESTROYメッセージ内でのDeleteObject(HFONT)を忘れないように。
	hFont1 = CreateFont(
		40, 0, 0, 0,
		FW_BOLD,
		false, false, false,
		SHIFTJIS_CHARSET, OUT_DEFAULT_PRECIS,
		CLIP_DEFAULT_PRECIS, DEFAULT_QUALITY,
		(VARIABLE_PITCH | FF_DONTCARE), _T("メイリオ"));

	hFont2 = CreateFont(
		22, 0, 0, 0,
		FW_MEDIUM,
		false, false, false,
		SHIFTJIS_CHARSET, OUT_DEFAULT_PRECIS,
		CLIP_DEFAULT_PRECIS, DEFAULT_QUALITY,
		(VARIABLE_PITCH | FF_DONTCARE), _T("メイリオ"));

	hFont3 = CreateFont(
		30, 0, 0, 0,
		FW_MEDIUM,
		false, false, false,
		SHIFTJIS_CHARSET, OUT_DEFAULT_PRECIS,
		CLIP_DEFAULT_PRECIS, DEFAULT_QUALITY,
		(VARIABLE_PITCH | FF_DONTCARE), _T("メイリオ"));

	hFontBig = CreateFont(
		30, 0, 0, 0,
		FW_MEDIUM,
		false, false, false,
		SHIFTJIS_CHARSET, OUT_DEFAULT_PRECIS,
		CLIP_DEFAULT_PRECIS, DEFAULT_QUALITY,
		(VARIABLE_PITCH | FF_DONTCARE), _T("メイリオ"));

	hFontSmall = CreateFont(
		16, 0, 0, 0,
		FW_MEDIUM,
		false, false, false,
		SHIFTJIS_CHARSET, OUT_DEFAULT_PRECIS,
		CLIP_DEFAULT_PRECIS, DEFAULT_QUALITY,
		(VARIABLE_PITCH | FF_DONTCARE), _T("メイリオ"));

	// 送信
	SendMessage(GetDlgItem(hWnd, ID_STATIC_CLIPBORD), WM_SETFONT, (WPARAM)hFont1, MAKELPARAM(false, 0));
	SendMessage(GetDlgItem(hWnd, ID_EDIT_CLIPBORD_1), WM_SETFONT, (WPARAM)hFont2, MAKELPARAM(false, 0));
	SendMessage(GetDlgItem(hWnd, ID_BUTTON_CLEAR), WM_SETFONT, (WPARAM)hFont3, MAKELPARAM(false, 0));
	SendMessage(GetDlgItem(hWnd, ID_BUTTON_EDIT_CLEAR), WM_SETFONT, (WPARAM)hFont3, MAKELPARAM(false, 0));
	return true;
}

// 表示内容の更新
bool UpadateClipDisp(HWND hWnd) {
	
	// エディットボックス内の文字列を消去
	SetWindowText(GetDlgItem(hWnd, ID_EDIT_CLIPBORD_1), _T(""));
	// クリップボードの履歴を逆順コピー
	if (clipboardHistory.empty()) {
		SetWindowText(GetDlgItem(hWnd, ID_EDIT_CLIPBORD_1), _T("コピーする対象がまだありません。"));
	}
	else {
		int i = 1;
		for (auto iter = clipboardHistory.rbegin(); iter != clipboardHistory.rend(); ++iter) {
			string clipContent = *iter;
			//string str = clipContent +
				//_T("\r\n---------------------------------------------------------------------------------------------------------------------------------------------------------------------\r\n");
			string str = _T("[") + to_string(i) + _T("]\r\n") + clipContent + _T("\r\n");
			SendMessage(GetDlgItem(hWnd, ID_EDIT_CLIPBORD_1), EM_REPLACESEL, TRUE, (LPARAM)str.c_str());
			i++;
		}
		// スクロールバーの位置を一番上に持っていく
		SendMessage(GetDlgItem(hWnd, ID_EDIT_CLIPBORD_1), WM_VSCROLL, SB_TOP, 0);
	}

	// WM_PAINTの呼び出し
	//RedrawWindow(hWnd, NULL, NULL, RDW_INVALIDATE | RDW_UPDATENOW);

	return true;
}
// クリップボードから画像を保存
bool SaveClipBoardImage(HANDLE hClip) {

	
		LPVOID lpvMem = GlobalLock(hClip);
		if (lpvMem)
		{
			// DIBデータからGDI+のBitmapオブジェクトを作成
			Gdiplus::Bitmap* bitmap = Gdiplus::Bitmap::FromBITMAPINFO((BITMAPINFO*)lpvMem, FindDIBBits(lpvMem));
			if (bitmap)
			{
				bitmap->GetHBITMAP(Color(0, 0, 0, 0), &hBitmap);
				
				clipboardImageHistory.push_back(hBitmap);
				delete bitmap;
			}
		}
		else
		{
			// エラーハンドリング
			DWORD error = GetLastError();
			LPVOID lpMsgBuf{};
			DWORD bufLen = FormatMessage(
				FORMAT_MESSAGE_ALLOCATE_BUFFER |
				FORMAT_MESSAGE_FROM_SYSTEM |
				FORMAT_MESSAGE_IGNORE_INSERTS,
				NULL,
				error,
				MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
				(LPTSTR)&lpMsgBuf,
				0, NULL);
			if (bufLen)
			{
				LPCSTR lpMsgStr = (LPCSTR)lpMsgBuf;
				std::string result(lpMsgStr, lpMsgStr + bufLen);

				MessageBox(NULL, result.c_str(), "Error", MB_OK);

				LocalFree(lpMsgBuf);
			}
		}
	
		GlobalUnlock(hClip);

	return true;
}

bool PaintImages(HDC hdc){
	// 仮想デバイスコンテキストの取得
    hdcMem_paintImages = CreateCompatibleDC(hdc);

	for (HBITMAP hBitmap : clipboardImageHistory){
		BITMAP bm{};

		// ビットマップの幅と高さを格納する
		GetObject(hBitmap, sizeof(bm), &bm);
		HBITMAP hDestBitmap = CreateCompatibleBitmap(hdc, 50, 50); // 新しいサイズ
		HBITMAP hOldBitmap = (HBITMAP)SelectObject(hdcMem_paintImages, hDestBitmap);
		
		// 画像をリサイズして、DCにコピー
		StretchBlt(hdcMem_paintImages, 0, 0, 50, 50, hdc, 0, 0, bm.bmWidth, bm.bmHeight, SRCCOPY);

		// 元のビットマップを仮想DCに再度関連付け（次の画像描画の準備）
		SelectObject(hdcMem_paintImages, hBitmapOld_paintImages);
		DeleteObject(hBitmap);
	}
    // 仮想デバイスコンテキストの解放
	DeleteDC(hdcMem_paintImages);
	return true;
}

//// 得られたGDI+のBitmapからHBITMAPの取得
//bool GdiBitmapToHbitmap(const vector<Gdiplus::Bitmap*>& clipboardHistory) {
//	HBITMAP hBitmap{};
//	for (Gdiplus::Bitmap*  bitmap : clipboardHistory) {
//		bitmap->GetHBITMAP(Color(0, 0, 0, 0), )
//	}
//	return true;
//}

//// Device Independent Bitmap (DIB) 構造の中からピクセルデータをポインタとして取得する
LPVOID FindDIBBits(LPVOID lpDIB)
{
	LPBITMAPINFO pbmi = (LPBITMAPINFO)lpDIB;
	LPVOID pBits;
	if (pbmi->bmiHeader.biSize == sizeof(BITMAPCOREHEADER))
	{
		pBits = (LPVOID)((LPBYTE)pbmi +
			sizeof(BITMAPCOREHEADER) +
			DIBNumColors(lpDIB) * sizeof(RGBTRIPLE));
	}
	else
	{
		pBits = (LPVOID)((LPBYTE)pbmi +
			pbmi->bmiHeader.biSize +
			DIBNumColors(lpDIB) * sizeof(RGBQUAD));
	}
	return pBits;
}
//
// DIBデータの色数を返す
int DIBNumColors(LPVOID lpDIB)
{
	LPBITMAPINFO pbmi = (LPBITMAPINFO)lpDIB;
	if (pbmi->bmiHeader.biClrUsed != 0)
	{
		return pbmi->bmiHeader.biClrUsed;
	}
	else if (pbmi->bmiHeader.biBitCount <= 8)
	{
		return (int)(1 << pbmi->bmiHeader.biBitCount);
	}
	else
	{
		return 0;
	}
}

//// 描画ターゲットの設定
//bool CreateD2DTarget(HWND hWnd) {
//	// ウィンドウのクライアント領域のサイズを取得
//	RECT rect;
//	GetClientRect(hWnd, &rect);
//	int width = rect.right - rect.left;
//	int height = rect.bottom - rect.top;
//
//	// ビットマップのプロパティを設定
//	D2D1_BITMAP_PROPERTIES1 bitmapProperties =
//		D2D1::BitmapProperties1(
//			D2D1_BITMAP_OPTIONS_TARGET,
//			D2D1::PixelFormat(DXGI_FORMAT_B8G8R8A8_UNORM, D2D1_ALPHA_MODE_PREMULTIPLIED)
//		);
//
//	// ビットマップを作成
//	ID2D1Bitmap1* pTargetBitmap;
//	d2dContext->CreateBitmap(
//		D2D1::SizeU(width, height),
//		nullptr, // initial data
//		0,       // pitch
//		&bitmapProperties,
//		&pTargetBitmap
//	);
//
//	// ビットマップを描画ターゲットとして設定
//	d2dContext->SetTarget(pTargetBitmap);
//
//	// 描画ターゲットビットマップをリリース
//	pTargetBitmap->Release();
//
//	return true;
//}

BOOL DesktopCenterWindow(HWND hWnd)
{
	RECT    rc1{};        // デスクトップ領域
	RECT    rc2;        // ウインドウ領域
	INT     cx, cy;     // ウインドウ位置
	INT     sx, sy;     // ウインドウサイズ

	// サイズの取得
	GetMonitorRect(&rc1);                            // デスクトップのサイズ
	GetWindowRect(hWnd, &rc2);                            // ウインドウのサイズ
	// いろいろと計算
	sx = (rc2.right - rc2.left);                            // ウインドウの横幅
	sy = (rc2.bottom - rc2.top);                            // ウインドウの高さ
	cx = (((rc1.right - rc1.left) - sx) / 2 + rc1.left);    // 横方向の中央座標軸
	cy = (((rc1.bottom - rc1.top) - sy) / 2 + rc1.top);     // 縦方向の中央座標軸
	// 画面中央に移動
	return SetWindowPos(hWnd, NULL, cx, cy, 0, 0, (SWP_NOSIZE | SWP_NOZORDER | SWP_NOOWNERZORDER));
}