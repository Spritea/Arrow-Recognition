#include <opencv2/opencv.hpp>
#include <iostream>

using namespace cv;
using namespace std;

int main( )
{
    Mat src, hsv;
/*    VideoCapture cap(0);
    if(!cap.isOpened())
  {
    cout<<"cap open failed..."<<endl;
    return -1;
  }
    while(cap.read(src))
*/
//{
    src=imread("/home/ubuntu/code/block/hotkey/test1/build/t10.jpg");
    //resize(src,src,Size(src.cols/5,src.rows/5));
  //  cap>>src;
    cvtColor(src, hsv, CV_BGR2HSV);

    // Quantize the hue to 30 levels
    // and the saturation to 32 levels
    int hbins = 30, sbins = 32;
    int histSize[] = {hbins};
    // hue varies from 0 to 179, see cvtColor
    float hranges[] = { 0, 180 };
    // saturation varies from 0 (black-gray-white) to
    // 255 (pure spectrum color)
    float sranges[] = { 0, 256 };
    const float* ranges[] = { hranges};
    MatND hist;
    // we compute the histogram from the 0-th and 1-st channels
    int channels[] = {0};

    calcHist( &hsv, 1, channels, Mat(), // do not use mask
             hist, 1, histSize, ranges,
             true, // the histogram is uniform
             false );
    double maxVal=0;
    minMaxLoc(hist, 0, &maxVal, 0, 0);

    int scale = 10;
    int size=hist.rows;
    Mat histImg(size,size,CV_8U,Scalar(255));
    // 设置最大峰值为图像高度的90%
    int hpt=static_cast<int>(0.9*size);
 
    for(int h=0;h<size;h++)
    {
        float binVal=hist.at<float>(h);
        int intensity=static_cast<int>(binVal*hpt/maxVal);
        line(histImg,Point(h,size),Point(h,size-intensity),Scalar::all(0));
    }
 

    namedWindow( "Source", 1 );
    imshow( "Source", src );

    namedWindow( "H Histogram", false);
    imshow( "H Histogram", histImg );
    waitKey(0);
//}
    return 0;
}
