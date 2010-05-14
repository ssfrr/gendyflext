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
void print_log(const char *msg, int arg1, int arg2, int arg3, int level);
void print_log(const char *msg, unsigned int arg1, int level);
void print_log(const char *msg, float arg1, int level);

#endif /* LOG_H */
