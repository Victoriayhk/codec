#pragma once
#include "stdafx.h"
#include "def.h"


int write_frames_to_file(Frame *pframe, FILE *fout);
int yuv_read(FILE *fin, Frame &frame);;