#pragma once

#include "basis/seadTypes.h"
#include "stream/seadBufferStream.h"
#include "stream/seadStreamSrc.h"

namespace sead
{

class PrintStreamSrc : public StreamSrc
{
public:
    PrintStreamSrc();

    u32 read(void* data, u32 size) override;
    u32 write(const void* data, u32 size) override;
    u32 skip(s32 offset) override;
    void rewind() override;
    bool isEOF() override;

    static PrintStreamSrc sPrintStreamSrc;
};

class PrintWriteStream : public WriteStream
{
public:
    PrintWriteStream(Modes mode);
    PrintWriteStream(StreamFormat* format);
    ~PrintWriteStream() override;

private:
    BufferMultiByteNullTerminatedTextWriteStreamSrc mSrc;
    u8* mBuffer[0x80];
};

}  // namespace sead
