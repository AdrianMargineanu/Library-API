#include "client.h"
#include <arpa/inet.h>
#include <netdb.h>
#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>
#include "helpers.h"
#include "parson.h"
#include "requests.h"

#define TRUE 1
#define FALSE 0

#define COMMAND_LEN 15
#define INPUTLEN 100

#define HOST_MAIN_SERVER "ec2-3-8-116-10.eu-west-2.compute.amazonaws.com"
#define PORT_MAIN 8080
#define IP_HOST_MAIN "3.8.116.10"

#define DATA_TYPE_JSON "application/json"

#define URL_REGISTER "/api/v1/tema/auth/register"
#define URL_LOGIN "/api/v1/tema/auth/login"
#define URL_ENTER_LIBRARY "/api/v1/tema/library/access"
#define URL_BOOKS "/api/v1/tema/library/books"
#define URL_BOOK "/api/v1/tema/library/books/"
#define URL_LOGOUT "/api/v1/tema/auth/logout"

#define COMMAND_REGISTER "register"
#define COMMAND_LOGIN "login"
#define COMMAND_ENTER_LIBRARY "enter_library"
#define COMMAND_GET_BOOKS "get_books"
#define COMMAND_GET_BOOK "get_book"
#define COMMAND_ADD_BOOK "add_book"
#define COMMAND_DELETE_BOOK "delete_book"
#define COMMAND_LOGOUT "logout"
#define COMMAND_EXIT "exit"

int error_verification(char *response) {
  if (response == NULL) {
    return -1;
  }
  char *res = response + 9;
  if (memcmp(res, "200", 3) == 0) {
    return 0;
  }
  if (memcmp(res, "201", 3) == 0) {
    return 0;
  }
  if (memcmp(res, "400", 3) == 0) {
    return 1;
  }
  if (memcmp(res, "401", 3) == 0) {
    return 2;
  }
  if (memcmp(res, "403", 3) == 0) {
    return 3;
  }
  if (memcmp(res, "404", 3) == 0) {
    return 4;
  }
  return -1;
}

void sign_in(int sockfd, struct client *cl) {
  char username[INPUTLEN];
  char password[INPUTLEN];
  char *message;
  char *data = malloc(LINELEN * sizeof(char));
  char *response;
  printf("username=");
  scanf("%s", username);
  printf("password=");
  scanf("%s", password);

  sprintf(data, "{\"username\":\"%s\",\"password\":\"%s\"}", username,
          password);
  message = compute_post_request(HOST_MAIN_SERVER, URL_REGISTER, NULL,
                                 DATA_TYPE_JSON, &data, 1, NULL, 0);
  send_to_server(sockfd, message);
  response = receive_from_server(sockfd);
  if (cl->debug == TRUE) {
    printf("<Request sent>, metadata: %s\n", message);
    printf("<Response received>, metadata: %s\n", response);
  }

  int error = error_verification(response);
  if (error == -1) {
    printf("Internal error, please remake the last command\n");
    return;
  }

  if (error == 1) {
    printf("Sorry but username %s is taken, please try another one.\n",
           username);
  } else {
    printf("Wellcome to the server, %s.\n", username);
  }

  free(response);
  free(message);
  free(data);
}

void login(int sockfd, struct client *cl) {
  char username[INPUTLEN];
  char password[INPUTLEN];
  char *message;
  char *data = malloc(LINELEN * sizeof(char));
  char *response;
  printf("username=");
  scanf("%s", username);
  printf("password=");
  scanf("%s", password);

  sprintf(data, "{\"username\":\"%s\",\"password\":\"%s\"}", username,
          password);
  message = compute_post_request(HOST_MAIN_SERVER, URL_LOGIN, NULL,
                                 DATA_TYPE_JSON, &data, 1, NULL, 0);
  send_to_server(sockfd, message);
  response = receive_from_server(sockfd);

  if (cl->debug == TRUE) {
    printf("<Request sent>, metadata: %s\n", message);
    printf("<Response received>, metadata: %s\n", response);
  }

  int error = error_verification(response);
  if (error == -1) {
    printf("Internal error, please remake the last command\n");
    return;
  }

  if (error == 1) {
    printf("Username and/or password incorrect.\n");
  } else {
    printf("Wellcome back %s.\n", username);

    char *cookie = calloc(LINELEN, sizeof(char));
    char *lo = strstr(response, "Set-Cookie: ");
    lo += strlen("Set-Cookie: ");
    char *hi = strstr(lo, "\r\n");
    strncpy(cookie, lo, hi - lo + 1);
    memcpy(cl->cookie[cl->nr_cookie], cookie, LINELEN);
    cl->nr_cookie++;
    if (cl->debug == TRUE) {
      printf("<Cookie received>, metadata: %s\n",
             cl->cookie[cl->nr_cookie - 1]);
    }
  }

  free(response);
  free(message);

  return;
}

void enter_library(int sockfd, struct client *cl) {
  char *message;
  message = compute_get_request(HOST_MAIN_SERVER, URL_ENTER_LIBRARY, NULL, NULL,
                                cl->cookie, cl->nr_cookie);
  send_to_server(sockfd, message);
  char *response = receive_from_server(sockfd);
  if (cl->debug == TRUE) {
    printf("<Request sent>, metadata: %s\n", message);
    printf("<Response received>, metadata: %s\n", response);
  }

  int error = error_verification(response);
  if (error == -1) {
    printf("Internal error, please remake the last command\n");
    return;
  }
  if (error == 2) {
    printf("You are not autherised to enter, please login!\n");
    return;
  }
  printf("Wellcome to the library, take a book and have a seat.\n");

  char *token = calloc(LINELEN, sizeof(char));
  char *lo = strstr(response, "token");
  lo += strlen("token") + 3;
  char *hi = strstr(lo, "\"}");

  strncpy(token, lo, hi - lo);
  cl->token = calloc(LINELEN, sizeof(char));
  memcpy(cl->token, token, hi - lo + 1);
  if (cl->debug == TRUE) {
    printf("<token>, metadata: %s\n", cl->token);
  }

  free(response);
  free(message);
}

void get_books(int sockfd, struct client *cl) {
  char *message;
  message = compute_get_request(HOST_MAIN_SERVER, URL_BOOKS, cl->token, NULL,
                                cl->cookie, cl->nr_cookie);
  send_to_server(sockfd, message);
  char *response = receive_from_server(sockfd);
  if (cl->debug == TRUE) {
    printf("<Request sent>, metadata: %s\n", message);
    printf("<Response received>, metadata: %s\n", response);
  }

  int error = error_verification(response);
  if (error == 3 || error == -1) {
    printf("You have not enter in the library, please enter in the library.\n");
    return;
  }
  char *json = calloc(LINELEN, sizeof(char));
  char *lo = strstr(response, "[");
  char *hi = strstr(lo, "]");
  strncpy(json, lo, hi - lo + 1);

  JSON_Value *root_value;
  JSON_Array *commits;
  JSON_Object *commit;
  root_value = json_parse_string(json);
  commits = json_value_get_array(root_value);
  if (json_array_get_count(commits) == 0) {
    printf("We do not have any books\n");
    return;
  }
  printf("Here are the books the we have:\n");
  printf("%-10.10s %-10.10s\n", "id", "title");
  for (int i = 0; i < json_array_get_count(commits); i++) {
    commit = json_array_get_object(commits, i);
    printf("%-10d %.10s \n", (int)json_object_get_number(commit, "id"),
           json_object_get_string(commit, "title"));
  }

  free(response);
  free(message);
  return;
}

void get_book(int sockfd, struct client *cl) {
  char *message;
  char *id = calloc(10, sizeof(char));
  char *url = calloc(sizeof(URL_BOOK) + 10, sizeof(char));
  printf("id=");
  scanf("%s", id);
  sprintf(url, URL_BOOK);
  strcat(url, id);
  message = compute_get_request(HOST_MAIN_SERVER, url, cl->token, NULL,
                                cl->cookie, cl->nr_cookie);
  send_to_server(sockfd, message);
  char *response = receive_from_server(sockfd);
  if (cl->debug == TRUE) {
    printf("<Request sent>, metadata: %s\n", message);
    printf("<Response received>, metadata: %s\n", response);
  }
  int error = error_verification(response);

  if (error == 3 || error == -1) {
    printf("You have not enter in the library, please enter in the library.\n");
    return;
  }
  if (error == 4) {
    printf(
        "We do not have this book, please use get_books to see all books.\n");
    return;
  }

  printf("Here is your book\n");
  char *json = calloc(LINELEN, sizeof(char));
  char *lo = strstr(response, "[");
  char *hi = strstr(lo, "]");
  strncpy(json, lo, hi - lo + 1);

  JSON_Value *root_value;
  JSON_Array *commits;
  JSON_Object *commit;
  root_value = json_parse_string(json);
  commits = json_value_get_array(root_value);
  for (int i = 0; i < json_array_get_count(commits); i++) {
    commit = json_array_get_object(commits, i);
    printf(
        "title:      %s\nauthor:     %s\npublisher:  %s\ngenre:      "
        "%s\npage_count: %d\n",
        json_object_get_string(commit, "title"),
        json_object_get_string(commit, "author"),
        json_object_get_string(commit, "publisher"),
        json_object_get_string(commit, "genre"),
        (int)json_object_get_number(commit, "page_count"));
  }
}

void add_book(int sockfd, struct client *cl) {
  JSON_Value *root_value = json_value_init_object();
  JSON_Object *root_object = json_value_get_object(root_value);
  char *data = NULL;
  char *input = calloc(100, sizeof(char));
  int pg_nr = 0;
  printf("title=");
  scanf("%s", input);
  json_object_set_string(root_object, "title", input);
  printf("author=");
  scanf("%s", input);
  json_object_set_string(root_object, "author", input);
  printf("genre=");
  scanf("%s", input);
  json_object_set_string(root_object, "genre", input);
  printf("publisher=");
  scanf("%s", input);
  json_object_set_string(root_object, "publisher", input);
  printf("page_count=");
  scanf("%d", &pg_nr);
  json_object_set_number(root_object, "page_count", pg_nr);
  data = json_serialize_to_string_pretty(root_value);

  char *message = compute_post_request(HOST_MAIN_SERVER, URL_BOOKS, cl->token,
                                       DATA_TYPE_JSON, &data, 1, cl->cookie,
                                       cl->nr_cookie);  // aici ai refas
  send_to_server(sockfd, message);
  char *response = receive_from_server(sockfd);
  int error = error_verification(response);
  if (cl->debug == TRUE) {
    printf("<Request sent>, metadata: %s\n", message);
    printf("<Response received>, metadata: %s\n", response);
  }
  if (error == -1) {
    printf("Internal error, please remake the last command\n");
  }
  if (error == 1) {
    printf("The information was introduced was incorrect\n");
  }
  if (error == 3) {
    printf("You have not enter in the library, please enter in the library.\n");
  }
  if (error == 0) {
    printf("Your book was added\n");
  }

  json_free_serialized_string(data);
  json_value_free(root_value);
}

void delete_book(int sockfd, struct client *cl) {
  char *message;
  char *id = calloc(10, sizeof(char));
  char *url = calloc(sizeof(URL_BOOK) + 10, sizeof(char));
  int nr;
  printf("id=");
  scanf("%d", &nr);
  sprintf(id, "%d", nr);
  sprintf(url, URL_BOOK);
  strcat(url, id);
  message = compute_delete_request(HOST_MAIN_SERVER, url, cl->token, cl->cookie,
                                   cl->nr_cookie);
  send_to_server(sockfd, message);
  char *response = receive_from_server(sockfd);
  if (cl->debug == TRUE) {
    printf("<Request sent>, metadata: %s\n", message);
    printf("<Response received>, metadata: %s\n", response);
  }
  int error = error_verification(response);
  if (error == 3 || error == -1) {
    printf("You have not enter in the library, please enter in the library.\n");
    return;
  }
  if (error == 4) {
    printf(
        "We do not have this book, please use get_books to see all books.\n");
    return;
  }
  printf("The book %s was deleted\n", id);
}

void logout(int sockfd, struct client *cl) {
  char *message = compute_get_request(HOST_MAIN_SERVER, URL_LOGOUT, NULL, NULL,
                                      cl->cookie, cl->nr_cookie);
  send_to_server(sockfd, message);
  char *response = receive_from_server(sockfd);
  if (cl->debug == TRUE) {
    printf("<Request sent>, metadata: %s\n", message);
    printf("<Response received>, metadata: %s\n", response);
  }
  int error = error_verification(response);
  if (error == -1) {
    printf("Internal error, please remake the last command\n");
    return;
  }
  if (error == 1) {
    printf("You are not logged.\n");
    return;
  }
  printf("Bye, see you next time\n");
  if (cl->nr_cookie != 0) {
    for (int i = 0; i < cl->nr_cookie; i++) {
      free(cl->cookie[i]);
    }
    cl->cookie = calloc(5, sizeof(char *));
    for (int i = 0; i < 5; i++) {
      cl->cookie[i] = calloc(LINELEN, sizeof(char));
    }
    cl->nr_cookie = 0;
    if (cl->token != NULL) {
      free(cl->token);
    }
    cl->token = calloc(LINELEN, sizeof(char));
  }
}

int main(int argc, char *argv[]) {
  struct client *cl = calloc(1, sizeof(struct client));
  cl->cookie = calloc(5, sizeof(char *));
  for (int i = 0; i < 5; i++) {
    cl->cookie[i] = calloc(LINELEN, sizeof(char));
  }
  cl->token = NULL;
  cl->nr_cookie = 0;
  if (argc > 1) {
    if (memcmp(argv[1], "debug", 6) == 0) {
      cl->debug = TRUE;
    }
  } else {
    cl->debug = FALSE;
  }

  char command[15];
  int on = TRUE;
  int sockfd;
  while (on) {
    scanf("%s", command);
    sockfd = open_connection(IP_HOST_MAIN, PORT_MAIN, AF_INET, SOCK_STREAM, 0);
    if (memcmp(command, COMMAND_REGISTER, sizeof(COMMAND_REGISTER)) == 0) {
      sign_in(sockfd, cl);
    }
    if (memcmp(command, COMMAND_LOGIN, sizeof(COMMAND_LOGIN)) == 0) {
      login(sockfd, cl);
    }
    if (memcmp(command, COMMAND_ENTER_LIBRARY, sizeof(COMMAND_ENTER_LIBRARY)) ==
        0) {
      enter_library(sockfd, cl);
    }
    if (memcmp(command, COMMAND_GET_BOOKS, sizeof(COMMAND_GET_BOOKS)) == 0) {
      get_books(sockfd, cl);
    }
    if (memcmp(command, COMMAND_GET_BOOK, sizeof(COMMAND_GET_BOOK)) == 0) {
      get_book(sockfd, cl);
    }
    if (memcmp(command, COMMAND_ADD_BOOK, sizeof(COMMAND_ADD_BOOK)) == 0) {
      add_book(sockfd, cl);
    }
    if (memcmp(command, COMMAND_DELETE_BOOK, sizeof(COMMAND_DELETE_BOOK)) ==
        0) {
      delete_book(sockfd, cl);
    }
    if (memcmp(command, COMMAND_LOGOUT, sizeof(COMMAND_LOGOUT)) == 0) {
      logout(sockfd, cl);
    }
    if (memcmp(command, COMMAND_EXIT, sizeof(COMMAND_EXIT)) == 0) {
      if (cl->nr_cookie != 0) {
        for (int i = 0; i < cl->nr_cookie; i++) {
          free(cl->cookie[i]);
        }
        cl->cookie = calloc(5, sizeof(char *));
        for (int i = 0; i < 5; i++) {
          cl->cookie[i] = calloc(LINELEN, sizeof(char));
        }
        cl->nr_cookie = 0;
        if (cl->token != NULL) {
          free(cl->token);
        }
        cl->token = calloc(LINELEN, sizeof(char));
        on = FALSE;
      }
    }
  }

  free(cl);
  close(sockfd);
  return 0;
}
