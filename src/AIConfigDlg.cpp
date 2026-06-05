#include "AIConfigDlg.h"
#include <string>

AIConfigDlg::AIConfigDlg()
{
}

AIConfigDlg::~AIConfigDlg()
{
}

INT_PTR AIConfigDlg::DoModal(HINSTANCE hInst, HWND hParent, HttpClient& client)
{
    m_pClient = &client;
    return DialogBoxParam(hInst, MAKEINTRESOURCE(IDD_AICONFIG_DIALOG),
        hParent, DlgProc, (LPARAM)this);
}

INT_PTR CALLBACK AIConfigDlg::DlgProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
    AIConfigDlg* pDlg = nullptr;

    if (msg == WM_INITDIALOG)
    {
        pDlg = reinterpret_cast<AIConfigDlg*>(lParam);
        SetWindowLongPtr(hWnd, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(pDlg));
        pDlg->m_hWnd = hWnd;
        pDlg->LoadFromClient();
        return TRUE;
    }

    pDlg = reinterpret_cast<AIConfigDlg*>(GetWindowLongPtr(hWnd, GWLP_USERDATA));
    if (pDlg)
        return pDlg->HandleMessage(msg, wParam, lParam);
    return FALSE;
}

INT_PTR AIConfigDlg::HandleMessage(UINT msg, WPARAM wParam, LPARAM lParam)
{
    switch (msg)
    {
    case WM_COMMAND:
        switch (LOWORD(wParam))
        {
        case IDOK:
            SaveToClient();
            EndDialog(m_hWnd, IDOK);
            break;
        case IDCANCEL:
            EndDialog(m_hWnd, IDCANCEL);
            break;
        }
        break;
    }
    return FALSE;
}

void AIConfigDlg::LoadFromClient()
{
    SetDlgItemTextW(m_hWnd, IDC_CFG_ENDPOINT, m_pClient->GetEndpoint().c_str());
    SetDlgItemTextW(m_hWnd, IDC_CFG_MODEL, m_pClient->GetModel().c_str());
    SetDlgItemTextW(m_hWnd, IDC_CFG_APIKEY, m_pClient->GetApiKey().c_str());
    SetDlgItemTextW(m_hWnd, IDC_CFG_SYSPROMPT, m_pClient->GetSystemPrompt().c_str());

    std::wstring tempStr = std::to_wstring(m_pClient->GetTemperature());
    SetDlgItemTextW(m_hWnd, IDC_CFG_TEMPERATURE, tempStr.c_str());

    std::wstring tokensStr = std::to_wstring(m_pClient->GetMaxTokens());
    SetDlgItemTextW(m_hWnd, IDC_CFG_MAXTOKENS, tokensStr.c_str());
}

void AIConfigDlg::SaveToClient()
{
    wchar_t buffer[4096];

    GetDlgItemTextW(m_hWnd, IDC_CFG_ENDPOINT, buffer, 4096);
    m_pClient->SetEndpoint(buffer);

    GetDlgItemTextW(m_hWnd, IDC_CFG_MODEL, buffer, 4096);
    m_pClient->SetModel(buffer);

    GetDlgItemTextW(m_hWnd, IDC_CFG_APIKEY, buffer, 4096);
    m_pClient->SetApiKey(buffer);

    GetDlgItemTextW(m_hWnd, IDC_CFG_SYSPROMPT, buffer, 4096);
    m_pClient->SetSystemPrompt(buffer);

    GetDlgItemTextW(m_hWnd, IDC_CFG_TEMPERATURE, buffer, 4096);
    double temp = _wtof(buffer);
    if (temp > 0.0 && temp <= 2.0)
        m_pClient->SetTemperature(temp);

    GetDlgItemTextW(m_hWnd, IDC_CFG_MAXTOKENS, buffer, 4096);
    int tokens = _wtoi(buffer);
    if (tokens > 0)
        m_pClient->SetMaxTokens(tokens);
}
