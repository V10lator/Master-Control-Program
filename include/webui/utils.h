#pragma once

#include "mongoose.h"

#define HTTP_STRING "Master Control Program " MCP_VERSION
#define HTTP_NEWLINE "\r\n"
#define CACHE_CONTROL_NOCACHE "Cache-Control: no-store, max-age=0" HTTP_NEWLINE

extern const char *htmlErrorTemplate;

static inline void httpFastReply(struct mg_connection *c, int err)
{
	const char *str = mg_http_status_code_str(err);

	mg_http_reply(c, err, "Content-Type: text/html; charset=utf-8" HTTP_NEWLINE CACHE_CONTROL_NOCACHE, htmlErrorTemplate, err, str, err, str);
}

static inline void httpReplyJson(struct mg_connection *c, void *data) // TODO
{
	mg_http_reply(c, 200, "Content-Type: application/json" HTTP_NEWLINE CACHE_CONTROL_NOCACHE, "TODO" HTTP_NEWLINE);
}
