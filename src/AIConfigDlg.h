#pragma once

#include <windows.h>
#include <string>
#include "HttpClient.h"
#include "resource.h"

class AIConfigDlg {
public:
    AIConfigDlg();
    ~AIConfigDlg();

    INT_PTR DoModal(HINSTANCE hInst, HWND hParent, HttpClient& client);

private:
    static INT_PTR CALLBACK DlgProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);
    INT_PTR HandleMessage(UINT msg, WPARAM wParam, LPARAM lParam);

    void LoadFromClient();
    void SaveToClient();

    HWND m_hWnd = nullptr;
    HttpClient* m_pClient = nullptr;
};
