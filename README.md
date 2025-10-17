# SourceMod WebSocket Extension

## What is this?
This is a [SourceMod](http://www.sourcemod.net/) extension that provides some methods for HTTP JSON and websocket communication

## Features
* Relies on [IXWebSocket](https://github.com/machinezone/IXWebSocket) which is C++ library for WebSocket client and server development. It has minimal dependencies
* Support TEXT and JSON data
* Support client and server
* Support permessage-deflate
* Support SSL
* Support x64
* Support HTTP RESTful API with JSON and form data

## Dependencies
* [sm-ext-yyjson](https://github.com/ProjectSky/sm-ext-yyjson) - Required for JSON parsing and generation

## Installation
1. Download and install [sm-ext-yyjson](https://github.com/ProjectSky/sm-ext-yyjson/releases) first
2. Download this extension from [sm-ext-websocket](https://github.com/ProjectSky/sm-ext-websocket/releases)
3. Extract the files to your SourceMod directory

## How to build this?
``` sh
sudo dpkg --add-architecture i386
sudo apt-get update
sudo apt-get install clang g++-multilib zlib1g-dev zlib1g-dev:i386 libssl-dev libssl-dev:i386
clone project
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
- [x] Use sourcemod extension interface for JSON functionality instead of bundling yyjson library

## NOTES
* Server will not process data during the hibernation. You can set sv_hibernate_when_empty to 0 to disable hibernation

## Example
* [Example Script](https://github.com/ProjectSky/sm-ext-websocket/tree/main/scripting)