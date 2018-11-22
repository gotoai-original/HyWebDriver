

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <sys/types.h> 
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <fcntl.h>
#include <errno.h>

#include "JavaScript.h"
#include "HyWebDriver.h"
#include "miso_types.h"
#include "miso_timestamp.h"
#include "miso_common.h"
#include "miso_log.h"
#include "miso_thread.h"



static void on_web_view_load_changed (WebKitWebView  *web_view,
                                     WebKitLoadEvent load_event,
                                     gpointer        user_data)
{
	HyWebDriver *theDriver = (HyWebDriver*) user_data;
	
	switch (load_event) {
    
    case WEBKIT_LOAD_STARTED:
        /* New load, we have now a provisional URI */
        //provisional_uri = webkit_web_view_get_uri (web_view);
        /* Here we could start a spinner or update the
         * location bar with the provisional URI */
    	m_log_msg(LOG_FINE, "WEBKIT_LOAD_STARTED.");
        break;
    case WEBKIT_LOAD_REDIRECTED:
        //redirected_uri = webkit_web_view_get_uri (web_view);
    	m_log_msg(LOG_FINE, "WEBKIT_LOAD_REDIRECTED.");
        break;
    case WEBKIT_LOAD_COMMITTED:
        /* The load is being performed. Current URI is
         * the final one and it won't change unless a new
         * load is requested or a navigation within the
         * same page is performed */
        //uri = webkit_web_view_get_uri (web_view);
    	m_log_msg(LOG_FINE, "WEBKIT_LOAD_COMMITTED.");
        break;
    case WEBKIT_LOAD_FINISHED:
        /* Load finished, we can now stop the spinner */
    	m_log_msg(LOG_FINE, "WEBKIT_LOAD_FINISHED.");
    	if(theDriver->getState() == HY_WEBDRIVER_STATE_IN_URL_LOADING) {
    		theDriver->setState(HY_WEBDRIVER_STATE_IN_URL_LOAD_FINISHED);
    	}
        break;
    }
}


static gboolean on_web_process_crashed (WebKitWebView *web_view,
               	   	   	   	   gpointer       user_data) {
	
	m_log_msg(LOG_WARNING, "Web process crashed.");
	return FALSE;
}


static gboolean on_close_web_view(WebKitWebView* webView, 
								  gpointer       user_data) {
	HyWebDriver *theDriver = (HyWebDriver*) user_data;
    gtk_widget_destroy(theDriver->getMainWindow());
    return TRUE;
}


void HyWebDriver::createWebView(GtkWidget* main_window) {
	
	m_main_window = main_window;
    
	m_web_view = WEBKIT_WEB_VIEW(webkit_web_view_new());
    gtk_container_add(GTK_CONTAINER(m_main_window), GTK_WIDGET(m_web_view));
	
    g_signal_connect(m_web_view, "close", 
    				G_CALLBACK(on_close_web_view), (gpointer)this);
    
	g_signal_connect(m_web_view, "load-changed", 
					G_CALLBACK(on_web_view_load_changed), (gpointer)this);
	
	g_signal_connect(m_web_view, "web-process-crashed", 
					G_CALLBACK(on_web_process_crashed), (gpointer)this);
	
	WebKitSettings *settings = webkit_web_view_get_settings (m_web_view);
	
	int val = webkit_settings_get_enable_developer_extras(settings);
	m_log_msg(LOG_FINE, "webkit_settings_get_enable_developer_extras() returned [%d].", val);
	
	const char *agent = webkit_settings_get_user_agent(settings);
	m_log_msg(LOG_FINE, "webkit_settings_get_user_agent() returned [%s].", agent);
}


void HyWebDriver::listenWaitConnection() {
	setState(HY_WEBDRIVER_STATE_LISTENING);
	
    m_listen_sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (m_listen_sockfd < 0) {
       m_log_msg(LOG_ERROR, "Failed to listen allocate socket, die.");
       exit(-1);
    }
    
    struct sockaddr_in serv_addr, cli_addr;
    
    memset(&serv_addr, 0, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_addr.s_addr = inet_addr(HY_WEBDRIVER_LISTEN_ADDRESS);
    serv_addr.sin_port = htons(m_port);
    
    if (bind(m_listen_sockfd, (struct sockaddr *) &serv_addr,
             sizeof(serv_addr)) < 0) {
        m_log_msg(LOG_ERROR, "Failed to bind listen address [%s], port [%], die.",
        		HY_WEBDRIVER_LISTEN_ADDRESS, m_port);
        exit(-1);
    }
    
    // Listen and blocking accept
    ::listen(m_listen_sockfd, 5);
    socklen_t clilen = (socklen_t) sizeof(cli_addr);
    m_conn_sockfd = accept(m_listen_sockfd, (struct sockaddr *) &cli_addr, &clilen);
         
    if (m_conn_sockfd < 0) {
        m_log_msg(LOG_ERROR, "Failed to accept new socket, die.");
        exit(-1);
    }
        
    ::close(m_listen_sockfd);
    
    // Set to non-blocking stream socket
    int flags = fcntl(m_conn_sockfd, F_GETFL, 0);
    if (flags < 0) { 
        /* Handle error */ 
        m_log_msg(LOG_ERROR, "Failed to get socket option, die.");
        exit(-1);
    } 

    if (fcntl(m_conn_sockfd, F_SETFL, flags | O_NONBLOCK) < 0) { 
        /* Handle error */ 
        m_log_msg(LOG_ERROR, "Failed to set socket option, die.");
        exit(-1);
    } 
    
    setState(HY_WEBDRIVER_STATE_CONNECTED);
}


gint HyWebDriver::checkOnce(){
	    
	switch(m_state) {
	
	case HY_WEBDRIVER_STATE_IN_COMMAND:
	case HY_WEBDRIVER_STATE_IN_URL:
	case HY_WEBDRIVER_STATE_IN_TITLE:
	case HY_WEBDRIVER_STATE_IN_SOURCE:
	case HY_WEBDRIVER_STATE_IN_SOURCE_RUNNING:
	case HY_WEBDRIVER_STATE_IN_TEXTBOXES:
		return TRUE;
		
	case HY_WEBDRIVER_STATE_READY:
		{
			int n_recv = recv(m_conn_sockfd, m_msg_buf, HY_WEBDRIVER_RECV_BUF_SIZE-1, 0);
			
			if(n_recv == 0) { // elegant close
				m_log_msg(LOG_FINE, "Detected elegant close by peer, quit.");
				return TRUE;
			} else if(n_recv < 0) {
				if (errno == EAGAIN || errno == EWOULDBLOCK) {
					// recv timeout, do nothing
					return TRUE;
				} else {
					m_log_msg(LOG_ERROR, "Failed to recv");
					return TRUE;
				}
			} else if (n_recv > 0) {
				
				setState(HY_WEBDRIVER_STATE_IN_COMMAND);
				
				m_msg_buf[n_recv] = '\0';
				m_msg_len = n_recv;
				m_log_msg(LOG_FINE, "Received message:\n%s", m_msg_buf);
				
				parseMessage();
			}
		}
	    return TRUE;
	    
	case HY_WEBDRIVER_STATE_IN_URL_LOADING:
		{
			mINT64 elapsed = current_timestamp() - m_loading_start_time;
			
			if(elapsed >= HY_WEBDRIVER_LOADPAGE_TIMEOUT_MS) { // loading timeout
				webkit_web_view_stop_loading(m_web_view);
				setState(HY_WEBDRIVER_STATE_IN_URL_LOAD_STOPPED);
				
			}
			
		}
		return TRUE;
		
	case HY_WEBDRIVER_STATE_IN_URL_LOAD_STOPPED:
		sendResponse(HY_WEBDRIVER_ERROR, "Loading timeout.", 0);
		setState(HY_WEBDRIVER_STATE_READY);
		return TRUE;
		
	case HY_WEBDRIVER_STATE_IN_URL_LOAD_FINISHED:
		sendResponse(HY_WEBDRIVER_OK, "Finished.", 0);
		setState(HY_WEBDRIVER_STATE_READY);
		return TRUE;
		
	case HY_WEBDRIVER_STATE_IN_SOURCE_STOPPED:
		sendResponse(HY_WEBDRIVER_ERROR, "Getting source stopped.", 0);
		setState(HY_WEBDRIVER_STATE_READY);
		return TRUE;
		
	case HY_WEBDRIVER_STATE_IN_SOURCE_FINISHED:
		sendResponse(HY_WEBDRIVER_OK, m_source_buf, m_source_len);
		setState(HY_WEBDRIVER_STATE_READY);
		return TRUE;	
		
	case HY_WEBDRIVER_STATE_END:
		return FALSE;
		
	default:
		return TRUE;
	}
		
}

void HyWebDriver::parseMessage() {
	
	static char command_buf[HY_WEBDRIVER_RECV_BUF_SIZE], data_buf[HY_WEBDRIVER_RECV_BUF_SIZE];
	
	int i=0;
	while((i<m_msg_len) && m_msg_buf[i] != '\n') ++i;
	
	if (i == m_msg_len) {
		m_log_msg(LOG_WARNING, "Could not parse command, skip.");
		setState(HY_WEBDRIVER_STATE_READY);
		return;
	}
	
	int command_len = i;
	if((command_len>0) && (m_msg_buf[command_len-1] == '\r')) --command_len; // Check if CRLF

	memcpy(command_buf, m_msg_buf, command_len);
	command_buf[command_len] = '\0';
	
	m_log_msg(LOG_FINE, "Received command [%s].", command_buf);
	
	if(0 == strcmp("POST /url HTTP/1.1", command_buf)) {
		
		while((i<m_msg_len) && ((m_msg_buf[i] == '\r') || (m_msg_buf[i] == '\n'))) ++i;
		
		int k = m_msg_len-1;
		while((k>i) && ((m_msg_buf[k] == '\n') || (m_msg_buf[k] == '\r'))) --k; // Check both LF and CRLF
		
		int data_len = k - i + 1;
		
		if(data_len <= 0) {
			m_log_msg(LOG_WARNING, "Could not parse url, skip.");
			setState(HY_WEBDRIVER_STATE_READY);
			return;
		}
		
		memcpy(data_buf, m_msg_buf+i, data_len);
		data_buf[data_len] = '\0';
		
		onPostUrl(data_buf);
		
		
	} else if(0 == strcmp("GET /title HTTP/1.1", command_buf)) {
		onGetTitle();
		
	} else if(0 == strcmp("GET /source HTTP/1.1", command_buf)) {
		onGetSource();
		
	} else if(0 == strcmp("DELETE / HTTP/1.1", command_buf)) {
		onDelete();
	} else {
		m_log_msg(LOG_WARNING, "Received unknown command [%s], skip.", command_buf);
		setState(HY_WEBDRIVER_STATE_READY);
		return;
	}
}




void HyWebDriver::onPostUrl(const char *url) {
	m_log_msg(LOG_FINE, "URL command, with url [%s].", url);
	
	//m_load_finished = false;
	//mINT64 t0 = current_timestamp();
	
	setState(HY_WEBDRIVER_STATE_IN_URL);
	//if(webkit_web_view_is_loading(m_web_view)) {
		webkit_web_view_stop_loading(m_web_view);
	//}
	
	m_loading_start_time = current_timestamp();
	setState(HY_WEBDRIVER_STATE_IN_URL_LOADING);
	webkit_web_view_load_uri(m_web_view, url);
	
}


void HyWebDriver::onGetTitle() {
	setState(HY_WEBDRIVER_STATE_IN_TITLE);
	
	m_log_msg(LOG_FINE, "TITLE command.");
	const char *title = (const char*) webkit_web_view_get_title(m_web_view);
	
	if(title == NULL) {
		sendResponse(HY_WEBDRIVER_ERROR, "Failed to get title.", 0);
		setState(HY_WEBDRIVER_STATE_READY);
		return;
		
	} else {
		sendResponse(HY_WEBDRIVER_OK, title, 0);
		setState(HY_WEBDRIVER_STATE_READY);
		return;
	}
}


static void
web_view_javascript_finished (GObject      *object,
                              GAsyncResult *result,
                              gpointer      user_data)
{
	HyWebDriver *theDriver = (HyWebDriver*) user_data;
	
    WebKitJavascriptResult *js_result;
    JSValueRef              value;
    JSGlobalContextRef      context;
    GError                 *error = NULL;

    js_result = webkit_web_view_run_javascript_finish (WEBKIT_WEB_VIEW (object), result, &error);
    if (!js_result) {
        g_warning ("Error running javascript: %s", error->message);
        g_error_free (error);
        theDriver->setState(HY_WEBDRIVER_STATE_IN_SOURCE_STOPPED);
        return;
    }

    context = webkit_javascript_result_get_global_context (js_result);
    value = webkit_javascript_result_get_value (js_result);
    if (JSValueIsString (context, value)) {
        JSStringRef js_str_value;
        
        //gchar      *str_value;
        gsize       str_length;

        js_str_value = JSValueToStringCopy (context, value, NULL);
        
        str_length = JSStringGetMaximumUTF8CStringSize (js_str_value);
        //str_value = (gchar *)g_malloc (str_length);
        theDriver->freeSourceBuf();
        theDriver->allocSourceBuf(str_length);
        
        //JSStringGetUTF8CString (js_str_value, str_value, str_length);
        JSStringGetUTF8CString (js_str_value, theDriver->getSourceBuf(), str_length);
        
        JSStringRelease (js_str_value);
        //g_print ("Script result: %s\n", str_value);
        
        //g_free (str_value);
        theDriver->setState(HY_WEBDRIVER_STATE_IN_SOURCE_FINISHED);
        
    } else {
        g_warning ("Error running javascript: unexpected return value");
        theDriver->setState(HY_WEBDRIVER_STATE_IN_SOURCE_STOPPED);
    }
    
    webkit_javascript_result_unref (js_result);
}


void HyWebDriver::onGetSource() {
	m_log_msg(LOG_FINE, "SOURCE command.");
	setState(HY_WEBDRIVER_STATE_IN_SOURCE);
	
	//gchar *script;

    //script = g_strdup_printf ("document.documentElement.outerHTML;");
    
	setState(HY_WEBDRIVER_STATE_IN_SOURCE_RUNNING);
	
	webkit_web_view_run_javascript (m_web_view, 
    								"document.documentElement.outerHTML;", 
    								NULL, 
									web_view_javascript_finished, 
									(gpointer)this);
    //g_free (script);
    
	//setState(HY_WEBDRIVER_STATE_READY);
}


void HyWebDriver::onDelete() {
	m_log_msg(LOG_FINE, "DELETE command.");
	setState(HY_WEBDRIVER_STATE_IN_DELETE);
	
	setState(HY_WEBDRIVER_STATE_END);
	
	close();
	exitAll();
}


void HyWebDriver::sendResponse(const char *result, const char *msg, int msg_len) {
	
	int off = 0, copied_len;
	
	static char buf[32];
	static const char *CRLF = "\r\n";
	
	if (msg_len <= 0) msg_len = strlen(msg);
	
	// OK or ERROR
	copied_len = m_strcpy(m_send_buf+off, 
			HY_WEBDRIVER_SEND_BUF_SIZE-off, 
			result, 
			strlen(result));
	
	off += copied_len;
			
	// <CRLF>
	copied_len = m_strcpy(m_send_buf+off, 
			HY_WEBDRIVER_SEND_BUF_SIZE-off, 
			CRLF, 
			strlen(CRLF));
	
	off += copied_len;
	
	// Content-Length
	snprintf(buf, sizeof(buf), "Content-Length: %d\r\n", msg_len);
	
	copied_len = m_strcpy(m_send_buf+off, 
			HY_WEBDRIVER_SEND_BUF_SIZE-off, 
			buf, 
			strlen(buf));
	
	off += copied_len;
	
	// <CRLF>
	copied_len = m_strcpy(m_send_buf+off, 
			HY_WEBDRIVER_SEND_BUF_SIZE-off, 
			CRLF, 
			strlen(CRLF));
	
	off += copied_len;
	
	// message
	copied_len = m_strcpy(m_send_buf+off, 
			HY_WEBDRIVER_SEND_BUF_SIZE-off, 
			msg, 
			strlen(msg));
	
	off += copied_len;
	
	// <CRLF>
	copied_len = m_strcpy(m_send_buf+off, 
			HY_WEBDRIVER_SEND_BUF_SIZE-off, 
			CRLF, 
			strlen(CRLF));
	
	off += copied_len;
	
	// send
	send(m_conn_sockfd, m_send_buf, off, 0);
}


void HyWebDriver::exitAll() {
	//gtk_widget_destroy((GtkWidget*)m_web_view);
	//gtk_window_close ((GtkWindow*) m_web_view);
	gtk_widget_destroy(m_main_window);
	gtk_main_quit();
	//exit(0);
}
