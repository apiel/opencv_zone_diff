#include <curl/curl.h>
#include <vector>

#include <opencv2/opencv.hpp>
#include <iostream>
#include <stdio.h>

using namespace cv;
using namespace std;

#define CAM 0
// #define CAM 1
#define IPCAM "http://192.168.0.108/snapshot.cgi?user=admin&pwd=admin"

#ifdef IPCAM
size_t write_data(char *ptr, size_t size, size_t nmemb, void *userdata)
{
    vector<uchar> *stream = (vector<uchar>*)userdata;
    size_t count = size * nmemb;
    stream->insert(stream->end(), ptr, ptr + count);
    return count;
}

cv::Mat curlImg(const char *img_url, int timeout=10)
{
    vector<uchar> stream;
    CURL *curl = curl_easy_init();
    curl_easy_setopt(curl, CURLOPT_URL, img_url); //the img url
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, write_data); // pass the writefunction
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, &stream); // pass the stream ptr to the writefunction
    curl_easy_setopt(curl, CURLOPT_TIMEOUT, timeout); // timeout if curl_easy hangs, 
    CURLcode res = curl_easy_perform(curl); // start curl
    curl_easy_cleanup(curl); // cleanup
    return imdecode(stream, -1); // 'keep-as-is'
}
#endif

void detectLight(Mat lightFrame)
{
    cvtColor(lightFrame, lightFrame, CV_BGR2GRAY);
    GaussianBlur(lightFrame, lightFrame, Size(11, 11), 0);
    threshold(lightFrame, lightFrame, 220, 255, THRESH_BINARY);    
    imshow("Light", lightFrame);
}

void detectBrightness(Mat lightFrame)
{
    cvtColor(lightFrame, lightFrame, CV_BGR2GRAY);   
    cout << "Brightness by sum intensity: " << mean(lightFrame)[0] << endl;
}

int main(int, char**)
{
    Mat frame;
    Mat prevFrame;
    Mat motionFrame;
    Mat blockFrame;
    Mat resultFrame;

    int val, x, y;

    #ifdef IPCAM
    frame = curlImg(IPCAM);
    #else
    VideoCapture cap;
    // open the default camera using default API
    cap.open(CAM);
    if (!cap.isOpened()) {
        cerr << "ERROR! Unable to open camera\n";
        return -1;
    }
    cap.read(frame);
    #endif   
    
    if (frame.empty()) {
        cerr << "ERROR! blank frame grabbed\n";
        return -1;
    }    

    int w = 8;
    int width = frame.cols / 8;
    int h = 6;
    int height = frame.rows / 6;

    int marginLeft = width / 2 - 3;
    int marginTop = height / 2 - 3;

    cout << "size: " << width << " - " << height << endl;

    //--- GRAB AND WRITE LOOP
    cout << "Start grabbing" << endl
        << "Press any key to terminate" << endl;
    for (;;)
    {
        #ifdef IPCAM
        frame = curlImg(IPCAM);
        #else
        cap.read(frame);
        #endif
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

            // imshow("Live", motionFrame);
            motionFrame.copyTo(resultFrame);
            cvtColor(resultFrame, resultFrame, CV_GRAY2BGR);

            val = countNonZero(motionFrame);

            if (val > 10) {
                cout << "Frame val: " << val << endl;
                
                for (x = 0; x < w; x++) {
                    for (y = 0; y < h; y++) {
                        blockFrame = Mat(motionFrame, Rect(x*width, y*height, width, height));
                        val = countNonZero(blockFrame);
                        if (val > 2) {
                            cout << "Block val (" << x <<","<< y <<"): " << val << endl;
                            putText(resultFrame, to_string(val).c_str(), cvPoint(x*width+marginLeft, y*height+marginTop), 
                                FONT_HERSHEY_COMPLEX_SMALL, 0.8, cvScalar(255,0,0), 1, CV_AA);
                        }                        
                        // imshow("Block", blockFrame);
                    }
                }
            }
            imshow("Result", resultFrame);
        }
        frame.copyTo(prevFrame);

        // detectLight(frame);
        detectBrightness(frame);

        imshow("Frame", frame);

        // show live and wait for a key with timeout long enough to show images

        if (waitKey(5) >= 0)
            break;
    }
    // the camera will be deinitialized automatically in VideoCapture destructor
    return 0;
}