#include "http_request.h"

static HttpRequest *GetHttpPointer(IPluginContext *pContext, Handle_t Handle)
{
	HandleError err;
	HandleSecurity sec(pContext->GetIdentity(), myself->GetIdentity());

	HttpRequest *httpClient;
	if ((err = handlesys->ReadHandle(Handle, g_htHttp, &sec, (void **)&httpClient)) != HandleError_None)
	{
		pContext->ReportError("Invalid httpClient handle %x (error %d)", Handle, err);
		return nullptr;
	}

	return httpClient;
}

static cell_t http_CreateRequest(IPluginContext *pContext, const cell_t *params)
{
	char *url;
	pContext->LocalToString(params[1], &url);

	HttpRequest* pHttpRequest = new HttpRequest(url);

	HandleError err;
	HandleSecurity sec(pContext->GetIdentity(), myself->GetIdentity());
	pHttpRequest->m_httpclient_handle = handlesys->CreateHandleEx(g_htHttp, pHttpRequest, &sec, nullptr, &err);

	if (!pHttpRequest->m_httpclient_handle)
	{
		pContext->ReportError("Could not create HttpRequest handle (error %d)", err);
		return 0;
	}

	return pHttpRequest->m_httpclient_handle;
}

static cell_t http_Get(IPluginContext *pContext, const cell_t *params)
{
	HttpRequest *pHttpRequest = GetHttpPointer(pContext, params[1]);
	if (!pHttpRequest) return 0;

	IPluginFunction *callback = pContext->GetFunctionById(params[2]);

	if (pHttpRequest->pResponseForward) {
		forwards->ReleaseForward(pHttpRequest->pResponseForward);
	}

	pHttpRequest->pResponseForward = forwards->CreateForwardEx(nullptr, ET_Ignore, 5, nullptr,
		Param_Cell, Param_String, Param_Cell, Param_Cell, Param_Cell);
	if (!pHttpRequest->pResponseForward || !pHttpRequest->pResponseForward->AddFunction(callback))
	{
		pContext->ReportError("Could not create response forward.");
		return 0;
	}

	cell_t value = params[3];
	return pHttpRequest->Get(callback, value);
}

static cell_t http_PostJson(IPluginContext *pContext, const cell_t *params)
{
	IJsonManager* pJsonManager = g_WebsocketExt.GetJsonManager();
	if (!pJsonManager)
	{
		return pContext->ThrowNativeError("JSON extension not loaded");
	}

	HttpRequest *pHttpRequest = GetHttpPointer(pContext, params[1]);
	JsonValue *pJsonValue = pJsonManager->GetValueFromHandle(pContext, params[2]);

	if (!pHttpRequest || !pJsonValue) return 0;

	IPluginFunction *callback = pContext->GetFunctionById(params[3]);

	if (pHttpRequest->pResponseForward) {
		forwards->ReleaseForward(pHttpRequest->pResponseForward);
	}

	pHttpRequest->pResponseForward = forwards->CreateForwardEx(nullptr, ET_Ignore, 5, nullptr,
		Param_Cell, Param_String, Param_Cell, Param_Cell, Param_Cell);
	if (!pHttpRequest->pResponseForward || !pHttpRequest->pResponseForward->AddFunction(callback))
	{
		pContext->ReportError("Could not create response forward.");
		return 0;
	}

	cell_t value = params[4];
	return pHttpRequest->PostJson(pJsonValue, callback, value);
}

static cell_t http_AppendFormParam(IPluginContext *pContext, const cell_t *params)
{
	HttpRequest *pHttpRequest = GetHttpPointer(pContext, params[1]);
	if (!pHttpRequest) return 0;

	char *key, *value;
	pContext->LocalToString(params[2], &key);
	pContext->LocalToString(params[3], &value);

	pHttpRequest->AppendFormParam(key, value);
	return 1;
}

static cell_t http_PostForm(IPluginContext *pContext, const cell_t *params)
{
	HttpRequest *pHttpRequest = GetHttpPointer(pContext, params[1]);
	if (!pHttpRequest) return 0;

	IPluginFunction *callback = pContext->GetFunctionById(params[2]);

	if (pHttpRequest->pResponseForward) {
		forwards->ReleaseForward(pHttpRequest->pResponseForward);
	}

	pHttpRequest->pResponseForward = forwards->CreateForwardEx(nullptr, ET_Ignore, 5, nullptr,
		Param_Cell, Param_String, Param_Cell, Param_Cell, Param_Cell);
	if (!pHttpRequest->pResponseForward || !pHttpRequest->pResponseForward->AddFunction(callback))
	{
		pContext->ReportError("Could not create response forward.");
		return 0;
	}

	cell_t value = params[3];
	return pHttpRequest->PostForm(callback, value);
}

static cell_t http_PutJson(IPluginContext *pContext, const cell_t *params)
{
	IJsonManager* pJsonManager = g_WebsocketExt.GetJsonManager();
	if (!pJsonManager)
	{
		return pContext->ThrowNativeError("JSON extension not loaded");
	}

	HttpRequest *pHttpRequest = GetHttpPointer(pContext, params[1]);
	JsonValue *pJsonValue = pJsonManager->GetValueFromHandle(pContext, params[2]);

	if (!pHttpRequest || !pJsonValue) return 0;

	IPluginFunction *callback = pContext->GetFunctionById(params[3]);

	if (pHttpRequest->pResponseForward) {
		forwards->ReleaseForward(pHttpRequest->pResponseForward);
	}

	pHttpRequest->pResponseForward = forwards->CreateForwardEx(nullptr, ET_Ignore, 5, nullptr,
		Param_Cell, Param_String, Param_Cell, Param_Cell, Param_Cell);
	if (!pHttpRequest->pResponseForward || !pHttpRequest->pResponseForward->AddFunction(callback))
	{
		pContext->ReportError("Could not create response forward.");
		return 0;
	}

	cell_t value = params[4];
	return pHttpRequest->PutJson(pJsonValue, callback, value);
}

static cell_t http_PatchJson(IPluginContext *pContext, const cell_t *params)
{
	IJsonManager* pJsonManager = g_WebsocketExt.GetJsonManager();
	if (!pJsonManager)
	{
		return pContext->ThrowNativeError("JSON extension not loaded");
	}

	HttpRequest *pHttpRequest = GetHttpPointer(pContext, params[1]);
	JsonValue *pJsonValue = pJsonManager->GetValueFromHandle(pContext, params[2]);

	if (!pHttpRequest || !pJsonValue) return 0;

	IPluginFunction *callback = pContext->GetFunctionById(params[3]);

	if (pHttpRequest->pResponseForward) {
		forwards->ReleaseForward(pHttpRequest->pResponseForward);
	}

	pHttpRequest->pResponseForward = forwards->CreateForwardEx(nullptr, ET_Ignore, 5, nullptr,
		Param_Cell, Param_String, Param_Cell, Param_Cell, Param_Cell);
	if (!pHttpRequest->pResponseForward || !pHttpRequest->pResponseForward->AddFunction(callback))
	{
		pContext->ReportError("Could not create response forward.");
		return 0;
	}

	cell_t value = params[4];
	return pHttpRequest->PatchJson(pJsonValue, callback, value);
}

static cell_t http_Delete(IPluginContext *pContext, const cell_t *params)
{
	HttpRequest *pHttpRequest = GetHttpPointer(pContext, params[1]);
	if (!pHttpRequest) return 0;

	IPluginFunction *callback = pContext->GetFunctionById(params[2]);

	if (pHttpRequest->pResponseForward) {
		forwards->ReleaseForward(pHttpRequest->pResponseForward);
	}

	pHttpRequest->pResponseForward = forwards->CreateForwardEx(nullptr, ET_Ignore, 5, nullptr,
		Param_Cell, Param_String, Param_Cell, Param_Cell, Param_Cell);
	if (!pHttpRequest->pResponseForward || !pHttpRequest->pResponseForward->AddFunction(callback))
	{
		pContext->ReportError("Could not create response forward.");
		return 0;
	}

	cell_t value = params[3];
	return pHttpRequest->Delete(callback, value);
}

static cell_t http_SetBody(IPluginContext *pContext, const cell_t *params)
{
	HttpRequest *pHttpRequest = GetHttpPointer(pContext, params[1]);
	if (!pHttpRequest) return 0;

	char *body;
	pContext->LocalToString(params[2], &body);
	pHttpRequest->SetBody(body);
	return 1;
}

static cell_t http_SetJsonBody(IPluginContext *pContext, const cell_t *params)
{
	IJsonManager* pJsonManager = g_WebsocketExt.GetJsonManager();
	if (!pJsonManager)
	{
		return pContext->ThrowNativeError("JSON extension not loaded");
	}

	HttpRequest *pHttpRequest = GetHttpPointer(pContext, params[1]);
	JsonValue *pJsonValue = pJsonManager->GetValueFromHandle(pContext, params[2]);

	if (!pHttpRequest || !pJsonValue) return 0;

	pHttpRequest->SetJsonBody(pJsonValue);
	return 1;
}

static cell_t http_AddHeader(IPluginContext *pContext, const cell_t *params)
{
	HttpRequest *pHttpRequest = GetHttpPointer(pContext, params[1]);
	if (!pHttpRequest) return 0;

	char *key, *value;
	pContext->LocalToString(params[2], &key);
	pContext->LocalToString(params[3], &value);

	pHttpRequest->AddHeader(key, value);
	return 1;
}

static cell_t http_GetResponseHeader(IPluginContext *pContext, const cell_t *params)
{
	HttpRequest *pHttpRequest = GetHttpPointer(pContext, params[1]);
	if (!pHttpRequest) return 0;

	char *key;
	pContext->LocalToString(params[2], &key);

	const std::string& value = pHttpRequest->GetResponseHeader(key);
	pContext->StringToLocalUTF8(params[3], params[4], value.c_str(), nullptr);

	return !value.empty();
}

static cell_t http_HasResponseHeader(IPluginContext *pContext, const cell_t *params)
{
	HttpRequest *pHttpRequest = GetHttpPointer(pContext, params[1]);
	if (!pHttpRequest) return 0;

	char *key;
	pContext->LocalToString(params[2], &key);

	return pHttpRequest->HasResponseHeader(key);
}

static cell_t http_GetTimeout(IPluginContext *pContext, const cell_t *params)
{
	HttpRequest *pHttpRequest = GetHttpPointer(pContext, params[1]);
	if (!pHttpRequest) return 0;

	return pHttpRequest->GetTimeout();
}

static cell_t http_SetTimeout(IPluginContext *pContext, const cell_t *params)
{
	HttpRequest *pHttpRequest = GetHttpPointer(pContext, params[1]);
	if (!pHttpRequest) return 0;

	pHttpRequest->SetTimeout(params[2]);
	return 1;
}

static cell_t http_GetFollowRedirect(IPluginContext *pContext, const cell_t *params)
{
	HttpRequest *pHttpRequest = GetHttpPointer(pContext, params[1]);
	if (!pHttpRequest) return 0;

	return pHttpRequest->GetFollowRedirect();
}

static cell_t http_SetFollowRedirect(IPluginContext *pContext, const cell_t *params)
{
	HttpRequest *pHttpRequest = GetHttpPointer(pContext, params[1]);
	if (!pHttpRequest) return 0;

	pHttpRequest->SetFollowRedirect(params[2]);
	return 1;
}

static cell_t http_GetCompression(IPluginContext *pContext, const cell_t *params)
{
	HttpRequest *pHttpRequest = GetHttpPointer(pContext, params[1]);
	if (!pHttpRequest) return 0;

	return pHttpRequest->GetCompression();
}

static cell_t http_SetCompression(IPluginContext *pContext, const cell_t *params)
{
	HttpRequest *pHttpRequest = GetHttpPointer(pContext, params[1]);
	if (!pHttpRequest) return 0;

	pHttpRequest->SetCompression(params[2]);

	return 1;
}

static cell_t http_GetMaxRedirects(IPluginContext *pContext, const cell_t *params)
{
	HttpRequest *pHttpRequest = GetHttpPointer(pContext, params[1]);
	if (!pHttpRequest) return 0;

	return pHttpRequest->GetMaxRedirects();
}

static cell_t http_SetMaxRedirects(IPluginContext *pContext, const cell_t *params)
{
	HttpRequest *pHttpRequest = GetHttpPointer(pContext, params[1]);
	if (!pHttpRequest) return 0;

	pHttpRequest->SetMaxRedirects(params[2]);

	return 1;
}

static cell_t http_GetVerbose(IPluginContext *pContext, const cell_t *params)
{
	HttpRequest *pHttpRequest = GetHttpPointer(pContext, params[1]);
	if (!pHttpRequest) return 0;

	return pHttpRequest->GetVerbose();
}

static cell_t http_SetVerbose(IPluginContext *pContext, const cell_t *params)
{
	HttpRequest *pHttpRequest = GetHttpPointer(pContext, params[1]);
	if (!pHttpRequest) return 0;

	pHttpRequest->SetVerbose(params[2]);

	return 1;
}

static cell_t http_SetTLSCertAndKey(IPluginContext *pContext, const cell_t *params)
{
	HttpRequest* pHttpRequest = GetHttpPointer(pContext, params[1]);
	if (!pHttpRequest) return 0;

	char *certFile, *keyFile;
	pContext->LocalToString(params[2], &certFile);
	pContext->LocalToString(params[3], &keyFile);

	char certAbsolutePath[PLATFORM_MAX_PATH];
	char keyAbsolutePath[PLATFORM_MAX_PATH];
	smutils->BuildPath(Path_Game, certAbsolutePath, sizeof(certAbsolutePath), "%s", certFile);
	smutils->BuildPath(Path_Game, keyAbsolutePath, sizeof(keyAbsolutePath), "%s", keyFile);

	pHttpRequest->SetTLSCertAndKey(certAbsolutePath, keyAbsolutePath);

	if (!pHttpRequest->m_tlsOptions.isValid())
	{
		return pContext->ThrowNativeError("TLS configuration error: %s",
			pHttpRequest->m_tlsOptions.getErrorMsg().c_str());
	}

	return 1;
}

static cell_t http_SetTLSCAFile(IPluginContext *pContext, const cell_t *params)
{
	HttpRequest* pHttpRequest = GetHttpPointer(pContext, params[1]);
	if (!pHttpRequest) return 0;

	char *caFile;
	pContext->LocalToString(params[2], &caFile);

	if (strcmp(caFile, "SYSTEM") == 0 || strcmp(caFile, "NONE") == 0)
	{
		pHttpRequest->SetTLSCAFile(caFile);
	}
	else
	{
		char absolutePath[PLATFORM_MAX_PATH];
		smutils->BuildPath(Path_Game, absolutePath, sizeof(absolutePath), "%s", caFile);
		pHttpRequest->SetTLSCAFile(absolutePath);
	}

	if (!pHttpRequest->m_tlsOptions.isValid())
	{
		return pContext->ThrowNativeError("TLS configuration error: %s",
			pHttpRequest->m_tlsOptions.getErrorMsg().c_str());
	}

	return 1;
}

static cell_t http_SetTLSCiphers(IPluginContext *pContext, const cell_t *params)
{
	HttpRequest* pHttpRequest = GetHttpPointer(pContext, params[1]);
	if (!pHttpRequest) return 0;

	char *ciphers;
	pContext->LocalToString(params[2], &ciphers);
	pHttpRequest->SetTLSCiphers(ciphers);

	return 1;
}

static cell_t http_HostnameValidation(IPluginContext *pContext, const cell_t *params)
{
	HttpRequest* pHttpRequest = GetHttpPointer(pContext, params[1]);
	if (!pHttpRequest) return 0;

	if (params[0] == 2) {
		pHttpRequest->SetHostnameValidation(params[2] != 0);
		return 1;
	}

	return pHttpRequest->GetHostnameValidation();
}

static cell_t http_KeepAlive(IPluginContext *pContext, const cell_t *params)
{
	HttpRequest* pHttpRequest = GetHttpPointer(pContext, params[1]);
	if (!pHttpRequest) return 0;

	if (params[0] == 2) {
		pHttpRequest->SetKeepAlive(params[2] != 0);
		return 1;
	}

	return pHttpRequest->GetKeepAlive();
}

static cell_t http_UseConnectionPool(IPluginContext *pContext, const cell_t *params)
{
	HttpRequest* pHttpRequest = GetHttpPointer(pContext, params[1]);
	if (!pHttpRequest) return 0;

	if (params[0] == 2) {
		pHttpRequest->SetUseConnectionPool(params[2] != 0);
		return 1;
	}

	return pHttpRequest->GetUseConnectionPool();
}

static cell_t http_SetProxy(IPluginContext *pContext, const cell_t *params)
{
	HttpRequest* pHttpRequest = GetHttpPointer(pContext, params[1]);
	if (!pHttpRequest) return 0;

	char *proxyUrl;
	pContext->LocalToString(params[2], &proxyUrl);
	pHttpRequest->SetProxy(proxyUrl);

	return 1;
}

static cell_t http_ClearProxy(IPluginContext *pContext, const cell_t *params)
{
	HttpRequest* pHttpRequest = GetHttpPointer(pContext, params[1]);
	if (!pHttpRequest) return 0;

	pHttpRequest->ClearProxy();
	return 1;
}

static cell_t http_HasProxy(IPluginContext *pContext, const cell_t *params)
{
	HttpRequest* pHttpRequest = GetHttpPointer(pContext, params[1]);
	if (!pHttpRequest) return 0;

	return pHttpRequest->HasProxy();
}

static cell_t http_SetBasicAuth(IPluginContext *pContext, const cell_t *params)
{
	HttpRequest* pHttpRequest = GetHttpPointer(pContext, params[1]);
	if (!pHttpRequest) return 0;

	char *username, *password;
	pContext->LocalToString(params[2], &username);
	pContext->LocalToString(params[3], &password);
	pHttpRequest->SetBasicAuth(username, password);

	return 1;
}

static cell_t http_SetBearerAuth(IPluginContext *pContext, const cell_t *params)
{
	HttpRequest* pHttpRequest = GetHttpPointer(pContext, params[1]);
	if (!pHttpRequest) return 0;

	char *token;
	pContext->LocalToString(params[2], &token);
	pHttpRequest->SetBearerAuth(token);

	return 1;
}

static cell_t http_ClearAuth(IPluginContext *pContext, const cell_t *params)
{
	HttpRequest* pHttpRequest = GetHttpPointer(pContext, params[1]);
	if (!pHttpRequest) return 0;

	pHttpRequest->ClearAuth();
	return 1;
}

const sp_nativeinfo_t http_natives[] =
{
	{"HttpRequest.HttpRequest", http_CreateRequest},
	{"HttpRequest.Get", http_Get},
	{"HttpRequest.PostJson", http_PostJson},
	{"HttpRequest.PostForm", http_PostForm},
	{"HttpRequest.PutJson", http_PutJson},
	{"HttpRequest.PatchJson", http_PatchJson},
	{"HttpRequest.Delete", http_Delete},
	{"HttpRequest.AppendFormParam", http_AppendFormParam},
	{"HttpRequest.SetBody", http_SetBody},
	{"HttpRequest.SetJsonBody", http_SetJsonBody},
	{"HttpRequest.AddHeader", http_AddHeader},
	{"HttpRequest.GetResponseHeader", http_GetResponseHeader},
	{"HttpRequest.HasResponseHeader", http_HasResponseHeader},
	{"HttpRequest.Timeout.get", http_GetTimeout},
	{"HttpRequest.Timeout.set", http_SetTimeout},
	{"HttpRequest.FollowRedirect.get", http_GetFollowRedirect},
	{"HttpRequest.FollowRedirect.set", http_SetFollowRedirect},
	{"HttpRequest.Compression.get", http_GetCompression},
	{"HttpRequest.Compression.set", http_SetCompression},
	{"HttpRequest.MaxRedirects.get", http_GetMaxRedirects},
	{"HttpRequest.MaxRedirects.set", http_SetMaxRedirects},
	{"HttpRequest.Verbose.get", http_GetVerbose},
	{"HttpRequest.Verbose.set", http_SetVerbose},
	{"HttpRequest.SetTLSCertAndKey", http_SetTLSCertAndKey},
	{"HttpRequest.SetTLSCAFile", http_SetTLSCAFile},
	{"HttpRequest.SetTLSCiphers", http_SetTLSCiphers},
	{"HttpRequest.HostnameValidation.get", http_HostnameValidation},
	{"HttpRequest.HostnameValidation.set", http_HostnameValidation},
	{"HttpRequest.KeepAlive.get", http_KeepAlive},
	{"HttpRequest.KeepAlive.set", http_KeepAlive},
	{"HttpRequest.UseConnectionPool.get", http_UseConnectionPool},
	{"HttpRequest.UseConnectionPool.set", http_UseConnectionPool},
	{"HttpRequest.SetProxy", http_SetProxy},
	{"HttpRequest.ClearProxy", http_ClearProxy},
	{"HttpRequest.HasProxy.get", http_HasProxy},
	{"HttpRequest.SetBasicAuth", http_SetBasicAuth},
	{"HttpRequest.SetBearerAuth", http_SetBearerAuth},
	{"HttpRequest.ClearAuth", http_ClearAuth},
	{nullptr, nullptr}
};