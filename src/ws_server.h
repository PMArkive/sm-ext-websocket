#include "extension.h"

class WebSocketServer
{
public:
	WebSocketServer(const std::string& host, int port, int addressFamily, int pingInterval);
	~WebSocketServer();

public:
	void OnMessage(const std::string &message, std::shared_ptr<ix::ConnectionState> connectionState, ix::WebSocket* client);
	void OnOpen(ix::WebSocketOpenInfo openInfo, std::shared_ptr<ix::ConnectionState> connectionState);
	void OnClose(ix::WebSocketCloseInfo closeInfo, std::shared_ptr<ix::ConnectionState> connectionState);
	void OnError(ix::WebSocketErrorInfo errorInfo, std::shared_ptr<ix::ConnectionState> connectionState);
	void broadcastMessage(const std::string& message);
	bool sendToClient(const std::string& clientId, const std::string& message);
	bool disconnectClient(const std::string& clientId);
	std::vector<std::string> getClientIds();
	
	ix::WebSocketServer m_webSocketServer;
	Handle_t m_webSocketServer_handle = BAD_HANDLE;

    std::mutex m_headersMutex;
    std::unordered_map<std::string, ix::WebSocketHttpHeaders> m_connectionHeaders;

	IChangeableForward *pMessageForward = nullptr;
	IChangeableForward *pOpenForward = nullptr;
	IChangeableForward *pCloseForward = nullptr;
	IChangeableForward *pErrorForward = nullptr;
};