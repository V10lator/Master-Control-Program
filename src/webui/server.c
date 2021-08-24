#include <pthread.h>
#include <signal.h>
#include <stdio.h>

#include "mongoose.h"
#include "threadutils.h"
#include "webui/utils.h"

#define WEBUI_ROOT	"/home/technics/MCP/www/"
#define WEBUI_ADDY	"http://0.0.0.0:80"
#define MCP_HEADERS	NULL
#define EXTRA_MIME	NULL

struct mg_http_serve_opts opts = {
	.extra_headers = MCP_HEADERS,
	.mime_types = EXTRA_MIME,
	.fs = NULL,

};

static inline bool fileExists(const char *path)
{
	struct stat fs;
	return stat(path, &fs) == 0 && S_ISREG(fs.st_mode);
}

static inline void deliverContent(struct mg_connection *c, unsigned int argc, char **argv)
{
	if(argc != 1)
	{
		httpFastReply(c, 400);
		return;
	}

	//TODO
	httpReplyJson(c, NULL);
}

static inline void parseApiRequest(struct mg_connection *c, char *requestPath)
{
	unsigned int argc = 0;
	char *argv[1024];
	bool args = true;
	int l = strlen(requestPath);
	if(l > 1024)
		l = 1024;

	for(int i = 0; i < l; i++)
	{
		if(args)
		{
			argv[argc++] = requestPath + i;
			args = false;
		}
		else if(requestPath[i] == '/')
		{
			requestPath[i] = '\0';
			args = true;
		}
	}

	if(argc > 0)
	{
		argc--;
		char **realArgv = argv + 1;

		if(strcmp(argv[0], "c") == 0)
		{
			deliverContent(c, argc, realArgv);
			return;
		}
	}

	httpFastReply(c, 400);
}

static void webuiCallback(struct mg_connection *c, int ev, void *ev_data, void *fn_data)
{
	if (ev == MG_EV_HTTP_MSG)
	{
		struct mg_http_message *msg = (struct mg_http_message *)ev_data;
		char path[msg->message.len + 1];
		path[0] = '\0';
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

		if(strlen(path) == 0 || path[0] != '/')
		{
			httpFastReply(c, 403);
			return;
		}

		if(strlen(path) > 4 &&
			path[1] == 'a' &&
			path[2] == 'p' &&
			path[3] == 'i' &&
			path[4] == '/'
		)
		{
			parseApiRequest(c, path + 5);
			return;
		}

		if(path[1] == '\0') // Moved permanently to /index.html
		{
			mg_http_reply(c, 301, "Location: /index.html\r\n", HTTP_STRING);
			return;
		}

		char realPath[strlen(path) + strlen(WEBUI_ROOT) + 1];
		strcpy(realPath, WEBUI_ROOT);
		strcat(realPath, path);

		if(!fileExists(realPath))
		{
			httpFastReply(c, 403);
			return;
		}

		mg_http_serve_file(c, ev_data, realPath, &opts);
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
