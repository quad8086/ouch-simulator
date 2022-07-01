
#include "ouch_structs.h"

using namespace elf;
using namespace std;

void
OUCH42::NewOrder::prepare_send() {
  px = htonl(px);
  qty = htonl(qty);
  minqty = htonl(minqty);
}

void
OUCH42::CancelOrder::prepare_send() {
  qty = htonl(qty);
}

void
OUCH::set_alpha_field(const string& src, char* dest, size_t len) {
  size_t srclen = src.size();

  for(size_t i=0; i<len; i++)
    dest[i] = i<srclen ? src[i] : ' ';
}

size_t
OUCH42::message_size(const char msgtype) {
  switch(msgtype) {
  case MessageType::SystemEvent:
    return sizeof(OUCH42::SystemEvent);
  case MessageType::OrderAck:
    return sizeof(OUCH42::OrderAck);
  default:
    return 0;
  }
}

int
OUCH::ouch_to_native_int(int s) {
  return ntohl(s);
}
