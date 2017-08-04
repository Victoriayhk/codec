/*
 *  huffcode - Encode/Decode files using Huffman encoding.
 *  http://huffman.sourceforge.net
 *  Copyright (C) 2003  Douglas Ryan Richardson
 */

//#include "F:\Project\huffman\huffman\config.h"
#include "huffman.h"
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <stdlib.h>
#include <assert.h>

#ifdef WIN32
#include <malloc.h>
extern int getopt(int, char**, char*);
extern char* optarg;
#else
#include <unistd.h>
#endif

static int memory_encode_file(FILE *in, FILE *out);
static int memory_decode_file(FILE *in, FILE *out);

static void
version(FILE *out)
{
	fputs("huffcode 0.3\n"
	      "Copyright (C) 2003 Douglas Ryan Richardson"
	      "; Gauss Interprise, Inc\n",
	      out);
}

static void
usage(FILE* out)
{
	fputs("Usage: huffcode [-i<input file>] [-o<output file>] [-d|-c]\n"
		  "-i - input file (default is standard input)\n"
		  "-o - output file (default is standard output)\n"
		  "-d - decompress\n"
		  "-c - compress (default)\n"
		  "-m - read file into memory, compress, then write to file (not default)\n",
		  out);
}


static int
memory_encode_file(FILE *in, FILE *out)
{
	unsigned char *buf = NULL, *bufout = NULL;
	unsigned int len = 0, cur = 0, inc = 1024, bufoutlen = 0;

	assert(in && out);

	/* Read the file into memory. */
	while(!feof(in))
	{
		unsigned char *tmp;
		len += inc;
		tmp = (unsigned char*)realloc(buf, len);
		if(!tmp)
		{
			if(buf)
				free(buf);
			return 1;
		}

		buf = tmp;
		cur += fread(buf + cur, 1, inc, in);
	}

	if(!buf)
		return 1;

	/* Encode the memory. */
	if(huffman_encode_memory(buf, cur, &bufout, &bufoutlen))
	{
		free(buf);
		return 1;
	}

	free(buf);

	/* Write the memory to the file. */
	if(fwrite(bufout, 1, bufoutlen, out) != bufoutlen)
	{
		free(bufout);
		return 1;
	}

	free(bufout);

	return 0;
}

static int
memory_decode_file(FILE *in, FILE *out)
{
	unsigned char *buf = NULL, *bufout = NULL;
	unsigned int len = 0, cur = 0, inc = 1024, bufoutlen = 0;
	assert(in && out);

	/* Read the file into memory. */
	while(!feof(in))
	{
		unsigned char *tmp;
		len += inc;
		tmp = (unsigned char*)realloc(buf, len);
		if(!tmp)
		{
			if(buf)
				free(buf);
			return 1;
		}

		buf = tmp;
		cur += fread(buf + cur, 1, inc, in);
	}

	if(!buf)
		return 1;

	/* Decode the memory. */
	if(huffman_decode_memory(buf, cur, &bufout, &bufoutlen))
	{
		free(buf);
		return 1;
	}

	free(buf);

	/* Write the memory to the file. */
	if(fwrite(bufout, 1, bufoutlen, out) != bufoutlen)
	{
		free(bufout);
		return 1;
	}

	free(bufout);

	return 0;
}
