#pragma once
#include<Windows.h>
#include<string>

std::string GetOSVersion();

bool SendVersionInfoToMessageBox(HWND);
