#include "my_serial_simple.hpp"
//这是发送程序
int fd;
//char s[5];//each number takes up 5 BYTE.BMC
//int s;
int open_serial(int k)
{   if(k==0)/*串口选择*/
    {   fd=open("/dev/pts/14",O_RDWR|O_NOCTTY);/*读写方式打开串口*/
        //perror("/dev/ttyTHS2");
    }
    else {
        fd=open("/dev/pts/20",O_RDWR|O_NOCTTY);
        //perror("open/dev/pts/12");
    }
    if(fd==-1)/*打开失败*/
        return -1;
    else
        return 0;
}
bool serial_startup()
{
    struct termios option;
    //perror("look me");
    if(-1 == open_serial(0))return false; /*打开串口1*/
    tcgetattr(fd,&option);
    cfmakeraw(&option);
    cfsetispeed(&option,B115200);/*波特率设置为115200bps*/
    cfsetospeed(&option,B115200);
    tcflush(fd, TCIFLUSH);//清此时的串口缓冲区.BMC
    tcsetattr(fd,TCSANOW,&option);
    return true;
}
void wait_info()
{
	int retv;
	char* rbuf;
        char hd[10];
	//char lost2[18]="-000-000-000-0000";
	int ncount=0;
	int answer=0;
	rbuf=hd;
	retv=read(fd,rbuf,1); /*接收数据*/
	if(retv==-1)
	{ 
	  perror("read");/*读状态标志判断*/
	  //return -1;
	}
	while(retv)
	{
	//write(fd,lost2,18);
	  while(*rbuf!='\n')/*判断数据是否接收完毕*/
	  { 
	    ncount+=1;
	    rbuf++; 
	    retv=read(fd,rbuf,1);
	    if(retv==-1)
	    { perror("read");}
	  }

	if((hd[0]=='B')&&(hd[1]=='C'))//begin check
	{
	    answer=100;
	    break;
	}
	else
	answer=0;

	rbuf=&hd[0];
	memset(hd,0,sizeof(hd));
	retv=read(fd,rbuf,1);
	ncount=0;
	}
}

//write(fd,buff,strlen(buff));

