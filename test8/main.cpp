#include "opencv2/highgui/highgui.hpp"
#include "opencv2/imgproc/imgproc.hpp"
#include <iostream>
#include <stdio.h>
#include "my_serial_simple.hpp"

#define BUFFER_EXPOSURE_VALUE 50
//#define DEFAULT_FRAME_WIDTH 1280
//#define DEFAULT_FRAME_HEIGHT 720
#define DEFAULT_FRAME_WIDTH 640
#define DEFAULT_FRAME_HEIGHT 480

using namespace cv;
using namespace std;
const int ARROW_AREA_MIN=3000;//variable,change now!!!!!!
const int ARROW_AREA_MAX=600*440;
const int H_LOW_B=80;	//xiangji de.
const int H_HIGH_B=110;
const int H_LOW_R_Light=0;
const int H_HIGH_R_Light=20;
const int H_LOW_R_Dark=150;
const int H_HIGH_R_Dark=180;
int flag_B=0;
int flag_R=0;
Mat pro_after;
Mat pro_after_B;
Mat pro_after_R;

int Color_detect(Mat frame);
int Arrow_direction(Mat pro_after);
int Circle_detect(Mat src);
int Color_choose(Mat HSVImage,bool IsBlue);
/** @function main */
void set_camera_exposure(int val) {

    int cam_fd;
    if ((cam_fd = open("/dev/video0", O_RDWR)) == -1) {
        cerr << "Camera open error" << endl;
        exit(0);
    }

    struct v4l2_control control_s;
    control_s.id = V4L2_CID_EXPOSURE_AUTO;
    control_s.value = V4L2_EXPOSURE_MANUAL;
    ioctl(cam_fd, VIDIOC_S_CTRL, &control_s);

    control_s.id = V4L2_CID_EXPOSURE_ABSOLUTE;
    control_s.value = val;
    ioctl(cam_fd, VIDIOC_S_CTRL, &control_s);
    close(cam_fd);
}

int main(int argc, char** argv)
{
    int direction=0;//1,2,3,4=arrow,5=circle,6=robo.
    int Iscircle;
    int Isarrow;
    
    if(!serial_startup()){
      cout<<"serial start up failed..."<<endl;
      return -1;
    }
    /// Read the image
    //src = imread( argv[1], 1 );
    //double t = (double)getTickCount();
    //resize(src,src,Size(src.cols/5,src.rows/5));
    
    Mat src;
    char output[2];
    VideoCapture cap(0);
    cap.set(CV_CAP_PROP_FRAME_WIDTH, DEFAULT_FRAME_WIDTH);
    cap.set(CV_CAP_PROP_FRAME_HEIGHT, DEFAULT_FRAME_HEIGHT);
    
    set_camera_exposure(BUFFER_EXPOSURE_VALUE);
    
    if(!cap.isOpened()){
      cout<<"cap open failed..."<<endl;
      exit(0);
    }
    cap.read(src);
    cap.read(src);
    cap.read(src);
    while(cap.read(src))
{
    imshow("src",src);
    if('q'==(char)waitKey(7)) break;
    Iscircle=Circle_detect(src);
    if(Iscircle==-1)
    {
        cout<<"image read no data in circle detect..."<<endl;
        continue;
    }
    if(Iscircle==1)
        direction=5;
    else
    {
        Isarrow=Color_detect(src);//normal =1,wrong=0.BMC
        if(Isarrow==1)
        {
            direction=Arrow_direction(pro_after);
            if(direction==7)
            {
                cout<<"the arrow position wrong..."<<endl;
                continue;
            }
        }
        else
            direction=6;
    }
	//cout<<endl<<"time "<<((double)getTickCount() - t) /(double)getTickFrequency()<<endl;
    cout<<"direct: "<<direction<<endl;
    sprintf(output,"%d",direction);
    write(fd,output,2);
}
    return 0;

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
    else
        return 7;
}
int Color_detect(Mat frame)
{
    Mat HSVImage;
    int area_B;
    int area_R;
    cvtColor(frame, HSVImage, CV_BGR2HSV);
    area_B=Color_choose(HSVImage,1);
    area_R=Color_choose(HSVImage,0);
    if(flag_B<3&&flag_R<3)
        return 0;
    else
    {
        if(area_B>area_R)
            pro_after=pro_after_B;
        else
            pro_after=pro_after_R;
        return 1;
    }
    //cout<<endl<<"time "<<((double)getTickCount() - t) /(double)getTickFrequency()<<endl;
}

int Circle_detect(Mat src)
{
    Mat src_gray;
    if( !src.data )
    {
        return -1;
    }
    /// Convert it to gray
    cvtColor( src, src_gray, CV_BGR2GRAY );
    /// Reduce the noise so we avoid false circle detection
    //medianBlur(src_gray, src_gray, 5);
    GaussianBlur( src_gray, src_gray, Size(9, 9), 2, 2 );
//     imshow("gaussian",src_gray);
    vector<Vec3f> circles;

    /// Apply the Hough Transform to find the circles
    HoughCircles( src_gray, circles, CV_HOUGH_GRADIENT, 1, src_gray.rows/4, 100, 30, 15, 30 );
    //mainly depends 100,30 the 2 parameters.BMC
    //HoughCircles(src_gray, circles, CV_HOUGH_GRADIENT, 1, 10,100, 30, 15, 30);
    // change the last two parameters
    // (min_radius & max_radius) to detect larger circles
    cout<<"circles num: "<<circles.size()<<endl;
    if(circles.size()>=2)   //circle num >=2.BMC
    {
        cout<<"hhh "<<endl;
/*        /// Draw the circles detected
        for( size_t i = 0; i < circles.size(); i++ )
        {
            Point center(cvRound(circles[i][0]), cvRound(circles[i][1]));
            int radius = cvRound(circles[i][2]);
            // circle center
            circle( src, center, 3, Scalar(0,255,0), -1, 8, 0 );
            // circle outline
            circle( src, center, radius, Scalar(0,0,255), 3, 8, 0 );
        }
*/
        /// Show your results
//     namedWindow( "Hough Circle Transform Demo", CV_WINDOW_AUTOSIZE );
//     imshow( "Hough Circle Transform Demo", src );
//     waitKey(0);
        return 1;
    }
    else
        return 0;
}
int Color_choose(Mat HSVImage,bool IsBlue)
{
    vector<Mat> HSVSplit;
    Mat element = getStructuringElement(MORPH_RECT, Size(5, 5));
    vector<vector<Point> > contours;
    Mat out;
    Mat HBlue;
    Mat HRed;
    Mat HRed_Light;
    Mat HRed_Dark;
    Mat SBoth;
    Rect r;
    Rect max_tmp;
    
    split(HSVImage,HSVSplit);
    if(IsBlue)
    {
        inRange(HSVSplit[0], Scalar(H_LOW_B), Scalar(H_HIGH_B), HBlue);
        //inRange(HSVSplit[0], Scalar(0), Scalar(30), HRed);
        //imshow("hgreen",HGreen);
        threshold(HSVSplit[1], SBoth, 80, 255, THRESH_BINARY);
        // imshow("sgreen",SGreen);
        //imshow("h",HSVSplit[0]);
        //imshow("v",HSVSplit[2]);
        cv::bitwise_and(HBlue, SBoth, out);
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
            cout<<"no blue contour..."<<endl;
            flag_B=1;
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
        {flag_B=2;
            return 0;}
        Mat pro;
        solid(max_tmp).copyTo(pro);
        //imshow("pro",pro);
        pro_after_B=pro.clone();
        //string ty =  type2str( pro_after.type() );
        //printf("Matrix: %s %dx%d \n", ty.c_str(), pro_after.cols, pro_after.rows );
        //rectangle(result, max_tmp, Scalar(255), 2);
        flag_B=3;
        cout<<"blue area "<<max_tmp.area()<<endl;
        return max_tmp.area();
    }
    else
    {
        inRange(HSVSplit[0], Scalar(H_LOW_R_Light), Scalar(H_HIGH_R_Light), HRed_Light);
	inRange(HSVSplit[0], Scalar(H_LOW_R_Dark), Scalar(H_HIGH_R_Dark), HRed_Dark);
	cv::bitwise_or(HRed_Light, HRed_Dark, HRed);
        //inRange(HSVSplit[0], Scalar(0), Scalar(30), HRed);
        //imshow("hgreen",HGreen);
        threshold(HSVSplit[1], SBoth, 80, 255, THRESH_BINARY);
        // imshow("sgreen",SGreen);
        //imshow("h",HSVSplit[0]);
        //imshow("v",HSVSplit[2]);
        cv::bitwise_and(HRed, SBoth, out);
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
            cout<<"no red contour..."<<endl;
            flag_R=1;
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
        {flag_R=2;
            return 0;}
        Mat pro;
        solid(max_tmp).copyTo(pro);
        //imshow("pro",pro);
        pro_after_R=pro.clone();
        //string ty =  type2str( pro_after.type() );
        //printf("Matrix: %s %dx%d \n", ty.c_str(), pro_after.cols, pro_after.rows );
        //rectangle(result, max_tmp, Scalar(255), 2);
        flag_R=3;
        cout<<"red area "<<max_tmp.area()<<endl;
        return max_tmp.area();
    }
}




























