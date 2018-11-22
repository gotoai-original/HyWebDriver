#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <pwd.h>

#include <gtk/gtk.h>

#include "HyWebDriver.h"
#include "miso_types.h"
#include "miso_common.h"
#include "miso_spinlock.h"
#include "miso_timestamp.h"
#include "miso_log.h"



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


static void on_destroy_window(GtkWidget *widget,
        					  gpointer   data)
{
    // close the driver
    the_driver.close();
    gtk_main_quit();
}


static gint on_timeout (gpointer data) {
	return the_driver.checkOnce();
}


int main(int argc, char* argv[])
{
	// Initialize HyWebDriver settings.
	hy_init(argc, argv);
	
	// Listen and wait connection
	the_driver.listenWaitConnection();
	
	// Initialize GTK+
    gtk_init(&argc, &argv);

    GtkWidget *main_window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
    gtk_window_set_default_size(GTK_WINDOW(main_window), 1366, 768);

    g_signal_connect(main_window, "destroy", G_CALLBACK(on_destroy_window), NULL);

    the_driver.createWebView(main_window);
    
    // Set the timer
    g_timeout_add(100, on_timeout, NULL);
    
    //gtk_widget_grab_focus(GTK_WIDGET(webView));
    //gtk_widget_show_all(main_window);
    
    the_driver.setState(HY_WEBDRIVER_STATE_READY);
    
    // Run the main GTK+ event loop
    gtk_main();

    return 0;
}





