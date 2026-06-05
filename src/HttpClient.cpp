#include "HttpClient.h"
#include <windows.h>
#include <winhttp.h>
#include <sstream>
#include <regex>

#pragma comment(lib, "winhttp.lib")

HttpClient::HttpClient()
{
    m_endpoint = L"https://api.openai.com/v1";
    m_model = L"gpt-4o-mini";
    m_apiKey = L"";
}

HttpClient::~HttpClient()
{
}

bool HttpClient::EnsureInitialized()
{
    if (!m_initialized)
        m_initialized = true;
    return true;
}

void HttpClient::SetEndpoint(const std::wstring& endpoint)
{
    m_endpoint = endpoint;
}

void HttpClient::SetApiKey(const std::wstring& apiKey)
{
    m_apiKey = apiKey;
}

void HttpClient::SetModel(const std::wstring& model)
{
    m_model = model;
}

void HttpClient::SetSystemPrompt(const std::wstring& prompt)
{
    m_systemPrompt = prompt;
}

void HttpClient::SetTemperature(double temp)
{
    m_temperature = temp;
}

void HttpClient::SetMaxTokens(int tokens)
{
    m_maxTokens = tokens;
}

std::string HttpClient::BuildRequestBody(const std::vector<ChatMessage>& messages)
{
    std::ostringstream body;
    body << "{";
    int wideLen = (int)m_model.size();
    int utf8Len = WideCharToMultiByte(CP_UTF8, 0, m_model.c_str(), wideLen, nullptr, 0, nullptr, nullptr);
    std::string modelStr(utf8Len, 0);
    WideCharToMultiByte(CP_UTF8, 0, m_model.c_str(), wideLen, &modelStr[0], utf8Len, nullptr, nullptr);
    body << "\"model\":\"" << modelStr << "\",";
    body << "\"stream\":true,";
    body << "\"temperature\":" << m_temperature << ",";
    body << "\"max_tokens\":" << m_maxTokens << ",";
    body << "\"messages\":[";

    for (size_t i = 0; i < messages.size(); ++i)
    {
        if (i > 0) body << ",";
        body << "{";
        body << "\"role\":\"" << messages[i].role << "\",";
        body << "\"content\":\"";

        std::string escaped;
        for (char c : messages[i].content)
        {
            switch (c)
            {
            case '"': escaped += "\\\""; break;
            case '\\': escaped += "\\\\"; break;
            case '\n': escaped += "\\n"; break;
            case '\r': escaped += "\\r"; break;
            case '\t': escaped += "\\t"; break;
            default: escaped += c;
            }
        }
        body << escaped << "\"";
        body << "}";
    }

    body << "]}";
    return body.str();
}

bool HttpClient::ParseSSE(const std::string& data, std::string& content, bool& done)
{
    content.clear();
    done = false;

    std::istringstream stream(data);
    std::string line;
    while (std::getline(stream, line))
    {
        if (line.find("data: ") == 0)
        {
            std::string jsonStr = line.substr(6);
            if (jsonStr == "[DONE]")
            {
                done = true;
                return true;
            }

            if (jsonStr.find("\"finish_reason\":\"length\"") != std::string::npos)
                m_truncated = true;

            size_t contentPos;
            if ((contentPos = jsonStr.find("\"content\":\"")) != std::string::npos)
            {
                contentPos += 11;
                size_t endPos = contentPos;
                bool escaped = false;
                while (endPos < jsonStr.size())
                {
                    if (escaped)
                    {
                        escaped = false;
                        endPos++;
                        continue;
                    }
                    if (jsonStr[endPos] == '\\')
                    {
                        escaped = true;
                        endPos++;
                        continue;
                    }
                    if (jsonStr[endPos] == '"')
                        break;
                    endPos++;
                }
                content = jsonStr.substr(contentPos, endPos - contentPos);

                size_t pos = 0;
                while ((pos = content.find("\\n", pos)) != std::string::npos)
                {
                    content.replace(pos, 2, "\n");
                    pos++;
                }
                pos = 0;
                while ((pos = content.find("\\\"", pos)) != std::string::npos)
                {
                    content.replace(pos, 2, "\"");
                    pos++;
                }
                pos = 0;
                while ((pos = content.find("\\t", pos)) != std::string::npos)
                {
                    content.replace(pos, 2, "\t");
                    pos++;
                }
                pos = 0;
                while ((pos = content.find("\\r", pos)) != std::string::npos)
                {
                    content.replace(pos, 2, "\r");
                    pos++;
                }
                return true;
            }
        }
    }
    return false;
}

bool HttpClient::SendChat(const std::vector<ChatMessage>& messages, StreamCallback callback)
{
    m_truncated = false;
    if (!EnsureInitialized())
        return false;

    if (m_apiKey.empty())
    {
        callback("Error: API key not configured. Please configure the plugin first.", true);
        return false;
    }

    std::wstring host;
    std::wstring path;
    bool isSecure = true;

    std::wstring endpoint = m_endpoint;
    if (endpoint.find(L"://") == std::wstring::npos)
        endpoint = L"https://" + endpoint;

    size_t schemeEnd = endpoint.find(L"://");
    std::wstring scheme = endpoint.substr(0, schemeEnd);
    isSecure = (scheme == L"https");

    size_t pathStart = endpoint.find(L"/", schemeEnd + 3);
    if (pathStart != std::wstring::npos)
    {
        host = endpoint.substr(schemeEnd + 3, pathStart - schemeEnd - 3);
        path = endpoint.substr(pathStart);
        while (!path.empty() && path.back() == L'/')
            path.pop_back();
    }
    else
    {
        host = endpoint.substr(schemeEnd + 3);
    }

    path += L"/chat/completions";

    HINTERNET hSession = WinHttpOpen(L"NppAIChat/1.0",
        WINHTTP_ACCESS_TYPE_DEFAULT_PROXY, NULL, NULL, 0);
    if (!hSession)
    {
        callback("Error: Failed to create HTTP session", true);
        return false;
    }

    HINTERNET hConnect = WinHttpConnect(hSession, host.c_str(),
        isSecure ? INTERNET_DEFAULT_HTTPS_PORT : INTERNET_DEFAULT_HTTP_PORT, 0);
    if (!hConnect)
    {
        WinHttpCloseHandle(hSession);
        callback("Error: Failed to connect to host", true);
        return false;
    }

    DWORD flags = WINHTTP_FLAG_REFRESH;
    if (isSecure)
        flags |= WINHTTP_FLAG_SECURE;

    HINTERNET hRequest = WinHttpOpenRequest(hConnect, L"POST", path.c_str(),
        NULL, WINHTTP_NO_REFERER, WINHTTP_DEFAULT_ACCEPT_TYPES, flags);
    if (!hRequest)
    {
        WinHttpCloseHandle(hConnect);
        WinHttpCloseHandle(hSession);
        callback("Error: Failed to create request", true);
        return false;
    }

    std::wstring authHeader = L"Authorization: Bearer " + m_apiKey;
    std::wstring contentType = L"Content-Type: application/json";

    WinHttpAddRequestHeaders(hRequest, authHeader.c_str(), (ULONG)-1L, WINHTTP_ADDREQ_FLAG_ADD);
    WinHttpAddRequestHeaders(hRequest, contentType.c_str(), (ULONG)-1L, WINHTTP_ADDREQ_FLAG_ADD);

    std::string bodyStr = BuildRequestBody(messages);

    if (!WinHttpSendRequest(hRequest, WINHTTP_NO_ADDITIONAL_HEADERS, 0,
        (LPVOID)bodyStr.c_str(), (DWORD)bodyStr.length(), (DWORD)bodyStr.length(), 0))
    {
        WinHttpCloseHandle(hRequest);
        WinHttpCloseHandle(hConnect);
        WinHttpCloseHandle(hSession);
        callback("Error: Failed to send request", true);
        return false;
    }

    if (!WinHttpReceiveResponse(hRequest, NULL))
    {
        WinHttpCloseHandle(hRequest);
        WinHttpCloseHandle(hConnect);
        WinHttpCloseHandle(hSession);
        callback("Error: Failed to receive response", true);
        return false;
    }

    DWORD statusCode = 0;
    DWORD statusSize = sizeof(DWORD);
    WinHttpQueryHeaders(hRequest, WINHTTP_QUERY_STATUS_CODE | WINHTTP_QUERY_FLAG_NUMBER,
        NULL, &statusCode, &statusSize, NULL);

    if (statusCode != 200)
    {
        std::string errorBuf;
        DWORD bytesRead = 0;
        char buffer[4096];
        while (WinHttpReadData(hRequest, buffer, sizeof(buffer) - 1, &bytesRead) && bytesRead > 0)
        {
            buffer[bytesRead] = 0;
            errorBuf += buffer;
        }

        std::string errMsg = "Error: HTTP " + std::to_string(statusCode) + "\n" + errorBuf;
        callback(errMsg, true);

        WinHttpCloseHandle(hRequest);
        WinHttpCloseHandle(hConnect);
        WinHttpCloseHandle(hSession);
        return false;
    }

    std::string sseBuffer;
    DWORD bytesRead = 0;
    char buffer[4096];

    while (WinHttpReadData(hRequest, buffer, sizeof(buffer) - 1, &bytesRead) && bytesRead > 0)
    {
        buffer[bytesRead] = 0;
        sseBuffer += buffer;

        size_t pos;
        while ((pos = sseBuffer.find("\n\n")) != std::string::npos)
        {
            std::string chunk = sseBuffer.substr(0, pos);
            sseBuffer.erase(0, pos + 2);

            std::string content;
            bool done = false;
            if (ParseSSE(chunk, content, done))
            {
                if (!content.empty())
                    callback(content, false);
                if (done)
                {
                    callback("", true);
                }
            }
        }
    }

    callback("", true);

    WinHttpCloseHandle(hRequest);
    WinHttpCloseHandle(hConnect);
    WinHttpCloseHandle(hSession);
    return true;
}
