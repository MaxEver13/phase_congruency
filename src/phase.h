/*
 * @Descripttion: 
 * @version: 
 * @Author: Jiawen Ji
 * @Date: 2021-12-22 10:52:25
 * @LastEditors: Jiawen Ji
 * @LastEditTime: 2021-12-27 10:13:08
 */
#pragma once
#include <vector>
#include <opencv2/core/types.hpp>
#include <opencv2/imgproc.hpp>
#include <opencv2/highgui/highgui.hpp>

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