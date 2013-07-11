#ifndef __MESSAGE_PASSER_H_GUARD__
#define __MESSAGE_PASSER_H_GUARD__

#include <queue>
#include <pthread.h>
#include "message.hpp"

const int MESSAGE_BASE_PORT = 6000;

char const * const PROCESS_NAME[] = {"poll-based",
                                     "event-driven",
                                     "lalala",
                                     "...."};


const int PROCESS_COUNT = sizeof(PROCESS_NAME) / sizeof(PROCESS_NAME[0]);
const int INVALID_PROCESS = -1;
const int INVALID_PORT = -1;

class MessagePasser
{
protected:
  int process_index;
  void (*message_handler)(void *parameter, Message message);
  void *parameter;
  int listener_fd[PROCESS_COUNT];
  int client_fd[PROCESS_COUNT];
  std::queue<Message> message_queue;
  pthread_t thread;

public:

  enum SendStatus{SUCCESS,
                  NOT_FOUND,
                  NOT_CONNECTED};

  MessagePasser();
  MessagePasser(const char *process_name);
  MessagePasser(const char *process_name, void (*message_handler)(void *parameter, Message message), void *parameter);

  MessagePasser::SendStatus send_message(int other_process_index, Message message);
  MessagePasser::SendStatus send_message(const char *other_process_name, Message message);

  void read_messages(std::queue<Message> *message);

  void run();

};

int get_process_index(const char *name);
int get_port(int process_index1, int process_index2);

#endif

