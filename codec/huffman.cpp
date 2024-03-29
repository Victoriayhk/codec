/*
 *  huffman - Encode/Decode files using Huffman encoding.
 *  http://huffman.sourceforge.net
 *  Copyright (C) 2003  Douglas Ryan Richardson
 */
//#include "F:\Project\huffman\huffman\config.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <cstring>
#include <assert.h>
#include <iostream>
#include "huffman.h"
#include<bitset>
extern int TABLE[1500][1500];

#define EIGHT_BIT_MODE 15
#define ZIG_ZAG_MODE 1

//#define WIN32

#ifdef WIN32
#include <winsock2.h>
#include <malloc.h>
#define alloca _alloca
#else
#include <netinet/in.h>
#endif

using namespace std;

unsigned int* s_len; 

// Z扫描使用的数组
int zag_zig_4_4[16][2] = {
	{3,3},{3,2},{2,3},{1,3},
	{2,2},{3,1},{3,0},{2,1},
	{1,2},{0,3},{0,2},{1,1},
	{2,0},{1,0},{0,1},{0,0}
};

typedef struct huffman_node_tag
{
	unsigned char isLeaf;
	unsigned long count;
	struct huffman_node_tag *parent;

	union
	{
		struct
		{
			struct huffman_node_tag *zero, *one;
		};
		unsigned char symbol;
	};
} huffman_node;

typedef struct huffman_code_tag
{
	/* The length of this code in bits. */
	unsigned long numbits;

	/* The bits that make up this code. The first
	   bit is at position 0 in bits[0]. The second
	   bit is at position 1 in bits[0]. The eighth
	   bit is at position 7 in bits[0]. The ninth
	   bit is at position 0 in bits[1]. */
	unsigned char *bits;
} huffman_code;

static unsigned long
numbytes_from_numbits(unsigned long numbits)
{
	return numbits / 8 + (numbits % 8 ? 1 : 0);
}

/*
 * get_bit returns the ith bit in the bits array
 * in the 0th position of the return value.
 */
static unsigned char
get_bit(unsigned char* bits, unsigned long i)
{
	return (bits[i / 8] >> i % 8) & 1;
}

static void
reverse_bits(unsigned char* bits, unsigned long numbits)
{
	unsigned long numbytes = numbytes_from_numbits(numbits);
	unsigned char *tmp =
	    (unsigned char*)alloca(numbytes);
	unsigned long curbit;
	long curbyte = 0;
	
	memset(tmp, 0, numbytes);

	for(curbit = 0; curbit < numbits; ++curbit)
	{
		unsigned int bitpos = curbit % 8;

		if(curbit > 0 && curbit % 8 == 0)
			++curbyte;
		
		tmp[curbyte] |= (get_bit(bits, numbits - curbit - 1) << bitpos);
	}

	memcpy(bits, tmp, numbytes);
}

/*
 * new_code builds a huffman_code from a leaf in
 * a Huffman tree.
 */
static huffman_code*
new_code(const huffman_node* leaf)
{
	/* Build the huffman code by walking up to
	 * the root node and then reversing the bits,
	 * since the Huffman code is calculated by
	 * walking down the tree. */
	unsigned long numbits = 0;
	unsigned char* bits = NULL;
	huffman_code *p;

	while(leaf && leaf->parent)
	{
		huffman_node *parent = leaf->parent;
		unsigned char cur_bit = (unsigned char)(numbits % 8);
		unsigned long cur_byte = numbits / 8;

		/* If we need another byte to hold the code,
		   then allocate it. */
		if(cur_bit == 0)
		{
			size_t newSize = cur_byte + 1;
			bits = (unsigned char*)realloc(bits, newSize);
			bits[newSize - 1] = 0; /* Initialize the new byte. */
		}

		/* If a one must be added then or it in. If a zero
		 * must be added then do nothing, since the byte
		 * was initialized to zero. */
		if(leaf == parent->one)
			bits[cur_byte] |= 1 << cur_bit;

		++numbits;
		leaf = parent;
	}

	if(bits)
		reverse_bits(bits, numbits);

	p = (huffman_code*)malloc(sizeof(huffman_code));
	p->numbits = numbits;
	p->bits = bits;
	return p;
}

#define MAX_SYMBOLS 256
typedef huffman_node* SymbolFrequencies[MAX_SYMBOLS];
typedef huffman_code* SymbolEncoder[MAX_SYMBOLS];

static huffman_node*
new_leaf_node(unsigned char symbol)
{
	huffman_node *p = (huffman_node*)malloc(sizeof(huffman_node));
	p->isLeaf = 1;
	p->symbol = symbol;
	p->count = 0;
	p->parent = 0;
	return p;
}

static huffman_node*
new_nonleaf_node(unsigned long count, huffman_node *zero, huffman_node *one)
{
	huffman_node *p = (huffman_node*)malloc(sizeof(huffman_node));
	p->isLeaf = 0;
	p->count = count;
	p->zero = zero;
	p->one = one;
	p->parent = 0;
	
	return p;
}

static void
free_huffman_tree(huffman_node *subtree)
{
	if(subtree == NULL)
		return;

	if(!subtree->isLeaf)
	{
		free_huffman_tree(subtree->zero);
		free_huffman_tree(subtree->one);
	}
	
	free(subtree);
}

static void
free_code(huffman_code* p)
{
	free(p->bits);
	free(p);
}

static void
free_encoder(SymbolEncoder *pSE)
{
	unsigned long i;
	for(i = 0; i < MAX_SYMBOLS; ++i)
	{
		huffman_code *p = (*pSE)[i];
		if(p)
			free_code(p);
	}

	free(pSE);
}

static void
init_frequencies(SymbolFrequencies *pSF)
{
	memset(*pSF, 0, sizeof(SymbolFrequencies));
#if 0
	unsigned int i;
	for(i = 0; i < MAX_SYMBOLS; ++i)
	{
		unsigned char uc = (unsigned char)i;
		(*pSF)[i] = new_leaf_node(uc);
	}
#endif
}

typedef struct buf_cache_tag
{
	unsigned char *cache;
	unsigned int cache_len;
	unsigned int cache_cur;
	unsigned char **pbufout;
	unsigned int *pbufoutlen;
} buf_cache;

static int init_cache(buf_cache* pc,
					  unsigned int cache_size,
					  unsigned char **pbufout,
					  unsigned int *pbufoutlen)
{
	assert(pc && pbufout && pbufoutlen);
	if(!pbufout || !pbufoutlen)
		return 1;
	
	pc->cache = (unsigned char*)malloc(cache_size);
	pc->cache_len = cache_size;
	pc->cache_cur = 0;
	pc->pbufout = pbufout;
	*pbufout = NULL;
	pc->pbufoutlen = pbufoutlen;
	*pbufoutlen = 0;

	return pc->cache ? 0 : 1;
}

static void free_cache(buf_cache* pc)
{
	assert(pc);
	if(pc->cache)
	{
		free(pc->cache);
		pc->cache = NULL;
	}
}

static int flush_cache(buf_cache* pc)
{
	assert(pc);
	
	if(pc->cache_cur > 0)
	{
		unsigned int newlen = pc->cache_cur + *pc->pbufoutlen;
		unsigned char* tmp = (unsigned char*)realloc(*pc->pbufout, newlen);
		if(!tmp)
			return 1;

		memcpy(tmp + *pc->pbufoutlen, pc->cache, pc->cache_cur);

		*pc->pbufout = tmp;
		*pc->pbufoutlen = newlen;
		pc->cache_cur = 0;
	}

	return 0;
}

static int write_cache(buf_cache* pc,
					   const void *to_write,
					   unsigned int to_write_len)
{
	unsigned char* tmp;

	assert(pc && to_write);
	assert(pc->cache_len >= pc->cache_cur);
	
	/* If trying to write more than the cache will hold
	 * flush the cache and allocate enough space immediately,
	 * that is, don't use the cache. */
	if(to_write_len > pc->cache_len - pc->cache_cur)
	{
		unsigned int newlen;
		flush_cache(pc);
		newlen = *pc->pbufoutlen + to_write_len;
		tmp = (unsigned char*)realloc(*pc->pbufout, newlen);
		if(!tmp)
			return 1;
		memcpy(tmp + *pc->pbufoutlen, to_write, to_write_len);
		*pc->pbufout = tmp;
		*pc->pbufoutlen = newlen;
	}
	else
	{
		/* Write the data to the cache. */
		memcpy(pc->cache + pc->cache_cur, to_write, to_write_len);
		pc->cache_cur += to_write_len;
	}

	return 0;
}

static unsigned int
get_symbol_frequencies(SymbolFrequencies *pSF, FILE *in)
{
	int c;
	unsigned int total_count = 0;
	
	/* Set all frequencies to 0. */
	init_frequencies(pSF);
	
	/* Count the frequency of each symbol in the input file. */
	while((c = fgetc(in)) != EOF)
	{
		unsigned char uc = c;
		if(!(*pSF)[uc])
			(*pSF)[uc] = new_leaf_node(uc);
		++(*pSF)[uc]->count;
		++total_count;
	}

	return total_count;
}

static unsigned int
get_symbol_frequencies_from_memory(SymbolFrequencies *pSF,
								   const unsigned char *bufin,
								   unsigned int bufinlen)
{
	unsigned int i;
	unsigned int total_count = 0;
	
	/* Set all frequencies to 0. */
	init_frequencies(pSF);
	
	/* Count the frequency of each symbol in the input file. */
	for(i = 0; i < bufinlen; ++i)
	{
		unsigned char uc = bufin[i];
		if(!(*pSF)[uc])
			(*pSF)[uc] = new_leaf_node(uc);
		++(*pSF)[uc]->count;
		++total_count;
	}

	return total_count;
}

/*
 * When used by qsort, SFComp sorts the array so that
 * the symbol with the lowest frequency is first. Any
 * NULL entries will be sorted to the end of the list.
 */
static int
SFComp(const void *p1, const void *p2)
{
	const huffman_node *hn1 = *(const huffman_node**)p1;
	const huffman_node *hn2 = *(const huffman_node**)p2;

	/* Sort all NULLs to the end. */
	if(hn1 == NULL && hn2 == NULL)
		return 0;
	if(hn1 == NULL)
		return 1;
	if(hn2 == NULL)
		return -1;
	
	if(hn1->count > hn2->count)
		return 1;
	else if(hn1->count < hn2->count)
		return -1;

	return 0;
}

#if 0
static void
print_freqs(SymbolFrequencies * pSF)
{
	size_t i;
	for(i = 0; i < MAX_SYMBOLS; ++i)
	{
		if((*pSF)[i])
			printf("%d, %ld\n", (*pSF)[i]->symbol, (*pSF)[i]->count);
		else
			printf("NULL\n");
	}
}
#endif

/*
 * build_symbol_encoder builds a SymbolEncoder by walking
 * down to the leaves of the Huffman tree and then,
 * for each leaf, determines its code.
 */
static void
build_symbol_encoder(huffman_node *subtree, SymbolEncoder *pSF)
{
	if(subtree == NULL)
		return;

	if(subtree->isLeaf)
		(*pSF)[subtree->symbol] = new_code(subtree);
	else
	{
		build_symbol_encoder(subtree->zero, pSF);
		build_symbol_encoder(subtree->one, pSF);
	}
}

/*
 * calculate_huffman_codes turns pSF into an array
 * with a single entry that is the root of the
 * huffman tree. The return value is a SymbolEncoder,
 * which is an array of huffman codes index by symbol value.
 */
static SymbolEncoder*
calculate_huffman_codes(SymbolFrequencies * pSF)
{
	unsigned int i = 0;
	unsigned int n = 0;
	huffman_node *m1 = NULL, *m2 = NULL;
	SymbolEncoder *pSE = NULL;
	
#if 0
	printf("BEFORE SORT\n");
	print_freqs(pSF);
#endif

	/* Sort the symbol frequency array by ascending frequency. */
	qsort((*pSF), MAX_SYMBOLS, sizeof((*pSF)[0]), SFComp);

#if 0	
	printf("AFTER SORT\n");
	print_freqs(pSF);
#endif

	/* Get the number of symbols. */
	for(n = 0; n < MAX_SYMBOLS && (*pSF)[n]; ++n)
		;

	/*
	 * Construct a Huffman tree. This code is based
	 * on the algorithm given in Managing Gigabytes
	 * by Ian Witten et al, 2nd edition, page 34.
	 * Note that this implementation uses a simple
	 * count instead of probability.
	 */
	for(i = 0; i < n - 1; ++i)
	{
		/* Set m1 and m2 to the two subsets of least probability. */
		m1 = (*pSF)[0];
		m2 = (*pSF)[1];

		/* Replace m1 and m2 with a set {m1, m2} whose probability
		 * is the sum of that of m1 and m2. */
		(*pSF)[0] = m1->parent = m2->parent =
			new_nonleaf_node(m1->count + m2->count, m1, m2);
		(*pSF)[1] = NULL;
		
		/* Put newSet into the correct count position in pSF. */
		qsort((*pSF), n, sizeof((*pSF)[0]), SFComp);
	}

	/* Build the SymbolEncoder array from the tree. */
	pSE = (SymbolEncoder*)malloc(sizeof(SymbolEncoder));
	memset(pSE, 0, sizeof(SymbolEncoder));

	//gaowk 2017/8/7 修改全0串进入时崩溃的bug
	if((*pSF)[0]->isLeaf)
	{
		(*pSE)[(*pSF)[0]->symbol] = new_code((*pSF)[0]);
		(*pSE)[(*pSF)[0]->symbol]->bits = (unsigned char*)malloc(1);
		*(*pSE)[(*pSF)[0]->symbol]->bits = 1;
		(*pSE)[(*pSF)[0]->symbol]->numbits = 1;
	}
	else
		build_symbol_encoder((*pSF)[0], pSE);

	return pSE;
}

/*
 * Write the huffman code table. The format is:
 * 4 byte code count in network byte order.
 * 4 byte number of bytes encoded
 *   (if you decode the data, you should get this number of bytes)
 * code1
 * ...
 * codeN, where N is the count read at the begginning of the file.
 * Each codeI has the following format:
 * 1 byte symbol, 1 byte code bit length, code bytes.
 * Each entry has numbytes_from_numbits code bytes.
 * The last byte of each code may have extra bits, if the number of
 * bits in the code is not a multiple of 8.
 */
static int
write_code_table(FILE* out, SymbolEncoder *se, uint32_t symbol_count)
{
	uint32_t i, count = 0;
	
	/* Determine the number of entries in se. */
	for(i = 0; i < MAX_SYMBOLS; ++i)
	{
		if((*se)[i])
			++count;
	}

	/* Write the number of entries in network byte order. */
	//i = htonl(count);
	i = count;
	if(fwrite(&i, sizeof(i), 1, out) != 1)
		return 1;

	/* Write the number of bytes that will be encoded. */
	//symbol_count = t_htonl(symbol_count);
	if(fwrite(&symbol_count, sizeof(symbol_count), 1, out) != 1)
		return 1;

	/* Write the entries. */
	for(i = 0; i < MAX_SYMBOLS; ++i)
	{
		huffman_code *p = (*se)[i];
		if(p)
		{
			unsigned int numbytes;
			/* Write the 1 byte symbol. */
			fputc((unsigned char)i, out);
			/* Write the 1 byte code bit length. */
			fputc(p->numbits, out);
			/* Write the code bytes. */
			numbytes = numbytes_from_numbits(p->numbits);
			if(fwrite(p->bits, 1, numbytes, out) != numbytes)
				return 1;
		}
	}

	return 0;
}

/*
 * Allocates memory and sets *pbufout to point to it. The memory
 * contains the code table.
 */
static int
write_code_table_to_memory(buf_cache *pc,
						   SymbolEncoder *se,
						   uint32_t symbol_count)
{
	uint32_t i, count = 0;

	/* Determine the number of entries in se. */
	for(i = 0; i < MAX_SYMBOLS; ++i)
	{
		if((*se)[i])
			++count;
	}

	/* Write the number of entries in network byte order. */
	//i = htonl(count);
	i =count;
	
	if(write_cache(pc, &i, sizeof(i)))
		return 1;

	/* Write the number of bytes that will be encoded. */
	//symbol_count = t_htonl(symbol_count);
	if(write_cache(pc, &symbol_count, sizeof(symbol_count)))
		return 1;

	/* Write the entries. */
	for(i = 0; i < MAX_SYMBOLS; ++i)
	{
		huffman_code *p = (*se)[i];
		if(p)
		{
			unsigned int numbytes;
			/* The value of i is < MAX_SYMBOLS (256), so it can
			be stored in an unsigned char. */
			unsigned char uc = (unsigned char)i;
			/* Write the 1 byte symbol. */
			if(write_cache(pc, &uc, sizeof(uc)))
				return 1;
			/* Write the 1 byte code bit length. */
			uc = (unsigned char)p->numbits;
			if(write_cache(pc, &uc, sizeof(uc)))
				return 1;
			/* Write the code bytes. */
			numbytes = numbytes_from_numbits(p->numbits);
			if(write_cache(pc, p->bits, numbytes))
				return 1;
		}
	}

	return 0;
}

/*
 * read_code_table builds a Huffman tree from the code
 * in the in file. This function returns NULL on error.
 * The returned value should be freed with free_huffman_tree.
 */
static huffman_node*
read_code_table(FILE* in, unsigned int *pDataBytes)
{
	huffman_node *root = new_nonleaf_node(0, NULL, NULL);
	uint32_t count;
	
	/* Read the number of entries.
	   (it is stored in network byte order). */
	if(fread(&count, sizeof(count), 1, in) != 1)
	{
		free_huffman_tree(root);
		return NULL;
	}

	//count = ntohl(count);  //xiugai

	/* Read the number of data bytes this encoding represents. */
	if(fread(pDataBytes, sizeof(*pDataBytes), 1, in) != 1)
	{
		free_huffman_tree(root);
		return NULL;
	}

//	*pDataBytes = ntohl(*pDataBytes); //xiugai


	/* Read the entries. */
	while(count-- > 0)
	{
		int c;
		unsigned int curbit;
		unsigned char symbol;
		unsigned char numbits;
		unsigned char numbytes;
		unsigned char *bytes;
		huffman_node *p = root;
		
		if((c = fgetc(in)) == EOF)
		{
			free_huffman_tree(root);
			return NULL;
		}
		symbol = (unsigned char)c;
		
		if((c = fgetc(in)) == EOF)
		{
			free_huffman_tree(root);
			return NULL;
		}
		
		numbits = (unsigned char)c;
		numbytes = (unsigned char)numbytes_from_numbits(numbits);
		bytes = (unsigned char*)malloc(numbytes);
		if(fread(bytes, 1, numbytes, in) != numbytes)
		{
			free(bytes);
			free_huffman_tree(root);
			return NULL;
		}

		/*
		 * Add the entry to the Huffman tree. The value
		 * of the current bit is used switch between
		 * zero and one child nodes in the tree. New nodes
		 * are added as needed in the tree.
		 */
		for(curbit = 0; curbit < numbits; ++curbit)
		{
			if(get_bit(bytes, curbit))
			{
				if(p->one == NULL)
				{
					p->one = curbit == (unsigned char)(numbits - 1)
						? new_leaf_node(symbol)
						: new_nonleaf_node(0, NULL, NULL);
					p->one->parent = p;
				}
				p = p->one;
			}
			else
			{
				if(p->zero == NULL)
				{
					p->zero = curbit == (unsigned char)(numbits - 1)
						? new_leaf_node(symbol)
						: new_nonleaf_node(0, NULL, NULL);
					p->zero->parent = p;
				}
				p = p->zero;
			}
		}
		
		free(bytes);
	}

	return root;
}

static int
memread(const unsigned char* buf,
		unsigned int buflen,
		unsigned int *pindex,
		void* bufout,
		unsigned int readlen)
{
	assert(buf && pindex && bufout);
	assert(buflen >= *pindex);
	if(buflen < *pindex)
		return 1;
	if(readlen + *pindex >= buflen)
		return 1;
	memcpy(bufout, buf + *pindex, readlen);
	*pindex += readlen;
	return 0;
}

static huffman_node*
read_code_table_from_memory(const unsigned char* bufin,
							unsigned int bufinlen,
							unsigned int *pindex,
							uint32_t *pDataBytes)
{
	huffman_node *root = new_nonleaf_node(0, NULL, NULL);
	uint32_t count;
	
	/* Read the number of entries.
	   (it is stored in network byte order). */
	if(memread(bufin, bufinlen, pindex, &count, sizeof(count)))
	{
		free_huffman_tree(root);
		return NULL;
	}

	//count = ntohl(count); //xiugai

	/* Read the number of data bytes this encoding represents. */
	if(memread(bufin, bufinlen, pindex, pDataBytes, sizeof(*pDataBytes)))
	{
		free_huffman_tree(root);
		return NULL;
	}

//	*pDataBytes = ntohl(*pDataBytes); //xiugai

	/* Read the entries. */
	while(count-- > 0)
	{
		unsigned int curbit;
		unsigned char symbol;
		unsigned char numbits;
		unsigned char numbytes;
		unsigned char *bytes;
		huffman_node *p = root;

		if(memread(bufin, bufinlen, pindex, &symbol, sizeof(symbol)))
		{
			free_huffman_tree(root);
			return NULL;
		}

		if(memread(bufin, bufinlen, pindex, &numbits, sizeof(numbits)))
		{
			free_huffman_tree(root);
			return NULL;
		}
		
		numbytes = (unsigned char)numbytes_from_numbits(numbits);
		bytes = (unsigned char*)malloc(numbytes);
		if(memread(bufin, bufinlen, pindex, bytes, numbytes))
		{
			free(bytes);
			free_huffman_tree(root);
			return NULL;
		}

		/*
		 * Add the entry to the Huffman tree. The value
		 * of the current bit is used switch between
		 * zero and one child nodes in the tree. New nodes
		 * are added as needed in the tree.
		 */
		for(curbit = 0; curbit < numbits; ++curbit)
		{
			if(get_bit(bytes, curbit))
			{
				if(p->one == NULL)
				{
					p->one = curbit == (unsigned char)(numbits - 1)
						? new_leaf_node(symbol)
						: new_nonleaf_node(0, NULL, NULL);
					p->one->parent = p;
				}
				p = p->one;
			}
			else
			{
				if(p->zero == NULL)
				{
					p->zero = curbit == (unsigned char)(numbits - 1)
						? new_leaf_node(symbol)
						: new_nonleaf_node(0, NULL, NULL);
					p->zero->parent = p;
				}
				p = p->zero;
			}
		}
		
		free(bytes);
	}

	return root;
}

static int
do_file_encode(FILE* in, FILE* out, SymbolEncoder *se)
{
	unsigned char curbyte = 0;
	unsigned char curbit = 0;
	int c;
	
	while((c = fgetc(in)) != EOF)
	{
		unsigned char uc = (unsigned char)c;
		huffman_code *code = (*se)[uc];
		unsigned long i;
		
		for(i = 0; i < code->numbits; ++i)
		{
			/* Add the current bit to curbyte. */
			curbyte |= get_bit(code->bits, i) << curbit;

			/* If this byte is filled up then write it
			 * out and reset the curbit and curbyte. */
			if(++curbit == 8)
			{
				fputc(curbyte, out);
				curbyte = 0;
				curbit = 0;
			}
		}
	}

	/*
	 * If there is data in curbyte that has not been
	 * output yet, which means that the last encoded
	 * character did not fall on a byte boundary,
	 * then output it.
	 */
	if(curbit > 0)
		fputc(curbyte, out);

	return 0;
}

static int
do_memory_encode(buf_cache *pc,
				 const unsigned char* bufin,
				 unsigned int bufinlen,
				 SymbolEncoder *se)
{
	unsigned char curbyte = 0;
	unsigned char curbit = 0;
	unsigned int i;
	
	for(i = 0; i < bufinlen; ++i)
	{
		unsigned char uc = bufin[i];
		huffman_code *code = (*se)[uc];
		unsigned long i;
		
		for(i = 0; i < code->numbits; ++i)
		{
			/* Add the current bit to curbyte. */
			curbyte |= get_bit(code->bits, i) << curbit;

			/* If this byte is filled up then write it
			 * out and reset the curbit and curbyte. */
			if(++curbit == 8)
			{
				if(write_cache(pc, &curbyte, sizeof(curbyte)))
					return 1;
				curbyte = 0;
				curbit = 0;
			}
		}
	}

	/*
	 * If there is data in curbyte that has not been
	 * output yet, which means that the last encoded
	 * character did not fall on a byte boundary,
	 * then output it.
	 */
	return curbit > 0 ? write_cache(pc, &curbyte, sizeof(curbyte)) : 0;
}

/*
 * huffman_encode_file huffman encodes in to out.
 */
int
huffman_encode_file(FILE *in, FILE *out)
{
	SymbolFrequencies sf;
	SymbolEncoder *se;
	huffman_node *root = NULL;
	int rc;
	unsigned int symbol_count;

	/* Get the frequency of each symbol in the input file. */
	symbol_count = get_symbol_frequencies(&sf, in);

	/* Build an optimal table from the symbolCount. */
	se = calculate_huffman_codes(&sf);
	root = sf[0];

	/* Scan the file again and, using the table
	   previously built, encode it into the output file. */
	rewind(in);
	rc = write_code_table(out, se, symbol_count);
	if(rc == 0)
		rc = do_file_encode(in, out, se);

	/* Free the Huffman tree. */
	free_huffman_tree(root);
	free_encoder(se);
	return rc;
}

int
huffman_decode_file(FILE *in, FILE *out)
{
	huffman_node *root, *p;
	int c;
	unsigned int data_count;
	
	/* Read the Huffman code table. */
	root = read_code_table(in, &data_count);
	if(!root)
		return 1;

	/* Decode the file. */
	p = root;
	while(data_count > 0 && (c = fgetc(in)) != EOF)
	{
		unsigned char byte = (unsigned char)c;
		unsigned char mask = 1;
		while(data_count > 0 && mask)
		{
			p = byte & mask ? p->one : p->zero;
			mask <<= 1;

			if(p->isLeaf)
			{
				fputc(p->symbol, out);
				p = root;
				--data_count;
			}
		}
	}

	free_huffman_tree(root);
	return 0;
}

#define CACHE_SIZE 1024

int huffman_encode_memory(const unsigned char *bufin,
						  unsigned int bufinlen,
						  unsigned char **pbufout,
						  unsigned int *pbufoutlen)
{
	SymbolFrequencies sf;
	SymbolEncoder *se;
	huffman_node *root = NULL;
	int rc;
	unsigned int symbol_count;
	buf_cache cache;
	cache.cache = nullptr;

	/* Ensure the arguments are valid. */
	if(!pbufout || !pbufoutlen)
		return 1;

	if(init_cache(&cache, CACHE_SIZE, pbufout, pbufoutlen))
		return 1;

	/* Get the frequency of each symbol in the input memory. */
	symbol_count = get_symbol_frequencies_from_memory(&sf, bufin, bufinlen);

	/* Build an optimal table from the symbolCount. */
	se = calculate_huffman_codes(&sf);
	root = sf[0];

	/* Scan the memory again and, using the table
	   previously built, encode it into the output memory. */
	rc = write_code_table_to_memory(&cache, se, symbol_count);
	if(rc == 0)
		rc = do_memory_encode(&cache, bufin, bufinlen, se);

	/* Flush the cache. */
	flush_cache(&cache);
	
	/* Free the Huffman tree. */
	free_huffman_tree(root);
	free_encoder(se);
	free_cache(&cache);
	return rc;
}

int huffman_decode_memory(const unsigned char *bufin,
						  unsigned int bufinlen,
						  unsigned char **pbufout,
						  unsigned int *pbufoutlen)
{
	huffman_node *root, *p;
	unsigned int data_count;
	unsigned int i = 0;
	unsigned char *buf;
	unsigned int bufcur = 0;

	/* Ensure the arguments are valid. */
	if(!pbufout || !pbufoutlen)
		return 1;

	/* Read the Huffman code table. */
	root = read_code_table_from_memory(bufin, bufinlen, &i, &data_count);
	if(!root)
		return 1;

	buf = (unsigned char*)malloc(data_count);

	/* Decode the memory. */
	p = root;
	for(; i < bufinlen && data_count > 0; ++i) 
	{
		unsigned char byte = bufin[i];
		unsigned char mask = 1;
		while(data_count > 0 && mask)
		{
			p = byte & mask ? p->one : p->zero;
			mask <<= 1;

			if(p->isLeaf)
			{
				buf[bufcur++] = p->symbol;
				p = root;
				--data_count;
			}
		}
	}

	free_huffman_tree(root);
	*pbufout = buf;
	*pbufoutlen = bufcur;
	return 0;
}
//////////////////////////////////////////
/* 
* 实现宏块级流化处理
* 输入：
* f_x f_y l_x l_y	残差的左上角及右下角坐标
* rBlock			残差块
* para				配置文件
* stream			返回编码后的流
*/
int entropy_encode_block(int f_x, int f_y, int l_x, int l_y, ResidualBlock& rBlock, AVFormat& para, uint8_t **stream)
{
	int bit_num = 9;								//默认使用9bit流化
	int b_size = (l_x-f_x + 1) * (l_y-f_y + 1);		//数据占用字节数
	int sign_size = (b_size + 7) >>3;				//符号位占用字节数
	double quantization_num;						//量化系数
	if(rBlock.block_type == Block::Y)
	{
		quantization_num = para.quantizationY;
	}
	else if(rBlock.block_type == Block::U)
	{
		quantization_num = para.quantizationU;
	}
	else
	{
		quantization_num = para.quantizationV;
	}
	//if(quantization_num>79)	//4bit流流化，未弃用
	//{
	//	bit_num = 4;
	//	b_size = (l_x-f_x + 1) * (l_y-f_y + 1) /2;
	//	sign_size = (b_size) * 0.125;
	//}
	if(quantization_num>EIGHT_BIT_MODE)	//8bit流流化
	{
		bit_num = 8;
		b_size = (l_x-f_x + 1) * (l_y-f_y + 1);
		sign_size = 0;
	}

	uint8_t * tmp_stream = nullptr;
	uint8_t* out_stream = nullptr;
	
	entropy_to_stream_bit(f_x,f_y,l_x,l_y,rBlock, para, &tmp_stream,bit_num);	//宏块数据流化

	uint8_t *p = tmp_stream;

	unsigned int out_length;
	
	int buff_length = huffman_encode_memory(tmp_stream, (b_size + sign_size) * sizeof(uint8_t), &out_stream, &out_length);	//流化后数据进行huffman编码

	*stream = (uint8_t *)malloc(sizeof(unsigned int));
	uint8_t *point = *stream;
	toch4(out_length,point);

	*stream = (uint8_t *)realloc(*stream,sizeof(unsigned int) + out_length * sizeof(uint8_t));
	point = *stream + sizeof(unsigned int);
	memcpy(point,out_stream,out_length);

	free(tmp_stream);
	free(out_stream);

	return out_length + sizeof(unsigned int);
}

/* 
* 将残差数据取负
* 输入：
* f_x f_y l_x l_y	残差的左上角及右下角坐标
* rBlock			残差块
* width				残差块宽度
*/
void reverse_data(int f_x, int f_y, int l_x, int l_y,int num,ResidualBlock& rBlock, int width)
{
	int y = num / (l_x - f_x + 1);
	int x = num % (l_x - f_x + 1);
	if(y>l_y-f_y) return;
	else if(y==l_y-l_x&&x>l_x-f_x) return;

	rBlock.data[x+ y*width] = -rBlock.data[x+ y*width];
}

/*
* 将流反熵编码并将数据保存在输入的残差block中
* 输入：
* f_x f_y l_x l_y	残差的左上角及右下角坐标
* rBlock			残差块
* para				配置文件
* stream			返回编码后的流
* buff_length		流长度
*/
int entropy_decode_block(int f_x, int f_y, int l_x, int l_y, ResidualBlock& rBlock, AVFormat& para, uint8_t *stream, int buff_length)
{
	int bit_num = 9;
	double quantization_num;
	if(rBlock.block_type == Block::Y)
	{
		quantization_num = para.quantizationY;
	}
	else if(rBlock.block_type == Block::U)
	{
		quantization_num = para.quantizationU;
	}
	else
	{
		quantization_num = para.quantizationV;
	}
	
	//if(quantization_num>79)	//4bit流流化，未弃用
	//{
	//	bit_num = 4;
	//}
	if(quantization_num>EIGHT_BIT_MODE)	//8bit流流化，默认为9bit流化
	{
		bit_num = 8;
	}
	
	unsigned int out_length;
	uint8_t* out_tmp = nullptr;

	huffman_decode_memory(stream,buff_length,&out_tmp,&out_length);

	entropy_from_stream_bit(f_x,f_y,l_x,l_y,rBlock,para,out_tmp,bit_num);

	free(out_tmp);
	return 0;
}

/* 
* 残差块转9bit数据流
* 输入：
* f_x f_y l_x l_y	残差的左上角及右下角坐标
* rBlock			残差块
* para				配置信息
* stream			输出流信息
* 输出：			流长度
*/
int entropy_to_stream(int f_x, int f_y, int l_x, int l_y, ResidualBlock& rBlock, AVFormat& para, uint8_t** stream )
{
	int b_size = (l_x-f_x + 1) * (l_y-f_y + 1);
	int sign_size = (b_size + 7)>>3;
	//uint8_t* memory = (uint8_t*)malloc((l_x-f_x) * (l_y-f_y) * sizeof(uint8_t) * 2);
	//if(stream == nullptr)
	*stream =(uint8_t*)malloc(sizeof(uint8_t) *(b_size + sign_size));

	uint8_t* memory = *stream;

	uint8_t* sign_flag = memory;
	uint8_t* num_flag = memory + sign_size * sizeof(uint8_t);

	//int16_t* p = rBlock.data.data();
	//bitset<8> sign_group(positive);
	uint8_t sign_group = 0x0;	//存储符号位的临时存储区
	int sign_num = 0;	//当前sign_group中存储了几个符号位
	int width;
	int height;
	rBlock.getBlockSize(para,height,width);

	//p += f_y * width + f_x;
	//if(*p>=0)
	//	*sign_flag = positive;
	//else
	//	*sign_flag = negative;

	for(int i = f_y;i<l_y + 1;++i)
	{
		for(int j =f_x;j<l_x + 1;++j)
		{
			int temp = rBlock.data[TABLE[i][width] + j];
			if(temp>=0)	//符号位流化
			{
				//sign_group = sign_group<<1;
				sign_group <<= 1;
			}
			else
			{
				//sign_group = sign_group<<1;
				sign_group <<= 1;
				sign_group |= 0x01;
				temp = -temp;
			}
			++sign_num;
			if(sign_num>=8)	//符号位满8位进行写入
			{
				sign_num = 0;
				*sign_flag++ = sign_group;
				sign_group = 0x00;
			}

			*num_flag++ = temp;
		}
	}
	if(sign_num!=0)		//符号位未全部写入时补0处理
	{
		*sign_flag = sign_group;
		sign_num = sign_num<<(8-sign_num);
	}

	return b_size + sign_size;
}

/* 
* 将9bit数据流转残差块数据
* 输入：
* f_x f_y l_x l_y	残差的左上角及右下角坐标
* rBlock			残差块
* para				配置信息
* stream			输入流信息
* 输出：			流长度
*/
int entropy_from_stream(int f_x, int f_y, int l_x, int l_y, ResidualBlock& rBlock, AVFormat& para, uint8_t* stream)
{
	int b_size = (l_x-f_x + 1) * (l_y-f_y + 1);
	int sign_size = (b_size + 7) >>3;

	uint8_t* sign_flag = stream;
	uint8_t* num_flag = sign_flag + sign_size * sizeof(uint8_t);

	uint8_t* p = sign_flag;

	int width;
	int height;
	rBlock.getBlockSize(para,height,width);

	bool flag = 0;

	p = sign_flag + sign_size * sizeof(uint8_t);

	for(int i = f_y;i<l_y +1;++i)	//数据读取
	{
		for(int j = f_x;j<l_x +1;++j)
		{
			rBlock.data[TABLE[i][width] + j] = *p;

			//int16_t sign;
			//if(*p == positive) sign = 1;
			//else sign = -1;
			//rBlock.data[i*width + j] *= sign;

			++p;
		}
	}
	
	p = sign_flag;
	uint8_t temp;
	for(int i =0;i<sign_size;++i)		//符号读取
	{
		temp = *p++;
		bitset<8> b_temp(temp); 
		for(int j=7;j>=0;--j)
		{
			if(b_temp[j] == 1)
			{
				reverse_data(f_x,f_y,l_x,l_y,i*8 + 7 - j,rBlock,width);
			}
		}
	}

	return b_size + sign_size;
}

/* 
* slice级转二进制流的函数
* 输入：
* f_x f_y l_x l_y	残差的左上角及右下角坐标
* rBlock			残差块
* para				配置信息
* stream			输出流化后的信息
* len				数据流长度
*/
int entropy_encode_slice(ResidualBlock* rBlock ,int block_len, AVFormat& para, uint8_t **stream, unsigned int* len)
{
	int height,width;
	rBlock[0].getBlockSize(para,height,width);

	//int b_size = width * height;
	//int sign_size = (b_size + 7) *0.125;

	int stream_len = sizeof(unsigned int);

	//*stream = (uint8_t*)malloc(sizeof(unsigned int));
	*stream = (uint8_t*)malloc(10000000);
	uint8_t* point = *stream + sizeof(unsigned int);
	//s_len = new unsigned int[3600];

	for(int i = 0 ;i < block_len ; ++i)
	{
		unsigned int code_stream_len = 0;
		uint8_t* temp_stream = nullptr;
		code_stream_len = entropy_encode_block(0,0,width-1,height-1,rBlock[i],para, &temp_stream);
		
		//int point_diif = point - *stream;
		//*stream = (uint8_t*)realloc(*stream, code_stream_len + stream_len);
		//point = *stream + point_diif;
		//point += code_stream_len;

		//toch4(code_stream_len,point);
		//point += sizeof(code_stream_len);

		point = (uint8_t *)memcpy(point,temp_stream,code_stream_len);

		free(temp_stream);

		//s_len[i] = len;
		

		point += code_stream_len;
		stream_len += code_stream_len;
	}

	point = *stream;
	toch4(stream_len - 4,point);
	//stream_len += sizeof(unsigned int);
	//sprintf((char *)*stream, "%x", stream_len);

	*len = stream_len;

	return 0;
}

int entropy_decode_slice(ResidualBlock* rBlock,int block_num , AVFormat& para, uint8_t *stream, unsigned int buff_length)
{
	int height,width;
	rBlock[0].getBlockSize(para,height,width);

	uint8_t* p = stream;

	for(int i = 0 ;i < block_num ; ++i)
	{
		int block_len = 0;
		fromch4(block_len,p);
		
		p += sizeof(unsigned int);

		entropy_decode_block(0,0,height-1,width-1,rBlock[i],para,p,block_len);
		p += block_len;
	}

	return 0;
}

int entropy_encode_pkt(PKT& pkt, AVFormat& para, uint8_t **stream, unsigned int *len)
{
	*stream = (uint8_t *)malloc(sizeof(unsigned int));
	uint8_t * point = *(stream + 1);

	int i = 0;
	int s_len = sizeof(unsigned int);
	int s_remind_len = 0;
	ResidualBlock* rBlock = pkt.Ylist.data();
	int block_num = para.height * para.width / para.block_height / para.block_width;
	while(i<block_num)
	{
		unsigned int len;
		uint8_t* temp;
		entropy_encode_slice(rBlock, AVFormat::entropy_silce_size, para, &temp ,&len);
		i += AVFormat::entropy_silce_size;
		//point += len;

		if(s_remind_len < len)
		{
			s_len += len;
			*stream = (uint8_t *)realloc(*stream, s_len);
		}

		memcpy(point,temp,len);
		free(temp);
		point += len;
	}

	i = 0;
	while(i<block_num)
	{
		unsigned int len;
		uint8_t* temp;
		entropy_encode_slice(rBlock, AVFormat::entropy_silce_size, para, &temp ,&len);
		i += AVFormat::entropy_silce_size;
		//point += len;

		if(s_remind_len < len)
		{
			s_len += len;
			*stream = (uint8_t *)realloc(*stream, s_len);
		}

		memcpy(point,temp,len);
		free(temp);
		point += len;
	}

	i = 0;
	while(i<block_num)
	{
		unsigned int len;
		uint8_t* temp;
		entropy_encode_slice(rBlock, AVFormat::entropy_silce_size, para, &temp ,&len);
		i += AVFormat::entropy_silce_size;
		//point += len;

		if(s_remind_len < len)
		{
			s_len += len;
			*stream = (uint8_t *)realloc(*stream, s_len);
		}

		memcpy(point,temp,len);
		free(temp);
		point += len;
	}

	return 0;
}

//template<typename T>
//void toch4(T val, uint8_t* result)	// 将任意类型的数转化为uint8_t的数组
//{
//	int len = sizeof(T);
//	for(int i = 0;i<len;++i)
//	{
//		result[i] = (uint8_t)(val>>(8*(len-i -1)) & 0x000000ff);
//	}
//}
//
//template<typename T>
//void fromch4(T& result, uint8_t* val)	// 将任意类型的数转化为uint8_t的数组
//{
//	result = 0;
//	int len = sizeof(T);
//	for(int i = 0;i<len;++i)
//	{
//		result <<= 8;
//		result |= val[i];
//	}
//}
//
//void fromch4(unsigned int& result, uint8_t* val)
//{
//	result = 0;
//	int len = sizeof(unsigned int);
//	for(int i = 0;i<len;++i)
//	{
//		result <<= 8;
//		result |= val[i];
//	}
//}


/*
* 根据bit数对残差数据进行流化处理的函数
* f_x , f_y 左上角坐标
* l_x , l_y 右下角坐标
* rBlock 残差数据信息
* para 配置信息
* stream 返回流信息
* bit_len 采用流化的bit模式
*/
int entropy_to_stream_bit(int f_x, int f_y, int l_x, int l_y, ResidualBlock& rBlock, AVFormat& para, uint8_t** stream, int bit_len)
{
	int b_size = 0;		//数据占用总字节数
	int sign_size = 0;	//符号位占用总字节数

	int width = 0;		//宏块宽和高
	int height = 0; 
	rBlock.getBlockSize(para,height,width);

	if(bit_len == 9)	//9bit流化
	{
		return entropy_to_stream(f_x,f_y,l_x,l_y,rBlock,para,stream);
	}
	else if(bit_len == 8)	//8bit流化
	{
		if(ZIG_ZAG_MODE)
		{
			uint8_t zag_zig_stream[256];
			int zero_num = zag_zig(rBlock,para,zag_zig_stream);	//Z扫描编码
			rBlock.left_zero_num = zero_num;
			*stream =(uint8_t*)malloc(sizeof(uint8_t) *height * width - zero_num);
			memcpy(*stream,zag_zig_stream,sizeof(uint8_t) *height * width - zero_num);

			return height * width - zero_num;
		}
		else{
			b_size = (l_x-f_x + 1) * (l_y-f_y + 1);
			sign_size = 0;
			*stream =(uint8_t*)malloc(sizeof(uint8_t) *b_size);
			uint8_t *point = *stream;

			//width = 2;
			//height = 2;

			for(int i = f_y;i<l_y + 1;++i)
			{
				for(int j=f_x;j<l_x+1;++j){
					int temp = rBlock.data[TABLE[i][width] + j];
					uint8_t tmp2;
					if(temp>=0)
					{
						tmp2 = temp<<1;
					}
					else
					{
						tmp2 = ((-temp)<<1) - 1;
					}
					//*num_flag++ = temp;
					*point++ = tmp2;
				}
			}
		}
		return b_size+ sign_size;
	}
	else if(bit_len == 4)
	{
		b_size = (l_x-f_x + 1) * (l_y-f_y + 1) /2;
		sign_size = (b_size + 7)/8;

		*stream =(uint8_t*)malloc(sizeof(uint8_t) *(b_size + sign_size));

		uint8_t* memory = *stream;

		uint8_t* sign_flag = memory;
		uint8_t* num_flag = memory + sign_size * sizeof(uint8_t);

		uint8_t sign_group = 0x0;
		uint8_t num_group = 0x0;
		int sign_num = 0;
		int num_num = 0;
		int width;
		int height; 
		rBlock.getBlockSize(para,height,width);

		for(int i = f_y;i<l_y + 1;++i)
		{
			for(int j =f_x;j<l_x + 1;++j)
			{
				int temp = rBlock.data[TABLE[i][width] + j];
				if(temp>=0)
				{
					sign_group = sign_group<<1;
				}
				else
				{
					sign_group = sign_group<<1;
					sign_group |= 0x01;
					temp = -rBlock.data[TABLE[i][width] + j];
				}
				++sign_num;
				if(sign_num>=8)
				{
					sign_num = 0;
					*sign_flag++ = sign_group;
					sign_group = 0x00;
				}

				num_group <<= 4;
				num_group |= (0x0f & temp);
				num_num += 4;

				if(num_num>=8)
				{
					num_num = 0;
					*num_flag++ = temp;
					num_group = 0x00;
				}

				//*num_flag++ = temp;
			}
		}
		if(sign_num!=0)
		{
			*sign_flag = sign_group;
			sign_num = sign_num<<(8-sign_num);
		}

		return b_size+ sign_size;
	}
	return 0;
}

/*
* 根据bit数对数据流进行反流化处理的函数
* f_x , f_y 左上角坐标
* l_x , l_y 右下角坐标
* rBlock 残差数据信息
* para 配置信息
* stream 输入数据流
* bit_len 采用流化的bit模式
*/
int entropy_from_stream_bit(int f_x, int f_y, int l_x, int l_y, ResidualBlock& rBlock, AVFormat& para, uint8_t* stream, int bit_len)
{
	if(bit_len == 9)
	{
		return entropy_from_stream(f_x,f_y,l_x,l_y,rBlock,para,stream);
	}
	else if(bit_len == 8)
	{
		if(ZIG_ZAG_MODE)
		{
			return unzag_zig(rBlock,para,stream,rBlock.left_zero_num);	//将Z扫描编码数据保存到残差数据块
		}
		else{
			int b_size = (l_x-f_x + 1) * (l_y-f_y + 1);
			int sign_size = 0;

			uint8_t* p = stream;

			int width;
			int height;
			rBlock.getBlockSize(para,height,width);
			//width = 2;
			//height = 2;

			bool flag = false;

			for(int i = f_y;i<l_y +1;++i)
			{
				for(int j = f_x;j<l_x +1;++j)
				{
					if( *p%2 == 1)
					{
						rBlock.data[TABLE[i][width] + j] = -((*p + 1)>>1);
					}
					else
					{
						rBlock.data[TABLE[i][width] + j] = (*p)>>1;
					}

					++p;
				}

				return b_size + sign_size;
			}
		}
	}
	else if(bit_len == 4)
	{
		uint8_t temp;
		int b_size = (l_x-f_x + 1) * (l_y-f_y + 1) /2;
		int sign_size = (b_size + 7)/8;

		uint8_t* sign_flag = stream;
		uint8_t* num_flag = sign_flag + sign_size * sizeof(uint8_t);

		uint8_t* p = sign_flag;

		int width;
		int height;
		rBlock.getBlockSize(para,height,width);

		bool flag = 0;

		p = num_flag;

		for(int i = 0;i<b_size; ++i)
		{
			temp = *p++;
			//bitset<8> b_temp(temp); 
			for(int j = 1 ; j >= 0; --j)
			{
				rBlock.data[i*2 + 1 - j] = (temp>>(j*4)) & 0x0f;
			}
		}

		//for(int i = f_y;i<l_y +1;++i)
		//{
		//	for(int j = f_x;j<l_x +1;++j)
		//	{
		//		rBlock.data[i*width + j] = *p;

		//		++p;
		//	}
		//}

		p = sign_flag;
		for(int i =0;i<sign_size;++i)
		{
			temp = *p++;
			bitset<8> b_temp(temp); 
			for(int j=7;j>=0;--j)
			{
				if(b_temp[j] == 1)
				{
					reverse_data(f_x,f_y,l_x,l_y,i*8 + 7 - j,rBlock,width);
				}
			}
		}

		return b_size + sign_size;
	}
	return 0;
}

/*	帧级流化处理
*	rBlock		残差块数组
*	block_len	残差块数目
*	para		配置信息
*	stream		数据流
*	len			数据流长度
*/
int entropy_encode_by_frame(ResidualBlock* rBlock ,int block_len, AVFormat& para, uint8_t **stream,unsigned int* len)
{
	int height = 0,width = 0;	//宏块长宽
	unsigned int out_len = 0;	//huffman编码后输出码流长度
	rBlock[0].getBlockSize(para,height,width);

	int stream_len = 0;			//码流长度

	//*stream = nullptr;
	uint8_t* tmp_stream =  (uint8_t*)malloc(10000000);
	uint8_t* huffman_tmp_buff = nullptr;

	uint8_t* point = tmp_stream;

	for(int i = 0 ;i < block_len ; ++i)//逐宏块进行数据流化
	{
		unsigned int code_stream_len = 0;
		code_stream_len = entropy_encode_block_by_frame(0,0,width-1,height-1,rBlock[i],para, &point);

		point += code_stream_len;
		stream_len += code_stream_len;
	}

	huffman_encode_memory(tmp_stream,stream_len,&huffman_tmp_buff,&out_len);

	*stream = (uint8_t*)malloc(10000000);
	point = *stream;
	toch4(out_len,point);
	*len = out_len + 4;

	point += 4;
	memcpy(point,huffman_tmp_buff,out_len);

	free(huffman_tmp_buff);
	free(tmp_stream);

	return 0;
}

/*
* 采用帧级数据熵编码策略时使用的流化处理函数
* f_x , f_y 宏块左上角坐标
* l_x , l_y 宏块右下角坐标
* rBlock 残差宏块数据
* para 配置信息
* stream 返回二进制流
*/

int entropy_encode_block_by_frame(int f_x, int f_y, int l_x, int l_y, ResidualBlock& rBlock, AVFormat& para, uint8_t **stream)
{
	unsigned int out_length;
	int bit_num = 9;		//默认采用9bit流化策略
	//int b_size = (l_x-f_x + 1) * (l_y-f_y + 1);
	//int sign_size = (b_size + 7) * 0.125;
	double quantization_num;
	if(rBlock.block_type == Block::Y)
	{
		quantization_num = para.quantizationY;
	}
	else if(rBlock.block_type == Block::U)
	{
		quantization_num = para.quantizationU;
	}
	else
	{
		quantization_num = para.quantizationV;
	}
	//if(quantization_num>79)	//已经弃用的4bit流化策略
	//{
	//	bit_num = 4;
	//	b_size = (l_x-f_x + 1) * (l_y-f_y + 1) /2;
	//	sign_size = (b_size) * 0.125;
	//}
	if(quantization_num>EIGHT_BIT_MODE)	//当量化值高于阈值时采用8bit流化策略
	{
		bit_num = 8;
		//b_size = (l_x-f_x + 1) * (l_y-f_y + 1);
		//sign_size = 0;
	}

	uint8_t * tmp_stream = nullptr;
	uint8_t* out_stream = nullptr;
	uint8_t *point = *stream;
	out_length = entropy_to_stream_bit(f_x,f_y,l_x,l_y,rBlock, para, &tmp_stream,bit_num);		//数据流化

	uint8_t *p = tmp_stream;
	point = *stream;
	memcpy(point,tmp_stream,out_length);

	free(tmp_stream);
	free(out_stream);

	return out_length;
}

/*
* 采用帧级数据熵编码策略时使用的反流化处理函数
* f_x , f_y 宏块左上角坐标
* l_x , l_y 宏块右下角坐标
* rBlock 残差宏块数据
* para 配置信息
* stream 返回二进制流
* buff_length 流长度
*/

int entropy_decode_by_frame(ResidualBlock* rBlock,int block_num , AVFormat& para, uint8_t *stream, unsigned int buff_length)
{
	int height,width;
	rBlock[0].getBlockSize(para,height,width);

	int block_len = 0;
	//int stream_len;
	//unsigned char stream_len_ch[4];
	uint8_t* out_stream = nullptr;
	unsigned int out_len = 0;

	int x = 0;
	//fromch4(stream_len,stream);

	uint8_t* p = stream;

	huffman_decode_memory(p,buff_length,&out_stream,&out_len);

	p = out_stream;

	for(int i = 0 ;i < block_num ; ++i)
	{
		block_len = entropy_decode_block_by_frame(0,0,height-1,width-1,rBlock[i],para,p);
		//if(block_len == )
		p += block_len;
	}

	return 0;
}

/*
*	帧级下的宏块反流化处理
*	f_x，f_y	左上角坐标
*	l_x，l_y	右下角坐标
*	rBlock		残差块
*	para		配置信息
*	stream		数据流
*/
int entropy_decode_block_by_frame(int f_x, int f_y, int l_x, int l_y, ResidualBlock& rBlock, AVFormat& para, uint8_t *stream)
{
	int bit_num = 9;
	double quantization_num;
	if(rBlock.block_type == Block::Y)
	{
		quantization_num = para.quantizationY;
	}
	else if(rBlock.block_type == Block::U)
	{
		quantization_num = para.quantizationU;
	}
	else
	{
		quantization_num = para.quantizationV;
	}
	
	//if(quantization_num>79)
	//{
	//	bit_num = 4;
	//}
	if(quantization_num>16)
	{
		bit_num = 8;
	}

	return entropy_from_stream_bit(f_x,f_y,l_x,l_y,rBlock,para,stream,bit_num);

	//free(out_tmp);

}

/*
*	Z扫描处理
*	rBlock			残差数据
*	para			配置信息
*	zag_zig_stream	Z扫描后的流
*/
int zag_zig(ResidualBlock& rBlock, AVFormat& para, uint8_t* zag_zig_stream)
{
	int height,width;
	rBlock.getBlockSize(para,height,width);
	uint8_t* point = zag_zig_stream;
	int block_id = 0;
	int zero_num = 0;
	int length = 0;
	int x = 0;
	int y = 0;
	bool last_zero = true;
	int16_t tmp_16 = 0;
	uint8_t tmp;
	int i = 0;
	int zag_zig_stream_len = 0;
	int block_num_row = 0;
	int block_num = 0;

	if(height == 16&&width == 16)
	{
		block_num_row = 4;
		block_num = 16;
	}
	else if(height == 8 && width == 8)
	{
		block_num_row = 2;
		block_num = 4;
	}
	else return -1;

	//if(rBlock.data[width*3 + 3] != 0) return -1;

	while(i < 16)
	{
		x = zag_zig_4_4[i][0];
		y = zag_zig_4_4[i][1];

		for(block_id = 0;block_id<block_num;++block_id)
		{
			tmp_16 = rBlock.data[width*x + y + block_id/block_num_row * width * 4 + block_id % block_num_row * 4];

			if(last_zero)
			{
				if(tmp_16 != 0)
				{
					last_zero = false;
					//if(zero_num < 8) return -1;		//尾0个数小于8时不使用zag_zig排列方式

				}
				else
				{
					zero_num++;
				}
			}
			if(!last_zero) 
			{
				if(tmp_16>=0)
				{
					tmp = tmp_16<<1;
				}
				else
				{
					tmp = ((-tmp_16)<<1) - 1;
				}
				zag_zig_stream[zag_zig_stream_len++] = tmp;
			}
		}

		++i;
	}

	if(last_zero&&zero_num!=0)		//全0串仍要存储一个0
	{
		zag_zig_stream[zag_zig_stream_len] = 0;
		return zero_num - 1;
	}
	return zero_num;
}

/*
*	逆Z扫描处理函数
*	rBlock			保存数据的残差块
*	para			配置信息
*	zag_zig_stream	Z扫描流
*	zero_num		尾0个数
*/
int unzag_zig(ResidualBlock& rBlock, AVFormat& para, uint8_t* zag_zig_stream, int zero_num)
{
	int height,width;
	rBlock.getBlockSize(para,height,width);
	uint8_t* point = zag_zig_stream;
	int block_id = 0;
	int length = 0;
	int x = 0;
	int y = 0;
	bool last_zero = true;
	uint8_t tmp = 0;
	int16_t tmp_16 = 0;
	int i = 0;
	int zag_zig_stream_len = 0;
	int block_num_row = 0;
	int block_num = 0;
	int zag_zig_i = 0;

	if(height == 16&&width == 16)
	{
		block_num_row = 4;
		block_num = 16;
	}
	else if(height == 8 && width == 8)
	{
		block_num_row = 2;
		block_num = 4;
	}
	else return -1;

	if(rBlock.left_zero_num == 255)
	{
	}

	while(i < 16)
	{
		x = zag_zig_4_4[i][0];
		y = zag_zig_4_4[i][1];

		for(block_id = 0;block_id<block_num;++block_id)
		{
			if(zero_num > 0)
			{
				tmp = 0;
				--zero_num;
			}
			else
			{
				tmp = zag_zig_stream[zag_zig_i];
				++zag_zig_i;
			}

			if( tmp%2 == 1)
			{
				rBlock.data[width*x + y + block_id/block_num_row * width * 4 + block_id % block_num_row * 4] = -((tmp + 1)>>1);
			}
			else
			{
				rBlock.data[width*x + y + block_id/block_num_row * width * 4 + block_id % block_num_row * 4] = (tmp)>>1;
			}
		}

		++i;
	}

	return height*width - rBlock.left_zero_num;
}

//////////////////单元测试用函数////////////////
void head_test()
{
	AVFormat para;
	para.block_num = 10;
	para.height = 1280;
	para.width = 720;
	para.block_height = 2;
	para.block_width = 2;
	PKT pkt;
	pkt.init(para);
	ResidualBlock rblock[10];
	ResidualBlock rblock2[10];
	for(int i = 0 ; i< 10 ;++i)
	{
		rblock[i].data.push_back(i);
		rblock[i].data.push_back(-i);
		rblock[i].data.push_back(-2*i);
		rblock[i].data.push_back(-3*i);
		//rblock[i].block_id = 1;
		////rblock.block_type = 2;
		//rblock[i].order = 3;
		//rblock[i].type_slice = 2;

		//for(int j = 0 ; j< 4;++j)
		//{
		//	rblock[i].node.push_back(i*i);
		//}
	}

	uint8_t *tmp_stream = nullptr;
	uint8_t *stream = (uint8_t*)malloc(1000000);
	uint8_t *point = stream;
	uint8_t *out_stream = nullptr;
	uint8_t *out_stream2 = nullptr;
	int len,head_len = 0;

	unsigned int out_len,out_len2;
	entropy_to_stream_bit(0,0,1,1,rblock[1],para,&tmp_stream,8);
	entropy_from_stream_bit(0,0,1,1,rblock[0],para,tmp_stream,8);

	return;

	for(int j = 0;j<10;++j)
	{
		pkt.block_head2stream(para,&tmp_stream,rblock[j],&len);
		
		memcpy(point,tmp_stream,len);
		point += len;
		head_len += len;
	}
	huffman_encode_memory(stream,head_len,&out_stream,&out_len);

	huffman_decode_memory(out_stream,out_len,&out_stream2,&out_len2);
	point = out_stream2;
	for(int j = 0;j<10;++j)
	{
		fromch4(len,point);
		point += 4;

		pkt.block_stream2head(para,point,rblock2[j],len,&head_len);

		point += head_len;
	}

	return;
}

void zag_zig_test()
{
	AVFormat para;
	para.block_num = 10;
	para.height = 1280;
	para.width = 720;

	para.block_height = 16;
	para.block_width = 16;

	PKT pkt;
	pkt.init(para);
	ResidualBlock rblock = ResidualBlock(Block::Y,16,16);
	ResidualBlock rblock2= ResidualBlock(Block::Y,16,16);
	uint8_t tmp[1000];
	memset(tmp,0,1000);

	rblock.block_type = Block::Y;
	for(int i = 0 ; i< 16 ;++i)
	{
		for(int j = 0;j<16;++j)
		{
			rblock.data[i * 16 + j] = j%4 + i * 4;
		}
	}

	rblock.data[3*16+3] = 0;

	int zero = zag_zig(rblock,para,tmp);

	unzag_zig(rblock2,para,tmp,zero);

	return;
}