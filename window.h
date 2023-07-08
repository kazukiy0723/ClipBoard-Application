#pragma once
////////////
// 定数
////////////

#define GetMonitorRect(rc)  SystemParametersInfo(SPI_GETWORKAREA,0,rc,0)  // ワークエリア領域の取得

#define ID_BUTTON_1 2001

#define ID_STATIC_CLIPBORD 3001

#define ID_EDIT_CLIPBORD_1  4001

#define ID_BUTTON_CLEAR 5001
#define ID_BUTTON_EDIT_CLEAR 5002

#define TEXTEDITBOX _T("何かをコピーすると、ここにクリップボードの履歴が表示されます。\r\nファイルをドラッグ＆ドロップした場合は、ここにファイルの場所を表示します。")