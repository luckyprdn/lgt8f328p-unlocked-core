#ifndef LGT_HOST_FAKE_WSTRING_H
#define LGT_HOST_FAKE_WSTRING_H

#include <string.h>

// Minimal String for the host syntax harness: signatures mirror the AVR
// WString for the methods the examples use. The harness only parses
// (fsyntax-only) or runs fake-register code, so behavior is best-effort.
class String {
public:
  String() : p(const_cast<char *>("")) {}
  String(const char *s) : p(const_cast<char *>(s ? s : "")) {}
  String(const String &o) : p(o.p) {}
  const char *c_str() const { return p; }
  unsigned int length() const { return (unsigned int)strlen(p); }
  operator const char *() const { return p; }
  bool startsWith(const char *s) const { return strncmp(p, s, strlen(s)) == 0; }
  bool startsWith(const String &s) const { return startsWith(s.p); }
  bool endsWith(const char *s) const {
    size_t n = strlen(p), m = strlen(s);
    return n >= m && strcmp(p + n - m, s) == 0;
  }
  int indexOf(const char *s) const {
    const char *f = strstr(p, s);
    return f ? (int)(f - p) : -1;
  }
  int indexOf(const String &s) const { return indexOf(s.p); }
  int indexOf(const char *s, unsigned int) const { return indexOf(s); }
  int indexOf(const String &s, unsigned int from) const { return indexOf(s.p, from); }
  int lastIndexOf(const char *s) const {
    const char *f = strstr(p, s);
    return f ? (int)(f - p) : -1;
  }
  bool operator==(const String &o) const { return strcmp(p, o.p) == 0; }
  bool operator==(const char *s) const { return strcmp(p, s ? s : "") == 0; }

private:
  char *p;
};

#endif
