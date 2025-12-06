#ifndef WEBSOCKETEXTENSION_HTTP_REQUEST_H
#define WEBSOCKETEXTENSION_HTTP_REQUEST_H

#include "extension.h"

class HttpRequest
{
public:
	HttpRequest(const std::string &url);
	~HttpRequest();

	bool Get(IPluginFunction *callback, cell_t value);
	bool Delete(IPluginFunction *callback, cell_t value);
	bool PostJson(JsonValue* json, IPluginFunction *callback, cell_t value);
	bool PutJson(JsonValue* json, IPluginFunction *callback, cell_t value);
	bool PatchJson(JsonValue* json, IPluginFunction *callback, cell_t value);
	bool PostForm(IPluginFunction *callback, cell_t value);

	void AppendFormParam(const std::string &key, const std::string &value);

	void SetBody(const std::string &body);
	void SetJsonBody(JsonValue* json);
	void AddHeader(const std::string &key, const std::string &value);
	int GetTimeout() const;
	int GetMaxRedirects() const;
	bool GetFollowRedirect() const;
	bool GetCompression() const;
	bool GetVerbose() const;
	void SetTimeout(int timeout);
	void SetVerbose(bool verbose);
	void SetCompression(bool compress);
	void SetMaxRedirects(int maxRedirects);
	void SetFollowRedirect(bool follow);

	const std::string& GetResponseHeader(const std::string& key) const;
	bool HasResponseHeader(const std::string& key) const;
	const ix::WebSocketHttpHeaders& GetResponseHeaders() const;

	void onResponse(const ix::HttpResponsePtr response, cell_t value);

	// TLS configuration
	void SetTLSCertAndKey(const std::string& certFile, const std::string& keyFile);
	void SetTLSCAFile(const std::string& caFile);
	void SetTLSCiphers(const std::string& ciphers);
	void SetHostnameValidation(bool enable);
	bool GetHostnameValidation() const;

	// Connection options
	void SetKeepAlive(bool enable);
	bool GetKeepAlive() const;
	void SetUseConnectionPool(bool enable);
	bool GetUseConnectionPool() const;

	// Proxy
	void SetProxy(const std::string& proxyUrl);
	void ClearProxy();
	bool HasProxy() const;

	// Authentication
	void SetBasicAuth(const std::string& username, const std::string& password);
	void SetBearerAuth(const std::string& token);
	void ClearAuth();

	ix::HttpClient m_httpclient;
	ix::HttpRequestArgsPtr m_request;
	ix::SocketTLSOptions m_tlsOptions;
	ix::ProxyConfig m_proxyConfig;

	Handle_t m_httpclient_handle = BAD_HANDLE;
	IChangeableForward *pResponseForward = nullptr;

private:
	mutable std::mutex m_headersMutex;
	ix::WebSocketHttpHeaders m_responseHeaders;
	std::map<std::string, std::string> m_formParams;

	std::string BuildFormData();
};

class HttpResponseTaskContext : public ITaskContext
{
public:
	HttpResponseTaskContext(HttpRequest* client, const ix::HttpResponsePtr& response, cell_t value)
		: m_client(client), m_response(response), m_value(value){}

	virtual void OnCompleted() override;

private:
	HttpRequest* m_client;
	ix::HttpResponsePtr m_response;
	cell_t m_value;
};
#endif // WEBSOCKETEXTENSION_HTTP_REQUEST_H