#pragma once

#include "ResidualBlock.h"
#include "decode_buffer_pool.h"

int decode(Frame &frame,AVFormat &para,PKT &pkt,vector<FrameBufferPool>  &frame_pool);