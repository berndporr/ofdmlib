/**
* @file block-ring-buffer.cpp
* @author Kamil Rog
*
*/


#include "block-ring-buffer.h"


BlockRingBuffer::BlockRingBuffer(size_t blockSize, size_t nBlocks) :
    m_BlockSize(blockSize),
    m_nBlocks(nBlocks),
    m_BlockRingBufferDataPostion(0)
{
    // Allocate memory for the buffer
    m_BlockRingBuffer = (double*) calloc(m_nBlocks*m_BlockSize, sizeof(double));
}

/**
* Default Destructor 
*
*/
BlockRingBuffer::~BlockRingBuffer()
{
    free(m_BlockRingBuffer);
}