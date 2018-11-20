

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <sys/types.h> 
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <errno.h>
#include "HyWebDriver.h"
#include "miso_log.h"

static void *routine_hywebdriver_parse(void *ptr) {
	HyWebDriver *theDriver = (HyWebDriver*) ptr;
	
	theDriver->run();
	
	return NULL;
}


void HyWebDriver::start() {
	
	pthread_t thread;
    int res = pthread_create(&thread, NULL, routine_hywebdriver_parse, (void*)this);
    if(res) {
        m_log_msg(LOG_ERROR, "Failed to create HyWebDriver's parse thread, die.");
        exit(-1);
    } else {
    	m_log_msg(LOG_FINE, "Created HyWebDriver's parse thread.");
    }
}


void HyWebDriver::stop() {
	
}


void HyWebDriver::run() {
	
	m_log_msg(LOG_FINE, "HyWebDriver's parse thread started.");
	
	listen();
	
	parseLoop();
	
	m_log_msg(LOG_FINE, "HyWebDriver's parse thread quits.");
}


void HyWebDriver::listen() {
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
    
    ::listen(m_listen_sockfd, 5);
    socklen_t clilen = (socklen_t) sizeof(cli_addr);
    m_conn_sockfd = accept(m_listen_sockfd, (struct sockaddr *) &cli_addr, &clilen);
         
    if (m_conn_sockfd < 0) {
        m_log_msg(LOG_ERROR, "Failed to accept new socket, die.");
        exit(-1);
    }
        
    close(m_listen_sockfd);
}


void HyWebDriver::parseLoop(){
	
    struct timeval timeout;      
    timeout.tv_sec = HY_WEBDRIVER_RECV_TIMEOUT_MS / 1000;
    timeout.tv_usec = (HY_WEBDRIVER_RECV_TIMEOUT_MS % 1000) * 1000;

    if (setsockopt (m_conn_sockfd, SOL_SOCKET, SO_RCVTIMEO, (char *)&timeout,
                sizeof(timeout)) < 0) {
        m_log_msg(LOG_ERROR, "Failed to set timeout option, die.",
        		HY_WEBDRIVER_LISTEN_ADDRESS, m_port);
        exit(-1);
    }
    
    
	while(true) {
	    int n_recv = recv(m_conn_sockfd, m_msg_buf, HY_WEBDRIVER_RECV_BUF_SIZE-1, 0);
	    
	    if(n_recv == 0) { // elegant close
	    	m_log_msg(LOG_FINE, "Detected elegant close by peer, quit.");
	    	break;
	    } else if(n_recv < 0) {
	    	if (errno == EAGAIN || errno == EWOULDBLOCK) {
	    		
	    	} else {
	    		m_log_msg(LOG_ERROR, "Failed to recv");
	    	}
	    } else if (n_recv > 0) {
	    	m_msg_buf[n_recv] = '\0';
	    	m_msg_len = n_recv;
	    	m_log_msg(LOG_FINE, "Received message:\n%s", m_msg_buf);
	    	
	    	parseMessage();
	    }
	}
		
}

void HyWebDriver::parseMessage() {
	
	static char command_buf[HY_WEBDRIVER_RECV_BUF_SIZE], data_buf[HY_WEBDRIVER_RECV_BUF_SIZE];
	
	int i=0;
	while((i<m_msg_len) && m_msg_buf[i] != '\n') ++i;
	
	if (i == m_msg_len) {
		m_log_msg(LOG_WARNING, "Could not parse command, skip.");
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
		return;
	}
}

void HyWebDriver::onPostUrl(const char *url) {
	m_log_msg(LOG_FINE, "URL command, with url [%s].", url);
	webkit_web_view_load_uri(m_web_view, url);
}

void HyWebDriver::onGetTitle() {
	m_log_msg(LOG_FINE, "TITLE command.");
}

void HyWebDriver::onGetSource() {
	m_log_msg(LOG_FINE, "SOURCE command.");
}

void HyWebDriver::onDelete() {
	m_log_msg(LOG_FINE, "DELETE command.");
}

