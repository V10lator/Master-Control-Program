#include <pthread.h>
#include <signal.h>
#include <stdio.h>

#include "mongoose.h"
#include "threadutils.h"

static const char *webuiRoot = "/home/technics/MCP/www/";
static const char *webuiAddy = "http://0.0.0.0:80";

static struct mg_mgr mgr;

static void webuiCallback(struct mg_connection *c, int ev, void *ev_data, void *fn_data)
{
	if (ev == MG_EV_HTTP_MSG)
	{
		struct mg_http_message *msg = (struct mg_http_message *)ev_data;
		char path[msg->message.len + 1];
		bool start = false;
		int j;
		for(int i = 0; i < msg->message.len; i++)
		{
			if(msg->message.ptr[i] == ' ')
			{
				if(start)
				{
					path[j] = '\0';
					break;
				}
				else
				{
					start = true;
					continue;
				}
			}

			if(!start)
			continue;

			path[j++] = msg->message.ptr[i];
		}

		printf("[WEB UI] Requested path: %s\n", path);

		//TODO: Just static file serving for now.
		struct mg_http_serve_opts opts = { .root_dir = webuiRoot };
		mg_http_serve_dir(c, ev_data, &opts);
	}
}

void *webui_thread_main(void *data)
{
	signal(SIGINT, dummyHandler);
	signal(SIGTERM, dummyHandler);
	signal(SIGQUIT, dummyHandler);
	signal(SIGUSR1, dummyHandler);

	mg_mgr_init(&mgr);
	mg_http_listen(&mgr, webuiAddy, webuiCallback, NULL);

	while(appRunning())
		mg_mgr_poll(&mgr, 20);

	mg_mgr_free(&mgr);
	pthread_exit(NULL);
}
