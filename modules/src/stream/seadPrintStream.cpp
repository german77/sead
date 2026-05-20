#include "stream/seadPrintStream.h"

namespace sead
{

u32 PrintStreamSrc::read([[maybe_unused]] void* data, [[maybe_unused]] u32 size)
{
    return 0;
}

u32 PrintStreamSrc::write([[maybe_unused]] const void* data, u32 size)
{
    return size;
}

u32 PrintStreamSrc::skip([[maybe_unused]] s32 offset)
{
    return 0;
}

void PrintStreamSrc::rewind() {}

bool PrintStreamSrc::isEOF()
{
    return false;
}

PrintWriteStream::PrintWriteStream(Modes mode) : mSrc(&PrintStreamSrc::sPrintStreamSrc, mBuffer, 0x7f)
{
    setSrc(&mSrc);
    setMode(mode);
}

PrintWriteStream::PrintWriteStream(StreamFormat* format) : mSrc(&PrintStreamSrc::sPrintStreamSrc, mBuffer, 0x7f)
{
    setSrc(&mSrc);
    setUserFormat(format);
}

PrintWriteStream::~PrintWriteStream()
{
    flush();
    setSrc(nullptr);
}
}  // namespace sead
