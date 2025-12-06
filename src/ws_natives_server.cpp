#include "ws_server.h"

static WebSocketServer *GetWsServerPointer(IPluginContext *pContext, Handle_t Handle)
{
	HandleError err;
	HandleSecurity sec(pContext->GetIdentity(), myself->GetIdentity());

	WebSocketServer *server;
	if ((err = handlesys->ReadHandle(Handle, g_htWsServer, &sec, (void **)&server)) != HandleError_None)
	{
		pContext->ReportError("Invalid WebSocketServer handle %x (error %d)", Handle, err);
		return nullptr;
	}

	return server;
}

static cell_t ws_CreateWebSocketServer(IPluginContext *pContext, const cell_t *params)
{
	char *url;
	pContext->LocalToString(params[1], &url);

	WebSocketServer* pWebsocketServer = new WebSocketServer(url, params[2], params[3] ? AF_INET6 : AF_INET, params[4]);

	HandleError err;
	HandleSecurity sec(pContext->GetIdentity(), myself->GetIdentity());
	pWebsocketServer->m_webSocketServer_handle = handlesys->CreateHandleEx(g_htWsServer, pWebsocketServer, &sec, nullptr, &err);

	if (!pWebsocketServer->m_webSocketServer_handle)
	{
		pContext->ReportError("Could not create WebSocketServer handle (error %d)", err);
		return 0;
	}

	return pWebsocketServer->m_webSocketServer_handle;
}

static cell_t ws_SetMessageCallback(IPluginContext *pContext, const cell_t *params)
{
	WebSocketServer *pWebsocketServer = GetWsServerPointer(pContext, params[1]);

	if (!pWebsocketServer) return 0;

	IPluginFunction *callback = pContext->GetFunctionById(params[2]);

	if (pWebsocketServer->pMessageForward) {
		forwards->ReleaseForward(pWebsocketServer->pMessageForward);
	}

	pWebsocketServer->pMessageForward = forwards->CreateForwardEx(nullptr, ET_Ignore, 6, nullptr, Param_Cell, Param_Cell, Param_String, Param_Cell, Param_String, Param_String);
	if (!pWebsocketServer->pMessageForward || !pWebsocketServer->pMessageForward->AddFunction(callback))
	{
		pContext->ReportError("Could not create message forward.");
		return 0;
	}

	return 1;
}

static cell_t ws_SetOpenCallback(IPluginContext *pContext, const cell_t *params)
{
	WebSocketServer *pWebsocketServer = GetWsServerPointer(pContext, params[1]);

	if (!pWebsocketServer) return 0;

	IPluginFunction *callback = pContext->GetFunctionById(params[2]);

	if (pWebsocketServer->pOpenForward) {
		forwards->ReleaseForward(pWebsocketServer->pOpenForward);
	}

	pWebsocketServer->pOpenForward = forwards->CreateForwardEx(nullptr, ET_Ignore, 3, nullptr, Param_Cell, Param_String, Param_String);
	if (!pWebsocketServer->pOpenForward || !pWebsocketServer->pOpenForward->AddFunction(callback))
	{
		pContext->ReportError("Could not create open forward.");
		return 0;
	}

	return 1;
}

static cell_t ws_SetCloseCallback(IPluginContext *pContext, const cell_t *params)
{
	WebSocketServer *pWebsocketServer = GetWsServerPointer(pContext, params[1]);

	if (!pWebsocketServer) return 0;

	IPluginFunction *callback = pContext->GetFunctionById(params[2]);

	if (pWebsocketServer->pCloseForward) {
		forwards->ReleaseForward(pWebsocketServer->pCloseForward);
	}

	pWebsocketServer->pCloseForward = forwards->CreateForwardEx(nullptr, ET_Ignore, 5, nullptr, Param_Cell, Param_Cell, Param_String, Param_String, Param_String);
	if (!pWebsocketServer->pCloseForward || !pWebsocketServer->pCloseForward->AddFunction(callback))
	{
		pContext->ReportError("Could not create close forward.");
		return 0;
	}

	return 1;
}

static cell_t ws_SetErrorCallback(IPluginContext *pContext, const cell_t *params)
{
	WebSocketServer *pWebsocketServer = GetWsServerPointer(pContext, params[1]);

	if (!pWebsocketServer) return 0;

	IPluginFunction *callback = pContext->GetFunctionById(params[2]);

	if (pWebsocketServer->pErrorForward) {
		forwards->ReleaseForward(pWebsocketServer->pErrorForward);
	}

	pWebsocketServer->pErrorForward = forwards->CreateForwardEx(nullptr, ET_Ignore, 4, nullptr, Param_Cell, Param_String, Param_String, Param_String);
	if (!pWebsocketServer->pErrorForward || !pWebsocketServer->pErrorForward->AddFunction(callback))
	{
		pContext->ReportError("Could not create error forward.");
		return 0;
	}

	return 1;
}

static cell_t ws_Start(IPluginContext *pContext, const cell_t *params)
{
	WebSocketServer* pWebsocketServer = GetWsServerPointer(pContext, params[1]);

	if (!pWebsocketServer) return 0;

	auto res = pWebsocketServer->m_webSocketServer.listen();

	if (res.has_value())
	{
		pContext->ReportError("Initiating WebSocket Server failed: %s", res->c_str());
		return 0;
	}

	pWebsocketServer->m_webSocketServer.start();

	return 1;
}

static cell_t ws_Stop(IPluginContext *pContext, const cell_t *params)
{
	WebSocketServer* pWebsocketServer = GetWsServerPointer(pContext, params[1]);

	if (!pWebsocketServer) return 0;

	pWebsocketServer->m_webSocketServer.stop();

	return 1;
}

static cell_t ws_SendMessageToClient(IPluginContext *pContext, const cell_t *params)
{
	WebSocketServer* pWebsocketServer = GetWsServerPointer(pContext, params[1]);

	if (!pWebsocketServer) return 0;

	char *clientId, *msg;
	pContext->LocalToString(params[2], &clientId);
	pContext->LocalToString(params[3], &msg);

	return pWebsocketServer->sendToClient(clientId, msg);
}

static cell_t ws_DisconnectClient(IPluginContext *pContext, const cell_t *params)
{
	WebSocketServer* pWebsocketServer = GetWsServerPointer(pContext, params[1]);

	if (!pWebsocketServer) return 0;

	char *clientId;
	pContext->LocalToString(params[2], &clientId);

	return pWebsocketServer->disconnectClient(clientId);
}

static cell_t ws_BroadcastMessage(IPluginContext *pContext, const cell_t *params)
{
	WebSocketServer* pWebsocketServer = GetWsServerPointer(pContext, params[1]);

	if (!pWebsocketServer) return 0;

	char *msg;
	pContext->LocalToString(params[2], &msg);

	pWebsocketServer->broadcastMessage(msg);

	return 1;
}

static cell_t ws_GetClientsCount(IPluginContext *pContext, const cell_t *params)
{
	WebSocketServer* pWebsocketServer = GetWsServerPointer(pContext, params[1]);

	if (!pWebsocketServer) return 0;

	return pWebsocketServer->m_webSocketServer.getClients().size();
}

static cell_t ws_SetOrGetPongEnable(IPluginContext *pContext, const cell_t *params)
{
	WebSocketServer* pWebsocketServer = GetWsServerPointer(pContext, params[1]);

	if (!pWebsocketServer) return 0;

	if (params[0] == 2) {
		pWebsocketServer->m_webSocketServer.setPong(params[2] != 0);
		return 1;
	}

	return pWebsocketServer->m_webSocketServer.isPongEnabled();
}

static cell_t ws_PerMessageDeflate(IPluginContext *pContext, const cell_t *params)
{
	WebSocketServer* pWebsocketServer = GetWsServerPointer(pContext, params[1]);
	if (!pWebsocketServer) return 0;

	if (params[0] == 2) {
		pWebsocketServer->m_webSocketServer.setPerMessageDeflate(params[2] != 0);
		return 1;
	}

	return pWebsocketServer->m_webSocketServer.isPerMessageDeflateEnabled();
}

static cell_t ws_GetHeader(IPluginContext *pContext, const cell_t *params)
{
	WebSocketServer* pWebsocketServer = GetWsServerPointer(pContext, params[1]);

	if (!pWebsocketServer) return 0;

	char *clientId, *headerKey;
	pContext->LocalToString(params[2], &clientId);
	pContext->LocalToString(params[3], &headerKey);

	ix::WebSocketHttpHeaders headers;
	if (!pWebsocketServer->getClientHeaders(clientId, headers))
	{
		return 0;
	}

	auto it = headers.find(headerKey);
	if (it == headers.end())
	{
		return 0;
	}

	pContext->StringToLocal(params[4], params[5], it->second.c_str());

	return 1;
}

static cell_t ws_GetClientIdByIndex(IPluginContext *pContext, const cell_t *params)
{
	WebSocketServer *pWebsocketServer = GetWsServerPointer(pContext, params[1]);
	if (!pWebsocketServer) return 0;

	cell_t index = params[2];
	const auto& ids = pWebsocketServer->getClientIds();

	if (index < 0 || static_cast<size_t>(index) >= ids.size())
	{
		return 0;
	}

	pContext->StringToLocal(params[3], params[4], ids[index].c_str());

	return 1;
}

static cell_t ws_GetMaxClientIdLength(IPluginContext *pContext, const cell_t *params)
{
	WebSocketServer *pWebsocketServer = GetWsServerPointer(pContext, params[1]);
	if (!pWebsocketServer)
	{
		return 0;
	}

	const auto& ids = pWebsocketServer->getClientIds();
	size_t maxLen = 0;

	for (const auto& id : ids)
	{
		if (id.length() > maxLen)
			maxLen = id.length();
	}

	return static_cast<cell_t>(maxLen + 1);
}

static cell_t ws_SetTLSCertAndKey(IPluginContext *pContext, const cell_t *params)
{
	WebSocketServer* pWebsocketServer = GetWsServerPointer(pContext, params[1]);
	if (!pWebsocketServer) return 0;

	char *certFile, *keyFile;
	pContext->LocalToString(params[2], &certFile);
	pContext->LocalToString(params[3], &keyFile);

	char certAbsolutePath[PLATFORM_MAX_PATH];
	char keyAbsolutePath[PLATFORM_MAX_PATH];
	smutils->BuildPath(Path_Game, certAbsolutePath, sizeof(certAbsolutePath), "%s", certFile);
	smutils->BuildPath(Path_Game, keyAbsolutePath, sizeof(keyAbsolutePath), "%s", keyFile);

	pWebsocketServer->m_tlsOptions.certFile = certAbsolutePath;
	pWebsocketServer->m_tlsOptions.keyFile = keyAbsolutePath;

	if (!pWebsocketServer->m_tlsOptions.isValid())
	{
		return pContext->ThrowNativeError("TLS configuration error: %s",
			pWebsocketServer->m_tlsOptions.getErrorMsg().c_str());
	}

	pWebsocketServer->m_webSocketServer.setTLSOptions(pWebsocketServer->m_tlsOptions);

	return 1;
}

static cell_t ws_SetTLSCAFile(IPluginContext *pContext, const cell_t *params)
{
	WebSocketServer* pWebsocketServer = GetWsServerPointer(pContext, params[1]);
	if (!pWebsocketServer) return 0;

	char *caFile;
	pContext->LocalToString(params[2], &caFile);

	if (strcmp(caFile, "SYSTEM") == 0 || strcmp(caFile, "NONE") == 0)
	{
		pWebsocketServer->m_tlsOptions.caFile = caFile;
	}
	else
	{
		char absolutePath[PLATFORM_MAX_PATH];
		smutils->BuildPath(Path_Game, absolutePath, sizeof(absolutePath), "%s", caFile);
		pWebsocketServer->m_tlsOptions.caFile = absolutePath;
	}

	if (!pWebsocketServer->m_tlsOptions.isValid())
	{
		return pContext->ThrowNativeError("TLS configuration error: %s",
			pWebsocketServer->m_tlsOptions.getErrorMsg().c_str());
	}

	pWebsocketServer->m_webSocketServer.setTLSOptions(pWebsocketServer->m_tlsOptions);

	return 1;
}

static cell_t ws_SetTLSCiphers(IPluginContext *pContext, const cell_t *params)
{
	WebSocketServer* pWebsocketServer = GetWsServerPointer(pContext, params[1]);
	if (!pWebsocketServer) return 0;

	char *ciphers;
	pContext->LocalToString(params[2], &ciphers);
	pWebsocketServer->m_tlsOptions.ciphers = ciphers;
	pWebsocketServer->m_webSocketServer.setTLSOptions(pWebsocketServer->m_tlsOptions);

	return 1;
}

static cell_t ws_TLSEnabled(IPluginContext *pContext, const cell_t *params)
{
	WebSocketServer* pWebsocketServer = GetWsServerPointer(pContext, params[1]);
	if (!pWebsocketServer) return 0;

	if (params[0] == 2) {
		pWebsocketServer->m_tlsOptions.tls = params[2] != 0;
		pWebsocketServer->m_webSocketServer.setTLSOptions(pWebsocketServer->m_tlsOptions);
		return 1;
	}

	return pWebsocketServer->m_tlsOptions.tls;
}

static cell_t ws_HostnameValidation(IPluginContext *pContext, const cell_t *params)
{
	WebSocketServer* pWebsocketServer = GetWsServerPointer(pContext, params[1]);
	if (!pWebsocketServer) return 0;

	if (params[0] == 2) {
		pWebsocketServer->m_tlsOptions.disable_hostname_validation = params[2] == 0;
		pWebsocketServer->m_webSocketServer.setTLSOptions(pWebsocketServer->m_tlsOptions);
		return 1;
	}

	return !pWebsocketServer->m_tlsOptions.disable_hostname_validation;
}

static cell_t ws_MaxConnectionsPerIp(IPluginContext *pContext, const cell_t *params)
{
	WebSocketServer* pWebsocketServer = GetWsServerPointer(pContext, params[1]);
	if (!pWebsocketServer) return 0;

	if (params[0] == 2) {
		pWebsocketServer->m_webSocketServer.setMaxConnectionsPerIp(params[2]);
		return 1;
	}

	return static_cast<cell_t>(pWebsocketServer->m_webSocketServer.getMaxConnectionsPerIp());
}

static cell_t ws_GetConnectionCountForIp(IPluginContext *pContext, const cell_t *params)
{
	WebSocketServer* pWebsocketServer = GetWsServerPointer(pContext, params[1]);
	if (!pWebsocketServer) return 0;

	char *ip;
	pContext->LocalToString(params[2], &ip);

	return static_cast<cell_t>(pWebsocketServer->m_webSocketServer.getConnectionCountForIp(ip));
}

static cell_t ws_PingTimeout(IPluginContext *pContext, const cell_t *params)
{
	WebSocketServer* pWebsocketServer = GetWsServerPointer(pContext, params[1]);
	if (!pWebsocketServer) return 0;

	if (params[0] == 2) {
		ix::WebSocketTimeouts timeouts = pWebsocketServer->m_webSocketServer.getTimeouts();
		timeouts.pingTimeoutSecs = params[2];
		pWebsocketServer->m_webSocketServer.setTimeouts(timeouts);
		return 1;
	}

	return pWebsocketServer->m_webSocketServer.getTimeouts().pingTimeoutSecs;
}

static cell_t ws_IdleTimeout(IPluginContext *pContext, const cell_t *params)
{
	WebSocketServer* pWebsocketServer = GetWsServerPointer(pContext, params[1]);
	if (!pWebsocketServer) return 0;

	if (params[0] == 2) {
		ix::WebSocketTimeouts timeouts = pWebsocketServer->m_webSocketServer.getTimeouts();
		timeouts.idleTimeoutSecs = params[2];
		pWebsocketServer->m_webSocketServer.setTimeouts(timeouts);
		return 1;
	}

	return pWebsocketServer->m_webSocketServer.getTimeouts().idleTimeoutSecs;
}

static cell_t ws_SendTimeout(IPluginContext *pContext, const cell_t *params)
{
	WebSocketServer* pWebsocketServer = GetWsServerPointer(pContext, params[1]);
	if (!pWebsocketServer) return 0;

	if (params[0] == 2) {
		ix::WebSocketTimeouts timeouts = pWebsocketServer->m_webSocketServer.getTimeouts();
		timeouts.sendTimeoutSecs = params[2];
		pWebsocketServer->m_webSocketServer.setTimeouts(timeouts);
		return 1;
	}

	return pWebsocketServer->m_webSocketServer.getTimeouts().sendTimeoutSecs;
}

static cell_t ws_CloseTimeout(IPluginContext *pContext, const cell_t *params)
{
	WebSocketServer* pWebsocketServer = GetWsServerPointer(pContext, params[1]);
	if (!pWebsocketServer) return 0;

	if (params[0] == 2) {
		ix::WebSocketTimeouts timeouts = pWebsocketServer->m_webSocketServer.getTimeouts();
		timeouts.closeTimeoutSecs = params[2];
		pWebsocketServer->m_webSocketServer.setTimeouts(timeouts);
		return 1;
	}

	return pWebsocketServer->m_webSocketServer.getTimeouts().closeTimeoutSecs;
}

static cell_t ws_AddSubProtocol(IPluginContext *pContext, const cell_t *params)
{
	WebSocketServer* pWebsocketServer = GetWsServerPointer(pContext, params[1]);
	if (!pWebsocketServer) return 0;

	char *protocol;
	pContext->LocalToString(params[2], &protocol);

	pWebsocketServer->m_webSocketServer.addSubProtocol(protocol);

	return 1;
}

static cell_t ws_HandshakeTimeout(IPluginContext *pContext, const cell_t *params)
{
	WebSocketServer* pWebsocketServer = GetWsServerPointer(pContext, params[1]);
	if (!pWebsocketServer) return 0;

	if (params[0] == 2) {
		pWebsocketServer->m_webSocketServer.setHandshakeTimeoutSecs(params[2]);
		return 1;
	}

	return pWebsocketServer->m_webSocketServer.getHandshakeTimeoutSecs();
}

static cell_t ws_RemoveSubProtocol(IPluginContext *pContext, const cell_t *params)
{
	WebSocketServer* pWebsocketServer = GetWsServerPointer(pContext, params[1]);
	if (!pWebsocketServer) return 0;

	char *protocol;
	pContext->LocalToString(params[2], &protocol);
	pWebsocketServer->m_webSocketServer.removeSubProtocol(protocol);

	return 1;
}

static cell_t ws_ClearSubProtocols(IPluginContext *pContext, const cell_t *params)
{
	WebSocketServer* pWebsocketServer = GetWsServerPointer(pContext, params[1]);
	if (!pWebsocketServer) return 0;

	pWebsocketServer->m_webSocketServer.clearSubProtocols();
	return 1;
}

const sp_nativeinfo_t ws_natives_server[] =
{
	{"WebSocketServer.WebSocketServer",             ws_CreateWebSocketServer},
	{"WebSocketServer.SetMessageCallback",          ws_SetMessageCallback},
	{"WebSocketServer.SetOpenCallback",             ws_SetOpenCallback},
	{"WebSocketServer.SetCloseCallback",            ws_SetCloseCallback},
	{"WebSocketServer.SetErrorCallback",            ws_SetErrorCallback},
	{"WebSocketServer.Start",                       ws_Start},
	{"WebSocketServer.Stop",                        ws_Stop},
	{"WebSocketServer.BroadcastMessage",            ws_BroadcastMessage},
	{"WebSocketServer.SendMessageToClient",         ws_SendMessageToClient},
	{"WebSocketServer.DisconnectClient",            ws_DisconnectClient},
	{"WebSocketServer.GetHeader",                   ws_GetHeader},
	{"WebSocketServer.ClientsCount.get",            ws_GetClientsCount},
	{"WebSocketServer.EnablePong.get",              ws_SetOrGetPongEnable},
	{"WebSocketServer.EnablePong.set",              ws_SetOrGetPongEnable},
	{"WebSocketServer.GetClientIdByIndex",          ws_GetClientIdByIndex},
	{"WebSocketServer.MaxClientIdLength.get",       ws_GetMaxClientIdLength},
	{"WebSocketServer.PerMessageDeflate.get",       ws_PerMessageDeflate},
	{"WebSocketServer.PerMessageDeflate.set",       ws_PerMessageDeflate},
	{"WebSocketServer.SetTLSCertAndKey",            ws_SetTLSCertAndKey},
	{"WebSocketServer.SetTLSCAFile",                ws_SetTLSCAFile},
	{"WebSocketServer.SetTLSCiphers",               ws_SetTLSCiphers},
	{"WebSocketServer.TLSEnabled.get",              ws_TLSEnabled},
	{"WebSocketServer.TLSEnabled.set",              ws_TLSEnabled},
	{"WebSocketServer.HostnameValidation.get",      ws_HostnameValidation},
	{"WebSocketServer.HostnameValidation.set",      ws_HostnameValidation},
	{"WebSocketServer.MaxConnectionsPerIp.get",     ws_MaxConnectionsPerIp},
	{"WebSocketServer.MaxConnectionsPerIp.set",     ws_MaxConnectionsPerIp},
	{"WebSocketServer.GetConnectionCountForIp",     ws_GetConnectionCountForIp},
	{"WebSocketServer.PingTimeout.get",             ws_PingTimeout},
	{"WebSocketServer.PingTimeout.set",             ws_PingTimeout},
	{"WebSocketServer.IdleTimeout.get",             ws_IdleTimeout},
	{"WebSocketServer.IdleTimeout.set",             ws_IdleTimeout},
	{"WebSocketServer.SendTimeout.get",             ws_SendTimeout},
	{"WebSocketServer.SendTimeout.set",             ws_SendTimeout},
	{"WebSocketServer.CloseTimeout.get",            ws_CloseTimeout},
	{"WebSocketServer.CloseTimeout.set",            ws_CloseTimeout},
	{"WebSocketServer.AddSubProtocol",              ws_AddSubProtocol},
	{"WebSocketServer.RemoveSubProtocol",           ws_RemoveSubProtocol},
	{"WebSocketServer.ClearSubProtocols",           ws_ClearSubProtocols},
	{"WebSocketServer.HandshakeTimeout.get",        ws_HandshakeTimeout},
	{"WebSocketServer.HandshakeTimeout.set",        ws_HandshakeTimeout},
	{nullptr, nullptr}
};