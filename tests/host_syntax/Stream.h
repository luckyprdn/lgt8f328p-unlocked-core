#ifndef LGT_HOST_FAKE_STREAM_H
#define LGT_HOST_FAKE_STREAM_H
#include <stddef.h>
#include <stdint.h>
class Print {
  bool write_error_;
public:
  Print(): write_error_(false) {}
  virtual ~Print() {}
  void setWriteError(int err=1){write_error_=err!=0;}
  int getWriteError() const {return write_error_?1:0;}
  void clearWriteError(){write_error_=false;}
  virtual size_t write(uint8_t) = 0;
  virtual size_t write(const uint8_t *buffer, size_t size) {
    size_t n=0; while(size--){n += write(*buffer++);} return n;
  }
};
class Stream : public Print {
public:
  virtual int available(void)=0;
  virtual int read(void)=0;
  virtual int peek(void)=0;
  virtual void flush(void)=0;
};
#endif
