#ifndef LGT_AVR_STUB_STREAM_H
#define LGT_AVR_STUB_STREAM_H
#include <stddef.h>
#include <stdint.h>
class Print {
  uint8_t write_error_;
public:
  Print():write_error_(0){}
  virtual ~Print(){}
  void setWriteError(int e=1){write_error_=(uint8_t)(e?1:0);}
  int getWriteError() const{return write_error_;}
  void clearWriteError(){write_error_=0;}
  virtual size_t write(uint8_t)=0;
  virtual size_t write(const uint8_t *b,size_t n){size_t w=0;while(n--){w+=write(*b++);}return w;}
};
class Stream:public Print{
public:
  virtual int available()=0;
  virtual int read()=0;
  virtual int peek()=0;
  virtual void flush()=0;
};
#endif
