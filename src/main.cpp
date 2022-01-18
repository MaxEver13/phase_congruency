/*
 * @Descripttion: 
 * @version: 
 * @Author: Jiawen Ji
 * @Date: 2021-12-22 10:52:25
 * @LastEditors: Jiawen Ji
 * @LastEditTime: 2022-01-18 14:33:39
 */
#include "phase.h"
#include "file_dir.h"
#include <iostream>



using namespace cv;
using namespace std;


static void help()
{
    cout << "\nThis program seek for letter bounding box on image\n"
        << "Call:\n"
        << "/.edge input_image_name [output_image_name]"
        << endl;
}

int main(int argc, char** argv)
{
    try
    {
        const String inFileKey = "@inputImage";
        const String outFileKey = "@outputImage";
        const String keys =
            "{help h usage ?    |      | print this message }"
            "{" + inFileKey + " |<none>| input image        }"
            "{" + outFileKey + "|<none>| output image       }";
        CommandLineParser parser(argc, argv, keys);
        if (parser.has("help") || !parser.has(inFileKey))
        {
            help();
            return 0;
        }
        const string inputFileName = parser.get<String>(inFileKey);

        Mat image = imread(inputFileName, IMREAD_GRAYSCALE);
        Mat img_src = image.clone();

        // 读取文件
        // char outBuf[1024*1024];
        // ReadStrFromFile("/home/max/Projects/PhaseCongruency/example/test2.hex", outBuf, sizeof(outBuf));

        // const int height = 165;
        // const int width = 186;
        // cv::Mat image(height, width, CV_8UC1);
        // std::memcpy(image.data, outBuf, height*width);

        if (image.empty())
        {
            cout << "Cannot read image file " << inputFileName << endl;
            help();
            return -1;
        }
       
        // step1: resize
        auto size = image.size() * 3;
        resize(image, image, size);

        // 转换成rgb用于绘制角点
        Mat color;
        cvtColor(img_src, color, COLOR_GRAY2RGB);

        // step2: 构造
        PhaseCongruency pc(image.size(), 4, 6);        

        // step3: 检测角点
        vector<Corner> corners;
        pc.detectCorners(image, corners);

        // step4: 将角点画出来
        // 坐标还原
        double fx = (double)img_src.cols/size.width;
        double fy = (double)img_src.rows/size.height;
        for (auto it = corners.begin(); it != corners.end(); it++)
        {
            circle(color, Point((*it).y * fy, (*it).x * fx), 0.1, CV_RGB(0, 255, 0), 2);
        }
                
        std::cout << corners.size()  << std::endl;
        imwrite("corners.png", color);   
        

        if (parser.has(outFileKey))
        {
            const string outputFileName = parser.get<String>(outFileKey);
            imwrite(outputFileName, corners);
        }

    }
    catch (Exception& e)
    {
        const char* err_msg = e.what();
        std::cout << "Exception caught: " << err_msg << std::endl;
    }
    waitKey();

    return 0;
}

