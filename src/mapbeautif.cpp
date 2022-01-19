#include "mapbeautif.h"

#include <vector>

using namespace std;

namespace MapBeauti {
void MapBeauti::Beautifi(uint8_t* map_data, int map_height, int map_width) {
    vector<pair<int, int>> pixline_f;
    // 处理横向直线
    for (int j = 3; j < map_height - 3; j++) {
        for (int i = 3; i < map_width - 3; i++) {
            uint8_t gird_ij = map_data[j * map_width + i];

            int k = 1;

            if (gird_ij == 0) {
                for (int i_f = i; i_f < map_width - 3; i_f++) {
                    uint8_t pixvalue_f = map_data[j * map_width + i_f];
                    uint8_t near1      = map_data[(j + 1) * map_width + i_f];
                    uint8_t near2      = map_data[(j - 1) * map_width + i_f];
                    if ((near1 > 0 && near1 <= 255 && near1 != 127) &&
                        (near2 == 0 || near2 == 127)) {   // 判断压成方形的方向
                        k = -1;                           // 向上压
                    }
                    pair<int, int> curpix_f = pair<int, int>(i_f, j);
                    if (pixvalue_f == 0) {
                        pixline_f.push_back(curpix_f);
                    } else {
                        uint8_t pixvalue_e_1 = map_data[(j + k) * map_width + i_f];
                        uint8_t pixvalue_e_2 = map_data[(j + k) * map_width + i_f + 1];
                        uint8_t pixvalue_e_3 = map_data[(j + k) * map_width + i_f + 2];
                        pixvalue_e_2 = 0;
                        pixvalue_e_3 = 0;
                        /*uint8_t pixvalue_s_1 = map_data[(j + k) * map_width + pixline_f[0].first];*/
                        uint8_t pixvalue_s_2 = map_data[(j + k) * map_width + pixline_f[0].first - 1];
                        uint8_t pixvalue_s_3 = map_data[(j + k) * map_width + pixline_f[0].first - 2];
                        pixvalue_s_3 = 0;
                        /* pixvalue_s_1         = 0; */
                        if ((pixvalue_e_1 == 0 && pixvalue_e_2 == 0 && pixvalue_e_3 == 0) ||
                            (/*pixvalue_s_1 == 0 && */ pixvalue_s_2 == 0 && pixvalue_s_3 == 0)) {
                            for (uint32_t index_f = 0; index_f < pixline_f.size(); index_f++) {
                                map_data[pixline_f[index_f].second * map_width + pixline_f[index_f].first] =
                                        map_data[(pixline_f[index_f].second - k) * map_width +
                                                 pixline_f[index_f].first];
                                map_data[(pixline_f[index_f].second + k) * map_width + pixline_f[index_f].first] = 0;
                            }
                            pixline_f.clear();
                            i = i_f;
                            break;
                        } else {
                            pixline_f.clear();
                            i   = i_f;
                            i_f = map_width - 1;
                            break;
                        }
                    }
                }
            }
        }
    }

    pixline_f.clear();
    // 处理纵向直线
    for (int i = 3; i < map_width - 3; i++) {
        for (int j = 3; j < map_height - 3; j++) {
            uint8_t gird_ij = map_data[j * map_width + i];
            if (gird_ij == 0) {
                int k = 1;
                for (int j_f = j; j_f < map_height - 3; j_f++) {
                    uint8_t pixvalue_f      = map_data[j_f * map_width + i];
                    pair<int, int> curpix_f = pair<int, int>(i, j_f);
                    if (pixvalue_f == 0) {
                        pixline_f.push_back(curpix_f);
                        uint8_t near1 = map_data[j_f * map_width + i + 1];
                        uint8_t near2 = map_data[j_f * map_width + i - 1];
                        if ((near1 > 0 && near1 <= 255 && near1 != 127) && (near2 == 0 || near2 == 127)) {
                            k = -1;   //向左压
                        }
                    } else {
                        // 检测直线两端是否再接着直线
                        uint8_t pixvalue_e_1 = map_data[(j_f)*map_width + i + k];
                        uint8_t pixvalue_e_2 = map_data[(j_f + 1) * map_width + i + k];
                        uint8_t pixvalue_e_3 = map_data[(j_f + 2) * map_width + i + k];
                        pixvalue_e_2 = 0;
                        pixvalue_e_3 = 0;                        
                        /* uint8_t pixvalue_s_1 = map_data[(pixline_f[0].second) * map_width + i + k]; */
                        uint8_t pixvalue_s_2 = map_data[(pixline_f[0].second - 1) * map_width + i + k];
                        uint8_t pixvalue_s_3 = map_data[(pixline_f[0].second - 2) * map_width + i + k];
                        pixvalue_s_3 = 0;
                        /* pixvalue_s_1         = 0; */
                        if ((pixvalue_e_1 == 0 && pixvalue_e_2 == 0 && pixvalue_e_3 == 0) ||
                            (/*pixvalue_s_1 == 0 && */ pixvalue_s_2 == 0 && pixvalue_s_3 == 0)) {
                            for (uint32_t index_f = 0; index_f < pixline_f.size(); index_f++) {
                                map_data[pixline_f[index_f].second * map_width + pixline_f[index_f].first] =
                                        map_data[pixline_f[index_f].second * map_width + pixline_f[index_f].first - k];
                                map_data[pixline_f[index_f].second * map_width + pixline_f[index_f].first + k] = 0;
                            }
                            pixline_f.clear();
                            j   = j_f;
                            j_f = map_height - 1;
                            break;
                        } else {
                            pixline_f.clear();
                            j   = j_f;
                            j_f = map_height - 1;
                            break;
                        }
                    }
                }
            }
        }
    }

    //另一种美化地图的方法，方便继续改善
    {   // for (int j = 1; j < map_height -1 ; j++) {
        //    for (int i = 1; i < map_width-1; i++) {
        //        uint8_t gird_ij = map_data[j*map_width + i];
        //        uint8_t near1 = map_data[(j + 1)*map_width + i];
        //        uint8_t near2 = map_data[(j - 1)*map_width + i];
        //        if (gird_ij==0) {
        //            //stack<int> linelength;
        //            //prair<stack<pair<int, int>>, stack<pair<int, int>>> neighborpix;
        //            //stack<stack<pair<int, int>>> neighborpix;
        //            int curx = i;
        //            int cury = j;
        //            int gap = 0;
        //            for (int i_f = i; i_f < map_width - 1; i_f++) {
        //                uint8_t pixvalue_f = map_data[j * map_width + i_f];
        //                pair<int, int> curpix_f = pair<int, int>(i_f,j);
        //                if (pixvalue_f == 0) {
        //                    pixline_f.push_back(curpix_f);
        //                } else {
        //                    for (int i_s = i_f; i_s < map_width - 1; i_s++) {
        //                        uint8_t pixvalue_s = map_data[(j + 1) * map_width + i_s];
        //                        pair<int, int> curpix_s = pair<int, int>(i_s, j + 1);
        //                        if (pixvalue_s == 0) {
        //                            pixline_s.push_back(curpix_s);
        //                        } else {
        //                            if (pixline_s.empty()) {
        //                                pixline_f.clear();
        //                                i = i_f;
        //                                i_f = map_width - 1;
        //                                i_s = map_width - 1;
        //                                break;
        //                            } else {
        //                                if (pixline_f.size() >= pixline_s.size()) {
        //                                    for (int index_s = 0; index_s < pixline_s.size(); index_s++) {
        //                                        pixline_f.push_back(pixline_s[index_s]);
        //                                        //if (map_data[(pixline_s[index_s].second + 1) * map_width +
        //                                        pixline_s[index_s].first] != 0) {
        //                                        //    map_data[pixline_s[index_s].second * map_width +
        //                                        pixline_s[index_s].first] = 127;
        //                                        //}
        //                                        map_data[pixline_s[index_s].second * map_width +
        //                                        pixline_s[index_s].first]
        //                                            = map_data[(pixline_s[index_s].second + 1) * map_width +
        //                                            pixline_s[index_s].first];
        //                                        map_data[(pixline_s[index_s].second - 1) * map_width +
        //                                        pixline_s[index_s].first] = 0;
        //                                    }
        //                                    pixline_s.clear();
        //                                    i_f = i_s;
        //                                    break;
        //                                } else {
        //                                    for (int index_f = 0; index_f < pixline_f.size(); index_f++) {
        //                                        //if (map_data[(pixline_f[index_f].second - 1) * map_width +
        //                                        pixline_f[index_f].first] != 0) {
        //                                        //    map_data[pixline_f[index_f].second * map_width +
        //                                        pixline_f[index_f].first] = 127;
        //                                        //}
        //                                        map_data[pixline_f[index_f].second * map_width +
        //                                        pixline_f[index_f].first]
        //                                            = map_data[(pixline_f[index_f].second - 1) * map_width +
        //                                            pixline_f[index_f].first];
        //                                        map_data[(pixline_f[index_f].second + 1) * map_width +
        //                                        pixline_f[index_f].first] = 0;
        //                                    }
        //                                    pixline_f.clear();
        //                                    pixline_s.clear();
        //                                    i = i_f;
        //                                    i_f = map_width - 1;
        //                                    i_s = map_width - 1;
        //                                    break;
        //                                }
        //                            }
        //                        }
        //                    }
        //                }
        //            }
        //        }

        //    }
        //}
        // for (int i = 0; i < map_width - 1; i++) {
        //    for (int j = 0; j < map_height; j++) {
        //        uint8_t gird_ij = map_data[j*map_width + i];
        //        if (gird_ij == 0) {
        //            //stack<int> linelength;
        //            //prair<stack<pair<int, int>>, stack<pair<int, int>>> neighborpix;
        //            //stack<stack<pair<int, int>>> neighborpix;
        //            int curx = i;
        //            int cury = j;
        //            int gap = 0;
        //            for (int j_f = j; j_f < map_height - 1; j_f++) {
        //                uint8_t pixvalue_f = map_data[j_f * map_width + i];
        //                pair<int, int> curpix_f = pair<int, int>(i, j_f);
        //                if (pixvalue_f == 0) {
        //                    pixline_f.push_back(curpix_f);
        //                } else {
        //                    for (int j_s = j_f; j_s < map_height - 1; j_s++) {
        //                        uint8_t pixvalue_s = map_data[j_s * map_width + i + 1];
        //                        pair<int, int> curpix_s = pair<int, int>(i + 1, j_s);
        //                        if (pixvalue_s == 0) {
        //                            pixline_s.push_back(curpix_s);
        //                        } else {
        //                            if (pixline_s.empty()) {
        //                                pixline_f.clear();
        //                                j = j_f;
        //                                j_f = map_height - 1;
        //                                j_s = map_height - 1;
        //                                break;
        //                            } else {
        //                                if (pixline_f.size() >= pixline_s.size()) {
        //                                    for (int index_s = 0; index_s < pixline_s.size(); index_s++) {
        //                                        pixline_f.push_back(pixline_s[index_s]);
        //                                        map_data[pixline_s[index_s].second * map_width +
        //                                        pixline_s[index_s].first]
        //                                            = map_data[pixline_s[index_s].second * map_width +
        //                                            pixline_s[index_s].first + 1];
        //                                        map_data[pixline_s[index_s].second * map_width +
        //                                        pixline_s[index_s].first - 1] = 0;
        //                                    }
        //                                    pixline_s.clear();
        //                                    j_f = j_s;
        //                                    break;
        //                                } else {
        //                                    for (int index_f = 0; index_f < pixline_f.size(); index_f++) {
        //                                        map_data[pixline_f[index_f].second * map_width +
        //                                        pixline_f[index_f].first]
        //                                            = map_data[pixline_f[index_f].second * map_width +
        //                                            pixline_f[index_f].first - 1];
        //                                        map_data[pixline_f[index_f].second * map_width +
        //                                        pixline_f[index_f].first + 1] = 0;
        //                                    }
        //                                    pixline_f.clear();
        //                                    pixline_s.clear();
        //                                    j = j_f;
        //                                    j_f = map_height - 1;
        //                                    j_s = map_height - 1;
        //                                    break;
        //                                }
        //                            }
        //                        }
        //                    }
        //                }
        //            }
        //        }
        //    }
        //}
    }
}

}   // namespace MapBeauti
