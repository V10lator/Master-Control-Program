#pragma once

#include "mongoose.h"

#define HTTP_STRING "Master Control Program " MCP_VERSION
#define HTTP_NEWLINE "\r\n"
#define CACHE_CONTROL_NOCACHE "Cache-Control: no-store, max-age=0" HTTP_NEWLINE

static inline void httpFastReply(struct mg_connection *c, int err)
{
	const char *str = mg_http_status_code_str(err);

	mg_http_reply(c, err, "Content-Type: text/html; charset=utf-8" HTTP_NEWLINE CACHE_CONTROL_NOCACHE,
"<!DOCTYPE html>"
"<html>"
"<head>"
	"<meta charset=\"utf-8\">"
	"<meta name=\"viewport\" content=\"width=device-width, initial-scale=1\">"
	"<title>%i %s</title>"
	"<link href=\"/css/bootstrap.min.css\" rel=\"stylesheet\" />"
	"<link href=\"/css/technics.css\" rel=\"stylesheet\" />"
"</head>"
"<body>"
	"<div class=\"container bg-primary text-center\">"
		"<div class=\"row\">"
			"<div class=\"col-sm-100 p-5 bg-secondary text-center\">"
				"<h1>Technics SH-4060</h1>"
			"</div>"
		"</div>"
		"<div class=\"row\">"
			"<div class=\"col-sm-100\">"
				"<h2>%i %s</h2>"
				"<hr>"
				HTTP_STRING
			"</div>"
		"</div>"
	"</div>"
"</body>"
"</html>"
"<!-- We need some padding cause some browsers are stupid ant want to render their shit instead of this beautiful page we worked so much on... -->" HTTP_NEWLINE,
		err, str,
		err, str
	);
}

static inline void httpReplyJson(struct mg_connection *c, void *data) // TODO
{
	mg_http_reply(c, 200, "Content-Type: application/json" HTTP_NEWLINE CACHE_CONTROL_NOCACHE, "TODO" HTTP_NEWLINE);
}
