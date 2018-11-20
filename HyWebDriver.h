

#ifndef HY_WEBDRIVER_H
#define HY_WEBDRIVER_H

#include <webkit2/webkit2.h>

#define HY_WEBDRIVER_LISTEN_ADDRESS "127.0.0.1"
#define HY_WEBDRIVER_RECV_TIMEOUT_MS 250
#define HY_WEBDRIVER_RECV_BUF_SIZE 4096

/**
 * Adopt "Simple WebDriver-Like Command" protocol.
 * 
 * 1) URL
 * - request:
 *  ^POST /url HTTP/1.1<CRLF>
 *  <CRLF>
 *  <url><CRLF>
 *  $
 *  
 *  - OK response:
 *  ^OK<CRLF>
 *  $
 *  
 *  - Error response:
 *  ^ERROR<CRLF>
 *  <CRLF>
 *  <error message><CRLF>
 *  $
 *  
 *  2) TITLE
 *  - request:
 *  ^GET /title HTTP/1.1<CRLF>
 *  <CRLF>
 *  $
 *  
 *  - OK response:
 *  ^OK<CRLF>
 *  <CRLF>
 *  <title><CRLF>
 *  $
 *  
 *  - Error response:
 *  ^ERROR<CRLF>
 *  <CRLF>
 *  <error message><CRLF>
 *  $
 *  
 *  
 *  3) SOURCE
 *  - request:
 *  ^GET /source HTTP/1.1<CRLF>
 *  <CRLF>
 *  $
 *  
 *  - OK response:
 *  ^OK<CRLF>
 *  <CRLF>
 *  <source><CRLF>
 *  $
 *  
 *  - Error response:
 *  ^ERROR<CRLF>
 *  <CRLF>
 *  <error message><CRLF>
 *  $
 *  
 *  4) DELETE
 *  - request:
 *  ^DELETE / HTTP/1.1<CRLF>
 *  <CRLF>
 *  $
 *  
 *  - response:
 *  Nothing. Shall close the socket immediately after receiving DELETE command.
 *  
 */

class HyWebDriver {
	
private:
	
	int m_port;
	WebKitWebView *m_web_view;
	unsigned char *m_msg_buf;
	int m_msg_len;
	int m_listen_sockfd;
	int m_conn_sockfd;
	
private:
	void listen();
	void parseLoop();
	
	void parseMessage();
	void onPostUrl(const char *url);
	void onGetTitle();
	void onGetSource();
	void onDelete();
	
public:
	HyWebDriver() {
		m_msg_buf = (unsigned char *)malloc(HY_WEBDRIVER_RECV_BUF_SIZE);
	}
	
	~HyWebDriver() {
		free(m_msg_buf);
	}
	
	void setPort(int port) {m_port = port; }
	void setWebView(WebKitWebView *web_view) {m_web_view = web_view; }
	
	void start();
	void stop();
	void run();
};

















#endif // #ifndef HY_WEBDRIVER_H
