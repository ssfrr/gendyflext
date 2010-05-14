#include "log.h"
#include "m_pd.h"

// shows msg if allowed by LOG_LEVEL
#ifdef PD_MAJOR_VERSION
void print_log(const char *msg, int level){
  if (LOG_LEVEL >= level) {
    post(msg);
  }
}

void print_log(const char *msg, int arg1, int level){
  if (LOG_LEVEL >= level) {
    post(msg, arg1);
  }
}

void print_log(const char *msg, int arg1, int arg2, int arg3, int level) {
  if (LOG_LEVEL >= level) {
    post(msg, arg1, arg2, arg3);
  }
}

void print_log(const char *msg, unsigned int arg1, int level){
  if (LOG_LEVEL >= level) {
    post(msg, arg1);
  }
}

void print_log(const char *msg, float arg1, int level){
  if (LOG_LEVEL >= level) {
    post(msg, arg1);
  }
}
#else
#error
void print_log(const char *msg, int level) {}
void print_log(const char *msg, int arg1, int level) {}
void print_log(const char *msg, int arg1, int arg2, int arg3, int level) {}
void print_log(const char *msg, unsigned int arg1, int level) {}
void print_log(const char *msg, float arg1, int level){}
#endif
