#include "json.h"

#include <stdio.h>

#ifndef OVERRIDE_JSON
#define PL_JSON_IMPLEMENTATION
#include "../libs/pl_json.h"

#else

// #include "pd_api.h"
extern PlaydateAPI *playdate;

// Function to create a new node
json_node *create_json_node(const char *key, json_value_type type,
                            json_value value) {
  json_node *node = (json_node *)malloc(sizeof(json_node));
  if (!node)
    return NULL;

  node->key = key ? strdup(key) : NULL; // Duplicate key if present
  node->type = type;

  node->string = (type == kJSONString) ? strdup(value.data.stringval) : NULL;
  node->number = (type == kJSONFloat) ? value.data.floatval : 0;
  node->number = (type == kJSONInteger) ? value.data.intval : node->number;
  node->number = (type == kJSONTrue) ? 1 : node->number;
  node->number = (type == kJSONFalse) ? 0 : node->number;

  node->len = 0;
  node->parent = NULL;
  node->next = NULL;
  node->child = NULL;
  node->last_child = NULL;
  return node;
}

void free_json_node(json_node *node) {
  if (!node)
    return;

  // Recursively free all children
  json_node *child = node->child;
  while (child) {
    json_node *next = child->next; // Store next sibling before freeing
    free_json_node(child);
    child = next;
  }

  // Free dynamically allocated key
  if (node->key)
    free(node->key);
  if (node->string)
    free(node->string);

  // Finally, free the node itself
  free(node);
}

void json_destroy(json_t *n) { free_json_node(n); }

void dump_json_node(json_node *node, int level) {
  char tmp[32] = "    ";
  tmp[level] = 0;
  if (node->key) {
    printf("%s%s", tmp, node->key);
    if (node->type == kJSONString) {
      printf("=%s\n", node->string);
    } else {
      printf("=%f\n", node->number);
    }
  }
  json_node *n = node->child;
  while (n) {
    dump_json_node(n, level + 1);
    n = n->next;
  }
}

// Function to add a child node to a parent
void add_child(json_node *parent, json_node *child) {
  if (!parent || !child)
    return;

  child->parent = parent;

  if (!parent->child) {
    // If no child exists, set both child and last_child
    parent->child = child;
    parent->last_child = child;
  } else {
    // Append to the last child and update last_child
    parent->last_child->next = child;
    parent->last_child = child;
  }

  parent->len++;
}

static void decodeError(json_decoder *decoder, const char *error, int linenum) {
  PlaydateAPI *pd = playdate;
  pd->system->logToConsole("decode error line %i: %s", linenum, error);
}

const char *typeToName(json_value_type type) {
  switch (type) {
  case kJSONNull:
    return "null";
  case kJSONTrue:
    return "true";
  case kJSONFalse:
    return "false";
  case kJSONInteger:
    return "integer";
  case kJSONFloat:
    return "float";
  case kJSONString:
    return "string";
  case kJSONArray:
    return "array";
  case kJSONTable:
    return "table";
  default:
    return "???";
  }
}

static void willDecodeSublist(json_decoder *decoder, const char *name,
                              json_value_type type) {
  json_node *top_node = (json_node *)decoder->userdata;
  json_node *new_node = create_json_node(name, type, (json_value){0});
  add_child(top_node, new_node);
  decoder->userdata = new_node;
}

static int shouldDecodeTableValueForKey(json_decoder *decoder,
                                        const char *key) {
  return 1;
}

static void didDecodeTableValue(json_decoder *decoder, const char *key,
                                json_value value) {
  if (value.type == kJSONArray || value.type == kJSONTable) {
    return;
  }
  json_node *parent = (json_node *)decoder->userdata;
  json_node *new_node = create_json_node(key, value.type, value);
  add_child(parent, new_node);
}

static int shouldDecodeArrayValueAtIndex(json_decoder *decoder, int pos) {
  return 1;
}

static void didDecodeArrayValue(json_decoder *decoder, int pos,
                                json_value value) {
  if (value.type == kJSONArray || value.type == kJSONTable) {
    return;
  }
  PlaydateAPI *pd = playdate;
  json_node *parent = (json_node *)decoder->userdata;
  char key[32];
  sprintf(key, "%d", pos);
  json_node *new_node = create_json_node(key, value.type, value);
  add_child(parent, new_node);
}

static void *didDecodeSublist(json_decoder *decoder, const char *name,
                              json_value_type type) {
  PlaydateAPI *pd = playdate;
  json_node *n = (json_node *)decoder->userdata;
  decoder->userdata = n->parent;
  return n;
}

json_node *json_parse_data(uint8_t *data, uint32_t len) {
  PlaydateAPI *pd = playdate;
  json_node *doc = create_json_node(NULL, kJSONTable, (json_value){0});
  json_decoder decoder = {
      .userdata = doc,
      .decodeError = decodeError,
      .willDecodeSublist = willDecodeSublist,
      .shouldDecodeTableValueForKey = shouldDecodeTableValueForKey,
      .didDecodeTableValue = didDecodeTableValue,
      .shouldDecodeArrayValueAtIndex = shouldDecodeArrayValueAtIndex,
      .didDecodeArrayValue = didDecodeArrayValue,
      .didDecodeSublist = didDecodeSublist};

  json_value val;
  pd->json->decodeString(&decoder, data, &val);

  json_node *root = doc->child;
  // dump_json_node(root, 0);
  return root;
}

int json_tokenize(char *data, unsigned int len, json_token_t *tokens,
                  unsigned int tokens_len, unsigned int *parsed_size_req) {
  // not used
  return 1;
}

void json_parse_tokens(char *data, json_token_t *tokens, unsigned int len,
                       json_t *json) {
  // not used
}

double json_number(json_t *v) {
  if (!v || v->type > JSON_NUMBER) {
    return 0;
  }
  return v->number;
}

int json_bool(json_t *v) {
  if (!v) {
    return 0;
  }
  if (v->type > JSON_NUMBER) {
    return (v->len > 0);
  }
  return (v->number != 0);
}

char *json_string(json_t *v) {
  if (!v || v->type != JSON_STRING) {
    return NULL;
  }
  return v->string;
}

#define MAX_CHILDREN 1024 // Define a reasonable maximum limit

json_t *json_values(json_t *v) {
  static json_node *_values[MAX_CHILDREN + 1]; // +1 for NULL terminator
  int idx = 0;
  json_node *n = v->child;

  while (n && idx < MAX_CHILDREN) {
    _values[idx++] = n;
    n = n->next;
  }

  _values[idx] = NULL; // Null-terminate the array
  return (void *)&_values;
}

json_t *json_value_at(json_t *v, unsigned int i) {
  if (!v)
    return NULL;
  json_node *n = v->child;
  int idx = 0;
  while (n) {
    if (idx == i) {
      return n;
    }
    idx++;
    n = n->next;
  }
  return NULL;
}

char **json_keys(json_node *v) {
  if (!v)
    return NULL;
  static char *_keys[MAX_CHILDREN + 1]; // +1 for NULL terminator
  int idx = 0;
  json_node *n = v->child;

  while (n && idx < MAX_CHILDREN) {
    _keys[idx++] = n->key; // Store key pointer
    n = n->next;
  }

  _keys[idx] = NULL; // Null-terminate the array
  return _keys;
}

char *json_key_at(json_t *v, unsigned int i) {
  if (!v)
    return NULL;
  json_node *n = v->child;
  int idx = 0;
  while (n) {
    if (idx == i) {
      return n->key;
    }
    idx++;
    n = n->next;
  }
  return NULL;
}

json_t *json_value_for_key(json_t *v, char *key) {
  if (!v || v->type != JSON_OBJECT) {
    return NULL;
  }

  json_node *n = v->child;
  while (n) {
    if (n->key && strcmp(key, n->key) == 0) {
      // printf("%s\n", key);
      return n;
    }
    n = n->next;
  }
  return NULL;
}

#endif
