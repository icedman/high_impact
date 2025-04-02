#ifndef JSON_H
#define JSON_H

#ifdef PLATFORM_PLAYDATE
#define OVERRIDE_JSON
#endif

#ifndef OVERRIDE_JSON
#include "../libs/pl_json.h"
#define json_destroy free

#else

#include "pd_api.h"

// Define tree node structure
typedef struct json_node {
  char *key;            // Key if part of an object, NULL if in an array
  json_value_type type; // The type of value
  json_value value;     // The actual value

  char *string;
  float number;

  struct json_node *parent;     // Parent node
  struct json_node *next;       // Next sibling
  struct json_node *child;      // First child (if object/array)
  struct json_node *last_child; // First child (if object/array)

  int len;
} json_node;

typedef json_node json_t;
typedef int json_token_t;

#define JSON_NULL kJSONNull
#define JSON_TRUE kJSONTrue
#define JSON_FALSE kJSONFalse
#define JSON_NUMBER kJSONInteger
#define JSON_NUMBER kJSONFloat
#define JSON_STRING kJSONString
#define JSON_ARRAY kJSONArray
#define JSON_OBJECT kJSONTable

int json_tokenize(char *data, unsigned int len, json_token_t *tokens,
                  unsigned int tokens_len, unsigned int *parsed_size_req);
void json_parse_tokens(char *data, json_token_t *tokens, unsigned int len,
                       json_t *json);
double json_number(json_t *v);
int json_bool(json_t *v);
char *json_string(json_t *v);
json_t *json_values(json_t *v);
json_t *json_value_at(json_t *v, unsigned int i);
char **json_keys(json_t *v);
char *json_key_at(json_t *v, unsigned int i);
json_t *json_value_for_key(json_t *v, char *key);

void json_destroy(json_t *j);

#endif

#endif // JSON_H