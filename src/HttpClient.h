#pragma once

#include <string>
#include <vector>
#include <functional>

struct ChatMessage {
    std::string role;
    std::string content;
};

using StreamCallback = std::function<void(const std::string&, bool)>;

class HttpClient {
public:
    HttpClient();
    ~HttpClient();

    void SetEndpoint(const std::wstring& endpoint);
    void SetApiKey(const std::wstring& apiKey);
    void SetModel(const std::wstring& model);
    void SetSystemPrompt(const std::wstring& prompt);
    void SetTemperature(double temp);
    void SetMaxTokens(int tokens);

    std::wstring GetEndpoint() const { return m_endpoint; }
    std::wstring GetApiKey() const { return m_apiKey; }
    std::wstring GetModel() const { return m_model; }
    std::wstring GetSystemPrompt() const { return m_systemPrompt; }
    double GetTemperature() const { return m_temperature; }
    int GetMaxTokens() const { return m_maxTokens; }
    bool WasTruncated() const { return m_truncated; }

    bool SendChat(const std::vector<ChatMessage>& messages, StreamCallback callback);

private:
    std::wstring m_endpoint;
    std::wstring m_apiKey;
    std::wstring m_model;
    std::wstring m_systemPrompt;
    double m_temperature = 0.7;
    int m_maxTokens = 16384;
    bool m_initialized = false;
    bool m_truncated = false;

    bool EnsureInitialized();
    std::string BuildRequestBody(const std::vector<ChatMessage>& messages);
    bool ParseSSE(const std::string& data, std::string& content, bool& done);
};
