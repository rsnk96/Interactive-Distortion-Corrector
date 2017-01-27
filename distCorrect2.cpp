//Argument order: baseName, start, end, k1, k2

#include <iostream>
#include <sstream>
#include <cstdlib>
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

void humriProgressBar(float progress);

int main(int argc, char* argv[])
{
    string baseName = argv[1];
    string fileName = baseName + ".MP4";        //NOTE GOPRO videos are MP4...so... :P
    string OutputVideoName = baseName + "_processed.avi";
    Mat image, image2, opImage, map_x, map_y;
    VideoCapture cap(fileName);
    cap.read(image);
    int start = atoi(argv[2]);
    int end = atoi(argv[3]);
    float k1 = atof(argv[4])*(0.3*0.3), k2=0;
    if(argc==6)
        k2 = atof(argv[5])*(0.3*0.3);
    int myRows = cap.get(CV_CAP_PROP_FRAME_HEIGHT)/0.3, myCols = cap.get(CV_CAP_PROP_FRAME_WIDTH)/0.3;
    int centerPoint[]={myRows/2, myCols/2};
    int newHaHeight = ( float(myRows/2)*0.3 / (1+k1*(myRows*myRows/4+myCols*myCols/4)) );
    int newHaWidth  = ( float(myCols/2)*0.3 / (1+k1*(myRows*myRows/4+myCols*myCols/4)) );
    int oldX, oldY;
    float radiusSquared, factor;
    VideoWriter outVideo(OutputVideoName, CV_FOURCC('D','I','V','X'), int(cap.get(CV_CAP_PROP_FPS)), Size(2*newHaWidth, 2*newHaHeight), true);
    cap.set(CV_CAP_PROP_POS_FRAMES,start);
    int myRange=(end-start);
    cout<<"\nNow Trimming\n";
    for(int i =0; i<myRange; i++)
    {
        map_x.create( 2*newHaHeight, 2*newHaWidth, CV_32FC1 );
        map_y.create( 2*newHaHeight, 2*newHaWidth, CV_32FC1 );
        int ret = cap.read(image);
        if(ret==false)
            continue;
        if(cap.get(CV_CAP_PROP_POS_FRAMES) >= end+1)
            break;
        image2 = Mat(image.rows*1.0/0.3, image.cols*1.0/0.3, image.type());
        resize(image, image2, Size(), 1.0/0.3, 1.0/0.3, CV_INTER_LINEAR);
        image=image2;
        for(int i=0; i<image.rows; i++)
        {
            for(int j=0; j<image.cols; j++)
            {
                oldX = (j-centerPoint[1]);
                oldY = (i-centerPoint[0]);
                radiusSquared = oldX*oldX + oldY*oldY;
                if(argc==6)
                    factor = (1 + k1*radiusSquared + k2*radiusSquared*radiusSquared);
                else
                    factor = (1 + k1*radiusSquared);
                map_x.at<float>(( 0.3*oldY/factor)+newHaHeight, 0.3*oldX/factor+newHaWidth) = oldX+centerPoint[1];
                map_y.at<float>(( 0.3*oldY/factor)+newHaHeight, 0.3*oldX/factor+newHaWidth) = oldY+centerPoint[0];
            }
        }
        opImage.release();
        remap( image, opImage, map_x, map_y, CV_INTER_LINEAR, BORDER_CONSTANT, Scalar(0,0, 0) );
        outVideo<<(opImage);
        humriProgressBar(float(i+1)/myRange);
    }
    cout<<"\nDone Trimming\n";
    outVideo.release();
    return 0;
}
void humriProgressBar(float progress)
{
    string currStatus = "";
    string bar = "";
    int sizeOfProgressBar=20;
    if (progress >= 1)
        currStatus = "Done......\n";
    int numberOfEquals = (20*progress);
    for (int i = 0; i < sizeOfProgressBar; ++i)
    {
        if(i<numberOfEquals)
            bar+='=';
        else
            bar+=' ';
    }
    printf("\rPercent: [%s] %.2f %s",bar.c_str(), progress*100, (currStatus+"% corrected").c_str());
    fflush(stdout);
}