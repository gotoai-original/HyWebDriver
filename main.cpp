#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <pwd.h>

#include <gtk/gtk.h>
#include <webkit2/webkit2.h>

#include "HyWebDriver.h"
#include "miso_types.h"
#include "common.h"
#include "spinlock.h"
#include "timestamp.h"
#include "miso_log.h"

static void destroyWindowCb(GtkWidget* widget, GtkWidget* window);
static gboolean closeWebViewCb(WebKitWebView* webView, GtkWidget* window);


// HyWebDriver object
HyWebDriver the_driver;
static char s_HYWEBDRIVER_HOME[1024];
		

static void hy_check_create_dir(const char *dir) {
	struct stat st;
	memset(&st, 0, sizeof(struct stat));

	if (stat(dir, &st) == -1) {
	    mkdir(dir, 0755);
	}
}

void hy_init(int argc, char* argv[]) {
	char buf[1024], buf2[1024], buf3[1024];
	
	// initialize home dir
	struct passwd *pw = getpwuid(getuid());

	const char *homedir = pw->pw_dir;
	strcpy(s_HYWEBDRIVER_HOME, homedir);
	strcat(s_HYWEBDRIVER_HOME, "/.HyWebDriver");
	hy_check_create_dir(s_HYWEBDRIVER_HOME);
	
	// initialize log
	strcpy(buf, s_HYWEBDRIVER_HOME);
	strcat(buf, "/log");
	hy_check_create_dir(buf);
	
	::current_timestamp_str14(buf2, sizeof(buf2), mTRUE);
	snprintf(buf3, sizeof(buf3), "%s/hywebdriver_%s.log", buf, buf2);
	m_set_log_file(buf3);
	m_set_log_level(LOG_ALL);
	m_set_log_stdout(mTRUE);
	
	bool set_port = false;
	
	for(int i=0; i<argc; ++i) {
		char *arg = argv[i];
		if(0 == strncmp("--port=", arg, 7)) {
			int port = atoi(&arg[7]);
			the_driver.setPort(port);
			
			m_log_msg(LOG_FINE, "Set HyWebDriver's port to [%d].", port);
			set_port = true;
		}
	}
	
	if(!set_port) {
		m_log_msg(LOG_ERROR, "<--port> argument was not found, die.");
		exit(-1);
	}
	
	m_log_msg(LOG_INFO, "HyWebDriver initialization completed, with home dir [%s].",
			s_HYWEBDRIVER_HOME);
}


int main(int argc, char* argv[])
{
	// Initialize HyWebDriver settings.
	hy_init(argc, argv);
	
    // Initialize GTK+
    gtk_init(&argc, &argv);

    // Create an 800x600 window that will contain the browser instance
    GtkWidget *main_window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
    gtk_window_set_default_size(GTK_WINDOW(main_window), 800, 600);

    // Create a browser instance
    WebKitWebView *webView = WEBKIT_WEB_VIEW(webkit_web_view_new());

    // Put the browser area into the main window
    gtk_container_add(GTK_CONTAINER(main_window), GTK_WIDGET(webView));

    // Set up callbacks so that if either the main window or the browser instance is
    // closed, the program will exit
    g_signal_connect(main_window, "destroy", G_CALLBACK(destroyWindowCb), NULL);
    g_signal_connect(webView, "close", G_CALLBACK(closeWebViewCb), main_window);

    // Load a web page into the browser instance
    //webkit_web_view_load_uri(webView, "http://www.webkitgtk.org/");
    //webkit_web_view_load_uri(webView, "http://www.deeplearning.net/");

    // Make sure that when the browser area becomes visible, it will get mouse
    // and keyboard events
    gtk_widget_grab_focus(GTK_WIDGET(webView));

    // Make sure the main window and all its contents are visible
    gtk_widget_show_all(main_window);

    
    // Start the driver
    the_driver.setWebView(webView);
    the_driver.start();
    
    // Run the main GTK+ event loop
    gtk_main();

    return 0;
}


static void destroyWindowCb(GtkWidget* widget, GtkWidget* window)
{
    // stop the driver
    the_driver.stop();
    
    gtk_main_quit();
}

static gboolean closeWebViewCb(WebKitWebView* webView, GtkWidget* window)
{
    gtk_widget_destroy(window);
    return TRUE;
}


