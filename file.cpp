#include<Windows.h>
#include<tchar.h>
#include<string>
#include<vector>

using namespace std;

HANDLE hFile;
static char FileName[MAX_PATH];
DWORD dWFileSizeByte;

// ���L
extern vector<string> clipboardHistory;

// ���O��t���ĕۑ��_�C�A���O�{�b�N�X�̍쐬
bool SelectFile(HWND hWnd) {

	static OPENFILENAME ofn;

	memset(&ofn, 0, sizeof(ofn));
	memset(&FileName, 0, sizeof(FileName));

	// OPENFILENAME �\���̂̏�����
	ofn.lStructSize = sizeof(ofn);
	ofn.hwndOwner = hWnd;
	ofn.lpstrFilter = _T("Text Files(*.txt)\0*.txt\0All Files(*.*)\0*.*\0\0"); 
	ofn.lpstrFile = FileName;
	ofn.nMaxFile = MAX_PATH;
	ofn.lpstrDefExt = _T("txt");
	ofn.Flags = OFN_HIDEREADONLY | OFN_OVERWRITEPROMPT | OFN_PATHMUSTEXIST;
	ofn.lpstrTitle = "���O��t���ĕۑ�";

	if (!GetSaveFileName(&ofn)) {
		MessageBox(hWnd, _T("�t�@�C�����쐬�ł��܂���ł����B"), _T("�G���["), MB_OK | MB_ICONERROR);
		return false;
	}
	
	return true;
}

///////////////////////
// �t�@�C���ւ̃N���b�v�{�[�h�����̏�������
///////////////////////
bool file_open(HWND hWnd) {

	hFile = CreateFile(
		FileName,        // �t�@�C���̖��O
		GENERIC_READ | GENERIC_WRITE,   // �������݃��[�h
		0,               // ���L���[�h 
		NULL,            // �Z�L�����e�B
		OPEN_ALWAYS,   // �㏑���ۑ�
		FILE_ATTRIBUTE_NORMAL,
		NULL
	);
	if (hFile == INVALID_HANDLE_VALUE) {
		MessageBox(NULL, _T("�t�@�C���̍쐬�Ɏ��s���܂����B"), _T("�G���["), MB_OK);
		return false;
	}
	// �t�@�C���T�C�Y�̎擾
	dWFileSizeByte = GetFileSize(hFile, NULL);

	// �N���b�v�{�[�h�̓��e��string�^�ϐ��ɕۑ�
	string ClipBord = _T("");
	string ClipContents;
	for (auto iter = clipboardHistory.rbegin(); iter != clipboardHistory.rend(); ++iter) {
		ClipContents = *iter + _T("-----------------------------------------");
		ClipBord += ClipContents;
	}

	// �t�@�C���ւ̏�������
	WriteFile(hFile, ClipBord.data(), ClipBord.length(), &dWFileSizeByte, NULL);

	// �t�@�C���̃N���[�Y
	CloseHandle(hFile);

	return true;
}
