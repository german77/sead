#include <heap/seadUnitHeap.h>

#include <heap/seadHeapMgr.h>
#include <prim/seadScopedLock.h>

namespace sead
{
UnitHeap::UnitHeap(const SafeString& name, Heap* parent, void* address, size_t size, u32 blockSize,
                   bool isSomething)
    : Heap(name, parent, address, size, Heap::cHeapDirection_Forward, isSomething),
      mBlockSize(blockSize)
{
}

UnitHeap::~UnitHeap()
{
    Heap::destruct_();
}

void UnitHeap::destroy()
{
    Heap* parent = mParent;
    void* start = mStart;
    this->~UnitHeap();

    if (parent && parent->isFreeable())
    {
        parent->free(start);
    }
}

size_t UnitHeap::adjust()
{
    return mSize;
}

inline void invokeAllocFailedCallback(Heap* heap, HeapMgr* heapMgr, size_t size, s32 alignment,
                                      size_t alloc_size)
{
    if (heapMgr)
    {
        HeapMgr::IAllocFailedCallback* callback = heapMgr->getAllocFailedCallback();
        if (callback)
        {
            HeapMgr::AllocFailedCallbackArg arg{.heap = heap,
                                                .request_size = size,
                                                .request_alignment = alignment,
                                                .alloc_size = alloc_size,
                                                .alloc_alignment = alignment};
            callback->operator()(&arg);
        }
    }
}

inline bool isAligned(size_t size, size_t alignment)
{
    return (size & (alignment - 1)) == 0;
}

void* UnitHeap::tryAlloc(size_t size, s32 alignment)
{
    HeapMgr* heapMgr = HeapMgr::instance();
    if (alignment < 0 || !isAligned(mBlockSize, alignment) || mBlockSize < size)
    {
        invokeAllocFailedCallback(this, heapMgr, size, alignment, size);
        return nullptr;
    }

    ConditionalScopedLock<CriticalSection> lock(&mCS, isLockEnabled());
    void* block = mFreeList.alloc();

    if (block)
    {
        mFreeSize -= mBlockSize;
        return block;
    }

    invokeAllocFailedCallback(this, heapMgr, size, alignment, mBlockSize);
    return nullptr;
}

void UnitHeap::free(void* ptr)
{
    if (!ptr || !isInclude(ptr) || isDisposing())
        return;

    ConditionalScopedLock<CriticalSection> lock(&mCS, isLockEnabled());

    mFreeList.free(ptr);
    mFreeSize += mBlockSize;
}

void* UnitHeap::resizeFront(void*, size_t)
{
    return nullptr;
}

void* UnitHeap::resizeBack(void*, size_t)
{
    return nullptr;
}

inline u32 getElements(size_t bufferSize, size_t elementSize)
{
    return bufferSize / elementSize;
}

void UnitHeap::freeAll()
{
    ConditionalScopedLock<CriticalSection> lock(&mCS, isLockEnabled());
    Heap::dispose_(nullptr, nullptr);

    u32 blockSize = mBlockSize;
    u32 elements = getElements(mBlocksSize, mBlockSize);
    size_t blockSized = (s32)mBlockSize < 0 ? mBlockSize + 7 : mBlockSize;

    mFreeSize = elements * blockSize;
    mFreeList.setWork(mBlocks, blockSized, elements);
}

uintptr_t UnitHeap::getStartAddress() const
{
    return (uintptr_t)mStart;
}

uintptr_t UnitHeap::getEndAddress() const
{
    return (uintptr_t)PtrUtil::addOffset(mStart, mSize);
}

size_t UnitHeap::getSize() const
{
    return mSize;
}

size_t UnitHeap::getFreeSize() const
{
    return mFreeSize;
}

size_t UnitHeap::getMaxAllocatableSize(int alignment) const
{
    if (!mFreeList.getFree())
        return 0;
    return mBlockSize;
}

bool UnitHeap::isInclude(const void* ptr) const
{
    return PtrUtil::isInclude(ptr, mBlocks, PtrUtil::addOffset(mBlocks, mBlocksSize));
}

bool UnitHeap::isEmpty() const
{
    return mFreeSize == getElements(mBlocksSize, mBlockSize) * mBlockSize;
}

bool UnitHeap::isFreeable() const
{
    return true;
}

bool UnitHeap::isResizable() const
{
    return false;
}

bool UnitHeap::isAdjustable() const
{
    return false;
}

void UnitHeap::dump() const
{
    ConditionalScopedLock<CriticalSection> lock(const_cast<CriticalSection*>(&mCS),
                                                isLockEnabled());
    Heap::dump();
}

void UnitHeap::dumpYAML(WriteStream& stream, s32) const {}

void UnitHeap::doCreate(s32 alignment, bool isNan, sead::Heap* parent)
{
    ConditionalScopedLock<CriticalSection> lock(const_cast<CriticalSection*>(&mCS),
                                                isLockEnabled());

    u32 u_alignment = static_cast<u32>(alignment);
    u8* work_ptr = reinterpret_cast<u8*>(this) + sizeof(UnitHeap);

    u8* aligned_ptr =
        (u8*)(((uintptr_t)work_ptr + u_alignment - 1) & ~(uintptr_t)(u_alignment - 1));

    if (isNan && aligned_ptr == work_ptr)
        aligned_ptr += u_alignment;

    mBlockSize = (mBlockSize + u_alignment - 1) & -u_alignment;
    size_t suzer = mBlockSize + 7;
    if (true)
    {
        suzer = mBlockSize;
    }

    mBlocks = reinterpret_cast<UnitHeapBlock*>(aligned_ptr);
    mBlocksSize = (uintptr_t)this + mSize - (uintptr_t)mBlocks;

    u32 num = mBlocksSize / mBlockSize;
    mFreeSize = mBlockSize * num;
    mFreeList.setWork(mBlocks, suzer, num);

    parent->pushBackChild_(this);
}

UnitHeap* UnitHeap::tryCreate(u64 size, const SafeString& name, u32 block_size, s32 alignment,
                              Heap* parent, bool enableLock)
{
}

UnitHeap* UnitHeap::tryCreateWithBlockNum(u32 blockSize, u32 num, const SafeString& name,
                                          s32 alignment, Heap* parent, bool enableLock)
{
    UnitHeap* heap = nullptr;

    u32 align = alignment > -1 ? alignment : -alignment;
    if (blockSize == 0 || num == 0)
        return nullptr;

    u32 aligned = align > 7 ? 8 : align;

    if (!parent)
        parent = HeapMgr::instance()->getCurrentHeap();

    if (parent)
    {
        heap = new UnitHeap(name, parent, nullptr, 32, 32, enableLock);
        heap->doCreate(aligned, true, parent);
    }

    return heap;
}

size_t UnitHeap::getManagementAreaSize(s32 size)
{
    return size + sizeof(UnitHeap);
}

void UnitHeap::genInformation_(hostio::Context*) {}

}  // namespace sead
