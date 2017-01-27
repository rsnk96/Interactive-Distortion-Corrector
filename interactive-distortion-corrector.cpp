#include <iostream>
#include <sstream>
#include <cmath>
#include <string>

#include <opencv2/core.hpp>
#include <opencv2/core/utility.hpp>
#include <opencv2/imgproc.hpp>
#include <opencv2/calib3d.hpp>
#include <opencv2/imgcodecs.hpp>
#include <opencv2/videoio.hpp>
#include <opencv2/highgui.hpp>


using namespace cv;
using namespace std;



int blackBorderFactor = 25;
int k1 = 2466, k2, k3;          // k1=2520 is optimal if no resizing via Mat temp
Mat image, opImage;
Mat map_x, map_y;

void trackbarFunc(int, void*);

int framesToSkip=50;

void correctDistortion(int, void*);
void correctDistortion2(int, void*);
void changeCurrentImage(int, void*);
Point2f returnSourceCoord(Size srcSize, Size dstSize, int dx, int dy, float factor);
Point2f returnSourceCoord2(Size srcSize, Size dstSize, int dx, int dy, float factor);
float rx, ry;

string fileName;
int noframes();

int main(int argc, char* argv[])
{
    map_x.create( image.size(), CV_32FC1 );
    map_y.create( image.size(), CV_32FC1 );

    fileName=argv[1];

    namedWindow("Corrected", CV_WINDOW_NORMAL);

    createTrackbar( "Factor", "Corrected", &k1, 10000, correctDistortion);
    namedWindow("Input", CV_WINDOW_NORMAL);
    createTrackbar("framesToSkip", "Input", &framesToSkip, noframes(), changeCurrentImage);
    createTrackbar("Black Border Factor", "Input", &blackBorderFactor, 30, changeCurrentImage);

    while(1)
        if((char)waitKey(0)=='q')
            break;

    return 0;
}


int noframes()
{
    VideoCapture cap(fileName);
    return cap.get(CV_CAP_PROP_FRAME_COUNT);
}

void changeCurrentImage(int, void*)
{
    VideoCapture cap(fileName);
    for (int i = 0; i < framesToSkip; ++i)
    {
        cap.read(image);
    }
    cap.read(image);
    int newWidth = 1.0* blackBorderFactor/10 * image.cols, newHeight = 1.0*blackBorderFactor/10 * image.rows;
    Mat temp (newHeight, newWidth, CV_8UC3, Scalar(0,0,0));

    image.copyTo(temp(Rect((newWidth - image.cols)/2, (newHeight - image.rows)/2, image.cols, image.rows)));
    image = temp.clone();
    imshow("Input", image);
}


void correctDistortion(int, void*)
{
    float factor = (float)k1 / 1000;
    opImage = Mat::zeros(image.size(), image.type());
    for(int i=0; i<image.rows; i++)
    {
        for (int j=0; j<image.cols; j++)
        {
            Point2f srcCoord = returnSourceCoord (image.size(), image.size(), j, i, factor);
            opImage.at<Vec3b>(i,j) = image.at<Vec3b>(srcCoord.y,srcCoord.x);
        }
    }

    imshow("Corrected", opImage);
    imwrite("1Corrected.jpg", opImage);
}


Point2f returnSourceCoord(Size srcSize, Size dstSize, int dx, int dy, float factor)
{
    float rx = (float)dx-(dstSize.width/2);
    float ry = (float)dy-(dstSize.height/2);
    double r = norm(Point2f(rx,ry)) / norm(Point2f(srcSize.width, srcSize.height)) * factor * blackBorderFactor/10, theta;      //Calculate theta is scaling factor
    if (0==r)
        theta = 1.0;
    else
        theta = atan(r)/r;

    // back to absolute coordinates
    float sx = (srcSize.width/2)+theta*rx;
    float sy = (srcSize.height/2)+theta*ry;
    return Point2f((int(sx)),(int(sy)));
}


void correctDistortion2(int, void*)
{
    opImage = Mat::zeros(image.size(), image.type());

    for(int i=0; i<opImage.rows; i++)
    {
        for (int j=0; j<opImage.cols; j++)
        {
        // Point2f srcCoord = returnSourceCoord (image.size(), image.size(), j, i, factor);
        // opImage.at<Vec3b>(i,j) = image.at<Vec3b>(srcCoord.y,srcCoord.x);

        // map_x.at<float>(j,i) = 2*( i - src.cols*0.25 ) + 0.5 ;
        // map_y.at<float>(j,i) = 2*( j - src.rows*0.25 ) + 0.5 ;
        }
    }

    remap( image, opImage, map_x, map_y, CV_INTER_LINEAR, BORDER_CONSTANT, Scalar(0,0, 0) );

    imshow("Corrected", opImage);
    imwrite("1Corrected.jpg", opImage);
}


Point2f returnSourceCoord2(Size srcSize, Size dstSize, int dx, int dy, float factor)
{
    float rx = (float)dx-(dstSize.width/2);
    float ry = (float)dy-(dstSize.height/2);
    double r = norm(Point2f(rx,ry));

    // back to absolute coordinates
    float sx = rx / (1 + factor * r*r);
    float sy = ry / (1 + factor * r*r);
    return Point2f((int(sx)),(int(sy)));
}