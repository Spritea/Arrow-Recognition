
#ifndef _MY_SERIAL_SIMPLE_HPP_
#define _MY_SERIAL_SIMPLE_HPP_

#include<stdio.h>
#include<string.h>
#include<malloc.h>
#include<sys/types.h>
#include<sys/stat.h>
#include<fcntl.h>
#include<unistd.h>
#include<termios.h>
#include"math.h"

#include <errno.h>      
#include <sys/ioctl.h>
#include <linux/videodev2.h>  //for adjust exposure and resolution manually.

#endif
extern int fd;
int open_serial(int k);
bool serial_startup();
void wait_info();


