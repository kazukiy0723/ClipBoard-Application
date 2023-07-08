//////
//1.�E�B���h�E
//////
#include<windows.h>
#include<tchar.h>
#include<string>
#include<vector>
#include<gdiplus.h>
#include<gdipluscolor.h>

// ����w�b�_�t�@�C���̃��[�h
#include"main.h"
#include"window.h"
#include "resource.h"
#include"file.h"
#include"VersionInfo.h"

//////////////////////////////
// �r�W���A���X�^�C���L����
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
// �E�B���h�E�̔w�i�F�̒�`
/////////////////////////////
const HBRUSH BackGround_clear = CreateSolidBrush(RGB(255, 255, 255));
const HBRUSH BackGround_dark = CreateSolidBrush(RGB(0, 0, 0));

/////////////////////////////
// RECT�\���̕ϐ��̏�����
/////////////////////////////
RECT Winsize = { 0,0,1100,750 };

// �t�H���g�p
HFONT hFont1, hFont2, hFont3, hFontBig, hFontSmall;

// WM_PAINT
HDC hMemDC;
HDC hdcMem_paintImages;
HBITMAP hBitmap_paintImages;
HBITMAP hBitmapOld_paintImages;
HGDIOBJ hOld;

// �x�N�^�[
vector<string> clipboardHistory;
vector<HBITMAP> clipboardImageHistory;

ULONG_PTR gdiplusToken;

// Bitmap
HBITMAP hBitmap{};

// �u���V
HPEN hBluePen = CreatePen(PS_SOLID, 4, RGB(0, 203, 152));
HGDIOBJ hOldPen{};

//WinMain�֐�
int WINAPI WinMain(_In_ HINSTANCE hThisInst, _In_opt_ HINSTANCE hPrevInst, _In_ LPSTR lpszArgs, _In_ int nWinMode)
{
	HWND		hWnd{};
	MSG			msg;
	WNDCLASSEX	wcl{};
	HACCEL haccel{};

	// �A�v���P�[�V�����̓�d�N���h�~
	HANDLE hMutex{};
	if (hMutex = CreateMutex(NULL, TRUE, _T("uniqueApplicationName"))) {
		if (GetLastError() == ERROR_ALREADY_EXISTS) {
			// �A�v�������s���ł���ꍇ�͏I������B
			MessageBox(NULL, _T("���ɋN�����Ă��܂��B"), _T("�G���["), MB_OK);
			CloseHandle(hMutex);
			return 0;
		}
	}
	

	//�E�B���h�E�N���X�̒�`
	wcl.cbSize = sizeof(WNDCLASSEX);				//WNDCLASSEX�\���̂̃T�C�Y
	wcl.style = 0;									//�E�B���h�E�N���X�X�^�C��
	wcl.lpfnWndProc = windowfunc;					//�E�B���h�E�֐�
	wcl.cbClsExtra = 0;								//�E�B���h�E�N���X�̃G�L�X�g��
	wcl.cbWndExtra = 0;								//�E�B���h�E�C���X�^���X�̃G�L�X�g��
	wcl.hInstance = hThisInst;						//���̃v���O�����̃C���X�^���X�ւ̃n���h��
	wcl.hIcon = LoadIcon(NULL, IDI_APPLICATION);	//�A�C�R���ւ̃n���h��
	wcl.hCursor = LoadCursor(NULL, IDC_ARROW);		//�J�[�\���ւ̃n���h��
	wcl.hbrBackground = (HBRUSH)COLOR_WINDOW;		//�w�i�u���V�ւ̃n���h��
	wcl.lpszMenuName = MAKEINTRESOURCE(IDR_MENU1);						//���j���[
	wcl.lpszClassName = szWinName;					//�E�B���h�E�N���X��
	wcl.hIconSm = LoadIcon(NULL, IDI_WINLOGO);		//�X���[���A�C�R���ւ̃n���h��

	//�E�B���h�E�N���X�̓o�^
	if (!RegisterClassEx(&wcl)) {
		return(0);
	}

	//�E�B���h�E�̐���
	hWnd = CreateWindow(
		szWinName,				//�E�B���h�E�N���X��
		szWinName,				//�E�B���h�E��
		WS_OVERLAPPED | WS_CAPTION | WS_SYSMENU | WS_MINIMIZEBOX, //�E�B���h�E�X�^�C��
		CW_USEDEFAULT,			//x���W
		CW_USEDEFAULT,			//y���W
		Winsize.right,			//��
		Winsize.bottom,			//����
		HWND_DESKTOP,			//�e�E�B���h�E�ւ̃n���h��
		NULL,					//���j���[�ւ̃n���h��
		hThisInst,				//���̃v���O�����̃C���X�^���X�ւ̃n���h��
		NULL					//�ǉ�����
	);

	// �h���b�O���h���b�v�t�@�C���ւ̑Ή�
	DragAcceptFiles(hWnd, TRUE);

	// �N���b�v�{�[�h�̕ύX�����m���郊�X�i�[��o�^
	if (!AddClipboardFormatListener(hWnd)) {
		MessageBox(hWnd, _T("�N���b�v�{�[�h�`�����X�i�[�̓o�^���ł��܂���ł����B\r\nOS������ۏؑΏۊO�ł���\��������܂��B"), _T("Error"), MB_OK);
	}

	//�E�B���h�E�̕\��
	ShowWindow(hWnd, nWinMode);
	UpdateWindow(hWnd);
	
	// �A�v���̃A�C�R����ύX
	HICON hIcon;
	hIcon = (HICON)LoadImage(hThisInst, MAKEINTRESOURCE(IDI_ICON1), IMAGE_ICON, 48, 48, 0);
	SendMessage(hWnd, WM_SETICON, ICON_SMALL, (LPARAM)hIcon);

	//�L�[�{�[�h�A�N�Z�����[�^�̃��[�h
	haccel = LoadAccelerators(hThisInst, MAKEINTRESOURCE(IDR_ACCELERATOR1));

	//���b�Z�[�W���[�v�̐���
	while (GetMessage(&msg, NULL, 0, 0)) {
		if (!TranslateAccelerator(hWnd, haccel, &msg)) {
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}
	}
	// �~���[�e�b�N�X�����
	if (hMutex) {
		CloseHandle(hMutex);
	}
	
	return((int)msg.wParam);
}

//�E�B���h�E�֐�
LRESULT CALLBACK windowfunc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	switch (uMsg) {
	case WM_DESTROY:
		// ��n��
		// ���\�[�X�̉��
		DeleteDC(hdcMem_paintImages);
		// �u���V�̍폜
		SelectObject(hMemDC, hOldPen);
		DeleteObject(hBluePen);
		// �N���b�v�{�[�h�`�����X�i�[�̉���
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

		// ������DC����DC�ɃR�s�[
		BitBlt(ps.hdc,
			ps.rcPaint.left, ps.rcPaint.top,
			ps.rcPaint.right - ps.rcPaint.left, ps.rcPaint.bottom - ps.rcPaint.top,
			hMemDC,
			ps.rcPaint.left, ps.rcPaint.top,
			SRCCOPY);

		// �N���b�v�{�[�h�̉摜���f�B�X�v���C�ɃR�s�[
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
		// �E�B���h�E�Ƀt�@�C�����h���b�v���ꂽ
		HDROP hDrop = (HDROP)wParam;
		TCHAR IpszFilePath[MAX_PATH] = { 0 };
		UINT uiFileCount = DragQueryFile(hDrop, 0xFFFFFFFF, NULL, 0);
		for (size_t i = 0; i < uiFileCount; i++)
		{
			DragQueryFile(hDrop, i, IpszFilePath, MAX_PATH);
			string FilePath = IpszFilePath;
			// �N���b�v�{�[�h�Ƀp�X�����R�s�[
			clipboardHistory.push_back(FilePath);
		}
		DragFinish(hDrop);
		// �E�B���h�E�̕\�����e���X�V
		UpadateClipDisp(hWnd);
	}

		break;
	case WM_CLIPBOARDUPDATE:
		// �N���b�v�{�[�h���ύX���ꂽ
		OnClipBoardUpdate();
		// �G�f�B�b�g�{�b�N�X�̍X�V
		UpadateClipDisp(hWnd);
		break;
	case WM_CTLCOLORSTATIC:
		return (OnCtlColorStatic(wParam, lParam));
	case WM_ERASEBKGND:
		// �����������Ȃ�(��ʂ̂�����h�~�j
		return 1;
		break;

	case WM_CLOSE:
		if (MessageBox(hWnd, _T("�I�����܂����H"), _T("�I���m�F"), MB_YESNO | MB_ICONQUESTION) == IDYES) {
			DestroyWindow(hWnd);
		}
		break;
	default:
		return(DefWindowProc(hWnd, uMsg, wParam, lParam));
	}
	return(0);
}

// WM_CLIPBOARDUPDATE���̏���
bool OnClipBoardUpdate() {
	
	HANDLE hClip{};

	if (OpenClipboard(NULL))
	{
		if (IsClipboardFormatAvailable(CF_TEXT)) {
			hClip = GetClipboardData(CF_TEXT);
			if (hClip) {
				// �N���b�v�r�[�h�Ƀe�L�X�g���R�s�[���ꂽ�ꍇ
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
		//	// DIB�`���̏ꍇ
		//	hClip = GetClipboardData(CF_DIB);
		//	if (hClip) {
		//		SaveClipBoardImage(hClip);
		//	}
		//}
		//else if (IsClipboardFormatAvailable(CF_BITMAP)) {
		//	// BITMAP�`���̏ꍇ
		//	HBITMAP hBitmap_local = (HBITMAP)GetClipboardData(CF_BITMAP);
		//	if (hBitmap_local) {
		//		clipboardImageHistory.push_back(hBitmap);
		//	}
		//	
		//}
		//else
		//{
		//		MessageBox(NULL, _T("�R�s�[���̌`�����s���ł��B�i�\�[�X�R�[�h�v�m�F�j"), NULL, MB_OK);
		//}
		//
	}
	// �N���b�v�{�[�h�̃N���[�Y
	CloseClipboard();

	return true;
}

// WM_COMMAND���̏���
bool OnCommand(HWND hWnd, WPARAM wParam, LPARAM lParam) {
	switch (LOWORD(wParam)){
	case ID_BUTTON_CLEAR:
		if (MessageBox(hWnd, _T("�N���b�v�{�[�h�̗�����S�ď������܂��B\r\n���s���܂����H"), _T("�����m�F"), MB_YESNO | MB_ICONQUESTION) == IDYES) {
			if (!clipboardHistory.empty()) {
				clipboardHistory.clear();
			}else {
				MessageBox(hWnd, _T("�N���b�v�{�[�h����ł��B"), NULL, MB_OK);
			}
			
			SetWindowText(GetDlgItem(hWnd, ID_EDIT_CLIPBORD_1), TEXTEDITBOX);
		}
		break;
	case ID_MENU_CLIP:
		UpadateClipDisp(hWnd);
		break;
		
	case ID_BUTTON_EDIT_CLEAR:
		if (HIWORD(wParam) == BN_CLICKED) {
			// �G�f�B�b�g�{�b�N�X���̕����������
			SetWindowText(GetDlgItem(hWnd, ID_EDIT_CLIPBORD_1), _T(""));
		}
		break;
	case ID_EXIT:
		if (MessageBox(hWnd, _T("�I�����܂����H"), _T("�I���m�F"), MB_YESNO | MB_ICONQUESTION) == IDYES) {
			DestroyWindow(hWnd);
		}
		break;
	case ID_MENU_COPY_NEW:
		// �ŐV�̃R�s�[���e�݂̂��폜
		if (!clipboardHistory.empty()) {
			// �N���b�v�{�[�h����łȂ��ꍇ
			clipboardHistory.pop_back();
			MessageBox(hWnd, _T("�Ō�̗v�f���폜���܂����B"), _T("�폜����"), MB_OK);
			UpadateClipDisp(hWnd);
		}else{
			MessageBox(hWnd, _T("�N���b�v�{�[�h�͋�ł��B"), _T("�G���["), MB_OK);
		}
		break;
	case ID_MENU_HELP:
	{
		string temp2 = _T("�g�p�\��OS: Windows Vista�ȍ~");
		string temp3 = _T("�g����: ���j���[�o�[�ɂ���u�X�V�v���N���b�N���邩�A�L�[�{�[�h��ŁuCtrl + V�v�Ƒł��Ɓ@�@�@ �ŃN���b�v�{�[�h�̗�����\�������邱�Ƃ��ł��܂��B\r\n�@�@�@ �\����������A���e��ҏW���邱�Ƃ��\�ł��B\r\n\r\n");
		string temp4 = _T("�����̏����{�^��: �{�^���������ƁA�A�v����ŕۑ��ς݂̃N���b�v�{�[�h�̗������S�ā@�@�@�@�@�@�@  ��������܂��B�������邱�Ƃ͂ł��܂���\r\n\r\n");
		string temp5 = _T("���݂̕\�����e���N���A: �\�����e���������܂��B�N���b�v�{�[�h�̗����͏���\r\n�@�@�@�@�@�@�@�@ �@�@����܂���B\r\n\r\n");
		string all =  temp3 + temp4 + temp5 +  temp2;
		MessageBox(hWnd, all.c_str(), _T("�w���v"), MB_OK);
	}
		break;
	case ID_MENU_USE:
	{
		string temp1 = _T("�ǂ�ȃA�v��: �N���b�v�{�[�h�ւ̃R�s�[������\������ȑf��Windows��p\r\n�@�@�@�@�@�A�v���P�[�V�����ł��B\r\n\r\n");
		string temp2 = _T("�A�v���̗ǂ��Ƃ���: �N����ɃR�s�[�������e��S�Ċm�F���邱�Ƃ��\�ł��B�����\r\n�@�@�@�@�@�@ �@�N���b�v�{�[�h�ɃR�s�[�������e���㏑�����Ă��܂����ꍇ�ł��ߋ���\r\n�@�@�@�@�@�@ �@�R�s�[���e�����邱�Ƃ��ł��܂��B\r\n�@�@�@�@�@�@ �@Windows�W���A�v�������傫�ȉ�ʂŌ���邽�߁A\r\n�@�@�@�@�@�@ �@��ڂŃR�s�[�������m�F�ł���_�����̃A�v���̗D�ʐ��ł��B");
		
		string all = temp1 + temp2;
		MessageBox(hWnd, all.c_str(), _T("Hi"), MB_OK);
	}
	break;
	case ID_MENU_FILE:
		// ���O��t���ĕۑ��p�̃_�C�A���O�{�b�N�X
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

// WM_CREATE���̏���
bool OnCreate(HWND hWnd, WPARAM wParam, LPARAM lParam) {
	// GDI+�̏�����
	GdiplusStartupInput gdiplusStartupInput;
	GdiplusStartup(&gdiplusToken, &gdiplusStartupInput, NULL);
	// �q�E�B���h�E�̍쐬
	CreateChildWindow(hWnd, wParam, lParam);
	// �������f�o�C�X�R���e�L�X�g�̎擾
	GetWindowMemDC(hWnd);
	// �������f�o�C�X�R���e�L�X�g�ւ̕`��
	PaintToMemDC(hWnd);
	// �t�H���g�̓K�p
	OnFont(hWnd);
	// �f�X�N�g�b�v�𒆉��ɔz�u
	DesktopCenterWindow(hWnd);
	// ���s�t�@�C���̃p�X���̎擾
	TCHAR lpfileName[MAX_PATH];
	GetModuleFileName(NULL, lpfileName, sizeof(lpfileName));
	// �E�B���h�E�^�C�g���̕ύX
	SetWindowText(hWnd, lpfileName);
	return true;
}

// �������f�o�C�X�R���e�L�X�g�`��̎��O����
bool GetWindowMemDC(HWND hWnd) {

	HDC hDC;
	HBITMAP hBitmap;

	// �f�o�C�X�R���e�L�X�g�̎擾
	hDC = GetDC(hWnd);
	// �������f�o�C�X�R���e�L�X�g�̎擾
	hMemDC = CreateCompatibleDC(hDC);
	// �r�b�g�}�b�v�n���h���̎擾
	hBitmap = CreateCompatibleBitmap(hDC, Winsize.right, Winsize.bottom);
	// �f�o�C�X�R���e�L�X�g�̉��
	ReleaseDC(hWnd, hDC);
	// ������DC�Ƀr�b�g�}�b�v�������
	hOld = SelectObject(hMemDC, hBitmap);
	// �r�b�g�}�b�v�̍폜
	DeleteObject(hBitmap);

	return true;
}

// �������f�o�C�X�R���e�L�X�g�ւ̕`��
bool PaintToMemDC(HWND hWnd) {
	// �w�i�̓h��Ԃ�
	FillRect(hMemDC, &Winsize, BackGround_clear);
	// �G�f�B�b�g�{�b�N�X�̘g
	RECT recthEditClip = { Winsize.left + 30 - 2, 90 - 2, (Winsize.right - 50) + 2, Winsize.bottom - Winsize.top - 90 + 2 };
	
	hOldPen = SelectObject(hMemDC, hBluePen);
	
	// �`��J�n�ʒu��ݒ�
	MoveToEx(hMemDC, recthEditClip.left, recthEditClip.top, NULL);

	// ���ォ��E��ւ̐�
	LineTo(hMemDC, recthEditClip.right, recthEditClip.top);

	// �E�ォ��E���ւ̐�
	LineTo(hMemDC, recthEditClip.right, recthEditClip.bottom);

	// �E�����獶���ւ̐�
	LineTo(hMemDC, recthEditClip.left, recthEditClip.bottom);

	// �������獶��ւ̐�
	LineTo(hMemDC, recthEditClip.left, recthEditClip.top);

	return true;
}

// �q�E�B���h�E�̍쐬
bool CreateChildWindow(HWND hWnd, WPARAM wParam, LPARAM lParam) {
	
	HWND hStaticClipBord = CreateWindowEx(
			WS_EX_WINDOWEDGE,
			_T("STATIC"),
			_T("�N���b�v�{�[�h����"),
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
		_T("�����̏���"),
		WS_VISIBLE | WS_CHILD | BS_PUSHBUTTON,
		Winsize.right - 270, 30, 200, 50,
		hWnd, (HMENU)ID_BUTTON_CLEAR, ((LPCREATESTRUCT)(lParam))->hInstance, NULL);

	HWND hButtonEditClear = CreateWindowEx(
		WS_EX_WINDOWEDGE,
		_T("BUTTON"),
		_T("���݂̕\�����e���N���A"),
		WS_VISIBLE | WS_CHILD | BS_PUSHBUTTON,
		Winsize.right - 600, 30, 300, 50,
		hWnd, (HMENU)ID_BUTTON_EDIT_CLEAR, ((LPCREATESTRUCT)(lParam))->hInstance, NULL);

	return true;
}

// WM_CTLCOLORSTATIC���̏���
long OnCtlColorStatic(WPARAM wParam, LPARAM lParam) {
	int i = GetWindowLong((HWND)lParam, GWL_ID);
	if (i == 0) {
		return -1;
	}
	else if(i == ID_STATIC_CLIPBORD){
		// �F�̕ύX�p�R�[�h���L�q
		SetBkMode((HDC)wParam, TRANSPARENT);
		SetTextColor((HDC)wParam, RGB(20, 20, 20));
		return (long)CreateSolidBrush(RGB(255, 255, 255));
	}
	return true;
}

// �t�H���g�ݒ�
bool OnFont(HWND hWnd) {
	// �V����HFONT�^�ϐ���ǉ�����ꍇ�́AWM_DESTROY���b�Z�[�W���ł�DeleteObject(HFONT)��Y��Ȃ��悤�ɁB
	hFont1 = CreateFont(
		40, 0, 0, 0,
		FW_BOLD,
		false, false, false,
		SHIFTJIS_CHARSET, OUT_DEFAULT_PRECIS,
		CLIP_DEFAULT_PRECIS, DEFAULT_QUALITY,
		(VARIABLE_PITCH | FF_DONTCARE), _T("���C���I"));

	hFont2 = CreateFont(
		22, 0, 0, 0,
		FW_MEDIUM,
		false, false, false,
		SHIFTJIS_CHARSET, OUT_DEFAULT_PRECIS,
		CLIP_DEFAULT_PRECIS, DEFAULT_QUALITY,
		(VARIABLE_PITCH | FF_DONTCARE), _T("���C���I"));

	hFont3 = CreateFont(
		30, 0, 0, 0,
		FW_MEDIUM,
		false, false, false,
		SHIFTJIS_CHARSET, OUT_DEFAULT_PRECIS,
		CLIP_DEFAULT_PRECIS, DEFAULT_QUALITY,
		(VARIABLE_PITCH | FF_DONTCARE), _T("���C���I"));

	hFontBig = CreateFont(
		30, 0, 0, 0,
		FW_MEDIUM,
		false, false, false,
		SHIFTJIS_CHARSET, OUT_DEFAULT_PRECIS,
		CLIP_DEFAULT_PRECIS, DEFAULT_QUALITY,
		(VARIABLE_PITCH | FF_DONTCARE), _T("���C���I"));

	hFontSmall = CreateFont(
		16, 0, 0, 0,
		FW_MEDIUM,
		false, false, false,
		SHIFTJIS_CHARSET, OUT_DEFAULT_PRECIS,
		CLIP_DEFAULT_PRECIS, DEFAULT_QUALITY,
		(VARIABLE_PITCH | FF_DONTCARE), _T("���C���I"));

	// ���M
	SendMessage(GetDlgItem(hWnd, ID_STATIC_CLIPBORD), WM_SETFONT, (WPARAM)hFont1, MAKELPARAM(false, 0));
	SendMessage(GetDlgItem(hWnd, ID_EDIT_CLIPBORD_1), WM_SETFONT, (WPARAM)hFont2, MAKELPARAM(false, 0));
	SendMessage(GetDlgItem(hWnd, ID_BUTTON_CLEAR), WM_SETFONT, (WPARAM)hFont3, MAKELPARAM(false, 0));
	SendMessage(GetDlgItem(hWnd, ID_BUTTON_EDIT_CLEAR), WM_SETFONT, (WPARAM)hFont3, MAKELPARAM(false, 0));
	return true;
}

// �\�����e�̍X�V
bool UpadateClipDisp(HWND hWnd) {
	
	// �G�f�B�b�g�{�b�N�X���̕����������
	SetWindowText(GetDlgItem(hWnd, ID_EDIT_CLIPBORD_1), _T(""));
	// �N���b�v�{�[�h�̗������t���R�s�[
	if (clipboardHistory.empty()) {
		SetWindowText(GetDlgItem(hWnd, ID_EDIT_CLIPBORD_1), _T("�R�s�[����Ώۂ��܂�����܂���B"));
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
		// �X�N���[���o�[�̈ʒu����ԏ�Ɏ����Ă���
		SendMessage(GetDlgItem(hWnd, ID_EDIT_CLIPBORD_1), WM_VSCROLL, SB_TOP, 0);
	}

	// WM_PAINT�̌Ăяo��
	//RedrawWindow(hWnd, NULL, NULL, RDW_INVALIDATE | RDW_UPDATENOW);

	return true;
}
// �N���b�v�{�[�h����摜��ۑ�
bool SaveClipBoardImage(HANDLE hClip) {

	
		LPVOID lpvMem = GlobalLock(hClip);
		if (lpvMem)
		{
			// DIB�f�[�^����GDI+��Bitmap�I�u�W�F�N�g���쐬
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
			// �G���[�n���h�����O
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
	// ���z�f�o�C�X�R���e�L�X�g�̎擾
    hdcMem_paintImages = CreateCompatibleDC(hdc);

	for (HBITMAP hBitmap : clipboardImageHistory){
		BITMAP bm{};

		// �r�b�g�}�b�v�̕��ƍ������i�[����
		GetObject(hBitmap, sizeof(bm), &bm);
		HBITMAP hDestBitmap = CreateCompatibleBitmap(hdc, 50, 50); // �V�����T�C�Y
		HBITMAP hOldBitmap = (HBITMAP)SelectObject(hdcMem_paintImages, hDestBitmap);
		
		// �摜�����T�C�Y���āADC�ɃR�s�[
		StretchBlt(hdcMem_paintImages, 0, 0, 50, 50, hdc, 0, 0, bm.bmWidth, bm.bmHeight, SRCCOPY);

		// ���̃r�b�g�}�b�v�����zDC�ɍēx�֘A�t���i���̉摜�`��̏����j
		SelectObject(hdcMem_paintImages, hBitmapOld_paintImages);
		DeleteObject(hBitmap);
	}
    // ���z�f�o�C�X�R���e�L�X�g�̉��
	DeleteDC(hdcMem_paintImages);
	return true;
}

//// ����ꂽGDI+��Bitmap����HBITMAP�̎擾
//bool GdiBitmapToHbitmap(const vector<Gdiplus::Bitmap*>& clipboardHistory) {
//	HBITMAP hBitmap{};
//	for (Gdiplus::Bitmap*  bitmap : clipboardHistory) {
//		bitmap->GetHBITMAP(Color(0, 0, 0, 0), )
//	}
//	return true;
//}

//// Device Independent Bitmap (DIB) �\���̒�����s�N�Z���f�[�^���|�C���^�Ƃ��Ď擾����
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
// DIB�f�[�^�̐F����Ԃ�
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

//// �`��^�[�Q�b�g�̐ݒ�
//bool CreateD2DTarget(HWND hWnd) {
//	// �E�B���h�E�̃N���C�A���g�̈�̃T�C�Y���擾
//	RECT rect;
//	GetClientRect(hWnd, &rect);
//	int width = rect.right - rect.left;
//	int height = rect.bottom - rect.top;
//
//	// �r�b�g�}�b�v�̃v���p�e�B��ݒ�
//	D2D1_BITMAP_PROPERTIES1 bitmapProperties =
//		D2D1::BitmapProperties1(
//			D2D1_BITMAP_OPTIONS_TARGET,
//			D2D1::PixelFormat(DXGI_FORMAT_B8G8R8A8_UNORM, D2D1_ALPHA_MODE_PREMULTIPLIED)
//		);
//
//	// �r�b�g�}�b�v���쐬
//	ID2D1Bitmap1* pTargetBitmap;
//	d2dContext->CreateBitmap(
//		D2D1::SizeU(width, height),
//		nullptr, // initial data
//		0,       // pitch
//		&bitmapProperties,
//		&pTargetBitmap
//	);
//
//	// �r�b�g�}�b�v��`��^�[�Q�b�g�Ƃ��Đݒ�
//	d2dContext->SetTarget(pTargetBitmap);
//
//	// �`��^�[�Q�b�g�r�b�g�}�b�v�������[�X
//	pTargetBitmap->Release();
//
//	return true;
//}

BOOL DesktopCenterWindow(HWND hWnd)
{
	RECT    rc1{};        // �f�X�N�g�b�v�̈�
	RECT    rc2;        // �E�C���h�E�̈�
	INT     cx, cy;     // �E�C���h�E�ʒu
	INT     sx, sy;     // �E�C���h�E�T�C�Y

	// �T�C�Y�̎擾
	GetMonitorRect(&rc1);                            // �f�X�N�g�b�v�̃T�C�Y
	GetWindowRect(hWnd, &rc2);                            // �E�C���h�E�̃T�C�Y
	// ���낢��ƌv�Z
	sx = (rc2.right - rc2.left);                            // �E�C���h�E�̉���
	sy = (rc2.bottom - rc2.top);                            // �E�C���h�E�̍���
	cx = (((rc1.right - rc1.left) - sx) / 2 + rc1.left);    // �������̒������W��
	cy = (((rc1.bottom - rc1.top) - sy) / 2 + rc1.top);     // �c�����̒������W��
	// ��ʒ����Ɉړ�
	return SetWindowPos(hWnd, NULL, cx, cy, 0, 0, (SWP_NOSIZE | SWP_NOZORDER | SWP_NOOWNERZORDER));
}