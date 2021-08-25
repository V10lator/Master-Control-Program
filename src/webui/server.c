#include <pthread.h>
#include <signal.h>
#include <stdio.h>

#include "mongoose.h"
#include "threadutils.h"
#include "webui/utils.h"

#define WEBUI_ROOT	"/home/technics/MCP/www/"
#define WEBUI_ADDY	"http://0.0.0.0:80"

typedef struct
{
	char *requestPath;
	void *mgCallback;
} API_DATA;

static inline bool fileExists(const char *path)
{
	struct stat fs;
	return stat(path, &fs) == 0 && S_ISREG(fs.st_mode);
}

static inline off_t getFilesize(FILE *fp)
{
	off_t i = ftello(fp);
	if(fseek(fp, 0, SEEK_END) != 0)
		return -1;

	off_t fileSize = ftello(fp);

	fseeko(fp, i, SEEK_SET);
	return fileSize;
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

static void parseApiRequest(struct mg_connection *c, int ev, void *ev_data, void *fn_data)
{
	API_DATA *data = (API_DATA *)fn_data;
	c->pfn_data = NULL;
	c->pfn = data->mgCallback;
	char *requestPath = data->requestPath;
	free(fn_data);

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
			free(requestPath);
			return;
		}
	}

	free(requestPath);
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
			API_DATA *data = (API_DATA *)malloc(sizeof(API_DATA));
			if(data == NULL)
			{
				httpFastReply(c, 500);
				return;
			}
			data->requestPath = (char *)malloc(sizeof(path + 5) * sizeof(char));
			if(data->requestPath == NULL)
			{
				free((void *)data);
				httpFastReply(c, 500);
				return;
			}

			strcpy(data->requestPath, path + 5);
			data->mgCallback = c->pfn;
			c->pfn = parseApiRequest;
			c->pfn_data = (void *)data;
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

		FILE *f = fopen(realPath, "rb");
		if(f == NULL)
		{
			httpFastReply(c, 403);
			return;
		}

		off_t size = getFilesize(f);
		if(size == -1)
		{
			httpFastReply(c, 403);
			fclose(f);
			return;
		}

		struct mg_fd *fd = malloc(sizeof(struct mg_fd));
		if(fd == NULL)
		{
			httpFastReply(c, 500);
			fclose(f);
			return;
		}

		fd->fd = (void *)f;
		fd->fs = &mg_fs_posix;

		struct mg_str str = mg_guess_content_type(mg_str(realPath), NULL);
		mg_printf(c, "HTTP/1.1 200 OK\r\n"
			"Content-Type: %.*s\r\n"
			"Content-Length: %l\r\n\r\n",
			(int)str.len, str.ptr,
			size);

		c->pfn = mg_static_cb;
		c->pfn_data = fd;
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
