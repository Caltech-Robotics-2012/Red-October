#include "message.hpp"
#include <unistd.h>
#include <string.h>
#include <iostream>

using namespace std;

void message_handler(void *parameter, Message message)
{
  int length;
  char *data = message.getData(&length);

  cout << "data: " << data << "\n";

  delete[] data;

}

int main()
{
  MessagePasser mp("event-driven", message_handler, NULL);

  const char *s = "Message from event-driven";
  Message to_send(s, strlen(s) + 1);

  while (true) {
    sleep(1);
    mp.send_message("poll-based", to_send);
  }
}

