/**
* @file block-ring-buffer.h
* @author Kamil Rog
*
* 
*/
#ifndef BLOCK_RING_BUFFER_H
#define BLOCK_RING_BUFFER_H

#include <string>
#include <unistd.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <iostream>
#include <math.h>
#include <fftw3.h>
#include <cstring>


#include <cstddef>



/**
 * @brief Block based ring buffer
 * 
 */
class BlockRingBuffer {

public:

	/**
	* Constructor
	* 
	* @param blockSize number of elements in the block
	* @param nBlocks number of block the buffer holds
	*
	*/
	BlockRingBuffer(size_t blockSize, size_t nBlocks);

	/**
	* Destructor 
	*
	*/
	~BlockRingBuffer();


	double InsertData(const double *input, size_t Offset);

private:

    size_t m_BlockSize;
	size_t m_nBlocks;
	double *m_BlockRingBuffer;
	// Variables keeping track of the next position the data can be pasted in
	size_t m_BlockRingBufferDataPostion; 
};

#endif
