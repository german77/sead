#include "stream/seadStreamFormat.h"

#include "codec/seadBase64.h"
#include "math/seadMathCalcCommon.h"
#include "prim/seadStringUtil.h"
#include "stream/seadStreamSrc.h"
#include "thread/seadMutex.h"

namespace sead
{

static FixedSafeString<128> sTextData;
static Mutex sMutex;

u8 BinaryStreamFormat::readU8(StreamSrc* src, Endian::Types endian)
{
    u8 rawValue = 0;
    src->read(&rawValue, sizeof(u8));
    return Endian::toHostU8(endian, rawValue);
}

u16 BinaryStreamFormat::readU16(StreamSrc* src, Endian::Types endian)
{
    u16 rawValue = 0;
    src->read(&rawValue, sizeof(u16));
    return Endian::toHostU16(endian, rawValue);
}

u32 BinaryStreamFormat::readU32(StreamSrc* src, Endian::Types endian)
{
    u32 rawValue = 0;
    src->read(&rawValue, sizeof(u32));
    return Endian::toHostU32(endian, rawValue);
}

u64 BinaryStreamFormat::readU64(StreamSrc* src, Endian::Types endian)
{
    u64 rawValue = 0;
    src->read(&rawValue, sizeof(u64));
    return Endian::toHostU64(endian, rawValue);
}

s8 BinaryStreamFormat::readS8(StreamSrc* src, Endian::Types endian)
{
    s8 rawValue = 0;
    src->read(&rawValue, sizeof(s8));
    return Endian::toHostU8(endian, rawValue);
}

s16 BinaryStreamFormat::readS16(StreamSrc* src, Endian::Types endian)
{
    s16 rawValue = 0;
    src->read(&rawValue, sizeof(s16));
    return Endian::toHostU16(endian, rawValue);
}

s32 BinaryStreamFormat::readS32(StreamSrc* src, Endian::Types endian)
{
    s32 rawValue = 0;
    src->read(&rawValue, sizeof(s32));
    return Endian::toHostU32(endian, rawValue);
}

s64 BinaryStreamFormat::readS64(StreamSrc* src, Endian::Types endian)
{
    s64 rawValue = 0;
    src->read(&rawValue, sizeof(s64));
    return Endian::toHostU64(endian, rawValue);
}

// NON_MATCHING: Wrong loading order https://decomp.me/scratch/L5MO9
f32 BinaryStreamFormat::readF32(StreamSrc* src, Endian::Types endian)
{
    u32 rawValue = 0;
    src->read(&rawValue, sizeof(f32));
    return Endian::toHostF32(endian, &rawValue);
}

void BinaryStreamFormat::readBit(StreamSrc* src, void* data, u32 bits)
{
    u8* dataU8 = static_cast<u8*>(data);

    u32 size = bits / 8;
    src->read(dataU8, size);
    bits -= size * 8;

    if (bits <= 0)
        return;

    u8 lastByte;
    src->read(&lastByte, 1);

    u8 mask = 0xFF << bits;
    dataU8[size] &= mask;
    dataU8[size] |= lastByte & ~mask;
}

// NON_MATCHING: Potential wrong imlementation https://decomp.me/scratch/jCek1
void BinaryStreamFormat::readString(StreamSrc* src, BufferedSafeString* str, u32 size)
{
    u32 remainingSize = 0;
    if (size > (u32)str->getBufferSize())
    {
        remainingSize = size - str->getBufferSize();
        size = str->getBufferSize();
    }

    src->read(str->getBuffer(), size);

    if (size + 1 < (u32)str->getBufferSize())
        str->trim(size);
    else
        str->trim(str->getBufferSize() - 1);

    if (remainingSize != 0)
    {
        // Note: what happens if remaining size is bigger than the buffer?
        src->read(str->getBuffer(), remainingSize);
    }
}

u32 BinaryStreamFormat::readMemBlock(StreamSrc* src, void* buffer, u32 size)
{
    return src->read(buffer, size);
}

void BinaryStreamFormat::writeU8(StreamSrc* src, Endian::Types endian, u8 value)
{
    u8 rawValue = Endian::fromHostU8(endian, value);
    src->write(&rawValue, sizeof(u8));
}

void BinaryStreamFormat::writeU16(StreamSrc* src, Endian::Types endian, u16 value)
{
    u16 rawValue = Endian::fromHostU16(endian, value);
    src->write(&rawValue, sizeof(u16));
}

void BinaryStreamFormat::writeU32(StreamSrc* src, Endian::Types endian, u32 value)
{
    u32 rawValue = Endian::fromHostU32(endian, value);
    src->write(&rawValue, sizeof(u32));
}

void BinaryStreamFormat::writeU64(StreamSrc* src, Endian::Types endian, u64 value)
{
    u64 rawValue = Endian::fromHostU64(endian, value);
    src->write(&rawValue, sizeof(u64));
}

void BinaryStreamFormat::writeS8(StreamSrc* src, Endian::Types endian, s8 value)
{
    s8 rawValue = Endian::fromHostS8(endian, value);
    src->write(&rawValue, sizeof(s8));
}

void BinaryStreamFormat::writeS16(StreamSrc* src, Endian::Types endian, s16 value)
{
    s16 rawValue = Endian::fromHostS16(endian, value);
    src->write(&rawValue, sizeof(s16));
}

void BinaryStreamFormat::writeS32(StreamSrc* src, Endian::Types endian, s32 value)
{
    s32 rawValue = Endian::fromHostS32(endian, value);
    src->write(&rawValue, sizeof(s32));
}

void BinaryStreamFormat::writeS64(StreamSrc* src, Endian::Types endian, s64 value)
{
    s64 rawValue = Endian::fromHostS64(endian, value);
    src->write(&rawValue, sizeof(s64));
}

void BinaryStreamFormat::writeF32(StreamSrc* src, Endian::Types endian, f32 value)
{
    u32 rawValue = Endian::fromHostF32(endian, &value);
    src->write(&rawValue, sizeof(f32));
}

void BinaryStreamFormat::writeBit(StreamSrc* src, const void* data, u32 bits)
{
    const u8* dataU8 = static_cast<const u8*>(data);

    u8 size = bits / 8;
    src->write(dataU8, size);

    if (size * 8 == bits)
        return;

    const u8& lastByte = dataU8[size];
    src->write(&lastByte, 1);
}

void BinaryStreamFormat::writeString(StreamSrc* src, const SafeString& str, u32 size)
{
    u32 strSize = str.calcLength();
    if (strSize > size)
        strSize = size;

    src->write(str.cstr(), strSize);

    char nullchar = '\0';
    for (; strSize < size; strSize++)
        src->write(&nullchar, 1);
}

void BinaryStreamFormat::writeMemBlock(StreamSrc* src, const void* buffer, u32 size)
{
    src->write(buffer, size);
}

void BinaryStreamFormat::writeDecorationText([[maybe_unused]] StreamSrc* src,
                                             [[maybe_unused]] const SafeString& text)
{
}

void BinaryStreamFormat::writeNullChar([[maybe_unused]] StreamSrc* src) {}

void BinaryStreamFormat::skip(StreamSrc* src, u32 offset)
{
    src->skip(offset);
}

void BinaryStreamFormat::flush([[maybe_unused]] StreamSrc* src) {}

void BinaryStreamFormat::rewind(StreamSrc* src)
{
    src->rewind();
}

TextStreamFormat::TextStreamFormat() : mEntryTerminator(" \t\r\n") {}

u8 TextStreamFormat::readU8(StreamSrc* src, [[maybe_unused]] Endian::Types endian)
{
    sMutex.lock();
    u8 tmpValue = 0;
    getNextData_(src);
    StringUtil::tryParseU8(&tmpValue, sTextData.cstr(), StringUtil::CardinalNumber::BaseAuto);
    u8 value = tmpValue;
    sMutex.unlock();
    return value;
}

u16 TextStreamFormat::readU16(StreamSrc* src, [[maybe_unused]] Endian::Types endian)
{
    sMutex.lock();
    u16 tmpValue = 0;
    getNextData_(src);
    StringUtil::tryParseU16(&tmpValue, sTextData.cstr(), StringUtil::CardinalNumber::BaseAuto);
    u16 value = tmpValue;
    sMutex.unlock();
    return value;
}

u32 TextStreamFormat::readU32(StreamSrc* src, [[maybe_unused]] Endian::Types endian)
{
    sMutex.lock();
    u32 tmpValue = 0;
    getNextData_(src);
    StringUtil::tryParseU32(&tmpValue, sTextData.cstr(), StringUtil::CardinalNumber::BaseAuto);
    u32 value = tmpValue;
    sMutex.unlock();
    return value;
}

u64 TextStreamFormat::readU64(StreamSrc* src, [[maybe_unused]] Endian::Types endian)
{
    sMutex.lock();
    u64 tmpValue = 0;
    getNextData_(src);
    StringUtil::tryParseU64(&tmpValue, sTextData.cstr(), StringUtil::CardinalNumber::BaseAuto);
    u64 value = tmpValue;
    sMutex.unlock();
    return value;
}

s8 TextStreamFormat::readS8(StreamSrc* src, [[maybe_unused]] Endian::Types endian)
{
    sMutex.lock();
    s8 tmpValue = 0;
    getNextData_(src);
    StringUtil::tryParseS8(&tmpValue, sTextData.cstr(), StringUtil::CardinalNumber::BaseAuto);
    s8 value = tmpValue;
    sMutex.unlock();
    return value;
}

s16 TextStreamFormat::readS16(StreamSrc* src, [[maybe_unused]] Endian::Types endian)
{
    sMutex.lock();
    s16 tmpValue = 0;
    getNextData_(src);
    StringUtil::tryParseS16(&tmpValue, sTextData.cstr(), StringUtil::CardinalNumber::BaseAuto);
    s16 value = tmpValue;
    sMutex.unlock();
    return value;
}

s32 TextStreamFormat::readS32(StreamSrc* src, [[maybe_unused]] Endian::Types endian)
{
    sMutex.lock();
    s32 tmpValue = 0;
    getNextData_(src);
    StringUtil::tryParseS32(&tmpValue, sTextData.cstr(), StringUtil::CardinalNumber::BaseAuto);
    s32 value = tmpValue;
    sMutex.unlock();
    return value;
}

s64 TextStreamFormat::readS64(StreamSrc* src, [[maybe_unused]] Endian::Types endian)
{
    sMutex.lock();
    s64 tmpValue = 0;
    getNextData_(src);
    StringUtil::tryParseS64(&tmpValue, sTextData.cstr(), StringUtil::CardinalNumber::BaseAuto);
    s64 value = tmpValue;
    sMutex.unlock();
    return value;
}

f32 TextStreamFormat::readF32(StreamSrc* src, [[maybe_unused]] Endian::Types endian)
{
    sMutex.lock();
    f32 tmpValue = 0.0f;
    getNextData_(src);

    if (sTextData.calcLength() != 0)
    {
        std::sscanf(sTextData.cstr(), "%f", &tmpValue);
    }

    f32 value = tmpValue;
    sMutex.unlock();
    return value;
}

void TextStreamFormat::readBit(StreamSrc* src, void* data, u32 bits)
{
    sMutex.lock();
    getNextData_(src);

    if (sTextData.startsWith("0b"))
    {
        SafeString bitStr = sTextData.getPart(2);
        u8* dest = static_cast<u8*>(data);
        u8 currentByte = 0;
        u32 bitCount = 0;

        for (; bitCount < bits; ++bitCount)
        {
            u8 bit = (bitStr[bitCount] == '1') ? 1 : 0;

            currentByte = (currentByte << 1) | bit;

            if ((bitCount + 1) % 8 == 0)
            {
                dest[bitCount / 8] = currentByte;
                currentByte = 0;
            }
        }

        u32 remainder = bitCount % 8;
        if (remainder != 0)
        {
            u32 byteIdx = bitCount / 8;
            u8 mask = 0xFF << remainder;
            dest[byteIdx] = (dest[byteIdx] & mask) | currentByte;
        }
    }

    sMutex.unlock();
}

void TextStreamFormat::readString(StreamSrc* src, BufferedSafeString* str,
                                  [[maybe_unused]] u32 size)
{
    sMutex.lock();
    getNextData_(src);
    str->copy(sTextData);

    sMutex.unlock();
}

u32 TextStreamFormat::readMemBlock(StreamSrc* src, void* buffer, u32 size)
{
    sMutex.lock();
    getNextData_(src);
    u32 textSize = sTextData.calcLength();

    u64 readSize = 0;
    u64 tmpReadSize = 0;
    Base64::decode(buffer, size, sTextData.cstr(), textSize, &tmpReadSize);
    readSize = tmpReadSize;

    sMutex.unlock();
    return readSize;
}

void TextStreamFormat::writeU8(StreamSrc* src, [[maybe_unused]] Endian::Types endian, u8 value)
{
    sead::FixedSafeString<32> tmp;
    tmp.format("%u", value);
    u32 length = tmp.calcLength();

    src->write(tmp.cstr(), length);
    src->write(mEntryTerminator.cstr(), 1);
}

void TextStreamFormat::writeU16(StreamSrc* src, [[maybe_unused]] Endian::Types endian, u16 value)
{
    sead::FixedSafeString<32> tmp;
    tmp.format("%u", value);
    u32 length = tmp.calcLength();

    src->write(tmp.cstr(), length);
    src->write(mEntryTerminator.cstr(), 1);
}

void TextStreamFormat::writeU32(StreamSrc* src, [[maybe_unused]] Endian::Types endian, u32 value)
{
    sead::FixedSafeString<32> tmp;
    tmp.format("%u", value);
    u32 length = tmp.calcLength();

    src->write(tmp.cstr(), length);
    src->write(mEntryTerminator.cstr(), 1);
}

void TextStreamFormat::writeU64(StreamSrc* src, [[maybe_unused]] Endian::Types endian, u64 value)
{
    sead::FixedSafeString<32> tmp;
    tmp.format("%llu", value);
    u32 length = tmp.calcLength();

    src->write(tmp.cstr(), length);
    src->write(mEntryTerminator.cstr(), 1);
}

void TextStreamFormat::writeS8(StreamSrc* src, [[maybe_unused]] Endian::Types endian, s8 value)
{
    sead::FixedSafeString<32> tmp;
    tmp.format("%d", value);
    u32 length = tmp.calcLength();

    src->write(tmp.cstr(), length);
    src->write(mEntryTerminator.cstr(), 1);
}

void TextStreamFormat::writeS16(StreamSrc* src, [[maybe_unused]] Endian::Types endian, s16 value)
{
    sead::FixedSafeString<32> tmp;
    tmp.format("%d", value);
    u32 length = tmp.calcLength();

    src->write(tmp.cstr(), length);
    src->write(mEntryTerminator.cstr(), 1);
}

void TextStreamFormat::writeS32(StreamSrc* src, [[maybe_unused]] Endian::Types endian, s32 value)
{
    sead::FixedSafeString<32> tmp;
    tmp.format("%d", value);
    u32 length = tmp.calcLength();

    src->write(tmp.cstr(), length);
    src->write(mEntryTerminator.cstr(), 1);
}

void TextStreamFormat::writeS64(StreamSrc* src, [[maybe_unused]] Endian::Types endian, s64 value)
{
    sead::FixedSafeString<32> tmp;
    tmp.format("%lld", value);
    u32 length = tmp.calcLength();

    src->write(tmp.cstr(), length);
    src->write(mEntryTerminator.cstr(), 1);
}

void TextStreamFormat::writeF32(StreamSrc* src, [[maybe_unused]] Endian::Types endian, f32 value)
{
    sead::FixedSafeString<32> tmp;
    tmp.format("%.8f", value);
    u32 length = tmp.calcLength();

    src->write(tmp.cstr(), length);
    src->write(mEntryTerminator.cstr(), 1);
}

void TextStreamFormat::writeBit(StreamSrc* src, const void* data, u32 bits)
{
    const u8* dataU8 = static_cast<const u8*>(data);
    sMutex.lock();
    sTextData = "0b";

    for (u32 i = 0; i < (bits + 7) / 8; i++)
    {
        u32 bitsInByte = bits - i * 8;
        if (bitsInByte > 8)
            bitsInByte = 8;

        for (s32 j = bitsInByte - 1; j >= 0; j--)
        {
            if (dataU8[i] & (1 << j))
                sTextData.append('1');
            else
                sTextData.append('0');
        }
    }

    src->write(sTextData.cstr(), bits + 2);  // 0b + bits
    src->write(mEntryTerminator.cstr(), 1);
    sMutex.unlock();
}

void TextStreamFormat::writeString(StreamSrc* src, const SafeString& str, u32 size)
{
    u32 length = str.calcLength();
    if (size <= length)
        length = size;

    char quotes = '"';
    char backslash = '\\';
    src->write(&quotes, 1);
    for (u32 i = 0; i < length; i++)
    {
        if (str[i] == '"')
        {
            src->write(&backslash, 1);
        }
        else
        {
            if (i != 0)
                str.cstr();
            if (str[i] == '"')
                src->write(&backslash, 1);
        }

        src->write(&str.cstr()[i], 1);
    }
    src->write(&quotes, 1);
}

inline s32 toBase64Size(u32 size)
{
    s32 b = size / 3;
    if (size % 3)
        b++;
    return b * 4;
}

void TextStreamFormat::writeMemBlock(StreamSrc* src, const void* buffer, u32 size)
{
    sMutex.lock();
    sTextData.clear();

    if (toBase64Size(size) + 1 < sTextData.getBufferSize())
    {
        char* textBuffer = sTextData.getBuffer();

        textBuffer[toBase64Size(size)] = SafeString::cNullChar;
        Base64::encode(textBuffer, buffer, size, false);
        u32 length = sTextData.calcLength();

        src->write("\"", 1);
        src->write(sTextData.cstr(), length);
        src->write("\"", 1);
        src->write(mEntryTerminator.cstr(), 1);
    }

    sMutex.unlock();
}

void TextStreamFormat::writeDecorationText(StreamSrc* src, const SafeString& text)
{
    u32 length = text.calcLength();
    src->write(text.cstr(), length);
}

void TextStreamFormat::writeNullChar(StreamSrc* src)
{
    char nullchar = '\0';
    src->write(&nullchar, 1);
}

void TextStreamFormat::skip(StreamSrc* src, [[maybe_unused]] u32 offset)
{
    sMutex.lock();
    getNextData_(src);
    sMutex.unlock();
}

void TextStreamFormat::flush([[maybe_unused]] StreamSrc* src) {}

void TextStreamFormat::rewind(StreamSrc* src)
{
    src->rewind();
}

void TextStreamFormat::getNextData_(sead::StreamSrc* src)
{
    char cVar8;
    char value;

    sTextData.clear();
    if (src->read(&value, 1) == 0)
        return;

    bool inQuotes = false;
    char endChar = '\0';
    s32 inToken = 0;
    do
    {
        if (endChar != '\0')
        {
            if (value != endChar)
            {
                if (endChar == '/')
                    endChar = '*';
                continue;
            }

            if (endChar == '*')
            {
                endChar = '/';
                continue;
            }

            value = mEntryTerminator[0];
        }

        endChar = value;
        if (inQuotes)
        {
            if (value == '\"')
            {
                if (inToken < 2 || sTextData[inToken - 1] != '\\')
                {
                    return;
                }

                sTextData.append("\"", 1);
                inQuotes = true;
                inToken--;
                endChar = '\0';
                continue;
            }

            sTextData.append(value);
            inQuotes = true;
            inToken++;
            endChar = '\0';
            continue;
        }

        if (inToken == 0 && value == '\"')
        {
            inQuotes = true;
            endChar = '\0';
            continue;
        }

        if (mEntryTerminator.include(value) || value == '\0')
        {
            if (!sTextData.isEmpty())
            {
                return;
            }
            inQuotes = false;
            endChar = '\0';
            continue;
        }

        sTextData.append(value);
        if (inToken + 1 > 0 && sTextData[inToken] == '#')
        {
            sTextData.trim(inToken - 1);
            cVar8 = '\n';
        }
        else
        {
            cVar8 = '\0';
            inToken++;
        }

        if (inToken > 1 && sTextData[inToken - 2] == '/')
        {
            if (sTextData[inToken - 1] == '/')
            {
                sTextData.trim(inToken - 2);
                inQuotes = false;
                inToken -= 2;
                endChar = '\n';
                continue;
            }

            if (sTextData[inToken - 1] == '*')
            {
                sTextData.trim(inToken - 2);
                inQuotes = false;
                inToken -= 2;
                endChar = '*';
                continue;
            }
        }
        inQuotes = false;
        endChar = cVar8;

    } while (src->read(&value, 1) != 0);
}
}  // namespace sead
