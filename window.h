#pragma once
////////////
// �萔
////////////

#define GetMonitorRect(rc)  SystemParametersInfo(SPI_GETWORKAREA,0,rc,0)  // ���[�N�G���A�̈�̎擾

#define ID_BUTTON_1 2001

#define ID_STATIC_CLIPBORD 3001

#define ID_EDIT_CLIPBORD_1  4001

#define ID_BUTTON_CLEAR 5001
#define ID_BUTTON_EDIT_CLEAR 5002

#define TEXTEDITBOX _T("�������R�s�[����ƁA�����ɃN���b�v�{�[�h�̗������\������܂��B\r\n�t�@�C�����h���b�O���h���b�v�����ꍇ�́A�����Ƀt�@�C���̏ꏊ��\�����܂��B")