/*
 * @Descripttion: 
 * @version: 
 * @Author: Jiawen Ji
 * @Date: 2021-12-22 10:52:25
 * @LastEditors: Jiawen Ji
 * @LastEditTime: 2022-01-05 10:39:33
 */
#pragma once
#include <vector>
#include <opencv2/core/types.hpp>
#include <opencv2/imgproc.hpp>
#include <opencv2/highgui/highgui.hpp>


struct Corner
{
  int x;        //!< x-coordinate of corner in the image.
  int y;        //!< y-coordinate of corner in the image.
  float score;  //!< pixel value of the corner.
  Corner(int x, int y, float score) :
    x(x), y(y), score(score)
  {}
};

namespace cv
{
	class _InputArray;
	class _OutputArray;
    typedef const _InputArray& InputArray;
    typedef const _OutputArray& OutputArray;
}

struct PhaseCongruencyConst {
    double sigma;
    double mult = 2.0;
    double minwavelength = 1.5;
    double epsilon = 0.0002;
    double cutOff = 0.4;
    double g = 10.0;
    double k = 10.0;
    PhaseCongruencyConst();
    PhaseCongruencyConst(const PhaseCongruencyConst& _pcc);
    PhaseCongruencyConst& operator=(const PhaseCongruencyConst& _pcc);
};

class PhaseCongruency
{
public:
	PhaseCongruency(cv::Size _img_size, size_t _nscale, size_t _norient);
	~PhaseCongruency() {}

    /**
   * @breif: 提取灰度图像中的角点
   * @param {_image} 灰度图像
   * @param {_corners} 提取到的角点
   * @param {_threshold} 灰度阈值默认127，值越大，提取到的角点越少
   * @param {_cell_size} 栅格大小，默认为灰度图像15个像素宽度
   * @return : 0表示提取成功，-1表示提取失败
   */
  int detectCorners(cv::Mat _image, std::vector<Corner>& _corners, int _threshold = 127, int _cell_size = 15);

private:
  void setConst(PhaseCongruencyConst _pcc);
  void calc(cv::InputArray _src, std::vector<cv::Mat> &_pc);
  void feature(std::vector<cv::Mat> &_pc, cv::OutputArray _edges, cv::OutputArray _corners);
  void feature(cv::InputArray _src, cv::OutputArray _edges, cv::OutputArray _corners);

private:
    cv::Size size;
    size_t norient;
    size_t nscale;

    PhaseCongruencyConst pcc;

    std::vector<cv::Mat> filter;
};