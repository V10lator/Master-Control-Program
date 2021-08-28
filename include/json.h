#pragma once

typedef enum
{
	JSON_TYPE_OBJECT,
	JSON_TYPE_ARRAR,
	JSON_TYPE_BOOLEAN,
	JSON_TYPE_STRING,
	JSON_TYPE_INTEGER,
	JSON_TYPE_DOUBLE,
} JSON_TYPE;

typedef struct
{
	JSON_TYPE type;
	void *value;
} JSON_OBJECT;

static inline JSON_OBJECT jsonCreateObject(void *value)
{
	JSON_OBJECT *ret = malloc(sizeof(JSON_OBJECT));
	if(ret == NULL)
		return NULL;

	ret->type = type;
	ret->value = value;
}

static inline JSON_OBJECT jsonCreateArray(JS
