#ifndef LOG_H
#define LOG_H

// define and set log levels
#define LOG_NONE 0
#define LOG_ERROR 1
#define LOG_INFO 2
#define LOG_DEBUG 3

#define LOG_LEVEL LOG_INFO

void print_log(const char *msg, int level);
void print_log(const char *msg, int arg1, int level);
void print_log(const char *msg, unsigned int arg1, int level);
void print_log(const char *msg, float arg1, int level);

#endif /* LOG_H */
