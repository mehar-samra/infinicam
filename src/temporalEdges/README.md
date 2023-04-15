# temporalEdges


<hr>

temporalEdges is a Windows sample application for USB high-speed streaming camera [INFINICAM UC-1](https://www.photron.co.jp/products/hsvcam/infinicam/).

It illustrates how the INFINICAM could be used to achieve real-time live image processing with simple capturing API that mimics OpenCV's Video Capture class [VideoCapture](https://docs.opencv.org/3.4/d8/dfe/classcv_1_1VideoCapture.html).

The code sample shows how to integrate INFINICAM into an application that uses OpenCV. The class allows you to vectorize the images from the high speed camera and send vector data across the Internet. The hotkeys/camera modes are as follows: 1 - Live Cam; 2 - Edge Detect; 3 - Contour Detect; 4 - Contour + Live Cam; 's' - (un)freeze UI; 'x' - Export SVG as HTML.


## Environment
* installed Visual Studio 2019

    :warning: MFC Package is required.

* OpenCV Version 4.2.0 or higher (included in the Photron GitHub repository)

## Build
1. Download and install [PUCLIB](https://www.photron.co.jp/products/hsvcam/infinicam/tech.html) SDK.

2. Clone this source code.

3. Set the environment variable OPENCV_DIR and set to the directory you installed OpenCV

4. Copy the OpenCV DLL's to the bin folder
   
5. Open [temporalEdges.sln](./temporalEdges.sln) on visual studio.

6. Build

------------

## Operation

1. Connect INIFINICAM UC-1 to your Windows PC with USB-C cable.
2. Launch temporalEdges.exe in the bin folder.
3. The application will show the live output of the camera in a window.
4. Hotkeys for different camera mdoes are given upon program initialization. 
5. To exit, hit ESC key after focusing to the live window.


#### developed by: Photron Ltd.
