#include<Windows.h>
#include<tchar.h>
#include<string>
#include<vector>

using namespace std;

HANDLE hFile;
static char FileName[MAX_PATH];
DWORD dWFileSizeByte;

// 共有
extern vector<string> clipboardHistory;

// 名前を付けて保存ダイアログボックスの作成
bool SelectFile(HWND hWnd) {

	static OPENFILENAME ofn;

	memset(&ofn, 0, sizeof(ofn));
	memset(&FileName, 0, sizeof(FileName));

	// OPENFILENAME 構造体の初期化
	ofn.lStructSize = sizeof(ofn);
	ofn.hwndOwner = hWnd;
	ofn.lpstrFilter = _T("Text Files(*.txt)\0*.txt\0All Files(*.*)\0*.*\0\0"); 
	ofn.lpstrFile = FileName;
	ofn.nMaxFile = MAX_PATH;
	ofn.lpstrDefExt = _T("txt");
	ofn.Flags = OFN_HIDEREADONLY | OFN_OVERWRITEPROMPT | OFN_PATHMUSTEXIST;
	ofn.lpstrTitle = "名前を付けて保存";

	if (!GetSaveFileName(&ofn)) {
		MessageBox(hWnd, _T("ファイルを作成できませんでした。"), _T("エラー"), MB_OK | MB_ICONERROR);
		return false;
	}
	
	return true;
}

///////////////////////
// ファイルへのクリップボード履歴の書き込み
///////////////////////
bool file_open(HWND hWnd) {

	hFile = CreateFile(
		FileName,        // ファイルの名前
		GENERIC_READ | GENERIC_WRITE,   // 書き込みモード
		0,               // 共有モード 
		NULL,            // セキュリティ
		OPEN_ALWAYS,   // 上書き保存
		FILE_ATTRIBUTE_NORMAL,
		NULL
	);
	if (hFile == INVALID_HANDLE_VALUE) {
		MessageBox(NULL, _T("ファイルの作成に失敗しました。"), _T("エラー"), MB_OK);
		return false;
	}
	// ファイルサイズの取得
	dWFileSizeByte = GetFileSize(hFile, NULL);

	// クリップボードの内容をstring型変数に保存
	string ClipBord = _T("");
	string ClipContents;
	for (auto iter = clipboardHistory.rbegin(); iter != clipboardHistory.rend(); ++iter) {
		ClipContents = *iter + _T("-----------------------------------------");
		ClipBord += ClipContents;
	}

	// ファイルへの書き込み
	WriteFile(hFile, ClipBord.data(), ClipBord.length(), &dWFileSizeByte, NULL);

	// ファイルのクローズ
	CloseHandle(hFile);

	return true;
}
