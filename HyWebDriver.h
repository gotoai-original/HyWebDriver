

#ifndef HY_WEBDRIVER_H
#define HY_WEBDRIVER_H

#include <webkit2/webkit2.h>
#include "miso_types.h"

#define HY_WEBDRIVER_LISTEN_ADDRESS "127.0.0.1"
#define HY_WEBDRIVER_RECV_TIMEOUT_MS 0
#define HY_WEBDRIVER_RECV_BUF_SIZE 4096
#define HY_WEBDRIVER_SEND_BUF_SIZE (1024 * 1024 * 8)
#define HY_WEBDRIVER_LOADPAGE_TIMEOUT_MS (1000 * 30)
#define HY_WEBDRIVER_OK "OK"
#define HY_WEBDRIVER_ERROR "ERROR"

#define HY_WEBDRIVER_STATE_BEGIN						0
#define HY_WEBDRIVER_STATE_LISTENING					10
#define HY_WEBDRIVER_STATE_CONNECTED					20
#define HY_WEBDRIVER_STATE_READY						30
#define HY_WEBDRIVER_STATE_IN_COMMAND					35
#define HY_WEBDRIVER_STATE_IN_URL						40
#define HY_WEBDRIVER_STATE_IN_URL_LOADING				41
#define HY_WEBDRIVER_STATE_IN_URL_LOAD_STOPPED			42
#define HY_WEBDRIVER_STATE_IN_URL_LOAD_FINISHED			43
#define HY_WEBDRIVER_STATE_IN_TITLE						50
#define HY_WEBDRIVER_STATE_IN_SOURCE					60
#define HY_WEBDRIVER_STATE_IN_SOURCE_RUNNING			61
#define HY_WEBDRIVER_STATE_IN_SOURCE_STOPPED			62
#define HY_WEBDRIVER_STATE_IN_SOURCE_FINISHED			63
#define HY_WEBDRIVER_STATE_IN_TEXTBOXES					70
#define HY_WEBDRIVER_STATE_IN_DELETE					80
#define HY_WEBDRIVER_STATE_END							99

/**
 * Adopt "Simple WebDriver-Like Command" protocol.
 * Selected commands from W3C webdriver protocol
 * --------
 * GET /status
 * GET /timeouts
 * POST /timeouts
 * POST /url
 * GET /url
 * GET /title
 * GET /window/rect
 * POST /window/rect
 * GET /source
 * GET /textboxes
 * DELETE /
 * --------
 * 
 * 4) URL
 * - request:
 *  POST /url HTTP/1.1<CRLF>
 *  <CRLF>
 *  <url><CRLF>
 *   *  
 *  - OK response:
 *  OK<CRLF>
 *  Content-Length: nnn<CRLF>
 *  <CRLF>
 *  Finished.<CRLF>
 *  
 *  - Error response:
 *  ERROR<CRLF>
 *  Content-Length: nnn<CRLF>
 *  <CRLF>
 *  <error message><CRLF>
 *  
 *  
 *  6) TITLE
 *  - request:
 *  GET /title HTTP/1.1<CRLF>
 *  <CRLF>
 *  
 *  
 *  - OK response:
 *  OK<CRLF>
 *  Content-Length: nnn<CRLF>
 *  <CRLF>
 *  <title><CRLF>
 *  
 *  
 *  - Error response:
 *  ERROR<CRLF>
 *  Content-Length: nnn<CRLF>
 *  <CRLF>
 *  <error message><CRLF>
 *  
 *  
 *  
 *  9) SOURCE
 *  - request:
 *  GET /source HTTP/1.1<CRLF>
 *  <CRLF>
 *  
 *  
 *  - OK response:
 *  OK<CRLF>
 *  <CRLF>
 *  Content-Length: nnn<CRLF>
 *  <source><CRLF>
 *  
 *  
 *  - Error response:
 *  ERROR<CRLF>
 *  Content-Length: nnn<CRLF>
 *  <CRLF>
 *  <error message><CRLF>
 *  
 *  
 *  11) DELETE
 *  - request:
 *  DELETE / HTTP/1.1<CRLF>
 *  <CRLF>
 *  
 *  
 *  - response:
 *  Nothing. Shall close the socket and exit the process immediately after receiving DELETE command.
 *  
 */

class HyWebDriver {
	
private:
	
	int m_state;
	int m_port;
	GtkWidget* m_main_window;
	WebKitWebView *m_web_view;
	char *m_msg_buf;
	char *m_send_buf;
	char *m_source_buf;
	int m_source_len;
	int m_msg_len;
	int m_listen_sockfd;
	int m_conn_sockfd;
	
	mINT64 m_loading_start_time;
	
private:
		
	void parseMessage();
	void onPostUrl(const char *url);
	void onGetTitle();
	void onGetSource();
	void onDelete();
	
	void sendResponse(const char *result, const char *err_msg, int msg_len);
	
	
public:
	HyWebDriver() {
		m_state = HY_WEBDRIVER_STATE_BEGIN;
		m_msg_buf = (char *)malloc(HY_WEBDRIVER_RECV_BUF_SIZE);
		m_send_buf = (char *)malloc(HY_WEBDRIVER_SEND_BUF_SIZE);
		m_source_buf = NULL;
	}
	
	~HyWebDriver() {
		free(m_msg_buf);
		free(m_send_buf);
		freeSourceBuf();

	}
	
	void listenWaitConnection();
	
	void setPort(int port) {m_port = port; }
	void createWebView(GtkWidget* main_window);
	GtkWidget* getMainWindow() { return m_main_window; }
	gint checkOnce();
	
	void allocSourceBuf(int source_len) { m_source_buf = (char *)malloc(source_len); m_source_len = source_len; }
	void freeSourceBuf() {	if(m_source_buf != NULL) { free(m_source_buf); m_source_buf = NULL;} }
	char *getSourceBuf() { return m_source_buf; }
	
	void setState(int state) { m_state = state; }
	int getState() { return m_state; } 
	
	void close() { ::close(m_conn_sockfd); }
	void exitAll();
};

















#endif // #ifndef HY_WEBDRIVER_H
