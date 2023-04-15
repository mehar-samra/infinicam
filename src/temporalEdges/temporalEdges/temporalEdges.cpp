// temporalEdges.cpp : This file contains the 'main' function. Program execution begins and ends there.
//

#include <Windows.h>
#include <opencv2/core.hpp>
#include <opencv2/highgui.hpp>
#include <opencv2/imgproc/imgproc.hpp>

#include <fstream>
#include <string>
#include <iostream>
#include <stdio.h>
using namespace cv;
using namespace std;

#include "PhotronVideoCapture.h"

#define ENABLE_WEBSOCKET

#ifdef ENABLE_WEBSOCKET
#include "server/echoserver.h"
#include <QtCore/QCoreApplication>
#include <QtCore/QProcess>
#endif


#define USE_WEBCAMERA 1

enum {
    MODE_CAMERA,
    MODE_EDGE,
    MODE_EDGE_ON_BLACK,
    MODE_EDGE_CAMERA
};

int mode = MODE_EDGE_CAMERA;

void addHotkeyText(Mat frame) {
    cv::putText(frame,
        "1 - Live Cam; 2 - Edge Detect; 3 - Contour Detect",
        Point(25, 25),
        cv::FONT_HERSHEY_SIMPLEX, 0.5,
        (0, 255, 255),
        2, cv::LINE_4);
    cv::putText(frame,
        "4 - Contour + Live Cam; 's' - (un)freeze UI; 'x' - Export SVG as HTML",
        Point(25, 50),
        cv::FONT_HERSHEY_SIMPLEX, 0.5,
        (0, 255, 255),
        2, cv::LINE_4);
}


int main(int argc, char** argv)
{
#ifdef ENABLE_WEBSOCKET
    QCoreApplication a(argc, argv);
    int port = 1234;
    EchoServer* server = new EchoServer(port, false);
    QProcess process;
    QStringList args;
    args << "server/echoclient.py";
    process.start("python", args);
#endif

    bool noUi = false;
    if (argc>1 && std::string(argv[1]) == std::string("-noui"))
        noUi = true;

    Mat frame;
    //--- INITIALIZE VIDEOCAPTURE
#ifdef USE_WEBCAMERA
    cv::VideoCapture cap(0, CAP_MSMF);
#else
    photron::VideoCapture cap;
    // open the default camera using default API
    // cap.open(0);
    // OR advance usage: select any API backend
    int deviceID = 0;             // 0 = open default camera
    int apiID = cv::CAP_ANY;      // 0 = autodetect default API
    // open selected camera using selected API
    cap.open(deviceID, apiID);
#endif

    // check if we succeeded
    if (!cap.isOpened()) {
        cerr << "ERROR! Unable to open camera\n";
        return -1;
    }
    //--- GRAB AND WRITE LOOP
    cout << "Start grabbing" << endl
        << "Press any key to terminate" << endl;

    int prevmsec = 0;
    SYSTEMTIME st;
    bool showDc = true;

#ifndef USE_WEBCAMERA
    cap.setFrameSampleRate(40,1);
#endif

    int brighntessValue = 100;
    if (!noUi)
    {
        namedWindow("Temporal", 1);
        createTrackbar("Brightness", "Temporal", &brighntessValue, 400);
    }
    
    int counter = 0;
    bool exportSvg = false;
    Mat prevFullFrame;

    std::stringstream svgStream;

    for (;;)
    {
        GetSystemTime(&st);

        int dur;
        if (prevmsec > st.wMilliseconds) {
            dur = st.wMilliseconds + 1000 - prevmsec;
        }
        else {
            dur = st.wMilliseconds - prevmsec;
        }

        Mat frame;
#ifndef USE_WEBCAMERA
        cap.readProxy(frame);   
#endif
        //printf("%d\n", dur);
        prevmsec = st.wMilliseconds;


        if (!frame.empty() && !noUi)
            imshow("FastCam DC", frame);
        
        Mat fullFrame;
        cap.read(fullFrame);
#ifdef USE_WEBCAMERA
        cvtColor(fullFrame, fullFrame, COLOR_BGR2GRAY);
#endif

        Mat processedFrame;




        if (!prevFullFrame.empty()) {

            if (mode == MODE_CAMERA) {
                imshow("Temporal", fullFrame);
            }
            else {
                absdiff(prevFullFrame, fullFrame, processedFrame);
                processedFrame.convertTo(processedFrame, -1, brighntessValue / 100.0f, 0);
                int thresh = 100;
                Canny(processedFrame, processedFrame, thresh, thresh * 2);

                if (mode == MODE_EDGE)
                    imshow("Temporal", processedFrame);
                else {
                    Mat output;
                    if (mode == MODE_EDGE_CAMERA) {
                        cvtColor(fullFrame, output, COLOR_GRAY2RGB);
                    }
                    else {
                        output = Mat::zeros(processedFrame.size(), CV_8UC3);
                    }
                    vector<vector<Point> > contours;
                    vector<Vec4i> hierarchy;
                    findContours(processedFrame, contours, hierarchy, RETR_TREE, CHAIN_APPROX_SIMPLE);

                    if (exportSvg) {
                        svgStream << "<!DOCTYPE html>" << std::endl;
                        svgStream << "<html>" << std::endl;
                        svgStream << "<body>" << std::endl;
                        svgStream << "<svg height = \"" << output.size().height << "\" width = \"" << output.size().width  << "\">" << std::endl;
                    }
                    for (size_t i = 0; i < contours.size(); i++)
                    {
                        Scalar color = Scalar(0, 255, 0);
                        drawContours(output, contours, (int)i, color, 2, LINE_8, hierarchy, 0);


                        if (exportSvg 
#ifdef ENABLE_WEBSOCKET
                            || server != nullptr
#endif
                            ) {
                            // Add Polyline!
                            vector<Point>& points = contours[i];
                            svgStream << "<polyline points=\"";
                            for (size_t j = 0; j < points.size(); j++) {
                                float x = points[j].x;
                                float y = points[j].y;
                                svgStream << x << "," << y << " ";
                            }
                            svgStream <<"\" style=\"fill:none; stroke:green; stroke-width:3\" />" << std::endl;
                        }
                    }

                    if (exportSvg) {
                        svgStream << "Sorry, your browser does not support inline SVG." << std::endl;
                        svgStream << "</svg>" << std::endl;
                        svgStream << "</body>" << std::endl;
                        svgStream << "</html>" << std::endl;
                        std::ofstream out("index.html");
                        out << svgStream.str();
                        out.close();
                        svgStream.str("");                    
                        exportSvg = false; // Dont save again
                    }

#ifdef ENABLE_WEBSOCKET
                    if (server != nullptr) {
                        server->processTextMessage(svgStream.str().c_str());
                        svgStream.str("");
                    }
                    
#endif
                    if (!noUi) {
                        if (counter < 300) {
                            addHotkeyText(output);
                        }
                        imshow("Temporal", output);
                    }
                }
            }
        }
        

        int key = waitKey(1);
        if (key == 27)
            break;
        else if (key == 49)
            mode = MODE_CAMERA;
        else if (key == 50)
            mode = MODE_EDGE;
        else if (key == 51)
            mode = MODE_EDGE_ON_BLACK;
        else if (key == 52)
            mode = MODE_EDGE_CAMERA;
        else if (key == 's') {
            noUi = !noUi;

            cv::putText(processedFrame,
                "Frozen. Press 's' to unfreeze.", 
                Point(50,50),
                cv::FONT_HERSHEY_SIMPLEX, 1,
                (0, 255, 255),
                2,
                cv::LINE_4);

            cv::imshow("Temporal", processedFrame);

        }
        else if (key == 'x')
            exportSvg = true;

        ++counter;
        // copy current to previous
        prevFullFrame = fullFrame.clone();
    }
    // the camera will be deinitialized automatically in VideoCapture destructor
    return 0;
}