#include <windows.h>
#include <string>
#include <vector>
#include <fstream>
#include <shlwapi.h>
#include "AIChat.h"
#include "AIChatDlg.h"
#include "AIConfigDlg.h"
#include "HttpClient.h"

#pragma comment(lib, "shlwapi.lib")

NppData nppData;
HINSTANCE g_hInst = nullptr;

FuncItem funcItems[3];
AIChatDlg g_chatDlg;
HttpClient g_httpClient;

int g_cmdIdToggle = 0;
int g_cmdIdConfig = 0;
int g_cmdIdSend = 0;

std::wstring GetConfigFilePath()
{
    wchar_t configDir[MAX_PATH] = { 0 };
    SendMessage(nppData._nppHandle, NPPM_GETPLUGINSCONFIGDIR, MAX_PATH, (LPARAM)configDir);
    return std::wstring(configDir) + L"\\NppAIChat.ini";
}

void LoadConfig(HttpClient& client)
{
    std::wstring iniPath = GetConfigFilePath();

    wchar_t buffer[4096];
    GetPrivateProfileStringW(L"NppAIChat", L"Endpoint", L"https://api.openai.com/v1", buffer, 4096, iniPath.c_str());
    client.SetEndpoint(buffer);

    GetPrivateProfileStringW(L"NppAIChat", L"Model", L"gpt-4o-mini", buffer, 4096, iniPath.c_str());
    client.SetModel(buffer);

    GetPrivateProfileStringW(L"NppAIChat", L"ApiKey", L"", buffer, 4096, iniPath.c_str());
    client.SetApiKey(buffer);

    GetPrivateProfileStringW(L"NppAIChat", L"SystemPrompt",
        L"You are a helpful assistant. Always respond in the same language as the user's input.",
        buffer, 4096, iniPath.c_str());
    client.SetSystemPrompt(buffer);

    double temp = GetPrivateProfileIntW(L"NppAIChat", L"Temperature", 70, iniPath.c_str()) / 100.0;
    client.SetTemperature(temp);

    int tokens = GetPrivateProfileIntW(L"NppAIChat", L"MaxTokens", 4096, iniPath.c_str());
    client.SetMaxTokens(tokens);
}

void SaveConfig(const HttpClient& client)
{
    std::wstring iniPath = GetConfigFilePath();

    WritePrivateProfileStringW(L"NppAIChat", L"Endpoint", client.GetEndpoint().c_str(), iniPath.c_str());
    WritePrivateProfileStringW(L"NppAIChat", L"Model", client.GetModel().c_str(), iniPath.c_str());
    WritePrivateProfileStringW(L"NppAIChat", L"ApiKey", client.GetApiKey().c_str(), iniPath.c_str());
    WritePrivateProfileStringW(L"NppAIChat", L"SystemPrompt", client.GetSystemPrompt().c_str(), iniPath.c_str());

    int temp = (int)(client.GetTemperature() * 100);
    WritePrivateProfileStringW(L"NppAIChat", L"Temperature", std::to_wstring(temp).c_str(), iniPath.c_str());
    WritePrivateProfileStringW(L"NppAIChat", L"MaxTokens", std::to_wstring(client.GetMaxTokens()).c_str(), iniPath.c_str());
}

std::wstring GetSelectedText()
{
    int currentView = 0;
    SendMessage(nppData._nppHandle, NPPM_GETCURRENTSCINTILLA, 0, (LPARAM)&currentView);
    HWND hScintilla = (currentView == 0) ? nppData._scintillaMainHandle : nppData._scintillaSecondHandle;

    int selStart = (int)SendMessage(hScintilla, SCI_GETSELECTIONSTART, 0, 0);
    int selEnd = (int)SendMessage(hScintilla, SCI_GETSELECTIONEND, 0, 0);

    if (selStart == selEnd)
        return L"";

    int textLen = selEnd - selStart;
    std::vector<char> text(textLen + 1);

    Sci_TextRangeFull tr;
    tr.chrg.cpMin = selStart;
    tr.chrg.cpMax = selEnd;
    tr.lpstrText = text.data();
    SendMessage(hScintilla, SCI_GETTEXTRANGEFULL, 0, (LPARAM)&tr);

    std::string utf8Text(text.data());
    int wideLen = MultiByteToWideChar(CP_UTF8, 0, utf8Text.c_str(), -1, nullptr, 0);
    std::wstring wideText(wideLen, L'\0');
    MultiByteToWideChar(CP_UTF8, 0, utf8Text.c_str(), -1, &wideText[0], wideLen);
    wideText.resize(wideLen - 1);

    return wideText;
}

void ToggleChatPanel()
{
    if (g_chatDlg.IsVisible())
        g_chatDlg.Hide();
    else
        g_chatDlg.Show();
}

void ShowConfigDialog()
{
    AIConfigDlg configDlg;
    if (configDlg.DoModal(g_hInst, nppData._nppHandle, g_httpClient) == IDOK)
    {
        SaveConfig(g_httpClient);
        LoadConfig(g_httpClient);
    }
}

void SendSelectionToAI()
{
    std::wstring selectedText = GetSelectedText();
    if (selectedText.empty())
    {
        MessageBoxW(nppData._nppHandle,
            L"No text selected in the editor.\nSelect some code first.",
            PLUGIN_NAME, MB_OK | MB_ICONINFORMATION);
        return;
    }

    if (!g_chatDlg.GetHWND())
    {
        g_chatDlg.Init(g_hInst, nppData._nppHandle);
    }

    g_chatDlg.SetSelectedText(selectedText);
    g_chatDlg.Show();
}

extern "C" __declspec(dllexport) void setInfo(NppData data)
{
    nppData = data;
    g_hInst = GetModuleHandleW(L"NppAIChat.dll");

    LoadConfig(g_httpClient);
    g_chatDlg.Init(g_hInst, nppData._nppHandle);
}

extern "C" __declspec(dllexport) const wchar_t* getName()
{
    return PLUGIN_NAME;
}

extern "C" __declspec(dllexport) FuncItem* getFuncsArray(int* nbF)
{
    *nbF = 3;

    wcscpy_s(funcItems[0]._itemName, L"Toggle AI Chat Panel");
    funcItems[0]._pFunc = ToggleChatPanel;
    funcItems[0]._cmdID = ID_PLUGIN_TOGGLE_CHAT;
    funcItems[0]._init2Check = false;

    wcscpy_s(funcItems[1]._itemName, L"Send Selection to AI");
    funcItems[1]._pFunc = SendSelectionToAI;
    funcItems[1]._cmdID = ID_PLUGIN_SEND_SELECTION;
    funcItems[1]._init2Check = false;

    wcscpy_s(funcItems[2]._itemName, L"Configure AI...");
    funcItems[2]._pFunc = ShowConfigDialog;
    funcItems[2]._cmdID = ID_PLUGIN_CONFIGURE;
    funcItems[2]._init2Check = false;

    return funcItems;
}

extern "C" __declspec(dllexport) void beNotified(SCNotification* notifyCode)
{
    switch (notifyCode->nmhdr.code)
    {
    case NPPN_READY:
    {
        DockedWidgetData dwd;
        dwd.hClient = g_chatDlg.GetHWND();
        dwd.pszName = PLUGIN_NAME;
        dwd.dlgID = ID_PLUGIN_TOGGLE_CHAT;
        dwd.uMask = DWS_ICONTAB | DWS_DF_CONT_LEFT;
        dwd.hIconTab = NULL;
        dwd.pszModuleName = L"NppAIChat.dll";
        SendMessage(nppData._nppHandle, NPPM_DMMREGASDCKDLG, 0, (LPARAM)&dwd);
        break;
    }

    case NPPN_DARKMODECHANGED:
        g_chatDlg.OnNppDarkModeChanged();
        break;

    case NPPN_SHUTDOWN:
        SaveConfig(g_httpClient);
        break;
    }
}

extern "C" __declspec(dllexport) LRESULT messageProc(UINT Message, WPARAM wParam, LPARAM lParam)
{
    return FALSE;
}

extern "C" __declspec(dllexport) BOOL isUnicode()
{
    return TRUE;
}
