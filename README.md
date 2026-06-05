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

> **Note:** The path `/chat/completions` is appended automatically to the endpoint URL. Just provide the base URL.

## Provider Setup Guide

### DeepSeek (default)
| Setting | Value |
|---|---|
| API Endpoint | `https://api.deepseek.com` |
| Model | `deepseek-v4-flash` |
| API Key | Get one at [platform.deepseek.com](https://platform.deepseek.com) |

### OpenAI
| Setting | Value |
|---|---|
| API Endpoint | `https://api.openai.com` |
| Model | `gpt-4o` or `gpt-4o-mini` or `gpt-3.5-turbo` |
| API Key | Get one at [platform.openai.com](https://platform.openai.com/api-keys) |

### Ollama (local, free)
1. Install Ollama from [ollama.com](https://ollama.com) and pull a model, e.g. `ollama pull llama3.2`
2. Ensure Ollama is running (`ollama serve`)
3. Configure the plugin:
| Setting | Value |
|---|---|
| API Endpoint | `http://localhost:11434` |
| Model | `llama3.2` (or any model you pulled) |
| API Key | *(leave empty)* |

> Note: Ollama uses HTTP (not HTTPS). The plugin will use plain HTTP for localhost endpoints.

### GitHub Models
| Setting | Value |
|---|---|
| API Endpoint | `https://models.inference.ai.azure.com` |
| Model | `gpt-4o` or `gpt-4o-mini` |
| API Key | Get one from [GitHub Marketplace → Models](https://github.com/marketplace/models) |

### Anthropic (via third-party proxy)
Anthropic's API is **not** OpenAI-compatible by default. Use a proxy like [openrouter.ai](https://openrouter.ai) or [litellm](https://github.com/BerriAI/litellm):

| Setting | Value |
|---|---|
| API Endpoint | `https://openrouter.ai/api/v1` |
| Model | `anthropic/claude-3.5-sonnet` |
| API Key | Get one at [openrouter.ai](https://openrouter.ai/keys) |

### Azure OpenAI
| Setting | Value |
|---|---|
| API Endpoint | `https://YOUR_RESOURCE.openai.azure.com` |
| Model | Your deployment name (e.g. `gpt-4o-deployment`) |
| API Key | Get one from [Azure Portal](https://portal.azure.com) → Your OpenAI resource → Keys and Endpoint |

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
