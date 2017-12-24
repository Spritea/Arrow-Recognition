#include "my_serial.hpp"
//这是发送程序
int fd;
//char s[5];//each number takes up 5 BYTE.BMC
//int s;
int open_serial(int k)
{   if(k==0)/*串口选择*/
    {   fd=open("/dev/ttyTHS2",O_RDWR|O_NOCTTY);/*读写方式打开串口*/
        //perror("/dev/pts/1");
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
void preprocess(int pixel,char s[])//declare preprocess function.BMC
{
    //char s[5];
    //char* pchar;
    //pchar=s;
    sprintf(s, "%-4d", pixel);
    if (s[0] != '-')//为正数,先加上符号
    {
        for (int k = 3; k > 0; k--)
            s[k] = s[k - 1];
        s[0] = '+';
    }
    for (int i = 0; i <= 3; i++)
    {
        if (s[i] == ' ')//有空格情况,要移位
        {
            if (i==3)
            {
                for (int m = 3; m > 0; m--)
                    s[m] = s[m - 1];
                s[1] = '0';
            }
            else
            {
                s[3] = s[1];
                s[2] = '0';
                s[1] = '0';
            }
            break;
        }
    }
    //return pchar;
}
//write(fd,buff,strlen(buff));
void bind(char* s1, char* s2, char* s3, char* s4,char out[])
{
	for (int i = 0; i < 16; i++)
	{
		if (i <= 3)
			out[i] = s1[i];
		else if (i <= 7)
			out[i] = s2[i-4];
		else if (i <= 11)
			out[i] = s3[i-8];
		else out[i] = s4[i-12];
	}
	out[16]='1';//1 表示正常检测
	out[17]='\0';
	//return rout;
}
