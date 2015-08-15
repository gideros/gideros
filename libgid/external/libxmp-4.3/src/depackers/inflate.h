#ifndef XMP_INFLATE_H
#define XMP_INFLATE_H

struct inflate_data {
	struct huffman_tree_t *huffman_tree_len_static;
};

int inflate(FILE *, FILE *, uint32 *, int);

#endif
