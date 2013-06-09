#include <iostream>
#include <algorithm>
#include <unistd.h>
#include <string.h>
#include "message.hpp"
using namespace std;

int main()
{
  MessagePasser mp("poll-based");

  const char *s = "Message from poll-based";
  Message to_send(s, strlen(s) + 1);

  Message message;
  while (true) {
    sleep(1);
    if (mp.read_message(&message) == MessagePasser::GOT_MESSAGE) {
      int length;
      char *data = message.getData(&length);
      cout << "length: " << length << "\n";
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

