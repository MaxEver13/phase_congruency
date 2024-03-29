/*
 * @Descripttion: 
 * @version: 
 * @Author: Jiawen Ji
 * @Date: 2021-12-22 10:52:25
 * @LastEditors: Jiawen Ji
 * @LastEditTime: 2022-02-25 17:22:53
 */

#include "phase.h"
#include <iostream>

#define _USE_MATH_DEFINES
#include <math.h>

using namespace cv;
using namespace std;



// Rearrange the quadrants of Fourier image so that the origin is at
// the image center
void shiftDFT(InputArray _src, OutputArray _dst)
{
    Mat src = _src.getMat();
    Size size = src.size();

    _dst.create(size, src.type());
    auto dst = _dst.getMat();

    const int cx = size.width / 2;
    const int cy = size.height / 2; // image center

    Mat s1 = src(Rect(0, 0, cx, cy));
    Mat s2 = src(Rect(cx, 0, cx, cy));
    Mat s3 = src(Rect(cx, cy, cx, cy));
    Mat s4 = src(Rect(0, cy, cx, cy));

    Mat d1 = dst(Rect(0, 0, cx, cy));
    Mat d2 = dst(Rect(cx, 0, cx, cy));
    Mat d3 = dst(Rect(cx, cy, cx, cy));
    Mat d4 = dst(Rect(0, cy, cx, cy));

    Mat tmp;
    s3.copyTo(tmp);
    s1.copyTo(d3);
    tmp.copyTo(d1);

    s4.copyTo(tmp);
    s2.copyTo(d4);
    tmp.copyTo(d2);
}

#define MAT_TYPE CV_64FC1
#define MAT_TYPE_CNV CV_64F

// Making a filter
// src & dst arrays of equal size & type
PhaseCongruency::PhaseCongruency(Size _size, size_t _nscale, size_t _norient)
{
    size = _size;
    nscale = _nscale;
    norient = _norient;

    filter.resize(nscale * norient);

    const int dft_M = getOptimalDFTSize(_size.height);
    const int dft_N = getOptimalDFTSize(_size.width);

    Mat radius = Mat::zeros(dft_M, dft_N, MAT_TYPE);
    Mat matAr[2];
    matAr[0] = Mat::zeros(dft_M, dft_N, MAT_TYPE);
    matAr[1] = Mat::zeros(dft_M, dft_N, MAT_TYPE);
    Mat lp = Mat::zeros(dft_M, dft_N, MAT_TYPE);
    Mat angular = Mat::zeros(dft_M, dft_N, MAT_TYPE);
    std::vector<Mat> gabor(nscale);

    //Matrix values contain *normalised* radius 
    // values ranging from 0 at the centre to 
    // 0.5 at the boundary.
    int r;
    const int dft_M_2 = dft_M / 2;
    const int dft_N_2 = dft_N / 2;
    if (dft_M > dft_N) r = dft_N_2;
    else r = dft_M_2;
    const double dr = 1.0 / static_cast<double>(r);
    for (int row = dft_M_2 - r; row < dft_M_2 + r; row++)
    {
        auto radius_row = radius.ptr<double>(row);
        for (int col = dft_N_2 - r; col < dft_N_2 + r; col++)
        {
            int m = (row - dft_M_2);
            int n = (col - dft_N_2);
            radius_row[col] = sqrt(static_cast<double>(m * m + n * n)) * dr;
        }
    }
    lp = radius * 2.5;
    pow(lp, 20.0, lp);
    lp += Scalar::all(1.0);
    radius.at<double>(dft_M_2, dft_N_2) = 1.0;
    // The following implements the log-gabor transfer function.
    double mt = 1.0f;
    for (int scale = 0; scale < nscale; scale++)
    {
        const double wavelength = pcc.minwavelength * mt;
        gabor[scale] = radius * wavelength;
        log(gabor[scale], gabor[scale]);
        pow(gabor[scale], 2.0, gabor[scale]);
        gabor[scale] *= pcc.sigma;
        exp(gabor[scale], gabor[scale]);
        gabor[scale].at<double>(dft_M_2, dft_N_2) = 0.0;
        divide(gabor[scale], lp, gabor[scale]);
        mt = mt * pcc.mult;
    }
    const double angle_const = static_cast<double>(M_PI) / static_cast<double>(norient);
    for (int ori = 0; ori < norient; ori++)
    {
        double angl = (double)ori * angle_const;
        //Now we calculate the angular component that controls the orientation selectivity of the filter.
        for (int i = 0; i < dft_M; i++)
        {
            auto angular_row = angular.ptr<double>(i);
            for (int j = 0; j < dft_N; j++)
            {
                double m = atan2(-((double)j / (double)dft_N - 0.5), (double)i / (double)dft_M - 0.5);
                double s = sin(m);
                double c = cos(m);
                m = s * cos(angl) - c * sin(angl);
                double n = c * cos(angl) + s * sin(angl);
                s = fabs(atan2(m, n));

                angular_row[j] = (cos(min(s * (double)norient * 0.5, M_PI)) + 1.0) * 0.5;
            }
        }
        for (int scale = 0; scale < nscale; scale++)
        {
            multiply(gabor[scale], angular, matAr[0]); //Product of the two components.
            merge(matAr, 2, filter[nscale * ori + scale]);
        }//scale
    }//orientation
    //Filter ready
}

double PhaseCongruency::distance(int x1, int y1, int x2, int y2)
{
    return sqrt((x1-x2)*(x1-x2) + (y1-y2)*(y1-y2));
}

int PhaseCongruency::detectCorners(cv::Mat _image, std::vector<Corner>& _corners, int _threshold, int _cell_size, int _dis_thre)
{
    Mat image = _image.clone();

    if (image.empty())
    {
        cout << "Cannot read image file !" << endl;
        return -1;
    }
    
    Mat mat_edges, mat_corners;
    feature(image, mat_edges, mat_corners);

    namedWindow("corner_image");
    imshow("corner_image", mat_corners);
    cv::imwrite("corner_image.png", mat_corners);

    const int threshold = _threshold;
    const int dis_threshold = _dis_thre;

    // 将图片进行栅格化，每个栅格里面只选取最好的角点
    const int cell_size = _cell_size;
    const int grid_n_cols = ceil(static_cast<double>(mat_corners.cols)/cell_size);
    const int grid_n_rows = ceil(static_cast<double>(mat_corners.rows)/cell_size);
    vector<bool> grid(grid_n_cols *grid_n_rows, false);

    // 初值都为0
    vector<Corner> Corners(grid_n_cols *grid_n_rows, Corner(0, 0, 0));

    // 遍历找出大于127的点
    vector<Corner> detect_corners;
    for (int i = 0; i < mat_corners.rows; i++)
    {
        for (int j = 0; j < mat_corners.cols; j++)
        {
            if (threshold < mat_corners.at<uchar>(i, j))
            {
                detect_corners.push_back(Corner(i, j, (float)mat_corners.at<uchar>(i, j)));
            }
        }           
    }

    // 每个栅格里面只保存一个角点
    for (auto it = detect_corners.begin(); it != detect_corners.end(); it++)
    {   
        // 栅格id
        int index = static_cast<int>((*it).y/cell_size*grid_n_cols) + 
                    static_cast<int>((*it).x/cell_size);
        
        if (index < 0 || index > grid_n_cols *grid_n_rows - 1)
            continue;

        // 栅格已经有角点
        if (grid[index])
        {
            // 质量更好的角点，需要更新
            if ((*it).score > Corners.at(index).score)
            {
                Corners.at(index) = Corner((*it).x, (*it).y, (*it).score);
                continue;
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

    //　删除不同栅格但是距离较近的点
    for (int i = 0; i < (int)grid.size(); i++)
    {
        if (!grid[i])
            continue;

        Corner ci(Corners.at(i).x, Corners.at(i).y, Corners.at(i).score);
        // 找到距离近的点里面，质量最好的
        for (int j = 0; j < (int)grid.size(); j++)
        {
            if (!grid[j])
                continue;

            Corner cj(Corners.at(j).x, Corners.at(j).y, Corners.at(j).score);            
            if (distance(ci.x, ci.y, cj.x, cj.y) < dis_threshold)
            {
                // 将距离近且质量不高的角点全部标记为
                if (i != j && cj.score < ci.score)
                {   
                    grid[j] = false;
                } 
            }
        }
               
    }

    // 赋值
    for (int i = 0; i < (int)grid.size(); i++)
    {
        if (grid[i])
            _corners.push_back(Corner(Corners.at(i).x, Corners.at(i).y, Corners.at(i).score));

    }
    
    return 0;
}

void PhaseCongruency::setConst(PhaseCongruencyConst _pcc)
{
    pcc = _pcc;
}

//Phase congruency calculation
void PhaseCongruency::calc(InputArray _src, std::vector<cv::Mat> &_pc)
{
    Mat src = _src.getMat();

    CV_Assert(src.size() == size);

    const int width = size.width, height = size.height;

    Mat src64;
    src.convertTo(src64, MAT_TYPE_CNV, 1.0 / 255.0);

    const int dft_M_r = getOptimalDFTSize(src.rows) - src.rows;
    const int dft_N_c = getOptimalDFTSize(src.cols) - src.cols;

    _pc.resize(norient);
    std::vector<Mat> eo(nscale);
    Mat complex[2];
    Mat sumAn;
    Mat sumRe;
    Mat sumIm;
    Mat maxAn;
    Mat xEnergy;
    Mat tmp;
    Mat tmp1;
    Mat tmp2;
    Mat energy = Mat::zeros(size, MAT_TYPE);

    //expand input image to optimal size
    Mat padded;
    copyMakeBorder(src64, padded, 0, dft_M_r, 0, dft_N_c, BORDER_CONSTANT, Scalar::all(0));
    Mat planes[] = { Mat_<double>(padded), Mat::zeros(padded.size(), MAT_TYPE_CNV) };

    Mat dft_A;
    merge(planes, 2, dft_A);         // Add to the expanded another plane with zeros
    dft(dft_A, dft_A);            // this way the result may fit in the source matrix

    shiftDFT(dft_A, dft_A);

    for (unsigned o = 0; o < norient; o++)
    {
        double noise = 0;
        for (unsigned scale = 0; scale < nscale; scale++)
        {
            Mat filtered;
            mulSpectrums(dft_A, filter[nscale * o + scale], filtered, 0); // Convolution
            dft(filtered, filtered, DFT_INVERSE);
            filtered(Rect(0, 0, width, height)).copyTo(eo[scale]);

            split(eo[scale], complex);
            Mat eo_mag;
            magnitude(complex[0], complex[1], eo_mag);

            if (scale == 0)
            {
                //here to do noise threshold calculation
                auto tau = mean(eo_mag);
                tau.val[0] = tau.val[0] / sqrt(log(4.0));
                auto mt = 1.0 * pow(pcc.mult, nscale);
                auto totalTau = tau.val[0] * (1.0 - 1.0 / mt) / (1.0 - 1.0 / pcc.mult);
                auto m = totalTau * sqrt(M_PI / 2.0);
                auto n = totalTau * sqrt((4 - M_PI) / 2.0);
                noise = m + pcc.k * n;
                //xnoise = 0;
                //complex[0] -= xnoise;
                //max(complex[0], 0.0, complex[0]);

                eo_mag.copyTo(maxAn);
                eo_mag.copyTo(sumAn);
                complex[0].copyTo(sumRe);
                complex[1].copyTo(sumIm);
            }
            else
            {
                //complex[0] -= xnoise;
                //max(complex[0], 0.0, complex[0]);
                add(sumAn, eo_mag, sumAn);
                max(eo_mag, maxAn, maxAn);
                add(sumRe, complex[0], sumRe);
                add(sumIm, complex[1], sumIm);
            }
        } // next scale

        magnitude(sumRe, sumIm, xEnergy);
        xEnergy += pcc.epsilon;
        divide(sumIm, xEnergy, sumIm);
        divide(sumRe, xEnergy, sumRe);
        energy.setTo(0);
        for (int scale = 0; scale < nscale; scale++)
        {
            split(eo[scale], complex);

            multiply(complex[0], sumIm, tmp1);
            multiply(complex[1], sumRe, tmp2);

            absdiff(tmp1, tmp2, tmp);
            subtract(energy, tmp, energy);

            multiply(complex[0], sumRe, complex[0]);
            add(energy, complex[0], energy);
            multiply(complex[1], sumIm, complex[1]);
            add(energy, complex[1], energy);
            /*if (o == 0 && scale == 2)
            {
                energy -= noise / norient;
                max(energy, 0.0, energy);
                normalize(energy, tmp, 0, 1, NORM_MINMAX);
                imshow("energy", tmp);
            }*/
        } //next scale

        energy -= Scalar::all(noise); // -noise
        max(energy, 0.0, energy);
        maxAn += pcc.epsilon;

        divide(sumAn, maxAn, tmp, -1.0 / static_cast<double>(nscale));

        tmp += pcc.cutOff;
        tmp = tmp * pcc.g;
        exp(tmp, tmp);
        tmp += 1.0; // 1 / weight

        //PC
        multiply(tmp, sumAn, tmp);
        divide(energy, tmp, _pc[o]);
    }//orientation
}

//Build up covariance data for every point
void PhaseCongruency::feature(std::vector<cv::Mat>& _pc, cv::OutputArray _edges, cv::OutputArray _corners)
{
    _edges.create(size, CV_8UC1);
    _corners.create(size, CV_8UC1);
    auto edges = _edges.getMat();
    auto corners = _corners.getMat();

    Mat covx2 = Mat::zeros(size, MAT_TYPE);
    Mat covy2 = Mat::zeros(size, MAT_TYPE);
    Mat covxy = Mat::zeros(size, MAT_TYPE);
    Mat cos_pc, sin_pc, mul_pc;

    const double angle_const = M_PI / static_cast<double>(norient);

    for (unsigned o = 0; o < norient; o++)
    {
        auto angl = static_cast<double>(o) * angle_const;
        cos_pc = _pc[o] * cos(angl);
        sin_pc = _pc[o] * sin(angl);
        multiply(cos_pc, sin_pc, mul_pc);
        add(covxy, mul_pc, covxy);
        pow(cos_pc, 2, cos_pc);
        add(covx2, cos_pc, covx2);
        pow(sin_pc, 2, sin_pc);
        add(covy2, sin_pc, covy2);
    } // next orientation

      //Edges calculations
    covx2 *= 2.0 / static_cast<double>(norient);
    covy2 *= 2.0 / static_cast<double>(norient);
    covxy *= 4.0 / static_cast<double>(norient);
    Mat sub;
    subtract(covx2, covy2, sub);

    //denom += Scalar::all(epsilon);
    Mat denom;
    magnitude(sub, covxy, denom); // denom;
    Mat sum;
    add(covy2, covx2, sum);

    Mat minMoment, maxMoment;
    subtract(sum, denom, minMoment);//m = (covy2 + covx2 - denom) / 2;          % ... and minimum moment
    add(sum, denom, maxMoment); //M = (covy2+covx2 + denom)/2;          % Maximum moment

    maxMoment.convertTo(edges, CV_8U, 255);
    minMoment.convertTo(corners, CV_8U, 255);
}

//Build up covariance data for every point
void PhaseCongruency::feature(InputArray _src, cv::OutputArray _edges, cv::OutputArray _corners)
{
    std::vector<cv::Mat> pc;
    calc(_src, pc);
    feature(pc, _edges, _corners);
}

PhaseCongruencyConst::PhaseCongruencyConst()
{
    sigma = -1.0 / (2.0 * log(0.65) * log(0.65));
}

PhaseCongruencyConst::PhaseCongruencyConst(const PhaseCongruencyConst & _pcc)
{
    sigma = _pcc.sigma;
    mult = _pcc.mult;
    minwavelength = _pcc.minwavelength;
    epsilon = _pcc.epsilon;
    cutOff = _pcc.cutOff;
    g = _pcc.g;
    k = _pcc.k;
}

PhaseCongruencyConst& PhaseCongruencyConst::operator=(const PhaseCongruencyConst & _pcc)
{
    if (this == &_pcc) {
        return *this;
    }
    sigma = _pcc.sigma;
    mult = _pcc.mult;
    minwavelength = _pcc.minwavelength;
    epsilon = _pcc.epsilon;
    cutOff = _pcc.cutOff;
    g = _pcc.g;
    k = _pcc.k;

    return *this;
}
