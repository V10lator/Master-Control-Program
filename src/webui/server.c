#include <pthread.h>
#include <signal.h>
#include <stdio.h>

#include "mongoose.h"
#include "threadutils.h"

#define WEBUI_ROOT "/home/technics/MCP/www/"
#define WEBUI_ADDY "http://0.0.0.0:80"

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
		struct mg_http_serve_opts opts = { .root_dir = WEBUI_ROOT };
		mg_http_serve_dir(c, ev_data, &opts);
	}
}

void *webui_thread_main(void *data)
{
	signal(SIGUSR1, dummyHandler);

	struct mg_mgr *mgr = (struct mg_mgr *)malloc(sizeof(struct mg_mgr));
	mg_mgr_init(mgr);
	mg_http_listen(mgr, WEBUI_ADDY, webuiCallback, NULL);

	while(appRunning())
		mg_mgr_poll(mgr, 20);

	mg_mgr_free(mgr);
	free((void *)mgr);
	pthread_exit(NULL);
}
