#pragma once

#include <cstddef>

namespace elf {
  static constexpr size_t network_recv_size = 1500;

  struct RWBuffer {
  RWBuffer() : _buffer(nullptr), _len(0), _write_mark(0), _read_mark(0) {}
  RWBuffer(size_t size) : _buffer(nullptr), _len(0) { init(size); }
    ~RWBuffer();
    void init(size_t size);
    char* read_head()          { return _buffer + _read_mark; }
    char* write_head()         { return _buffer + _write_mark; }
    void mark_read(size_t len);
    void mark_written(size_t len);
    bool prepare_write(size_t len);
    void try_compact();
    size_t write_avail() const { return _len - _write_mark; }
    size_t read_avail() const  { return _write_mark - _read_mark; }
    void clear()               { _read_mark = _write_mark = 0; }

    template <typename T>
    const T*
    try_consume_struct() {
      if(read_avail() < sizeof(T))
        return nullptr;

      const T* p = reinterpret_cast<const T*>(read_head());
      mark_read(sizeof(T));
      return p;
    }

    char* _buffer;
    size_t _len;
    size_t _write_mark;
    size_t _read_mark;
    size_t _low_watermark;
    size_t _high_watermark;
  };
}
