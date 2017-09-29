/*
 * memcache.cpp
 *
 *  Created on: 26 sept. 2017
 *      Author: Nico
 */

class MemCache
{
 void ***Bucket;
 uint32_t *BucketPtr;
public:
//! Constructeur
 MemCache();
 virtual ~MemCache();
 void Reset();
 void *AllocateMemory(size_t Size);
 void FreeMemory(void *Memory);
 void *ResizeMemory(void *Old,size_t Size);
 virtual size_t GetSize(void *Memory);
 virtual void Reclaim();
 virtual void Cleanup();
 virtual void *MasterAllocateMemory(size_t Size)=0;
 virtual void MasterFreeMemory(void *Memory)=0;
 virtual size_t MasterGetSize(void *Memory)=0;
};

typedef uint32_t ulong;
void MemCache::Reset()
{
 Bucket=(void ***) MasterAllocateMemory(sizeof(void **)*16);
 BucketPtr=(ulong *) MasterAllocateMemory(sizeof(ulong)*16);
 memset(BucketPtr,0,sizeof(ulong)*16);
 int i;
 for (i=0;i<16;i++)
 {
  Bucket[i]=(void **) MasterAllocateMemory(sizeof(void *)*32);
  BucketPtr[i]=0;
 }
}

void *MemCache::AllocateMemory(size_t Size)
{
 if (!Size) Size++;
 Size=((Size+15)>>4)-1;
 if (Size>=16) return MasterAllocateMemory((Size+1)<<4);
 void *ret;
 if (!BucketPtr[Size])
  ret=MasterAllocateMemory(((Size+1)<<4));
 else
 {
  BucketPtr[Size]--;
  ret=Bucket[Size][BucketPtr[Size]];
 }
 return ret;
}

void MemCache::FreeMemory(void *Memory)
{
 size_t size=MasterGetSize(Memory);
 if (!size) size++;
 size=((size+15)>>4)-1;
 if ((size>=16)||(!Memory))
 {
  MasterFreeMemory(Memory);
  return;
 }
 if (BucketPtr[size]<32)
 {
  Bucket[size][BucketPtr[size]]=Memory;
  BucketPtr[size]++;
 }
 else
  MasterFreeMemory(Memory);
}

void *MemCache::ResizeMemory(void *Old,size_t Size)
{
 size_t oldsize=MasterGetSize(Old);
 if ((oldsize>=Size)&&((oldsize-Size)<1024)) return Old; // Don't bother freeing unsued mem
 void *n=AllocateMemory(Size);
 if (!n) return NULL;
 memcpy(n,Old,oldsize);
 FreeMemory(Old);
 return n;
}

void MemCache::Reclaim()
{
 int i;
 for (i=0;i<16;i++)
 {
  while (BucketPtr[i]--)
   MasterFreeMemory(Bucket[i][BucketPtr[i]]);
  BucketPtr[i]=0;
 }
}

void MemCache::Cleanup()
{
 int i;
 Reclaim();
 for (i=0;i<16;i++)
  MasterFreeMemory(Bucket[i]);
 MasterFreeMemory(Bucket);
 MasterFreeMemory(BucketPtr);
}

size_t MemCache::GetSize(void *Memory)
{
 return MasterGetSize(Memory);
}

MemCache::MemCache()
{
}

MemCache::~MemCache()
{
	Cleanup();
}
