#ifndef __MESSAGE_H_INCLUDE__
#define __MESSAGE_H_INCLUDE__

#include <queue>

const int MESSAGE_BASE_PORT = 6000;

char const * const PROCESS_NAME[] = {"poll-based",
                                     "event-driven",
                                     "lalala",
                                     "...."};


const int PROCESS_COUNT = sizeof(PROCESS_NAME) / sizeof(PROCESS_NAME[0]);
const int INVALID_PROCESS = -1;
const int INVALID_PORT = -1;



class Message
{
protected:
  char *data;
  int length;
public:
  Message();
  Message(const Message &other);
  Message(const char *data, int length);

  Message operator=(const Message other);

  ~Message();

  char *getData(int *length);
};

class MessagePasser
{
protected:
  int process_index;
  void (*message_handler)(void *parameter, Message message);
  void *parameter;
  int listener_fd[PROCESS_COUNT]; // TODO: only actually need (process_index - 1)
  int client_fd[PROCESS_COUNT]; // TODO: only actually need (PROCESS_COUNT - 1)
  std::queue<Message> message_queue;

public:

  enum SendStatus{SUCCESS,
                  NOT_FOUND,
                  NOT_CONNECTED};

  enum ReadStatus{GOT_MESSAGE,
                  NO_MESSAGE};

  MessagePasser();
  MessagePasser(const char *process_name);
  MessagePasser(const char *process_name, void (*message_handler)(void *parameter, Message message), void *parameter);

  MessagePasser::SendStatus send_message(int other_process_index, Message message);
  MessagePasser::SendStatus send_message(const char *other_process_name, Message message);

  MessagePasser::ReadStatus read_message(Message *message);

  void run();

};

int get_process_index(const char *name);
int get_port(int process_index1, int process_index2);

#endif

