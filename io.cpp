#include<cstdio>


////////////////////////////ÔÝ¶¨
/*
struct Video{
	static const short nFramePerSecond = 25;
	static const short nSecond         = 30;
	static const short nFrame          = nFramePerSecond * nSecond;
}video;
*/
/*
struct Frame{
	static const short nHeight = 720;
	static const short nWidth  = 1280;
	static const short nSize   = nHeight * nWidth;
	unsigned char *Ys;
	unsigned char *Us;
	unsigned char *Vs;
	Frame() {
		Ys = new unsigned char[nSize];
		Us = new unsigned char[nSize / 4];
		Vs = new unsigned char[nSize / 4];
	}
};


struct Block{
	
};
*/
///////////////////////////////

int yuv_read(FILE *fin, Frame &frame) {
	static short frame_cnt = 0;
	frame_cnt ++;
	if (frame_cnt > video.nFrame) {
		return -1;
	}

	if (fread(frame.Ys, sizeof(unsigned char), frame.nSize, fin) == -1) return -1;
	if (fread(frame.Us, sizeof(unsigned char), frame.nSize/4, fin) == -1) return -1;
	if (fread(frame.Vs, sizeof(unsigned char), frame.nSize/4, fin) == -1) return -1;

	return 0;
}

int write_frames_to_file(Frame *pframe, FILE *fout) {
	for (int i = 300; i < video.nFrame; i++) {
		fwrite(pframe[i].Ys, sizeof(unsigned char), pframe[i].nSize, fout);
		fwrite(pframe[i].Us, sizeof(unsigned char), pframe[i].nSize/4, fout);
		fwrite(pframe[i].Vs, sizeof(unsigned char), pframe[i].nSize/4, fout);
	}

	return 0;
}

int main(int argc, char *argv[]) {
	FILE *fin, *fout;
	fin = fopen(argv[1], "rb");
	fout = fopen(argv[2], "wb");


	Frame *frames = new Frame[video.nFrame];
	for (int i = 0; i < video.nFrame; i++) {
		printf("%d: %d\n", i, yuv_read(fin, frames[i]));
	}
	write_frames_to_file(frames, fout);
}