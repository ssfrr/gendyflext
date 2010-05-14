/*********************************************
 *
 * libgendy
 *
 * a library implementing Iannis Xenakis's Dynamic Stochastic Synthesis
 *
 * Copyright 2009,2010 Spencer Russell
 * Released under the GPLv3
 *
 * This file is part of libgendy.
 *
 * libgendy is free software: you can redistribute it and/or
 * modify it under the terms of the GNU General Public License version 3 as
 * published by the Free Software Foundation.
 *
 * libgendy is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY
 * or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License for
 * more details.
 *
 * You should have received a copy of the GNU General Public License along with
 * libgendy.  If not, see <http://www.gnu.org/licenses/>.
 *
 *
 ********************************************/




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
