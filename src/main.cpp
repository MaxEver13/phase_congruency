/*
 * @Descripttion: 
 * @version: 
 * @Author: Jiawen Ji
 * @Date: 2021-12-22 10:52:25
 * @LastEditors: Jiawen Ji
 * @LastEditTime: 2021-12-23 18:04:49
 */
#include "phase.h"

#include <opencv2/imgproc.hpp>
#include <opencv2/highgui/highgui.hpp>
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
       
        auto size = image.size() * 3;
        resize(image, image, size);

        // 转换成rgb
        Mat color;
        cvtColor(image, color, COLOR_GRAY2RGB);
        
        PhaseCongruency pc(image.size(), 4, 6);

        Mat edges, corners;
        pc.feature(image, edges, corners);

        int lable[corners.rows][corners.cols];

        // 筛选一下，避免点太密集
        for (size_t i = 0; i < corners.rows; i++)
        {
           for (size_t j = 0; j < corners.cols; j++)
           {
               if (127 < corners.at<uchar>(i, j) && lable[i][j] == 0)
               {
                   lable[i][j] = 1;
                   // 周围的点涂黑
                   for (size_t m = 1; m < 15; m++)
                   {
                       for (size_t n = 1; n < 15; n++)
                       {
                           if (corners.at<uchar>(i+m, j+n) > 127 && lable[i+m][j+n] == 0)
                           {
                               lable[i+m][j+n] = 1;
                               corners.at<uchar>(i+m, j+n) = 0;
                           }
                                
                       }
                       
                   }
               }
           }
           
        }

        // 遍历找出大于127的点
        for (size_t i = 0; i < corners.rows; i++)
        {
           for (size_t j = 0; j < corners.cols; j++)
           {
               if (127 < corners.at<uchar>(i, j))
               {
                   circle(color, Point(j, i), 1, CV_RGB(0, 255, 0), 2);
               }
           }
           
        }

        imwrite("corners.png", color);
        

        if (parser.has(outFileKey))
        {
            const string outputFileName = parser.get<String>(outFileKey);
            imwrite(outputFileName, corners);
        }
        else
        {
            namedWindow("image");
            imshow("image", corners);
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

