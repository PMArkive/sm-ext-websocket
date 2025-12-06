#include <sourcemod>
#include <websocket>

public Plugin myinfo = {
  name = "WebSocket Secure Server (WSS) Example",
  author = "ProjectSky",
  description = "Example of WSS (WebSocket over TLS/SSL) server configuration",
  version = "1.0.0",
  url = "https://github.com/ProjectSky/sm-ext-websocket"
}

WebSocketServer g_hWssServer;

public void OnPluginStart()
{
  RegServerCmd("wss_server_start", wss_StartWssServer);
  RegServerCmd("wss_server_start_mtls", wss_StartWssServerMutualTLS);
  RegServerCmd("wss_server_stop", wss_StopWssServer);
  RegServerCmd("wss_server_broadcast", wss_BroadcastMessage);
}

// Standard WSS server (single-way TLS, no client certificate required)
Action wss_StartWssServer(int args)
{
  g_hWssServer = new WebSocketServer("0.0.0.0", 8443);

  g_hWssServer.SetMessageCallback(OnMessage);
  g_hWssServer.SetOpenCallback(OnOpen);
  g_hWssServer.SetCloseCallback(OnClose);
  g_hWssServer.SetErrorCallback(OnError);

  g_hWssServer.SetTLSCertAndKey("addons/sourcemod/configs/ssl/server.crt", "addons/sourcemod/configs/ssl/server.key");

  // Don't require client certificates (standard HTTPS-like behavior)
  g_hWssServer.SetTLSCAFile("NONE");
  g_hWssServer.TLSEnabled = true;
  g_hWssServer.Start();

  PrintToServer("[WSS] Secure WebSocket server started on wss://0.0.0.0:8443");
  PrintToServer("[WSS] Mode: Single-way TLS (no client certificate required)");

  return Plugin_Handled;
}

// WSS server with mutual TLS (mTLS - requires client certificates)
Action wss_StartWssServerMutualTLS(int args)
{
  g_hWssServer = new WebSocketServer("0.0.0.0", 8443);

  g_hWssServer.SetMessageCallback(OnMessage);
  g_hWssServer.SetOpenCallback(OnOpen);
  g_hWssServer.SetCloseCallback(OnClose);
  g_hWssServer.SetErrorCallback(OnError);

  g_hWssServer.SetTLSCertAndKey("addons/sourcemod/configs/ssl/server.crt", "addons/sourcemod/configs/ssl/server.key");

  // Require client certificates signed by this CA
  // Only clients with valid certificates can connect
  g_hWssServer.SetTLSCAFile("addons/sourcemod/configs/ssl/client-ca.crt");
  g_hWssServer.TLSEnabled = true;
  g_hWssServer.Start();

  PrintToServer("[WSS] Secure WebSocket server started on wss://0.0.0.0:8443");
  PrintToServer("[WSS] Mode: Mutual TLS (client certificate REQUIRED)");

  return Plugin_Handled;
}

Action wss_StopWssServer(int args)
{
  if (g_hWssServer != null)
  {
    g_hWssServer.Stop();
    PrintToServer("[WSS] Server stopped");
  }
  return Plugin_Handled;
}

Action wss_BroadcastMessage(int args)
{
  if (g_hWssServer != null)
  {
    g_hWssServer.BroadcastMessage("Secure broadcast from WSS server");
    PrintToServer("[WSS] Broadcast message sent");
  }
  return Plugin_Handled;
}

void OnMessage(WebSocketServer ws, WebSocket client, const char[] message, int wireSize, const char[] RemoteAddr, const char[] RemoteId)
{
  PrintToServer("[WSS] Message from %s (%s): %s", RemoteAddr, RemoteId, message);

  char response[256];
  Format(response, sizeof(response), "Server received: %s", message);
  ws.SendMessageToClient(RemoteId, response);
}

void OnOpen(WebSocketServer ws, const char[] RemoteAddr, const char[] RemoteId)
{
  PrintToServer("[WSS] Client connected: %s (ID: %s)", RemoteAddr, RemoteId);
  PrintToServer("[WSS] Total clients: %d", ws.ClientsCount);

  ws.SendMessageToClient(RemoteId, "Welcome to the secure WebSocket server!");
}

void OnClose(WebSocketServer ws, int code, const char[] reason, const char[] RemoteAddr, const char[] RemoteId)
{
  PrintToServer("[WSS] Client disconnected: %s (ID: %s)", RemoteAddr, RemoteId);
  PrintToServer("[WSS] Close code: %d, Reason: %s", code, reason);
  PrintToServer("[WSS] Remaining clients: %d", ws.ClientsCount);
}

void OnError(WebSocketServer ws, const char[] errMsg, const char[] RemoteAddr, const char[] RemoteId)
{
  PrintToServer("[WSS] Error from %s (%s): %s", RemoteAddr, RemoteId, errMsg);
}

/*
 * SETUP INSTRUCTIONS:
 *
 * 1. Generate SSL certificates (for testing, use self-signed):
 *
 *    # Generate server private key
 *    openssl genrsa -out server.key 2048
 *
 *    # Generate server certificate (valid for 365 days)
 *    openssl req -x509 -new -nodes -key server.key -sha256 -days 365 -out server.crt \
 *      -subj "/CN=localhost"
 *
 *    # For mutual TLS: generate client CA (optional)
 *    openssl genrsa -out client-ca.key 2048
 *    openssl req -x509 -new -nodes -key client-ca.key -sha256 -days 365 -out client-ca.crt \
 *      -subj "/CN=ClientCA"
 *
 * 2. Place certificate files in:
 *    csgo/addons/sourcemod/configs/ssl/
 *
 * 3. For production, use certificates from a trusted CA (e.g., Let's Encrypt)
 *
 * 4. Test with wscat:
 *    # Without client certificate:
 *    wscat -c wss://localhost:8443 --no-check
 *
 *    # With client certificate (for mTLS):
 *    wscat -c wss://localhost:8443 --cert client.crt --key client.key --no-check
 *
 * SECURITY NOTES:
 * - Always use strong certificates in production
 * - Keep private keys (.key files) secure
 * - Use "NONE" for SetTLSCAFile only if you don't need client authentication
 * - For public servers, use single-way TLS (standard WSS)
 * - For internal/secure systems, consider mutual TLS (mTLS)
 */
