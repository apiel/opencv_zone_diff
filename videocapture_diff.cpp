#include <opencv2/opencv.hpp>
#include <iostream>
#include <stdio.h>

using namespace cv;
using namespace std;

int main(int, char**)
{
    Mat frame;
    Mat prevFrame;
    Mat motionFrame;

    //--- INITIALIZE VIDEOCAPTURE
    VideoCapture cap;
    // open the default camera using default API
    cap.open(0);
    // OR advance usage: select any API backend
    int deviceID = 0;             // 0 = open default camera
    int apiID = cv::CAP_ANY;      // 0 = autodetect default API
    // open selected camera using selected API
    cap.open(deviceID + apiID);
    // check if we succeeded
    if (!cap.isOpened()) {
        cerr << "ERROR! Unable to open camera\n";
        return -1;
    }

    //--- GRAB AND WRITE LOOP
    cout << "Start grabbing" << endl
        << "Press any key to terminate" << endl;
    for (;;)
    {
        // wait for a new frame from camera and store it into 'frame'
        cap.read(frame);
        // check if we succeeded
        if (frame.empty()) {
            cerr << "ERROR! blank frame grabbed\n";
            break;
        }
        if (!prevFrame.empty()) {
            // cout << "Frame cmp" << endl;
            absdiff(frame, prevFrame, motionFrame);

            cvtColor(motionFrame, motionFrame, CV_BGR2GRAY);
            threshold(motionFrame, motionFrame, 10, 255, THRESH_BINARY);
            // reduce noise
            erode(motionFrame, motionFrame, getStructuringElement(MORPH_RECT, Size(4,4)));

            imshow("Live", motionFrame);

            int res = countNonZero(motionFrame);
            cout << "Frame cmp: " << res << endl;
        }
        // prevFrame = frame;
        frame.copyTo(prevFrame);
        // show live and wait for a key with timeout long enough to show images

        if (waitKey(5) >= 0)
            break;
    }
    // the camera will be deinitialized automatically in VideoCapture destructor
    return 0;
}