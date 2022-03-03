#include "rational.hpp"

mini_util::ChunkMemoryPool *Rational::memPool = 0;

void *Rational::operator new(size_t size) 
{
    return memPool->alloc(size); 
}

void Rational::operator delete(void *doomed, size_t size)
{
    memPool->free(doomed);
}

void Rational::newMemPool() 
{
    memPool = new mini_util::ChunkMemoryPool(sizeof(Rational), 64); 
}

void Rational::deleteMemPool() 
{
    delete memPool; 
}

