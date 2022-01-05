/*
 * @Descripttion: 
 * @version: 
 * @Author: Jiawen Ji
 * @Date: 2021-12-22 10:52:25
 * @LastEditors: Jiawen Ji
 * @LastEditTime: 2022-01-05 10:41:06
 */
#include "phase.h"
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
        cvtColor(image, color, COLOR_GRAY2RGB);

        // step2: 构造
        PhaseCongruency pc(image.size(), 4, 6);        

        // step3: 检测角点
        vector<Corner> corners;
        pc.detectCorners(image, corners);    

        // step4: 将角点画出来
        for (auto it = corners.begin(); it != corners.end(); it++)
        {
            if ((*it).score > 127)
                circle(color, Point((*it).y, (*it).x), 1, CV_RGB(0, 255, 0), 2);
        }        

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

