#include <iostream>
#include <algorithm>
#include <unistd.h>
#include <string.h>
#include <queue>
#include "message.hpp"
#include "message_passer.hpp"
using namespace std;

int main()
{
  MessagePasser mp("poll-based");

  const char *s = "Message from poll-based";
  Message to_send(s, strlen(s) + 1);

  queue<Message> message_queue;
  while (true) {
    sleep(1);
    mp.read_messages(&message_queue);
    while (!message_queue.empty()) {
      Message message = message_queue.front();
      message_queue.pop();

      int length;
      char *data = message.getData(&length);
      cout << "data: " << data << "\n";
      delete[] data;
    }
    mp.send_message("event-driven", to_send);
  }
  // for (int i = 0; i < 4; i++) {
  //   for (int j = 0; j < 4; j++) {
  //     cout << get_port(i, j) << "\t";
  //   }
  //   cout << "\n";
  // }
}

