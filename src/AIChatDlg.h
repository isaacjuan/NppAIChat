#pragma once

#include <windows.h>
#include <string>
#include <vector>
#include "HttpClient.h"
#include "resource.h"

#define THINK_TIMER_ID 1001
#define WM_CHAT_STREAM (WM_APP + 1)
#define WM_CHAT_DONE   (WM_APP + 2)

struct BubbleItem {
    std::wstring role;
    std::wstring content;
};

struct ChatRequest {
    std::vector<ChatMessage> messages;
    std::wstring endpoint;
    std::wstring apiKey;
    std::wstring model;
    std::wstring systemPrompt;
    double temperature;
    int maxTokens;
    HWND hWnd;
};

class AIChatDlg {
public:
    AIChatDlg();
    ~AIChatDlg();

    void Init(HINSTANCE hInst, HWND hNpp);
    void Show();
    void Hide();
    bool IsVisible() const;
    HWND GetHWND() const { return m_hWnd; }

    void SetSelectedText(const std::wstring& text);
    void OnNppDarkModeChanged();

private:
    static INT_PTR CALLBACK DlgProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);
    INT_PTR HandleMessage(UINT msg, WPARAM wParam, LPARAM lParam);

    void OnCreate();
    void OnDestroy();
    void OnSend();
    void OnClear();
    void OnInsert();
    void OnSize();
    void OnThinkTimer();
    void AppendChatMessage(const std::wstring& role, const std::wstring& content);
    void AppendTextToChat(const std::wstring& text);
    void SetStatus(const std::wstring& text);
    void StartThinking(const std::wstring& modelName);
    void StopThinking();
    void UpdateDarkMode();
    std::wstring GetCurrentEditorSelection();

    void OnChatStream(const std::string& content);
    void OnChatDone(bool truncated, const std::string& error);

    int CalcBubbleHeight(const std::wstring& text, int listWidth);
    void DrawBubble(HDC hdc, const RECT& rect, const std::wstring& role,
                    const std::wstring& text, bool darkMode);

    static unsigned __stdcall ChatThreadProc(void* lpParam);

    HWND m_hWnd = nullptr;
    HWND m_hNpp = nullptr;
    HINSTANCE m_hInst = nullptr;
    HWND m_hChatList = nullptr;
    HWND m_hInputEdit = nullptr;
    HWND m_hSendBtn = nullptr;
    HWND m_hClearBtn = nullptr;
    HWND m_hInsertBtn = nullptr;
    HWND m_hStatusText = nullptr;

    HttpClient m_httpClient;
    std::vector<ChatMessage> m_messages;
    std::vector<BubbleItem> m_bubbles;
    std::wstring m_selectedText;
    bool m_visible = false;
    bool m_streaming = false;
    bool m_thinking = false;
    bool m_darkMode = false;
    int m_thinkDotCount = 0;
    int m_streamItemIndex = -1;
    HBRUSH m_hBgBrush = nullptr;
    std::string m_streamBuffer;
    std::string m_pendingError;
    bool m_hasResponse = false;
};
