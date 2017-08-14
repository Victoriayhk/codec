#pragma once

#include "def.h"

/* YUV文件读写
* 易惠康
*/

int yuv_read(AVFormat & para, Frame &frame);
int yuv_write(AVFormat & para, Frame &frame);