#pragma once

#include <basis/seadTypes.h>
#include <heap/seadHeap.h>
#include <prim/seadSafeString.h>
#include <container/seadFreeList.h>

namespace sead
{
class Thread;
class WriteStream;

namespace hostio
{
class Context;
}  // namespace hostio

struct UnitHeapBlock
{
    UnitHeapBlock* next;
};

class UnitHeap : public Heap
{
    SEAD_RTTI_OVERRIDE(UnitHeap, Heap)
public:
    UnitHeap(const SafeString& name, Heap* parent, void* address, size_t size, u32 blockSize,
             bool isSomething);
    ~UnitHeap() override;

    void destroy() override;
    size_t adjust() override;
    void* tryAlloc(size_t size, s32 alignment) override;
    void free(void* ptr) override;
    void* resizeFront(void*, size_t) override;
    void* resizeBack(void*, size_t) override;
    void freeAll() override;
    uintptr_t getStartAddress() const override;
    uintptr_t getEndAddress() const override;
    size_t getSize() const override;
    size_t getFreeSize() const override;
    size_t getMaxAllocatableSize(s32 alignment) const override;
    bool isInclude(const void*) const override;
    bool isEmpty() const override;
    bool isFreeable() const override;
    bool isResizable() const override;
    bool isAdjustable() const override;
    void dump() const override;
    void dumpYAML(WriteStream& stream, s32) const override;

    void doCreate(s32, bool, sead::Heap* heap);

    static UnitHeap* tryCreate(u64 size, const SafeString& name, u32 block_size, s32 alignment,
                               Heap* parent, bool enable_lock);
    static UnitHeap* tryCreateWithBlockNum(u32 block_size, u32 num, const SafeString& name,
                                           s32 alignment, Heap* parent, bool enable_lock);
    static size_t getManagementAreaSize(s32 size);

private:
    void genInformation_(hostio::Context*) override;

    u32 mBlockSize;
    UnitHeapBlock* mBlocks = nullptr;
    size_t mBlocksSize = 0;
    size_t mFreeSize = 0;
    FreeList mFreeList;
};

}  // namespace sead
