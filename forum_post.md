# [New Plugin] AI Chat — chat with LLMs directly inside Notepad++

**NppAIChat** is a new plugin that adds a dockable AI chat panel to Notepad++.

https://github.com/isaacjuan/NppAIChat

## Features

- **OpenAI-compatible** — works with DeepSeek, OpenAI, Ollama (local), GitHub Models, Azure OpenAI, and any API that follows the `/chat/completions` format
- **Streaming responses** — see the AI reply word by word as it's generated
- **Claude-style UI** — rounded bubbles with dark/light mode support
- **Code context** — select text in the editor and it's automatically sent as context to the AI
- **Insert into editor** — click a button to paste the last AI response at the cursor
- **Fully configurable** — endpoint, model, API key, system prompt, temperature, max tokens
- **Zero external dependencies** — uses WinHTTP for HTTPS and plain Win32 for the UI (no .NET, no curl, no Python)

## Quick start

1. Install via **Plugins → Plugins Admin → AI Chat** (once the PR is merged) or download from GitHub Releases
2. Open **Plugins → AI Chat → Configure** and set your API endpoint + key
3. **Plugins → AI Chat → Toggle Chat** to open the chat panel

Default configuration works with DeepSeek V4 Flash out of the box.

## Configuration examples

| Provider | Endpoint | Model |
|---|---|---|
| DeepSeek | `https://api.deepseek.com` | `deepseek-v4-flash` |
| OpenAI | `https://api.openai.com` | `gpt-4o` |
| Ollama (local) | `http://localhost:11434` | `llama3.2` |
| GitHub Models | `https://models.inference.ai.azure.com` | `gpt-4o` |

## Tech stack

- C++17, Visual Studio 2022, x64
- WinHTTP for HTTPS/streaming SSE
- Owner-drawn listbox for the bubble UI
- NPP dark mode aware

---

GitHub: https://github.com/isaacjuan/NppAIChat
Plugin Admin PR: https://github.com/notepad-plus-plus/nppPluginList/pull/1108
License: MIT

Feedback, issues, and contributions welcome!
