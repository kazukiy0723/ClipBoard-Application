
#pragma once
////////////////////
//�v���g�^�C�v�錾
////////////////////
#include<Windows.h>

// WM_CLIPBORADUPDATE���̏���
bool OnClipBoardUpdate();

// WM_COMMAND���̏���
bool OnCommand(HWND, WPARAM, LPARAM);

// WM_CREATE���̏���
bool OnCreate(HWND, WPARAM, LPARAM);

// �q�E�B���h�E�̍쐬
bool CreateChildWindow(HWND, WPARAM, LPARAM);

// WM_CTLCOLORSTATIC���̏���
long OnCtlColorStatic(WPARAM, LPARAM);

// �������f�o�C�X�R���e�L�X�g�`��̎��O����
bool GetWindowMemDC(HWND);

// �������f�o�C�X�R���e�L�X�g�ւ̕`��
bool PaintToMemDC(HWND);

// �t�H���g�̒�`
bool OnFont(HWND);

BOOL DesktopCenterWindow(HWND);
//
LPVOID FindDIBBits(LPVOID);
//
int DIBNumColors(LPVOID);

bool PaintImages(HDC);

bool SaveClipBoardImage(HANDLE);

bool UpadateClipDisp(HWND);