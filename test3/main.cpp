#include "opencv2/highgui/highgui.hpp"
#include "opencv2/imgproc/imgproc.hpp"
#include <iostream>
#include <stdio.h>

using namespace cv;
using namespace std;

/** @function main */
int main(int argc, char** argv)
{
  Mat src, src_gray;

  /// Read the image
  src = imread( argv[1], 1 );
  //resize(src,src,Size(src.cols/5,src.rows/5));
  if( !src.data )
    { return -1; }

  /// Convert it to gray
  cvtColor( src, src_gray, CV_BGR2GRAY );

  /// Reduce the noise so we avoid false circle detection
  medianBlur(src_gray, src_gray, 5);
  //GaussianBlur( src_gray, src_gray, Size(9, 9), 2, 2 );
  imshow("gaussian",src_gray);
  vector<Vec3f> circles;

  /// Apply the Hough Transform to find the circles
  //HoughCircles( src_gray, circles, CV_HOUGH_GRADIENT, 1, src_gray.rows/4, 200, 100, 0, 0 );
  HoughCircles(src_gray, circles, CV_HOUGH_GRADIENT, 1, 10,
                 100, 30, 15, 30 // change the last two parameters
                                // (min_radius & max_radius) to detect larger circles
                 );
  cout<<"circles num: "<<circles.size()<<endl;
  /// Draw the circles detected
  for( size_t i = 0; i < circles.size(); i++ )
  {
      Point center(cvRound(circles[i][0]), cvRound(circles[i][1]));
      int radius = cvRound(circles[i][2]);
      cout<<"radius: "<<radius<<endl;
      // circle center
      circle( src, center, 3, Scalar(0,255,0), -1, 8, 0 );
      // circle outline
      circle( src, center, radius, Scalar(0,0,255), 3, 8, 0 );
   }

  /// Show your results
  namedWindow( "Hough Circle Transform Demo", CV_WINDOW_AUTOSIZE );
  imshow( "Hough Circle Transform Demo", src );

  waitKey(0);
  return 0;
}
