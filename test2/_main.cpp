#include <opencv2/opencv.hpp>
#include <iostream>
#include <vector>
#include"my_serial.hpp"
using namespace std;
using namespace cv;
const int UAV_AREA_MIN=3000;//variable
const int UAV_AREA_MAX=600*440;
Point origin(320,250);//point
Point p3(0,0);
Point p4(10,10);
Rect no_contour(p3,p4);
Mat real_image;
int flag=0;

Rect Find_UAV(Mat img_in);
Point compute(Rect result);
int main()
{
    char lost[18]="-000-000-000-0000";
    char outsp[18];//out to serial port.BMC
    char expecx[5]="+000";
    char expecy[5]="+000";
    char nowx[5];//now x
    char nowy[5];//now y
    Mat img_in;
    Rect big;
    Point diff;

    Rect r;
    Rect max_tmp;
    Mat out;
    Mat HGreen;
    Mat SGreen;
    
    vector<Mat> HSVSplit;
    Mat element = getStructuringElement(MORPH_RECT, Size(5, 5));
    vector<vector<Point> > contours;
    if(!serial_startup()){
      cout<<"serial start up failed..."<<endl;
      return -1;
    }
    Mat frame;
    Mat HSVImage;
    VideoCapture cap(0);
    if(!cap.isOpened()){
      cout<<"cap open failed..."<<endl;
      exit(0);
    }
    cap.read(frame);
    cap.read(frame);
    cap.read(frame);
    while(cap.read(frame))
    {
      imshow("frame", frame);
      // double t = (double)getTickCount();
      
      cvtColor(frame, HSVImage, CV_BGR2HSV);
      split(HSVImage,HSVSplit);
      //inRange(HSVSplit[0], Scalar(40), Scalar(80), HGreen);
      //inRange(HSVSplit[0], Scalar(100), Scalar(140), HGreen);
      inRange(HSVSplit[0], Scalar(10), Scalar(30), HGreen);
      //imshow("hgreen",HGreen);
      threshold(HSVSplit[1], SGreen, 80, 255, THRESH_BINARY);
      //imshow("sgreen",SGreen);
      //imshow("h",HSVSplit[0]);
      //imshow("v",HSVSplit[2]);
      cv::bitwise_and(HGreen, SGreen, out);
      morphologyEx(out, out, MORPH_OPEN, element);//open operator,remove isolated noise points.
      morphologyEx(out, out, MORPH_CLOSE, element);//close operator,get connected domain.BMC
      findContours(out,contours,CV_RETR_EXTERNAL,CV_CHAIN_APPROX_NONE);
      if( contours.size() == 0 ){
	//write(fd,lost,18);
	continue;
      }
      Mat result(out.size(),CV_8U,Scalar(0));
      drawContours(result,contours,-1,Scalar(255),2);
      max_tmp=boundingRect(Mat(contours[0]));
      for(int i=1; i<contours.size(); i++)
      {
	r = boundingRect(Mat(contours[i]));
	max_tmp=(r.area()>max_tmp.area())?r:max_tmp;
      }
      rectangle(result, max_tmp, Scalar(255), 2);
      cout<<"area "<<max_tmp.area()<<endl;
      imshow("result",result);
      if('q' == (char)waitKey(3)) exit(0);
      if(max_tmp.area()<UAV_AREA_MIN||max_tmp.area()>UAV_AREA_MAX){
	//write(fd,lost,18);
      }
      else{
	diff=compute(max_tmp);
	preprocess(diff.x,nowx);
	preprocess(diff.y,nowy);//change the content of char s[5].BMC
	bind(expecx,expecy,nowx,nowy,outsp);
	//write(fd,outsp,18);
      }  
      // cout<<endl<<"time "<<((double)getTickCount() - t) / (double)getTickFrequency()<<endl;
    }
    
    return 0;
} 

Point compute(Rect result)
{
    Point diff;
    diff.x=floor((result.tl().x+result.br().x)/2.0)-origin.x;
    diff.y=floor((result.tl().y+result.br().y)/2.0)-origin.y;
    cout<<diff.x<<" "<<diff.y<<endl;
    return diff;
}
