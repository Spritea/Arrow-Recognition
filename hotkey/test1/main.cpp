#include <opencv2/opencv.hpp>
#include <iostream>

using namespace cv;
using namespace std;

int main( )
{
    Mat frame;
    VideoCapture cap(0);
    if(!cap.isOpened())
  {
    cout<<"cap open failed..."<<endl;
    return -1;
  }
  cap>>frame;
  cap>>frame;
  cap>>frame;
  
  cap>>frame;
  imwrite("t1.jpg",frame);
    return 0;
}
