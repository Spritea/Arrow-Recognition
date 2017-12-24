#include "opencv2/highgui/highgui.hpp"
#include "opencv2/imgproc/imgproc.hpp"
#include <iostream>
#include <stdio.h>
using namespace cv;
using namespace std;
const int ARROW_AREA_MIN=3000;//variable
const int ARROW_AREA_MAX=600*440;
int flag=0;
Mat pro_after;

string type2str(int type);
int Color_detect(Mat frame);
int Arrow_direction(Mat pro_after);
int Circle_detect(Mat src);
/** @function main */
int main(int argc, char** argv)
{
    int direction=0;//1,2,3,4=arrow,5=circle,6=robo.
    int Iscircle;
    int Isarrow;
    Mat src;
    /// Read the image
    src = imread( argv[1], 1 );
    resize(src,src,Size(src.cols/5,src.rows/5));
    Iscircle=Circle_detect(src);
    if(Iscircle==-1)
    {
        cout<<"image read no data in circle detect..."<<endl;
        return -1;
    }
    if(Iscircle==1)
        direction=5;
    else
    {
        Isarrow=Color_detect(src);
        if(Isarrow==1)
        {
            direction=Arrow_direction(pro_after);
            if(direction==-1)
            {
                cout<<"the arrow position wrong..."<<endl;
                return -1;
            }
        }
        else
            direction=6;
    }
    cout<<"direct: "<<direction<<endl;
    return 0;

}
string type2str(int type) 
{
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
    else
        return -1;
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
    inRange(HSVSplit[0], Scalar(100), Scalar(140), HGreen);
    //imshow("hgreen",HGreen);
    threshold(HSVSplit[1], SGreen, 80, 255, THRESH_BINARY);
    // imshow("sgreen",SGreen);
    //imshow("h",HSVSplit[0]);
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
    //imshow("pro",pro);
    pro_after=pro.clone();
    //string ty =  type2str( pro_after.type() );
    //printf("Matrix: %s %dx%d \n", ty.c_str(), pro_after.cols, pro_after.rows );
    rectangle(result, max_tmp, Scalar(255), 2);
    cout<<"area "<<max_tmp.area()<<endl;
    return 1;
    //imshow("result",result);
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
        /// Draw the circles detected
        for( size_t i = 0; i < circles.size(); i++ )
        {
            Point center(cvRound(circles[i][0]), cvRound(circles[i][1]));
            int radius = cvRound(circles[i][2]);
            // circle center
            circle( src, center, 3, Scalar(0,255,0), -1, 8, 0 );
            // circle outline
            circle( src, center, radius, Scalar(0,0,255), 3, 8, 0 );
        }
        /// Show your results
//     namedWindow( "Hough Circle Transform Demo", CV_WINDOW_AUTOSIZE );
//     imshow( "Hough Circle Transform Demo", src );
//     waitKey(0);
        return 1;
    }
    else
        return 0;
}





























