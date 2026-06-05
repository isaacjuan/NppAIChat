# NppAIChat

AI Chat plugin for **Notepad++** — chat with OpenAI-compatible LLMs directly inside the editor.

![screenshot](https://img.shields.io/badge/platform-Windows-blue)
![C++17](https://img.shields.io/badge/C++-17-blue)
![License](https://img.shields.io/badge/license-MIT-green)

## Features

- **Dockable chat panel** — toggle via toolbar or menu
- **Streaming responses** — see the AI reply as it's generated
- **Code context** — auto-sends selected editor text as context
- **Insert response** — paste the last AI reply at the cursor
- **System prompt** — customizable behaviour (language, style, etc.)
- **Rounded bubbles** — Claude-inspired chat UI with dark/light mode
- **Configurable** — endpoint, model, API key, temperature, max tokens
- **OpenAI-compatible** — works with DeepSeek, OpenAI, and any compatible API

## Installation

1. Close Notepad++.
2. Download `NppAIChat.zip` from [Releases](https://github.com/isaacjuan/NppAIChat/releases).
3. Extract into `%PROGRAMFILES%\Notepad++\plugins\` — the result should be:
   ```
   Notepad++\plugins\NppAIChat\NppAIChat.dll
   ```
4. Restart Notepad++. The chat panel appears under **Plugins → AI Chat → Toggle Chat**.

Alternatively, install via **Plugins Admin** inside Notepad++ (search "AI Chat").

## Configuration

Open **Plugins → AI Chat → Configure** to set:

| Setting | Description | Default |
|---|---|---|
| API Endpoint | Base URL of the API | `https://api.deepseek.com` |
| Model | Model name | `deepseek-v4-flash` |
| API Key | Your API key | *(empty)* |
| System Prompt | Instruction prepended to every chat | `Always respond in the same language as the user's input` |
| Temperature | Randomness (0.0 – 2.0) | `0.7` |
| Max Tokens | Max response length | `16384` |

> **Note:** For DeepSeek, set the endpoint to `https://api.deepseek.com` and model to `deepseek-v4-flash`. The path `/chat/completions` is appended automatically.

## Usage

| Action | How |
|---|---|
| Send message | Type in the input box and click **Send** or press Enter |
| Send code selection | Select text in the editor, then **Plugins → AI Chat → Send Selection** |
| Insert response | Click **Insert ▼** — pastes the last AI reply at the cursor |
| Copy a message | Right-click a bubble → **Copy Message**, or double-click to open a view dialog |
| Clear chat | Click **Clear** |

## Building from Source

Requires Visual Studio 2022 (any edition) with C++ desktop workload.

```
git clone https://github.com/isaacjuan/NppAIChat.git
cd NppAIChat
```

Open `NppAIChat.sln`, set configuration to **Release x64**, and build.

The plugin DLL is output to `build\x64\Release\NppAIChat.dll`.

## License

MIT
