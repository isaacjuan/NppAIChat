#include "AIChat.h"
#include "AIChatDlg.h"
#include <string>
#include <sstream>
#include <commctrl.h>

#pragma comment(lib, "comctl32.lib")

static const int BUBBLE_PAD_X = 12;
static const int BUBBLE_PAD_Y = 8;
static const int BUBBLE_RADIUS = 10;
static const int BUBBLE_MARGIN = 8;

static COLORREF DarkBg()          { return RGB(0x1e, 0x1e, 0x1e); }
static COLORREF LightBg()         { return RGB(0xf0, 0xf0, 0xf0); }
static COLORREF UserFill(bool dark) { return dark ? RGB(0x25, 0x33, 0x41) : RGB(0xe8, 0xf0, 0xfe); }
static COLORREF AsstFill(bool dark) { return dark ? RGB(0x2a, 0x2a, 0x2a) : RGB(0xff, 0xff, 0xff); }
static COLORREF UserText(bool dark) { return dark ? RGB(0xe6, 0xe6, 0xe6) : RGB(0x1a, 0x1a, 0x1a); }
static COLORREF AsstText(bool dark) { return dark ? RGB(0xcc, 0xcc, 0xcc) : RGB(0x1a, 0x1a, 0x1a); }
static COLORREF RoleLabel(bool dark) { return dark ? RGB(0x88, 0x88, 0x88) : RGB(0x88, 0x88, 0x88); }
static COLORREF AsstBorder(bool dark) { return dark ? RGB(0x3a, 0x3a, 0x3a) : RGB(0xe0, 0xe0, 0xe0); }

static std::wstring Utf8ToWide(const std::string& utf8)
{
    if (utf8.empty()) return L"";
    int len = MultiByteToWideChar(CP_UTF8, 0, utf8.c_str(), (int)utf8.size(), nullptr, 0);
    std::wstring wide(len, 0);
    MultiByteToWideChar(CP_UTF8, 0, utf8.c_str(), (int)utf8.size(), &wide[0], len);
    return wide;
}

extern void LoadConfig(HttpClient& client);
extern void SaveConfig(const HttpClient& client);

AIChatDlg::AIChatDlg()
{
}

AIChatDlg::~AIChatDlg()
{
}

void AIChatDlg::Init(HINSTANCE hInst, HWND hNpp)
{
    m_hInst = hInst;
    m_hNpp = hNpp;

    LoadConfig(m_httpClient);

    m_hWnd = CreateDialogParam(hInst, MAKEINTRESOURCE(IDD_AICHAT_DIALOG),
        hNpp, DlgProc, (LPARAM)this);
}

void AIChatDlg::Show()
{
    if (m_hWnd)
    {
        SendMessage(m_hNpp, NPPM_DMMSHOW, 0, (LPARAM)m_hWnd);
        m_visible = true;
    }
}

void AIChatDlg::Hide()
{
    if (m_hWnd)
    {
        SendMessage(m_hNpp, NPPM_DMMHIDE, 0, (LPARAM)m_hWnd);
        m_visible = false;
    }
}

bool AIChatDlg::IsVisible() const
{
    return m_visible;
}

void AIChatDlg::SetSelectedText(const std::wstring& text)
{
    m_selectedText = text;
}

void AIChatDlg::OnNppDarkModeChanged()
{
    UpdateDarkMode();
}

INT_PTR CALLBACK AIChatDlg::DlgProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
    AIChatDlg* pDlg = nullptr;

    if (msg == WM_INITDIALOG)
    {
        pDlg = reinterpret_cast<AIChatDlg*>(lParam);
        SetWindowLongPtr(hWnd, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(pDlg));
        pDlg->m_hWnd = hWnd;
        pDlg->OnCreate();
        return TRUE;
    }

    pDlg = reinterpret_cast<AIChatDlg*>(GetWindowLongPtr(hWnd, GWLP_USERDATA));
    if (pDlg)
        return pDlg->HandleMessage(msg, wParam, lParam);
    return FALSE;
}

INT_PTR AIChatDlg::HandleMessage(UINT msg, WPARAM wParam, LPARAM lParam)
{
    switch (msg)
    {
    case WM_SIZE:
        OnSize();
        break;

    case WM_TIMER:
        if (wParam == THINK_TIMER_ID)
            OnThinkTimer();
        break;

    case WM_COMMAND:
        switch (LOWORD(wParam))
        {
        case IDC_CHAT_SEND:
            OnSend();
            break;
        case IDC_CHAT_CLEAR:
            OnClear();
            break;
        case IDC_CHAT_INSERT:
            OnInsert();
            break;
        }
        if (LOWORD(wParam) == IDC_CHAT_EDIT && HIWORD(wParam) == LBN_DBLCLK)
        {
            int sel = (int)SendMessage(m_hChatList, LB_GETCURSEL, 0, 0);
            if (sel != LB_ERR && sel < (int)m_bubbles.size())
            {
                std::wstring title = m_bubbles[sel].role;
                std::wstring text = m_bubbles[sel].content;
                if (!text.empty())
                {
                    DialogBoxParam(m_hInst, MAKEINTRESOURCE(IDD_VIEWMSG_DIALOG),
                                   m_hWnd, [](HWND hDlg, UINT msg, WPARAM wParam, LPARAM lParam) -> INT_PTR
                    {
                        if (msg == WM_INITDIALOG)
                        {
                            const wchar_t* msgText = (const wchar_t*)lParam;
                            std::wstring cap(msgText, min(40, (int)wcslen(msgText)));
                            cap = L"Message - " + cap;
                            SetWindowTextW(hDlg, cap.c_str());
                            SetWindowTextW(GetDlgItem(hDlg, IDC_VIEWMSG_TEXT), msgText);
                            return TRUE;
                        }
                        if (msg == WM_COMMAND && (LOWORD(wParam) == IDOK || LOWORD(wParam) == IDCANCEL))
                        {
                            EndDialog(hDlg, 0);
                            return TRUE;
                        }
                        return FALSE;
                    }, (LPARAM)text.c_str());
                }
            }
        }
        break;

    case WM_CONTEXTMENU:
        if ((HWND)wParam == m_hChatList)
        {
            int sx = (short)LOWORD(lParam);
            int sy = (short)HIWORD(lParam);
            POINT pt = { sx, sy };
            ScreenToClient(m_hChatList, &pt);

            int itemIndex = (int)SendMessage(m_hChatList, LB_ITEMFROMPOINT, 0, MAKELPARAM(pt.x, pt.y));
            if (HIWORD(itemIndex) == 0 && LOWORD(itemIndex) < (int)m_bubbles.size())
            {
                itemIndex = LOWORD(itemIndex);
                HMENU hMenu = CreatePopupMenu();
                AppendMenuW(hMenu, MF_STRING, 1, L"Copy Message");
                int cmd = TrackPopupMenu(hMenu, TPM_RETURNCMD | TPM_NONOTIFY,
                                         sx, sy, 0, m_hWnd, nullptr);
                DestroyMenu(hMenu);

                if (cmd == 1)
                {
                    const std::wstring& text = m_bubbles[itemIndex].content;
                    if (!text.empty() && OpenClipboard(m_hWnd))
                    {
                        EmptyClipboard();
                        HGLOBAL hGlobal = GlobalAlloc(GMEM_MOVEABLE, (text.size() + 1) * sizeof(wchar_t));
                        if (hGlobal)
                        {
                            wchar_t* p = (wchar_t*)GlobalLock(hGlobal);
                            wcscpy_s(p, text.size() + 1, text.c_str());
                            GlobalUnlock(hGlobal);
                            SetClipboardData(CF_UNICODETEXT, hGlobal);
                        }
                        CloseClipboard();
                        SetStatus(L"Message copied to clipboard.");
                    }
                }
            }
        }
        break;

    case WM_MEASUREITEM:
    {
        LPMEASUREITEMSTRUCT mis = reinterpret_cast<LPMEASUREITEMSTRUCT>(lParam);
        if (mis->CtlID == IDC_CHAT_EDIT)
        {
            RECT rc;
            GetClientRect(m_hChatList, &rc);
            int availWidth = rc.right - rc.left - BUBBLE_MARGIN * 2 - BUBBLE_PAD_X * 2;
            if (availWidth < 50) availWidth = 50;

            if (mis->itemID < m_bubbles.size())
            {
                const auto& item = m_bubbles[mis->itemID];
                RECT textRc = {0, 0, availWidth, 0};
                HDC hdc = GetDC(m_hChatList);
                HFONT hFont = (HFONT)SendMessage(m_hWnd, WM_GETFONT, 0, 0);
                HFONT hOld = (HFONT)SelectObject(hdc, hFont);
                DrawTextW(hdc, item.content.c_str(), (int)item.content.size(), &textRc,
                          DT_CALCRECT | DT_WORDBREAK | DT_LEFT);
                SelectObject(hdc, hOld);
                ReleaseDC(m_hChatList, hdc);
                mis->itemHeight = textRc.bottom - textRc.top + BUBBLE_PAD_Y * 2 + 16;
                if (mis->itemHeight < 50) mis->itemHeight = 50;
            }
            else
            {
                mis->itemHeight = 50;
            }
        }
        return TRUE;
    }

    case WM_DRAWITEM:
    {
        LPDRAWITEMSTRUCT dis = reinterpret_cast<LPDRAWITEMSTRUCT>(lParam);
        if (dis->CtlID == IDC_CHAT_EDIT && dis->itemID != -1 &&
            dis->itemID < (UINT)m_bubbles.size())
        {
            DrawBubble(dis->hDC, dis->rcItem,
                       m_bubbles[dis->itemID].role,
                       m_bubbles[dis->itemID].content,
                       m_darkMode);
        }
        return TRUE;
    }

    case WM_CTLCOLORLISTBOX:
        if ((HWND)lParam == m_hChatList && m_hBgBrush)
        {
            SetBkColor((HDC)wParam, m_darkMode ? DarkBg() : LightBg());
            return (INT_PTR)m_hBgBrush;
        }
        break;

    case WM_DESTROY:
        OnDestroy();
        break;

    case WM_NOTIFY:
    {
        LPNMHDR nmhdr = reinterpret_cast<LPNMHDR>(lParam);
        if (nmhdr->code == DMN_CLOSE)
        {
            StopThinking();
            m_visible = false;
        }
        else if (nmhdr->code == DMN_DOCK)
            UpdateDarkMode();
        break;
    }
    }
    return FALSE;
}

void AIChatDlg::OnCreate()
{
    m_hChatList = GetDlgItem(m_hWnd, IDC_CHAT_EDIT);
    m_hInputEdit = GetDlgItem(m_hWnd, IDC_CHAT_INPUT);
    m_hSendBtn = GetDlgItem(m_hWnd, IDC_CHAT_SEND);
    m_hClearBtn = GetDlgItem(m_hWnd, IDC_CHAT_CLEAR);
    m_hInsertBtn = GetDlgItem(m_hWnd, IDC_CHAT_INSERT);
    m_hStatusText = GetDlgItem(m_hWnd, IDC_STATUS_TEXT);

    EnableWindow(m_hInsertBtn, FALSE);
    SetStatus(L"Ready. Select code and use 'Send Selection' or type a message.");
    UpdateDarkMode();
}

void AIChatDlg::OnDestroy()
{
    StopThinking();
    if (m_hBgBrush) { DeleteObject(m_hBgBrush); m_hBgBrush = nullptr; }
    m_hWnd = nullptr;
}

void AIChatDlg::OnSize()
{
    if (!m_hChatList || !m_hInputEdit || !m_hSendBtn)
        return;

    RECT rc;
    GetClientRect(m_hWnd, &rc);

    int margin = 6;
    int gap = 5;
    int btnHeight = 26;
    int statusHeight = 24;
    int inputHeight = 55;

    int btnAreaBottom = rc.bottom - margin;
    int btnAreaTop = btnAreaBottom - btnHeight;
    int inputBottom = btnAreaTop - gap;
    int inputTop = inputBottom - inputHeight;
    int statusBottom = inputTop - gap;
    int statusTop = statusBottom - statusHeight;
    int listHeight = statusTop - margin - gap;

    if (listHeight < 40) listHeight = 40;

    MoveWindow(m_hChatList, margin, margin,
        rc.right - margin * 2, listHeight, TRUE);

    MoveWindow(m_hStatusText, margin, listHeight + margin + gap,
        rc.right - margin * 2, statusHeight, TRUE);

    MoveWindow(m_hInputEdit, margin, inputTop,
        rc.right - margin * 2, inputHeight, TRUE);

    int btnWidth = 75;
    int insertWidth = 85;
    MoveWindow(m_hSendBtn, margin, btnAreaTop, btnWidth, btnHeight, TRUE);
    MoveWindow(m_hClearBtn, margin + btnWidth + gap, btnAreaTop, btnWidth, btnHeight, TRUE);
    MoveWindow(m_hInsertBtn, margin + (btnWidth + gap) * 2, btnAreaTop, insertWidth, btnHeight, TRUE);
}

void AIChatDlg::StartThinking(const std::wstring& modelName)
{
    m_thinking = true;
    m_thinkDotCount = 0;
    SetStatus(L"Thinking " + modelName);
    SetTimer(m_hWnd, THINK_TIMER_ID, 400, nullptr);
}

void AIChatDlg::StopThinking()
{
    if (m_thinking)
    {
        m_thinking = false;
        KillTimer(m_hWnd, THINK_TIMER_ID);
    }
}

void AIChatDlg::OnThinkTimer()
{
    if (!m_thinking) return;
    m_thinkDotCount = (m_thinkDotCount + 1) % 4;
    std::wstring dots;
    for (int i = 0; i < m_thinkDotCount; ++i) dots += L".";
    std::wstring modelName(m_httpClient.GetModel().begin(), m_httpClient.GetModel().end());
    SetStatus(L"Thinking" + dots + L"  [" + modelName + L"]");
}

std::wstring AIChatDlg::GetCurrentEditorSelection()
{
    extern NppData nppData;

    int currentView = 0;
    SendMessage(m_hNpp, NPPM_GETCURRENTSCINTILLA, 0, (LPARAM)&currentView);
    HWND hScintilla = (currentView == 0) ? nppData._scintillaMainHandle : nppData._scintillaSecondHandle;

    int selStart = (int)SendMessage(hScintilla, SCI_GETSELECTIONSTART, 0, 0);
    int selEnd = (int)SendMessage(hScintilla, SCI_GETSELECTIONEND, 0, 0);

    if (selStart == selEnd)
        return m_selectedText;

    int textLen = selEnd - selStart;
    std::vector<char> text(textLen + 1);

    Sci_TextRangeFull tr;
    tr.chrg.cpMin = selStart;
    tr.chrg.cpMax = selEnd;
    tr.lpstrText = text.data();
    SendMessage(hScintilla, SCI_GETTEXTRANGEFULL, 0, (LPARAM)&tr);

    std::string utf8Text(text.data());
    return Utf8ToWide(utf8Text);
}

void AIChatDlg::OnSend()
{
    if (m_streaming)
        return;

    int len = GetWindowTextLengthW(m_hInputEdit);
    if (len == 0 && m_selectedText.empty())
    {
        std::wstring currentSel = GetCurrentEditorSelection();
        if (currentSel.empty())
            return;
    }

    std::wstring input;
    if (len > 0)
    {
        input.resize(len + 1);
        GetWindowTextW(m_hInputEdit, &input[0], len + 1);
        input.resize(len);
        SetWindowTextW(m_hInputEdit, L"");
    }

    std::wstring context = GetCurrentEditorSelection();
    std::wstring apiInput = input;
    if (!context.empty())
    {
        if (!apiInput.empty())
            apiInput = L"[Code context]:\n```\n" + context + L"\n```\n\n" + apiInput;
        else
            apiInput = L"Please analyze this code:\n```\n" + context + L"\n```";
        m_selectedText.clear();
    }

    if (!apiInput.empty())
    {
        ChatMessage userMsg;
        userMsg.role = "user";
        int wlen = (int)apiInput.size();
        int ulen = WideCharToMultiByte(CP_UTF8, 0, apiInput.c_str(), wlen, nullptr, 0, nullptr, nullptr);
        userMsg.content.resize(ulen);
        WideCharToMultiByte(CP_UTF8, 0, apiInput.c_str(), wlen, &userMsg.content[0], ulen, nullptr, nullptr);
        m_messages.push_back(userMsg);

        if (input.empty())
            input = L"*(Code selection sent as context)*";
        AppendChatMessage(L"You", input);
        UpdateWindow(m_hChatList);
    }

    EnableWindow(m_hSendBtn, FALSE);
    EnableWindow(m_hInsertBtn, FALSE);

    BubbleItem placeholder;
    placeholder.role = L"Claude";
    placeholder.content = L"";
    m_bubbles.push_back(placeholder);
    m_streamItemIndex = (int)m_bubbles.size() - 1;
    int idx = (int)SendMessage(m_hChatList, LB_ADDSTRING, 0, (LPARAM)L"");
    SendMessage(m_hChatList, LB_SETITEMDATA, idx, (LPARAM)m_streamItemIndex);
    SendMessage(m_hChatList, LB_SETTOPINDEX, idx, 0);

    StartThinking(m_httpClient.GetModel());
    UpdateWindow(m_hChatList);

    m_streaming = true;
    m_streamBuffer.clear();

    bool firstChunk = true;

    std::vector<ChatMessage> apiMessages;
    std::wstring sysPrompt = m_httpClient.GetSystemPrompt();
    if (!sysPrompt.empty())
    {
        ChatMessage sys;
        sys.role = "system";
        int slen = (int)sysPrompt.size();
        int ulen = WideCharToMultiByte(CP_UTF8, 0, sysPrompt.c_str(), slen, nullptr, 0, nullptr, nullptr);
        sys.content.resize(ulen);
        WideCharToMultiByte(CP_UTF8, 0, sysPrompt.c_str(), slen, &sys.content[0], ulen, nullptr, nullptr);
        apiMessages.push_back(sys);
    }
    apiMessages.insert(apiMessages.end(), m_messages.begin(), m_messages.end());

    m_httpClient.SendChat(apiMessages, [this, firstChunk](const std::string& content, bool done) mutable
    {
        if (done)
        {
            if (m_streamItemIndex == -1) return;
            StopThinking();
            if (!m_streamBuffer.empty())
            {
                ChatMessage asstMsg;
                asstMsg.role = "assistant";
                asstMsg.content = m_streamBuffer;
                m_messages.push_back(asstMsg);
            }
            else if (!content.empty() && !m_streaming)
            {
                std::wstring werror = Utf8ToWide(content);
                BubbleItem errorItem;
                errorItem.role = L"Error";
                errorItem.content = werror;
                m_bubbles.push_back(errorItem);
                int idx = (int)SendMessage(m_hChatList, LB_ADDSTRING, 0, (LPARAM)L"");
                SendMessage(m_hChatList, LB_SETITEMDATA, idx, (LPARAM)(m_bubbles.size() - 1));
                SendMessage(m_hChatList, LB_SETTOPINDEX, idx, 0);
            }
            bool hasResponse = !m_streamBuffer.empty();

            if (hasResponse && m_httpClient.WasTruncated())
            {
                std::wstring warn = L"\n\n*⚠ Response truncated (max_tokens limit reached). Increase in Config or split your request.*";
                m_bubbles[m_streamItemIndex].content += warn;
                RECT rc;
                GetClientRect(m_hChatList, &rc);
                int newH = CalcBubbleHeight(m_bubbles[m_streamItemIndex].content, rc.right - rc.left);
                SendMessage(m_hChatList, LB_SETITEMHEIGHT, m_streamItemIndex, MAKELPARAM(newH, 0));
                InvalidateRect(m_hChatList, nullptr, TRUE);
            }

            m_streamBuffer.clear();
            m_streaming = false;
            m_streamItemIndex = -1;
            EnableWindow(m_hSendBtn, TRUE);
            EnableWindow(m_hInsertBtn, hasResponse);
            SetStatus(L"Ready.");
        }
        else
        {
            if (firstChunk)
            {
                StopThinking();
                firstChunk = false;
            }
            m_streamBuffer += content;
            std::wstring wtext = Utf8ToWide(m_streamBuffer);
            if (m_streamItemIndex >= 0 && m_streamItemIndex < (int)m_bubbles.size())
            {
                m_bubbles[m_streamItemIndex].content = wtext;
                RECT rc;
                GetClientRect(m_hChatList, &rc);
                int newH = CalcBubbleHeight(wtext, rc.right - rc.left);
                SendMessage(m_hChatList, LB_SETITEMHEIGHT, m_streamItemIndex, MAKELPARAM(newH, 0));
                InvalidateRect(m_hChatList, nullptr, TRUE);
                int count = (int)SendMessage(m_hChatList, LB_GETCOUNT, 0, 0);
                SendMessage(m_hChatList, LB_SETTOPINDEX, count - 1, 0);
            }
        }
    });
}

void AIChatDlg::OnInsert()
{
    extern NppData nppData;

    std::string lastResponse;
    for (auto it = m_messages.rbegin(); it != m_messages.rend(); ++it)
    {
        if (it->role == "assistant")
        {
            lastResponse = it->content;
            break;
        }
    }

    if (lastResponse.empty())
    {
        SetStatus(L"No AI response to insert.");
        return;
    }

    int currentView = 0;
    SendMessage(m_hNpp, NPPM_GETCURRENTSCINTILLA, 0, (LPARAM)&currentView);
    HWND hScintilla = (currentView == 0) ? nppData._scintillaMainHandle : nppData._scintillaSecondHandle;

    SendMessage(hScintilla, SCI_REPLACESEL, TRUE, (LPARAM)lastResponse.c_str());

    SetStatus(L"Response inserted at cursor.");
}

void AIChatDlg::OnClear()
{
    m_messages.clear();
    m_bubbles.clear();
    m_streamBuffer.clear();
    m_streaming = false;
    m_streamItemIndex = -1;
    SendMessage(m_hChatList, LB_RESETCONTENT, 0, 0);
    SetStatus(L"Chat cleared.");
    EnableWindow(m_hSendBtn, TRUE);
    EnableWindow(m_hInsertBtn, FALSE);
}

void AIChatDlg::AppendChatMessage(const std::wstring& role, const std::wstring& content)
{
    BubbleItem item;
    item.role = role;
    item.content = content;
    m_bubbles.push_back(item);

    int idx = (int)SendMessage(m_hChatList, LB_ADDSTRING, 0, (LPARAM)L"");
    SendMessage(m_hChatList, LB_SETITEMDATA, idx, (LPARAM)(m_bubbles.size() - 1));
    int count = (int)SendMessage(m_hChatList, LB_GETCOUNT, 0, 0);
    SendMessage(m_hChatList, LB_SETTOPINDEX, count - 1, 0);
}

void AIChatDlg::AppendTextToChat(const std::wstring& text)
{
    if (m_streamItemIndex >= 0 && m_streamItemIndex < (int)m_bubbles.size())
    {
        m_bubbles[m_streamItemIndex].content += text;
        RECT rc;
        GetClientRect(m_hChatList, &rc);
        int newH = CalcBubbleHeight(m_bubbles[m_streamItemIndex].content, rc.right - rc.left);
        SendMessage(m_hChatList, LB_SETITEMHEIGHT, m_streamItemIndex, MAKELPARAM(newH, 0));
        InvalidateRect(m_hChatList, nullptr, TRUE);
        int count = (int)SendMessage(m_hChatList, LB_GETCOUNT, 0, 0);
        SendMessage(m_hChatList, LB_SETTOPINDEX, count - 1, 0);
    }
}

void AIChatDlg::SetStatus(const std::wstring& text)
{
    SetWindowTextW(m_hStatusText, text.c_str());
}

void AIChatDlg::UpdateDarkMode()
{
    m_darkMode = (BOOL)SendMessage(m_hNpp, NPPM_ISDARKMODEENABLED, 0, 0) != 0;
    if (m_darkMode)
    {
        SendMessage(m_hNpp, NPPM_DARKMODESUBCLASSANDTHEME, 0x0000000B, (LPARAM)m_hWnd);
    }
    if (m_hBgBrush) DeleteObject(m_hBgBrush);
    m_hBgBrush = CreateSolidBrush(m_darkMode ? DarkBg() : LightBg());
    if (m_hChatList)
        InvalidateRect(m_hChatList, nullptr, TRUE);
}

int AIChatDlg::CalcBubbleHeight(const std::wstring& text, int listWidth)
{
    int availWidth = listWidth - BUBBLE_MARGIN * 2 - BUBBLE_PAD_X * 2;
    if (availWidth < 50) availWidth = 50;

    RECT textRc = {0, 0, availWidth, 0};
    HDC hdc = GetDC(m_hChatList);
    HFONT hFont = (HFONT)SendMessage(m_hWnd, WM_GETFONT, 0, 0);
    HFONT hOld = (HFONT)SelectObject(hdc, hFont);
    DrawTextW(hdc, text.c_str(), (int)text.size(), &textRc,
              DT_CALCRECT | DT_WORDBREAK | DT_LEFT);
    SelectObject(hdc, hOld);
    ReleaseDC(m_hChatList, hdc);

    int h = textRc.bottom - textRc.top + BUBBLE_PAD_Y * 2 + 16;
    if (h < 50) h = 50;
    return h;
}

void AIChatDlg::DrawBubble(HDC hdc, const RECT& rect, const std::wstring& role,
                            const std::wstring& text, bool dark)
{
    COLORREF bgColor = dark ? DarkBg() : LightBg();
    COLORREF fillColor = (role == L"You" || role == L"User") ? UserFill(dark) : AsstFill(dark);
    COLORREF textColor = (role == L"You" || role == L"User") ? UserText(dark) : AsstText(dark);

    HBRUSH bgBrush = CreateSolidBrush(bgColor);
    FillRect(hdc, &rect, bgBrush);
    DeleteObject(bgBrush);

    int bubbleL = rect.left + BUBBLE_MARGIN;
    int bubbleT = rect.top + 4;
    int bubbleR = rect.right - BUBBLE_MARGIN;
    int bubbleB = rect.bottom - 4;
    int bubbleW = bubbleR - bubbleL;
    int bubbleH = bubbleB - bubbleT;

    if (bubbleW <= 0 || bubbleH <= 0) return;

    HBRUSH fillBrush = CreateSolidBrush(fillColor);
    HPEN fillPen = CreatePen(PS_SOLID, 1, fillColor);
    HGDIOBJ oldBrush = SelectObject(hdc, fillBrush);
    HGDIOBJ oldPen = SelectObject(hdc, fillPen);

    RoundRect(hdc, bubbleL, bubbleT, bubbleR, bubbleB, BUBBLE_RADIUS * 2, BUBBLE_RADIUS * 2);

    SelectObject(hdc, oldBrush);
    SelectObject(hdc, oldPen);
    DeleteObject(fillBrush);
    DeleteObject(fillPen);

    if (!dark && role != L"You" && role != L"User")
    {
        HPEN borderPen = CreatePen(PS_SOLID, 1, AsstBorder(dark));
        HGDIOBJ oldPen2 = SelectObject(hdc, borderPen);
        HGDIOBJ oldBrush2 = SelectObject(hdc, GetStockObject(NULL_BRUSH));
        RoundRect(hdc, bubbleL, bubbleT, bubbleR, bubbleB, BUBBLE_RADIUS * 2, BUBBLE_RADIUS * 2);
        SelectObject(hdc, oldPen2);
        SelectObject(hdc, oldBrush2);
        DeleteObject(borderPen);
    }

    SetBkMode(hdc, TRANSPARENT);
    HFONT hFont = (HFONT)SendMessage(m_hWnd, WM_GETFONT, 0, 0);
    HFONT hOldFont = (HFONT)SelectObject(hdc, hFont);

    RECT textRect;
    textRect.left = bubbleL + BUBBLE_PAD_X;
    textRect.top = bubbleT + BUBBLE_PAD_Y;
    textRect.right = bubbleR - BUBBLE_PAD_X;
    textRect.bottom = bubbleB - BUBBLE_PAD_Y;

    SetTextColor(hdc, textColor);
    DrawTextW(hdc, text.c_str(), (int)text.size(), &textRect,
              DT_WORDBREAK | DT_LEFT | DT_NOPREFIX);

    SelectObject(hdc, hOldFont);
}
