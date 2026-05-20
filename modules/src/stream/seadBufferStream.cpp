#include "stream/seadBufferStream.h"

#include "math/seadMathCalcCommon.h"

namespace sead
{
BufferReadStreamSrc::BufferReadStreamSrc(StreamSrc* src, void* buffer, u32 buffer_size)
    : mSrc(src), mBuffer((u8*)buffer), mBufferSize(buffer_size)
{
}

BufferReadStreamSrc::~BufferReadStreamSrc() = default;

// NOTE: cannot take negative `offset`, but expects `mSrc->skip(X)` to work with negatives
u32 BufferReadStreamSrc::read(void* data, u32 size)
{
    u32 totalBytesRead = 0;
    while (true)
    {
        if (mCurrentPos < mCurrentSize)
        {
            u32 readSize = sead::Mathu::clampMax(size - totalBytesRead, mCurrentSize - mCurrentPos);

            MemUtil::copy((u8*)data + totalBytesRead, mBuffer + mCurrentPos, readSize);
            totalBytesRead += readSize;
            mCurrentPos += readSize;
        }

        if (size <= totalBytesRead)
            break;

        mCurrentSize = mSrc->read(mBuffer, mBufferSize);
        mCurrentPos = 0;

        if (mCurrentSize == 0)
            break;
    }
    return totalBytesRead;
}

u32 BufferReadStreamSrc::write([[maybe_unused]] const void* data, [[maybe_unused]] u32 size)
{
    return 0;
}

u32 BufferReadStreamSrc::skip(s32 offset)
{
    s32 remainingBytes = mCurrentSize - mCurrentPos;

    if (remainingBytes >= offset)
    {
        mCurrentPos += offset;
        return offset;
    }

    mCurrentSize = 0;
    mCurrentPos = 0;
    return mSrc->skip(offset - remainingBytes) + remainingBytes;
}

void BufferReadStreamSrc::rewind()
{
    mSrc->rewind();
    mCurrentSize = 0;
    mCurrentPos = 0;
}

bool BufferReadStreamSrc::isEOF()
{
    return mSrc->isEOF() && mCurrentPos >= mCurrentSize;
}

BufferReadStream::BufferReadStream(ReadStream* stream, const void* buffer, u32 buffer_size)
    : mSrc(stream->getSrc(), const_cast<void*>(buffer), buffer_size)
{
    setSrc(&mSrc);
    setUserFormat(stream->getUserFormat());
    setBinaryEndian(stream->getBinaryEndian());
}

BufferReadStream::~BufferReadStream()
{
    setSrc(nullptr);
}

BufferWriteStreamSrc::BufferWriteStreamSrc(StreamSrc* src, void* buffer, u32 buffer_size)
    : mSrc(src), mBuffer((u8*)buffer), mBufferSize(buffer_size)
{
}

BufferWriteStreamSrc::~BufferWriteStreamSrc() = default;

u32 BufferWriteStreamSrc::read([[maybe_unused]] void* data, [[maybe_unused]] u32 size)
{
    return 0;
}

u32 BufferWriteStreamSrc::write(const void* data, u32 size)
{
    u32 totalBytesWritten = 0;
    do
    {
        if (mCurrentPos >= mBufferSize)
            continue;

        u32 writeSize = sead::Mathu::min(mBufferSize - mCurrentPos, size - totalBytesWritten);

        MemUtil::copy(mBuffer + mCurrentPos, (u8*)data + totalBytesWritten, writeSize);
        totalBytesWritten += writeSize;
        mCurrentPos += writeSize;
    } while (totalBytesWritten < size && flush());

    return totalBytesWritten;
}

u32 BufferWriteStreamSrc::skip([[maybe_unused]] s32 offset)
{
    return 0;
}

void BufferWriteStreamSrc::rewind()
{
    flush();
    mSrc->rewind();
}

bool BufferWriteStreamSrc::flush()
{
    if (mCurrentPos == 0)
        return true;

    bool success = mSrc->write(mBuffer, mCurrentPos) >= mCurrentPos;
    mCurrentPos = 0;
    return success;
}

BufferWriteStream::BufferWriteStream(WriteStream* stream, void* buffer, u32 buffer_size)
    : mSrc(stream->getSrc(), buffer, buffer_size)
{
    setSrc(&mSrc);
    setUserFormat(stream->getUserFormat());
    setBinaryEndian(stream->getBinaryEndian());
}

BufferWriteStream::~BufferWriteStream()
{
    flush();
    setSrc(nullptr);
}

BufferMultiByteTextWriteStreamSrc::BufferMultiByteTextWriteStreamSrc(StreamSrc* src, void* buffer,
                                                                     u32 buffer_size)
    : BufferWriteStreamSrc(src, buffer, buffer_size)
{
}

u32 getWideCharLength(u8 b)
{
    if ((b & 0xE0) == 0xC0)
        return 2;
    if ((b & 0xF0) == 0xE0)
        return 3;
    if ((b & 0xF8) == 0xF0)
        return 4;
    return 0;
}

u32 BufferMultiByteTextWriteStreamSrc::write(const void* data, u32 size)
{
    const u8* pData = static_cast<const u8*>(data);
    u32 totalBytesWritten = 0;

    do
    {
        if (mCurrentPos >= mBufferSize)
            continue;

        u32 available = mBufferSize - mCurrentPos;
        u32 remaining = size - totalBytesWritten;
        u32 writeSize = available;

        if (available < remaining)
        {
            u8 lastByte = pData[totalBytesWritten + available - 1];

            if ((lastByte & 0x80) != 0)
            {
                s32 backtrack = 0;
                if ((lastByte & 0xC0) != 0x80)
                {
                    backtrack = 1;
                }
                else
                {
                    s32 searchLimit = sead::Mathi::min(available, 4);
                    for (s32 i = 1; i < searchLimit; ++i)
                    {
                        u8 b = pData[totalBytesWritten + available - 1 - i];
                        if ((b & 0xC0) != 0x80)
                        {
                            u32 expectedLen = getWideCharLength(b);
                            if (i + 1 < expectedLen)
                                backtrack = i + 1;
                            break;
                        }
                    }
                }
                if (backtrack != 0)
                {
                    mBuffer[mBufferSize - backtrack] = 0;
                    writeSize = available - backtrack;
                }
            }
        }
        else
        {
            writeSize = remaining;
        }

        MemUtil::copy(mBuffer + mCurrentPos, pData + totalBytesWritten, writeSize);
        totalBytesWritten += writeSize;
        mCurrentPos += writeSize;
    } while (totalBytesWritten < size && flush());

    return totalBytesWritten;
}

bool BufferMultiByteNullTerminatedTextWriteStreamSrc::flush()
{
    mBuffer[mCurrentPos] = 0;
    return BufferWriteStreamSrc::flush();
}

BufferMultiByteTextWriteStream::BufferMultiByteTextWriteStream(WriteStream* stream, void* buffer,
                                                               u32 buffer_size)
    : mSrc(stream->getSrc(), buffer, buffer_size)
{
    setSrc(&mSrc);
    setUserFormat(stream->getUserFormat());
    setBinaryEndian(stream->getBinaryEndian());
}

BufferMultiByteTextWriteStream::~BufferMultiByteTextWriteStream()
{
    flush();
    setSrc(nullptr);
}

}  // namespace sead
