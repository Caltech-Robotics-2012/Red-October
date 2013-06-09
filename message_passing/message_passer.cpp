#include <stdlib.h>
#include <string.h>
#include <algorithm>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <pthread.h>
#include <assert.h>
#include <iostream>

#include "message_passer.hpp"

using namespace std;

ssize_t send_all(int socket, void *buffer, size_t length, int flags)
{
  size_t bytes_sent = 0;
  while (bytes_sent < length) {
    ssize_t temp = send(socket, buffer, length - bytes_sent, flags);
    if (temp == -1) { // Error
      return (ssize_t) -1;
    }
    bytes_sent += (size_t) temp;
  }
  return (ssize_t) bytes_sent;
}

ssize_t recv_all(int socket, void *buffer, size_t length, int flags)
{
  return recv(socket, buffer, length, flags | MSG_WAITALL);
}



bool send_message_socket(int socket, Message message, int flags)
{
  int length;
  char *data = message.getData(&length);

  if (send_all(socket, (void *) &length, sizeof(length), flags) != sizeof(length)) {
    return false;
  }
  if (send_all(socket, (void *) data, length, flags) != length) {
    return false;
  }

  delete[] data;
  return true;
}

bool recv_message_socket(int socket, Message *message, int flags)
{
  int length;
  if (recv_all(socket, (void *) &length, sizeof(int), flags) != sizeof(int)) {
    return false;
  }

  char *data = new char[length];
  if (recv_all(socket, (void *) data, length, flags) != length) {
    delete[] data;
    return false;
  }

  *message = Message(data, length);

  delete[] data;
  return true;
}

int listen_to_port(int port)
{
  int sockfd = socket(AF_INET, SOCK_STREAM, 0);

  if (sockfd < 0) {
    fprintf(stderr, "Unable to open socket %d.\n", sockfd);
    exit(1);
  }
  
  // Zero buffer
  struct sockaddr_in serv_addr;
  bzero((char*) &serv_addr, sizeof(serv_addr));
  
  serv_addr.sin_family = AF_INET;
  serv_addr.sin_port = htons(port);
  serv_addr.sin_addr.s_addr = INADDR_ANY;
  
  // Allow reusability
  int yes = 1;
  setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(int));
  
  // Time to Bind
  if (bind(sockfd, (struct sockaddr *) &serv_addr, sizeof(serv_addr)) < 0) {
    fprintf(stderr, "Unable to bind socket %d.\n", sockfd);
    exit(1);
  }
  
  // Now time to listen for clients
  listen(sockfd, 32); // 32 is number of waiting line

  return sockfd;
}

int connect_to(const char *hostname, int port)
{
  int sockfd;
  struct sockaddr_in serv_addr;
  struct hostent *server;

  sockfd = socket(AF_INET, SOCK_STREAM, 0);
  if (sockfd < 0) {
    fprintf(stderr, "ERROR opening socket");
    exit(0);
  }

  server = gethostbyname(hostname);
  if (server == NULL) {
    fprintf(stderr,"ERROR, no such host\n");
    exit(0);
  }

  bzero((char *) &serv_addr, sizeof(serv_addr));
  serv_addr.sin_family = AF_INET;
  bcopy((char *)server->h_addr, 
       (char *)&serv_addr.sin_addr.s_addr,
       server->h_length);
  serv_addr.sin_port = htons(port);
  if (connect(sockfd,(struct sockaddr *) &serv_addr,sizeof(serv_addr)) < 0) {
    return -1;
  }
  return sockfd;
}

void *thread_function(void *arg)
{
  ((MessagePasser *) arg)->run();
  pthread_exit(NULL);
}



MessagePasser::MessagePasser()
{
  process_index = INVALID_PROCESS;
  message_handler = NULL;
  parameter = NULL;
}

MessagePasser::MessagePasser(const char *process_name)
{
  process_index = get_process_index(process_name);
  message_handler = NULL;
  parameter = NULL;

  // Poll-driven
  for (int i = 0; i < PROCESS_COUNT; i++) {
    client_fd[i] = -1;

    int port = get_port(process_index, i);
    listener_fd[i] = listen_to_port(port);
  }

  pthread_attr_t attr;
  pthread_attr_init(&attr);
  pthread_attr_setdetachstate (&attr, PTHREAD_CREATE_DETACHED);
  pthread_create(&thread, &attr, thread_function, (void *) this);
}

MessagePasser::MessagePasser(const char *process_name, void (*message_handler)(void *parameter, Message message), void *parameter)
{
  process_index = get_process_index(process_name);
  this->message_handler = message_handler;
  this->parameter = parameter;

  // Event-driven
  for (int i = 0; i < PROCESS_COUNT; i++) {
    client_fd[i] = -1;

    int port = get_port(process_index, i);
    listener_fd[i] = listen_to_port(port);
  }

  pthread_attr_t attr;
  pthread_attr_init(&attr);
  pthread_attr_setdetachstate (&attr, PTHREAD_CREATE_DETACHED);
  pthread_create(&thread, &attr, thread_function, (void *) this);
}

MessagePasser::SendStatus MessagePasser::send_message(int other_process_index, Message message)
{
  // TODO: Where to lock/unlock?
  if (other_process_index < 0 || other_process_index >= PROCESS_COUNT) {
    return NOT_FOUND;
  }

  if (client_fd[other_process_index] == -1) {
    ////////////////////////////////////////////////////////////////
    int port = get_port(other_process_index, process_index);
    client_fd[other_process_index] = connect_to("localhost", port);
  }

  if (!send_message_socket(client_fd[other_process_index], message, 0)) {
    int port = get_port(other_process_index, process_index);
    client_fd[other_process_index] = connect_to("localhost", port);
  }

  if (!send_message_socket(client_fd[other_process_index], message, 0)) {
    return NOT_CONNECTED;
  }

  return SUCCESS;
}

MessagePasser::SendStatus MessagePasser::send_message(const char *other_process_name, Message message)
{
  int other_process_index = get_process_index(other_process_name);
  if (other_process_index == INVALID_PROCESS) {
    return NOT_FOUND;
  }

  return send_message(other_process_index, message);
}

void MessagePasser::read_messages(queue<Message> *message_queue)
{
  // TODO: Lock
  *message_queue = this->message_queue;
  this->message_queue = queue<Message>();
  // TODO: Unlock
}

void MessagePasser::run()
{
  // TODO: Where lock/unlock?
  fd_set readfds;
  struct timeval tv;
  int retval;
  int nfds;
  struct sockaddr_in cli_addr;
  socklen_t clilen;



  while (true) {
    // Set the wait time
    tv.tv_sec = 1;
    tv.tv_usec = 0;

    nfds = -1;
    FD_ZERO(&readfds);
    for (int i = 0; i < PROCESS_COUNT; i++) {
      FD_SET(listener_fd[i], &readfds);
      nfds = max(nfds, listener_fd[i]);
    }

    for (int i = 0; i < PROCESS_COUNT; i++) {
      if (client_fd[i] != -1) {
        FD_SET(client_fd[i], &readfds);
        nfds = max(nfds, client_fd[i]);
      }
    }

    retval = select(nfds + 1, &readfds, NULL, NULL, &tv);
    // TODO: Use retval?

    for (int i = 0; i < PROCESS_COUNT; i++) {
      if (FD_ISSET(listener_fd[i], &readfds)) {
        clilen = sizeof(cli_addr);
        assert(client_fd[i] == -1);
        client_fd[i] = accept(listener_fd[i], (struct sockaddr *) &cli_addr, &clilen);
      }
    }

    for (int i = 0; i < PROCESS_COUNT; i++) {
      if (client_fd[i] != -1) {
        if (FD_ISSET(client_fd[i], &readfds)) {
          Message message;
          if (!recv_message_socket(client_fd[i], &message, 0)) {
            client_fd[i] = -1;
          }
          else {
            if (message_handler != NULL) {
              message_handler(parameter, message);
            }
            else {
              // TODO: Lock
              message_queue.push(message);
              // TODO: Unock
            }
          }
        }
      }
    }
  }
}


int get_process_index(const char *process_name)
{
  for (int i = 0; i < PROCESS_COUNT; i++) {
    if (strcmp(process_name, PROCESS_NAME[i]) == 0) {
      return i;
    }
  }
  return INVALID_PROCESS;
}

int get_port(int process_index1, int process_index2)
{
  int p1 = process_index1;
  int p2 = process_index2;

  if (p1 < 0 || p1 >= PROCESS_COUNT || p2 < 0 || p2 >= PROCESS_COUNT) {
    return INVALID_PORT;
  }

  return MESSAGE_BASE_PORT + p1 * PROCESS_COUNT + p2;
}

