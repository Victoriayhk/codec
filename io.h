#pragma once

#include "def.h"


//int write_frames_to_file(Frame *pframe, FILE *fout);
int yuv_read(AVFormat & para, Frame &frame);