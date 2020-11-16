#include <arpa/inet.h>
#include <netdb.h>
#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>
#include "helpers.h"

char *compute_get_request(char *host, char *url, char *tokken,
                          char *query_params, char **cookies,
                          int cookies_count) {
  char *message = calloc(BUFLEN, sizeof(char));
  char *line = calloc(LINELEN, sizeof(char));

  if (query_params != NULL) {
    sprintf(line, "GET %s?%s HTTP/1.1", url, query_params);
  } else {
    sprintf(line, "GET %s HTTP/1.1", url);
  }
  compute_message(message, line);

  memset(line, 0, LINELEN);
  sprintf(line, "Host: %s ", host);
  compute_message(message, line);
  if (tokken != NULL) {
    memset(line, 0, LINELEN);
    sprintf(line, "Authorization: Bearer %s", tokken);
    compute_message(message, line);
  }

  if (cookies_count != 0) {
    memset(line, 0, LINELEN);
    sprintf(line, "Cookie: ");
    for (int i = 0; i < cookies_count; i++) {
      strcat(line, cookies[i]);
      if (i != cookies_count - 1) {
        strcat(line, "; ");
      }
    }
    line[strlen(line) - 1] = '\0';
    compute_message(message, line);
  }

  compute_message(message, "");
  return message;
}

char *compute_post_request(char *host, char *url, char *tokken,
                           char *content_type, char **body_data,
                           int body_data_fields_count, char **cookies,
                           int cookies_count) {
  char *message = calloc(BUFLEN, sizeof(char));
  char *line = calloc(LINELEN, sizeof(char));
  int content_length = 0;

  sprintf(line, "POST %s HTTP/1.1", url);
  compute_message(message, line);

  memset(line, 0, LINELEN);
  sprintf(line, "Host: %s ", host);
  compute_message(message, line);

  for (int i = 0; i < body_data_fields_count; i++) {
    content_length += strlen(body_data[i]);
  }

  memset(line, 0, LINELEN);
  sprintf(line, "Content-Type: %s", content_type);
  compute_message(message, line);
  memset(line, 0, LINELEN);
  sprintf(line, "Content-Length: %d", content_length);
  compute_message(message, line);
  if (tokken != NULL) {
    memset(line, 0, LINELEN);
    sprintf(line, "Authorization: Bearer %s", tokken);
    compute_message(message, line);
  }

  if (cookies_count != 0) {
    memset(line, 0, LINELEN);
    sprintf(line, "Cookie: ");
    for (int i = 0; i < cookies_count; i++) {
      strcat(line, cookies[i]);
      if (i != cookies_count - 1) {
        strcat(line, "; ");
      }
    }
    line[strlen(line) - 1] = '\0';
    compute_message(message, line);
  }
  compute_message(message, "");

  memset(line, 0, LINELEN);
  for (int i = 0; i < body_data_fields_count; i++) {
    compute_message(message, body_data[i]);
  }
  compute_message(message, "");

  free(line);
  return message;
}

char *compute_delete_request(char *host, char *url, char *tokken,
                             char **cookies, int cookies_count) {
  char *message = calloc(BUFLEN, sizeof(char));
  char *line = calloc(LINELEN, sizeof(char));

  sprintf(line, "DELETE %s HTTP/1.1", url);
  compute_message(message, line);

  memset(line, 0, LINELEN);
  sprintf(line, "Host: %s ", host);
  compute_message(message, line);

  if (tokken != NULL) {
    memset(line, 0, LINELEN);
    sprintf(line, "Authorization: Bearer %s", tokken);
    compute_message(message, line);
  }

  if (cookies_count != 0) {
    memset(line, 0, LINELEN);
    sprintf(line, "Cookie: ");
    for (int i = 0; i < cookies_count; i++) {
      strcat(line, cookies[i]);
      if (i != cookies_count - 1) {
        strcat(line, "; ");
      }
    }
    line[strlen(line) - 1] = '\0';
    compute_message(message, line);
  }

  compute_message(message, "");

  free(line);
  return message;
}
