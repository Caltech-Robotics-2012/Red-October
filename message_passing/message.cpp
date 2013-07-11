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

using namespace std;

#include "message.hpp"

Message::Message()
{
  length = 0;
  data = NULL;
}

Message::Message(const Message &other)
{
  length = other.length;
  data = new char[length];
  memcpy(data, other.data, length);
}


Message::Message(const char *data, int length)
{
  this->length = length;
  this->data = new char[length];
  memcpy(this->data, data, length);
}

Message Message::operator=(const Message other)
{
  if (data == NULL) {
    delete[] data;
  }

  length = other.length;
  data = new char[length];
  memcpy(data, other.data, length);
}

Message::~Message()
{
  if (data != NULL) {
    //delete[] data;
  }
}

char *Message::getData(int *length)
{
  if (this->data == NULL) {
    *length = 0;
    return NULL;
  }

  char *data = new char[this->length];
  memcpy(data, this->data, this->length);

  *length = this->length;
  return data;
}

