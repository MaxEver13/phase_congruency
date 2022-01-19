/*
 * @Descripttion: 
 * @version: 
 * @Author: Jiawen Ji
 * @Date: 2021-12-22 10:52:25
 * @LastEditors: Jiawen Ji
 * @LastEditTime: 2022-01-19 20:47:30
 */
#include "phase.h"
#include "file_dir.h"
#include "mapbeautif.h"
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

static cv::Mat ContoursRemoveNoise(cv::Mat img, double pArea)
{
    cv::Mat res = img.clone();
	int color = 1;
	int nHeight = res.cols;
	int nWidth = res.rows;

	for (int i = 0; i < nWidth; ++i)
		for (int j = 0; j < nHeight; ++j)
		{
            // 黑点泛水填充
			if ( !res.at<uchar>(i, j) )
			{
				//FloodFill each point in connect area using different color
                // point是(_x,_y)形式 所以注意反写行列
				floodFill(res, Point(j,i), Scalar(color));
				color++;
			}
		}

	int ColorCount[255] = { 0 };
	for (int i = 0; i < nWidth; ++i)
	{
		for (int j = 0; j < nHeight; ++j)
		{
			//caculate the area of each area
			if (res.at<uchar>(i, j) != 255 && res.at<uchar>(i, j) != 127)
			{
				ColorCount[res.at<uchar>(i, j)]++;
			}
		}
	}
	//get rid of noise point
	for (int i = 0; i < nWidth; ++i)
	{
		for (int j = 0; j < nHeight; ++j)
		{
			if (ColorCount[res.at<uchar>(i, j)] <= pArea)
			{
				res.at<uchar>(i, j) = 255;
			}
		}
	}

    return res;
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
        Mat img_src = image.clone();
        cv::imwrite("src.png", image);

        // 读取文件
        // char outBuf[1024*1024];
        // ReadStrFromFile("/home/max/Projects/PhaseCongruency/example/src.hex", outBuf, sizeof(outBuf));

        // const int height = 165;
        // const int width = 186;
        // cv::Mat image(height, width, CV_8UC1);
        // std::memcpy(image.data, outBuf, height*width);
        // Mat img_src = image.clone();

        // 地图美化
        uchar* data = image.data;
        int w       = image.cols;
        int h       = image.rows;

        MapBeauti::MapBeauti::Beautifi(data, h, w);

        cv::imwrite("beauti.png", image);

        // 噪点去除
        cv::Mat clean = ContoursRemoveNoise(image, 10);
        cv::imwrite("clean.png", clean);
       
        // step1: resize
        auto size = clean.size() * 3;
        resize(clean, clean, size, 0.0, 0.0, cv::INTER_CUBIC);

        // 转换成rgb用于绘制角点
        Mat color;
        cvtColor(img_src, color, COLOR_GRAY2RGB);

        // step2: 构造
        PhaseCongruency pc(clean.size(), 4, 6);        

        // step3: 检测角点
        vector<Corner> corners;
        pc.detectCorners(clean, corners, 127, 10);

        // step4: 将角点画出来
        // 坐标还原
        double fx = (double)img_src.cols/size.width;
        double fy = (double)img_src.rows/size.height;
        for (auto it = corners.begin(); it != corners.end(); it++)
        {
            std::cout << (*it).y * fy << " " << (*it).x * fx << std::endl;
            circle(color, Point((*it).y * fy, (*it).x * fx), 0.1, CV_RGB(0, 255, 0), 2);
        }
                        
        std::cout << corners.size()  << std::endl;
        imwrite("corners.png", color);   
        

        // if (parser.has(outFileKey))
        // {
        //     const string outputFileName = parser.get<String>(outFileKey);
        //     imwrite(outputFileName, corners);
        // }

    }
    catch (Exception& e)
    {
        const char* err_msg = e.what();
        std::cout << "Exception caught: " << err_msg << std::endl;
    }
    waitKey();

    return 0;
}

