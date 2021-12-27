/*
 * @Descripttion: 
 * @version: 
 * @Author: Jiawen Ji
 * @Date: 2021-12-22 10:52:25
 * @LastEditors: Jiawen Ji
 * @LastEditTime: 2021-12-27 21:15:06
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
       
        auto size = image.size() * 3;
        resize(image, image, size);

        // 转换成rgb
        Mat color;
        cvtColor(image, color, COLOR_GRAY2RGB);
        
        PhaseCongruency pc(image.size(), 4, 6);

        Mat edges, corners;
        pc.feature(image, edges, corners);

        // 将图片进行栅格化，每个栅格里面只选取最好的角点
        const int cell_size = 15;
        const int grid_n_cols = ceil(static_cast<double>(corners.cols)/cell_size);
        const int grid_n_rows = ceil(static_cast<double>(corners.rows)/cell_size);
        vector<bool> grid(grid_n_cols *grid_n_rows, false);

        const int threshold = 127;

        // 初值都为127
        vector<Corner> Corners(grid_n_cols *grid_n_rows, Corner(0, 0, threshold));

        // 遍历找出大于127的点
        vector<Corner> detect_corners;
        for (size_t i = 0; i < corners.rows; i++)
        {
           for (size_t j = 0; j < corners.cols; j++)
           {
               if (127 < corners.at<uchar>(i, j))
               {
                   detect_corners.push_back(Corner(i, j, (float)corners.at<uchar>(i, j)));
               }
           }           
        }
        
        // 每个栅格里面只保存一个角点
        for (auto it = detect_corners.begin(); it != detect_corners.end(); it++)
        {
            // 栅格id
            int index = static_cast<int>((*it).x/cell_size)*grid_n_cols + 
                        static_cast<int>((*it).y/cell_size);

            // 栅格已经有角点
            if (grid[index])
            {
                // 质量更好的角点，需要更新
                if ((*it).score > Corners.at(index).score)
                {
                    Corners.at(index) = Corner((*it).x, (*it).y, (*it).score);
                } else {
                    // 否则不更新
                    continue;
                }
            } 
                
            
            // 新加入一个角点
            if ((*it).score > threshold)
            {
                Corners.at(index) = Corner((*it).x, (*it).y, (*it).score);
                grid[index] = true;
            }
        } 

        // 将角点画出来
        for (auto it = Corners.begin(); it != Corners.end(); it++)
        {
            if ((*it).score > threshold)
                circle(color, Point((*it).y, (*it).x), 1, CV_RGB(0, 255, 0), 2);
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

