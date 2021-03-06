
#ifndef _MY_SERIAL_HPP_
#define _MY_SERIAL_HPP_

#include<stdio.h>
#include<string.h>
#include<malloc.h>
#include<sys/types.h>
#include<sys/stat.h>
#include<fcntl.h>
#include<unistd.h>
#include<termios.h>
#include"math.h"

#endif
extern int fd;
int open_serial(int k);
bool serial_startup();
void preprocess(int pixel,char s[]);
void wait_info();
void bind(char* s1, char* s2, char* s3, char* s4,char out[]);

