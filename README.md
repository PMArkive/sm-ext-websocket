# SourceMod WebSocket Extension

## What is this?
This is a [SourceMod](http://www.sourcemod.net/) extension that provides some methods for HTTP JSON and websocket communication

## Features
* Relies on [IXWebSocket](https://github.com/ProjectSky/IXWebSocket/tree/cpp17-refactor) which is C++ library for WebSocket client and server development. It has minimal dependencies
* Support TEXT and JSON data
* Support client and server
* Support permessage-deflate
* Support SSL
* Support x64
* Support Proxy (HTTP/SOCKS5)
* Support HTTP RESTful API with JSON and form data

## Dependencies
* [sm-ext-json](https://github.com/ProjectSky/sm-ext-json) - **Optional**. Required only for JSON-related features. If not installed, TEXT-based WebSocket communication and non-JSON HTTP features will still work
* **For Linux users** - **Required**: SSL/TLS support requires OpenSSL development packages
  - For x64: `libssl-dev`
  - For x86: `libssl-dev:i386`

## Installation
1. **(Optional)** If you need JSON functionality, download and install [sm-ext-json](https://github.com/ProjectSky/sm-ext-json/releases) first
2. Download this extension from [sm-ext-websocket](https://github.com/ProjectSky/sm-ext-websocket/releases)
3. Extract the files to your SourceMod directory

## How to build this?
``` sh
sudo dpkg --add-architecture i386
sudo apt-get update
sudo apt-get install clang g++-multilib libssl-dev libssl-dev:i386
git clone --recursive https://github.com/ProjectSky/sm-ext-websocket.git
cd sm-ext-websocket
mkdir build && cd build
python ../configure.py --enable-optimize --symbol-files --sm-path=YOU_SOURCEMOD_PATH --targets=x86,x64
ambuild
```

## Native
* [websocket](https://github.com/ProjectSky/sm-ext-websocket/blob/main/scripting/include/websocket/ws.inc)
* [http](https://github.com/ProjectSky/sm-ext-websocket/blob/main/scripting/include/websocket/http.inc)

# Binary files
* [GitHub Releases](https://github.com/ProjectSky/sm-ext-websocket/releases)

## TODO
- [x] WebSocket server support
- [x] Windows support
- [x] HTTP support
- [x] Use sourcemod extension interface for JSON functionality instead of bundling json library
- [x] Allow JSON library as an optional dependency
- [x] TLS/SSL configuration API
- [x] Proxy support
- [x] Include IXWebSocket and libz via git submodules

## NOTES
* Server will not process data during the hibernation. You can set sv_hibernate_when_empty to 0 to disable hibernation
* This extension uses a [forked version of IXWebSocket](https://github.com/ProjectSky/IXWebSocket/tree/cpp17-refactor) with significant core modifications to support new features (proxy, connection pooling, statistics, timeouts, etc.). The upstream IXWebSocket library is not compatible with this extension

## Example
* [Example Script](https://github.com/ProjectSky/sm-ext-websocket/tree/main/scripting)