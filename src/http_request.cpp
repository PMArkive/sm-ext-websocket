#include "http_request.h"

HttpRequest::HttpRequest(const std::string &url) : m_httpclient(true)
{
	m_request = m_httpclient.createRequest(url);

	m_request->logger = [](const std::string& msg) {
		printf("[HTTP] %s", msg.c_str());
	};
}

HttpRequest::~HttpRequest()
{
	if (pResponseForward) forwards->ReleaseForward(pResponseForward);
}

void HttpRequest::SetBody(const std::string &body)
{
	m_request->body = body;
}

void HttpRequest::SetJsonBody(JsonValue* json)
{
	if (!json) return;

	IJsonManager* pJsonManager = g_WebsocketExt.GetJsonManager();
	if (!pJsonManager)
	{
		smutils->LogError(myself, "JSON extension not loaded, cannot set JSON body");
		return;
	}

	char* jsonStr = pJsonManager->WriteToStringPtr(json);
	if (!jsonStr) return;

	m_request->body = jsonStr;
	m_request->extraHeaders["Content-Type"] = "application/json";
	free(jsonStr);
}

void HttpRequest::AddHeader(const std::string &key, const std::string &value)
{
	m_request->extraHeaders[key] = value;
}

void HttpRequest::SetTimeout(int timeout)
{
	m_request->connectTimeout = timeout;
}

int HttpRequest::GetTimeout() const
{
	return m_request->connectTimeout;
}

void HttpRequest::SetFollowRedirect(bool follow)
{
	m_request->followRedirects = follow;
}

bool HttpRequest::GetFollowRedirect() const
{
	return m_request->followRedirects;
}

void HttpRequest::SetCompression(bool compress)
{
	m_request->compress = compress;
}

bool HttpRequest::GetCompression() const
{
	return m_request->compress;
}

void HttpRequest::SetMaxRedirects(int maxRedirects)
{
	m_request->maxRedirects = maxRedirects;
}

int HttpRequest::GetMaxRedirects() const
{
	return m_request->maxRedirects;
}

void HttpRequest::SetVerbose(bool verbose)
{
	m_request->verbose = verbose;
}

bool HttpRequest::GetVerbose() const
{
	return m_request->verbose;
}

void HttpRequest::onResponse(const ix::HttpResponsePtr response, cell_t value)
{
	if (!pResponseForward || !pResponseForward->GetFunctionCount())
	{
		return;
	}

	if (response) {
		std::lock_guard<std::mutex> lock(m_headersMutex);
		m_responseHeaders = response->headers;
	}

	HttpResponseTaskContext *context = new HttpResponseTaskContext(this, response, value);
	g_TaskQueue.Push(context);
}

void HttpResponseTaskContext::OnCompleted()
{
	HandleSecurity sec(nullptr, myself->GetIdentity());

	m_client->pResponseForward->PushCell(m_client->m_httpclient_handle);
	m_client->pResponseForward->PushString(m_response->body.c_str());
	m_client->pResponseForward->PushCell(m_response->statusCode);
	m_client->pResponseForward->PushCell(m_response->body.length() + 1);
	m_client->pResponseForward->PushCell(m_value);
	m_client->pResponseForward->Execute(nullptr);

	handlesys->FreeHandle(m_client->m_httpclient_handle, &sec);
}

bool HttpRequest::Get(IPluginFunction *callback, cell_t value)
{
	m_request->verb = "GET";
	return m_httpclient.performRequest(m_request,
		[this, value](const ix::HttpResponsePtr& response) {
			onResponse(response, value);
		});
}

bool HttpRequest::PostJson(JsonValue* json, IPluginFunction *callback, cell_t value)
{
	if (!json) return false;

	m_request->verb = "POST";
	SetJsonBody(json);
	return m_httpclient.performRequest(m_request,
		[this, value](const ix::HttpResponsePtr& response) {
			onResponse(response, value);
		});
}

bool HttpRequest::PutJson(JsonValue* json, IPluginFunction *callback, cell_t value)
{
	if (!json) return false;

	m_request->verb = "PUT";
	SetJsonBody(json);
	return m_httpclient.performRequest(m_request,
		[this, value](const ix::HttpResponsePtr& response) {
			onResponse(response, value);
		});
}

bool HttpRequest::PatchJson(JsonValue* json, IPluginFunction *callback, cell_t value)
{
	if (!json) return false;

	m_request->verb = "PATCH";
	SetJsonBody(json);
	return m_httpclient.performRequest(m_request,
		[this, value](const ix::HttpResponsePtr& response) {
			onResponse(response, value);
		});
}

bool HttpRequest::PostForm(IPluginFunction *callback, cell_t value)
{
	m_request->verb = "POST";
	m_request->body = BuildFormData();
	m_request->extraHeaders["Content-Type"] = "application/x-www-form-urlencoded";
	return m_httpclient.performRequest(m_request,
		[this, value](const ix::HttpResponsePtr& response) {
			onResponse(response, value);
		});
}

bool HttpRequest::Delete(IPluginFunction *callback, cell_t value)
{
	m_request->verb = "DELETE";
	return m_httpclient.performRequest(m_request,
		[this, value](const ix::HttpResponsePtr& response) {
			onResponse(response, value);
		});
}

std::string HttpRequest::BuildFormData()
{
	std::string formData;
	bool first = true;

	for (const auto& param : m_formParams) {
		if (!first) {
			formData += "&";
		}
		formData += m_httpclient.urlEncode(param.first) + "=" + m_httpclient.urlEncode(param.second);
		first = false;
	}

	return formData;
}

void HttpRequest::AppendFormParam(const std::string &key, const std::string &value)
{
	m_formParams[key] = value;
}

const std::string& HttpRequest::GetResponseHeader(const std::string& key) const
{
	static const std::string empty;
	std::lock_guard<std::mutex> lock(m_headersMutex);
	auto it = m_responseHeaders.find(key);
	return it != m_responseHeaders.end() ? it->second : empty;
}

bool HttpRequest::HasResponseHeader(const std::string& key) const
{
	std::lock_guard<std::mutex> lock(m_headersMutex);
	return m_responseHeaders.find(key) != m_responseHeaders.end();
}

const ix::WebSocketHttpHeaders& HttpRequest::GetResponseHeaders() const
{
	std::lock_guard<std::mutex> lock(m_headersMutex);
	return m_responseHeaders;
}

void HttpRequest::SetTLSCertAndKey(const std::string& certFile, const std::string& keyFile)
{
	m_tlsOptions.certFile = certFile;
	m_tlsOptions.keyFile = keyFile;

	if (m_tlsOptions.isValid())
	{
		m_httpclient.setTLSOptions(m_tlsOptions);
	}
}

void HttpRequest::SetTLSCAFile(const std::string& caFile)
{
	m_tlsOptions.caFile = caFile;

	if (m_tlsOptions.isValid())
	{
		m_httpclient.setTLSOptions(m_tlsOptions);
	}
}

void HttpRequest::SetTLSCiphers(const std::string& ciphers)
{
	m_tlsOptions.ciphers = ciphers;
	m_httpclient.setTLSOptions(m_tlsOptions);
}

void HttpRequest::SetHostnameValidation(bool enable)
{
	m_tlsOptions.disable_hostname_validation = !enable;
	m_httpclient.setTLSOptions(m_tlsOptions);
}

bool HttpRequest::GetHostnameValidation() const
{
	return !m_tlsOptions.disable_hostname_validation;
}

void HttpRequest::SetKeepAlive(bool enable)
{
	m_httpclient.setKeepAlive(enable);
}

bool HttpRequest::GetKeepAlive() const
{
	return m_httpclient.isKeepAliveEnabled();
}

void HttpRequest::SetUseConnectionPool(bool enable)
{
	m_httpclient.setUseConnectionPool(enable);
}

bool HttpRequest::GetUseConnectionPool() const
{
	return m_httpclient.isUseConnectionPoolEnabled();
}

void HttpRequest::SetProxy(const std::string& proxyUrl)
{
	m_proxyConfig = ix::ProxyConfig::fromUrl(proxyUrl);
	m_httpclient.setProxyConfig(m_proxyConfig);
}

void HttpRequest::ClearProxy()
{
	m_proxyConfig = ix::ProxyConfig();
	m_httpclient.setProxyConfig(m_proxyConfig);
}

bool HttpRequest::HasProxy() const
{
	return m_proxyConfig.isEnabled();
}

void HttpRequest::SetBasicAuth(const std::string& username, const std::string& password)
{
	m_request->setBasicAuth(username, password);
}

void HttpRequest::SetBearerAuth(const std::string& token)
{
	m_request->setBearerAuth(token);
}

void HttpRequest::ClearAuth()
{
	m_request->authType = ix::HttpAuthType::None;
	m_request->authUsername.clear();
	m_request->authPassword.clear();
	m_request->authToken.clear();
}