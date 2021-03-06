#include <opencv2/opencv.hpp>
#include <iostream>
#include <vector>
#include<cstdio>
#include"my_serial.hpp"
using namespace std;
using namespace cv;
const int ARROW_AREA_MIN=3000;//variable
const int ARROW_AREA_MAX=600*440;
int flag=0;
Mat pro_after;

string type2str(int type);
int Arrow_direction(Mat pro_after);
int Color_detect(Mat frame);
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
    bool Isarrow;
    
    Mat frame;
    frame=imread("/home/ubuntu/code/block/hotkey/test1/build/t10.jpg");
    //resize(frame,frame,Size(frame.cols/5,frame.rows/5));
    //double t = (double)getTickCount();
    vector<Mat> HSVSplit;
    Mat element = getStructuringElement(MORPH_RECT, Size(5, 5));
    vector<vector<Point> > contours;
    Mat HSVImage;
    Mat out;
    Mat HGreen_light;
    Mat HGreen_dark;
    Mat HGreen;
    Mat SGreen;
    Rect r;
    Rect max_tmp;

    cvtColor(frame, HSVImage, CV_BGR2HSV);
    split(HSVImage,HSVSplit);
    //inRange(HSVSplit[0], Scalar(40), Scalar(80), HGreen);
    //inRange(HSVSplit[0], Scalar(100), Scalar(140), HGreen);
    //inRange(HSVSplit[0], Scalar(100), Scalar(140), HGreen);//蓝色可用
    //  inRange(HSVSplit[0], Scalar(0), Scalar(30), HGreen);//红色的
    //inRange(HSVSplit[0], Scalar(80), Scalar(110), HGreen);//蓝色,摄像头的,亮/暗光
    inRange(HSVSplit[0], Scalar(0), Scalar(20), HGreen_light);//红色,摄像头的,亮
    inRange(HSVSplit[0], Scalar(150), Scalar(180), HGreen_dark);//红色，摄像头，暗
    cv::bitwise_or(HGreen_light, HGreen_dark, HGreen);
    imshow("hgreen",HGreen);
    threshold(HSVSplit[1], SGreen, 80, 255, THRESH_BINARY);
    imshow("sgreen",SGreen);
    imshow("h",HSVSplit[0]);
    waitKey(0);
    //imshow("v",HSVSplit[2]);
    cv::bitwise_and(HGreen, SGreen, out);
    morphologyEx(out, out, MORPH_OPEN, element);//open operator,remove isolated noise points.
    morphologyEx(out, out, MORPH_CLOSE, element);//close operator,get connected domain.BMC
    //imshow("before contour",out);
    Mat solid;
    solid=out.clone();
    //imshow("pro",solid);
    findContours(out,contours,CV_RETR_EXTERNAL,CV_CHAIN_APPROX_NONE);
    if( contours.size() == 0 ) 
    {
        //write(fd,lost,18);
        //continue;
        cout<<"no contour..."<<endl;
        Isarrow=0;
	return -1;
    }
    Mat result(out.size(),CV_8U,Scalar(0));
    drawContours(result,contours,-1,Scalar(255),2);
    max_tmp=boundingRect(Mat(contours[0]));
    for(int i=1; i<contours.size(); i++)
    {
        r = boundingRect(Mat(contours[i]));
        max_tmp=(r.area()>max_tmp.area())?r:max_tmp;
    }
    if(max_tmp.area()<ARROW_AREA_MIN||max_tmp.area()>ARROW_AREA_MAX)
    {	
        Isarrow=0;
  	cout<<"area wrong..."<<endl;
        return -1;
    }
    Mat pro;
    solid(max_tmp).copyTo(pro);
    imshow("pro",pro);
    pro_after=pro.clone();
    string ty =  type2str( pro_after.type() );
    printf("Matrix: %s %dx%d \n", ty.c_str(), pro_after.cols, pro_after.rows );
    rectangle(result, max_tmp, Scalar(255), 2);
    cout<<"area "<<max_tmp.area()<<endl;
    Isarrow= 1;
    imshow("result",result);
    waitKey(0);
    //cout<<endl<<"time "<<((double)getTickCount() - t) /(double)getTickFrequency()<<endl;
    if(Isarrow)
    int arrow=Arrow_direction(pro_after);

    return 0;
}

string type2str(int type) {
    string r;

    uchar depth = type & CV_MAT_DEPTH_MASK;
    uchar chans = 1 + (type >> CV_CN_SHIFT);

    switch ( depth ) {
    case CV_8U:
        r = "8U";
        break;
    case CV_8S:
        r = "8S";
        break;
    case CV_16U:
        r = "16U";
        break;
    case CV_16S:
        r = "16S";
        break;
    case CV_32S:
        r = "32S";
        break;
    case CV_32F:
        r = "32F";
        break;
    case CV_64F:
        r = "64F";
        break;
    default:
        r = "User";
        break;
    }

    r += "C";
    r += (chans+'0');

    return r;
}

int Arrow_direction(Mat pro_after)
{
    Mat proj;
    bool  Istop_down;
    if(pro_after.rows>pro_after.cols)
        Istop_down=true;
    else
        Istop_down=false;
    if(Istop_down)
        reduce(pro_after,proj,1,CV_REDUCE_AVG,-1);
    else
        reduce(pro_after,proj,0,CV_REDUCE_AVG,-1);
    //string t1 =  type2str( proj.type() );
    //printf("Matrix: %s %dx%d \n", t1.c_str(), proj.cols, proj.rows );
    cout<<"mat size "<<proj.rows<<" "<<proj.cols<<endl;
    int position=0;
    int max=100;
    if(Istop_down)
    {
        for(int row=0; row<proj.rows; ++row)
        {
            const uchar *ptr = proj.ptr(row);
            if((int)ptr[0]>max)
            {
                max=(int)ptr[0];
                position=row;
            }
        }
    }
    else
    {
        const uchar *ptr = proj.ptr(0);
        for(int col=0; col<proj.cols; ++col)
        {

            if((int)ptr[col]>max)
            {
                max=(int)ptr[col];
                position=col;
            }
        }
    }
    cout<<"position "<<position<<endl;
    double scale=(double)position/((proj.rows>proj.cols)?proj.rows:proj.cols);
    cout<<scale<<endl;
    if(scale<0.4&&Istop_down)
    {
        cout<<"direction: top"<<endl;
        return 1;
    }
    else if(scale>0.6&&Istop_down)
    {
        cout<<"direction: down"<<endl;
        return 2;
    }
    else if(scale<0.4&&!Istop_down)
    {
        cout<<"direction: left"<<endl;
        return 3;
    }
    else if(scale>0.6&&!Istop_down)
    {
        cout<<"direction: right"<<endl;
        return 4;
    }
}
int Color_detect(Mat frame)
{
    vector<Mat> HSVSplit;
    Mat element = getStructuringElement(MORPH_RECT, Size(5, 5));
    vector<vector<Point> > contours;
    Mat HSVImage;
    Mat out;
    Mat HGreen;
    Mat SGreen;
    Rect r;
    Rect max_tmp;

    cvtColor(frame, HSVImage, CV_BGR2HSV);
    split(HSVImage,HSVSplit);
    //inRange(HSVSplit[0], Scalar(40), Scalar(80), HGreen);
    //inRange(HSVSplit[0], Scalar(100), Scalar(140), HGreen);
    //inRange(HSVSplit[0], Scalar(100), Scalar(140), HGreen);//蓝色可用
      inRange(HSVSplit[0], Scalar(0), Scalar(30), HGreen);//红色的
    imshow("hgreen",HGreen);
    threshold(HSVSplit[1], SGreen, 80, 255, THRESH_BINARY);
    imshow("sgreen",SGreen);
    imshow("h",HSVSplit[0]);
    //imshow("v",HSVSplit[2]);
    cv::bitwise_and(HGreen, SGreen, out);
    morphologyEx(out, out, MORPH_OPEN, element);//open operator,remove isolated noise points.
    morphologyEx(out, out, MORPH_CLOSE, element);//close operator,get connected domain.BMC
    //imshow("before contour",out);
    Mat solid;
    solid=out.clone();
    //imshow("pro",solid);
    findContours(out,contours,CV_RETR_EXTERNAL,CV_CHAIN_APPROX_NONE);
    if( contours.size() == 0 ) 
    {
        //write(fd,lost,18);
        //continue;
        cout<<"no contour..."<<endl;
        return 0;
    }
    Mat result(out.size(),CV_8U,Scalar(0));
    drawContours(result,contours,-1,Scalar(255),2);
    max_tmp=boundingRect(Mat(contours[0]));
    for(int i=1; i<contours.size(); i++)
    {
        r = boundingRect(Mat(contours[i]));
        max_tmp=(r.area()>max_tmp.area())?r:max_tmp;
    }
    if(max_tmp.area()<ARROW_AREA_MIN||max_tmp.area()>ARROW_AREA_MAX)
        return 0;
    Mat pro;
    solid(max_tmp).copyTo(pro);
    imshow("pro",pro);
    pro_after=pro.clone();
    string ty =  type2str( pro_after.type() );
    printf("Matrix: %s %dx%d \n", ty.c_str(), pro_after.cols, pro_after.rows );
    rectangle(result, max_tmp, Scalar(255), 2);
    cout<<"area "<<max_tmp.area()<<endl;
    return 1;
    imshow("result",result);
    waitKey(0);
    //cout<<endl<<"time "<<((double)getTickCount() - t) /(double)getTickFrequency()<<endl;
}
