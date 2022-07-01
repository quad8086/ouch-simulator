#include "rwbuffer.h"

#include <algorithm>
#include <cassert>
#include <cstdlib>
#include <cstring>
#include <stdexcept>

using namespace elf;

void
RWBuffer::init(size_t len) {
  if(_buffer)
    throw std::runtime_error("rwbuffer: fatal: already initialized");
  if(len<=network_recv_size)
    throw std::runtime_error("rwbuffer: buffer too small");

  _buffer = reinterpret_cast<char*>(std::calloc(len, 1));
  _len = len;
  _read_mark = _write_mark = 0;
  _low_watermark = std::max(network_recv_size, _len / 10);
  _high_watermark = _low_watermark * 9;
}

RWBuffer::~RWBuffer() {
  if(_buffer)
    std::free(_buffer);
  _buffer = 0;
  _len = 0;
}

void
RWBuffer::mark_read(size_t len) {
  assert(_read_mark+len <= _write_mark);
  _read_mark += len;
}

void
RWBuffer::mark_written(size_t len) {
  assert(_write_mark+len <= _len);
  _write_mark += len;
}

bool
RWBuffer::prepare_write(size_t len) {
  if(len > write_avail())
    try_compact();
  if(len > write_avail())
    return false;

  return true;
}

void
RWBuffer::try_compact() {
  size_t content_len = read_avail();
  std::memmove(_buffer, read_head(), content_len);
  _read_mark = 0;
  _write_mark = content_len;
}
