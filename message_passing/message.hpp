#ifndef __MESSAGE_H_INCLUDE__
#define __MESSAGE_H_INCLUDE__

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

#endif

