/**
 * @file mapbeautif.h
 * @author TanHuan (th@ldrobot.com)
 * @brief
 * @version 0.1
 * @date 2021-06-23
 * @copyright Copyright (c) {2021} 深圳乐动机器人版权所有
 */

#pragma once

#include <stdint.h>

namespace MapBeauti {
class MapBeauti {
 public:
    static void Beautifi(uint8_t *mapData, int mapHeight, int mapWidth);
};
}   // namespace MapBeauti
