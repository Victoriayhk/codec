#include<cstdio>
#include<vector>
#include<cstring>
using namespace std;

////////////////////////////�ݶ�

typedef unsigned char uint8_t;

class AVFormat	//ÊÓÆµ»ù±¾ÐÅÏ¢ÒÔ¼°ÊÓÆµ±àÂë¸÷Ïî²ÎÊý
{
	/**
	*  ÊÓÆµÃû³Æ
	*/
public:
	FILE* video;
	char file_name[100];
	/**
	*  ÊÓÆµµÄ¸ß¶ÈÓë¿í¶È
	*/
	static const int height = 720;
	static const int width = 1280;

	/**
	*  ÊÓÆµµÄfpsÓëÊÓÆµ×ÜÖ¡Êý
	*/
	int fps;
	int frame_num;

	/**
	*  ºê¿é´óÐ¡
	*/
	static const int block_width = 16;
	static const int block_height = 16;

	/**
	*  µ±Ç°frameid
	*/
	int frame_id;

	//ÊÓÆµ±àÂë¸÷Ïî²ÎÊý
	AVFormat()
	{
		video = nullptr;
	}
};

struct Video {
	static const int nFramePerSecond = 25;
	static const int nSecond         = 30;
	static const int nFrame          = nFramePerSecond * nSecond;
} video;

struct Block {
	uint8_t *data;
};

struct Frame {
	static const int height = 720;
	static const int width  = 1280;
	static const int nsize   = height * width;
	// unsigned char *Ys;
	// unsigned char *Us;
	// unsigned char *Vs;
	// Frame() {
	// 	Ys = new unsigned char[nsize];
	// 	Us = new unsigned char[nsize / 4];
	// 	Vs = new unsigned char[nsize / 4];
	// }
	std::vector<Block> Yblock;
	std::vector<Block> Ublock;
	std::vector<Block> Vblock;
};

///////////////////////////////

inline Block make_block(uint8_t *stream, int col_id, int block_h, int block_w) {
	Block block;
	block.data = new uint8_t[block_h * block_w];
	for (int i = 0; i < block_h; i++) {
		memcpy(&block.data[i * block_w], &stream[i * AVFormat::width + col_id * block_w], sizeof(uint8_t)*block_w);
	}
	return block;
}

int yuv_read(FILE *fin, Frame &frame) {
	static int frame_cnt = 0;
	frame_cnt ++;
	if (frame_cnt > video.nFrame) {
		return -1;
	}

	uint8_t *Ys = new uint8_t[Frame::nsize];
	uint8_t *Us = new uint8_t[Frame::nsize/4];
	uint8_t *Vs = new uint8_t[Frame::nsize/4];
	
	// if (fread(frame.Ys, sizeof(unsigned char), Frame::nSize, fin) == -1) return -1;
	// if (fread(frame.Us, sizeof(unsigned char), frame.nSize/4, fin) == -1) return -1;
	// if (fread(frame.Vs, sizeof(unsigned char), frame.nSize/4, fin) == -1) return -1;

	if (fread(Ys, sizeof(unsigned char), Frame::nsize, fin) == -1) return -1;
	unsigned char *tmp = new unsigned char[Frame::nsize/2];
	if (fread(tmp, sizeof(unsigned char), Frame::nsize/2, fin) == -1) return -1;
	for (int i = 0; i < Frame::nsize/2; i+=2) {
		frame.Us[i/2] = tmp[i];
		frame.Vs[i/2] = tmp[i+1];
	}
	if (fread(Us, sizeof(unsigned char), Frame::nsize/4, fin) == -1) return -1;
	if (fread(Vs, sizeof(unsigned char), Frame::nsize/4, fin) == -1) return -1;

	int n_row = AVFormat::height / AVFormat::block_height;
	int n_col = AVFormat::width / AVFormat::block_width;
	for (int i = 0; i < n_row; i++) {
		for (int j = 0; j < n_col; j++) {
			frame.Yblock.push_back(make_block(&Ys[i * AVFormat::block_height], j, AVFormat::block_height, AVFormat::block_width));
			frame.Ublock.push_back(make_block(&Us[i * AVFormat::block_height/2], j, AVFormat::block_height/2, AVFormat::block_width/2));
			frame.Vblock.push_back(make_block(&Vs[i * AVFormat::block_height/2], j, AVFormat::block_height/2, AVFormat::block_width/2));
		}
	}


	delete Ys;
	delete Us;
	delete Vs;

	return 0;
}

// int write_frames_to_file(Frame *pframe, FILE *fout) {
// 	for (int i = 0; i < video.nFrame; i++) {
// 		fwrite(pframe[i].Ys, sizeof(unsigned char), Frame::nsize, fout);
// 		fwrite(pframe[i].Us, sizeof(unsigned char), Frame::nsize/4, fout);
// 		fwrite(pframe[i].Vs, sizeof(unsigned char), Frame::nsize/4, fout);
// 	}

// 	return 0;
// }

int main(int argc, char *argv[]) {
	FILE *fin, *fout;
	fin = fopen(argv[1], "rb");
	fout = fopen(argv[2], "wb");


	Frame *frames = new Frame[video.nFrame];
	for (int i = 0; i < video.nFrame; i++) {
		printf("%d: %d\n", i, yuv_read(fin, frames[i]));
	}
	// write_frames_to_file(frames, fout);
}