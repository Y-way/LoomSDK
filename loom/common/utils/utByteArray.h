/*
 * ===========================================================================
 * Loom SDK
 * Copyright 2011, 2012, 2013
 * The Game Engine Company, LLC
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 * ===========================================================================
 */

#ifndef _UT_BYTEARRAY_H_
#define _UT_BYTEARRAY_H_

//#include "core/assert.h"
#include "utTypes.h"
#include "utString.h"
#include "loom/common/core/assert.h"

class utByteArray {
protected:
    // stored as little endian
    utArray<unsigned char> _data;
    UTsize _position;

    void memcpyUnaligned(void *destination, const void *source, size_t num)
    {
        // TODO smarter
        // Loop copying byte by byte to avoid unaligned access
        for (size_t i = 0; i < num; i++)
        {
            ((char*)destination)[i] = ((char*)source)[i];
        }
    }

    template<typename T>
    T readValue()
    {
        lmAssert(_position <= _data.size() - sizeof(T), "ByteArray out of data on read of size %u", sizeof(T));
        
        T *ptr  = (T *)&_data[_position];
        T value;

        // memcpy is necessary for some platforms as we can get
        // an unaligned read exception (e.g. SIGBUS on Android/ARM) otherwise
        // TODO: benchmark vs. assignment
        memcpyUnaligned(&value, ptr, sizeof(T));

        _position += sizeof(T);

        value = convertLEndianToHost(value);

        return value;
    }

    template<typename T>
    void writeValue(T value)
    {
        if (_data.size() < _position + sizeof(T))
        {
            _data.resize(_position + sizeof(T));
        }

        value = convertHostToLEndian(value);

        T *ptr = (T *)&_data[_position];
        
        // Needed for unaligned writes, see readValue for more info
        memcpyUnaligned(ptr, &value, sizeof(T));

        _position += sizeof(T);
    }

    static int copyBytesInternal(utByteArray *dstByteArray, utByteArray *srcByteArray, int offset = 0, int length = 0, bool dstOffset = true)
    {
        if (!srcByteArray || !dstByteArray)
        {
            return 0;
        }

        unsigned char *dst = dstByteArray->_data.ptr();
        unsigned char *src = srcByteArray->_data.ptr();

        if (offset < 0) offset = 0;
        if (length < 0) return 0;

        if (!length) {
            length = srcByteArray->_data.size();
        }

        if (dstOffset) {
            offset = utClamp<int>(offset, 0, dstByteArray->_data.size());
            dst += offset;
            src += srcByteArray->_position;
        } else {
            offset = utClamp<int>(offset, 0, srcByteArray->_data.size());
            dst += dstByteArray->_position;
            src += offset;
        }

        unsigned char *srcEnd = srcByteArray->_data.ptr() + srcByteArray->_data.size();
        if (src + length > srcEnd)
        {
            length = (int)(srcEnd - src);
            if (length < 0) return 0;
        }

        unsigned char *dstEnd = dstByteArray->_data.ptr() + dstByteArray->_data.size();
        if (dst + length > dstEnd)
        {
            int off = (int)(dst - dstByteArray->_data.ptr());
            dstByteArray->_data.resize((UTsize)(dst - dstByteArray->_data.ptr() + length));
            // We have a different backing array now, point pointer to the new one.
            dst = dstByteArray->_data.ptr() + off;
        }
        
        memcpy(dst, src, length);
        if (dstOffset) {
            srcByteArray->_position += length;
        } else {
            dstByteArray->_position += length;
        }

        return length;
    }

public:
    utByteArray()
    {
        _position = 0;
    }

    void clear(bool useCache = false);

    void setPosition(unsigned int value)
    {
        _position = (UTsize) value;
    }

    unsigned int getPosition() const
    {
        return _position;
    }

    unsigned int bytesAvailable() const
    {
        UTsize size = getSize();
        return (size < _position ? 0 : size-_position);
    }

    bool readBoolean()
    {
        return readValue<bool>();
    }

    void writeBoolean(bool value)
    {
        writeValue<bool>(value);
    }

    signed char readByte()
    {
        return (int)readValue<signed char>();
    }

    void writeByte(signed char value)
    {
        writeValue<signed char>(value);
    }

    unsigned char readUnsignedByte()
    {
        return readValue<unsigned char>();
    }

    void writeUnsignedByte(unsigned char value)
    {
        writeValue<unsigned char>(value);
    }

    short readShort()
    {
        return readValue<short>();
    }

    void writeShort(short value)
    {
        writeValue<short>(value);
    }

    unsigned short readUnsignedShort()
    {
        return readValue<unsigned short>();
    }

    void writeUnsignedShort(unsigned short value)
    {
        writeValue<unsigned short>(value);
    }

    int readInt()
    {
        return readValue<int>();
    }

    void writeInt(int value)
    {
        writeValue<int>(value);
    }

    float readFloat()
    {
        return readValue<float>();
    }

    void writeFloat(float value)
    {
        writeValue<float>(value);
    }

    double readDouble()
    {
        return readValue<double>();
    }

    void writeDouble(double value)
    {
        writeValue<double>(value);
    }

    unsigned int readUnsignedInt()
    {
        return readValue<unsigned int>();
    }

    void writeUnsignedInt(unsigned int value)
    {
        writeValue<unsigned int>(value);
    }

    unsigned long long readUnsignedInt64()
    {
        return readValue<unsigned long long>();
    }

    void writeUnsignedInt64(UTuint64 value)
    {
        writeValue<unsigned long long>(value);
    }

    void writeString(const char *value)
    {
        if (!value)
        {
            writeValue<int>(0);
            return;
        }

        UTsize length = (UTsize)strlen(value);

        if (!length)
        {
            writeValue<int>(0);
            return;
        }

        writeValue<int>(length);

        if ((int)_data.size() < _position + length)
        {
            _data.resize(_position + length);
        }

        char *ptr = (char *)&_data[_position];
        memcpyUnaligned(ptr, value, length);

        _position += length;
    }

    // note that the string returned is only valid between reads
    const char *readString()
    {
        static utString svalue;

        int length = readValue<int>();

        if (!length)
        {
            return "";
        }

        lmAssert(_position + length <= _data.size(), "Insufficient data available for length of %d (use readStringBytes if you don't have a 32-bit integer length header)", length);

        svalue.alloc(length);
        memcpyUnaligned((void*)svalue.c_str(), &_data[_position], length);
        
        _position += length;

        return svalue.c_str();
    }


    // TODO: String isn't Unicode ready yet
    const char *readUTF()
    {
        unsigned short length = readValue<unsigned short>();

        if (!length)
        {
            return "";
        }

        lmAssert(_position + length <= _data.size(), "Insufficient data available for length of %d (use readUTFBytes if you don't have a 16-bit unsigned integer length header)", length);

        return readUTFBytes(length);
    }

    const char *readUTFBytes(unsigned int length)
    {
        static utString svalue;
        
        svalue.fromBytes(&_data[_position], length);

        _position += length;

        return svalue.c_str();
    }

    void writeUTF(const char *value)
    {
        if (!value)
        {
            writeValue<unsigned short>(0);
            return;
        }

        UTsize length = (UTsize)strlen(value);

        // Unable to write length in writeUTF, length is larger than 65535
        assert(length < 0xFFFF);

        writeValue<unsigned short>((unsigned short)length);

        writeUTFInternal(value, length);
    }

    void writeUTFBytes(const char *value)
    {
        writeUTFInternal(value, (UTsize)strlen(value));
    }

    void writeUTFInternal(const char *value, UTsize length)
    {
        if (!length) return;

        if ((UTsize)_data.size() < _position + length)
        {
            _data.resize(_position + length);
        }

        char *ptr = (char *)&_data[_position];
        memcpyUnaligned(ptr, value, length);

        _position += length;
    }


    void writeBytes(utByteArray *byteArray, int offset = 0, int length = 0)
    {
        copyBytesInternal(this, byteArray, offset, length, false);
    }

    int readBytes(utByteArray *byteArray, int offset = 0, int length = 0)
    {
        return copyBytesInternal(byteArray, this, offset, length, true);
    }

    /*
     * Return the utByteArray as a string, please not that the string buffer
     * returned is only valid between calls
     */
    const char *toString()
    {
        static utString svalue;

        if (_data.ptr() != NULL)
        {
            svalue  = (const char *)_data.ptr();
            svalue += "\0"; // ensure null terminated
        }
        else
        {
            svalue = "";
        }

        return svalue.c_str();
    }

    /*
     * Given a source memory pointer and size, initialize the utByteArray with
     * the data and reset position to 0
     */
    void allocateAndCopy(void *src, int size)
    {
        _data.resize(size);
        memcpy(_data.ptr(), src, size);
        _position = 0;
    }

    void attach(void *memory, UTsize size)
    {
        _position = 0;
        _data.attach(memory, size);
    }

    /*
     * Direct access to the utByteArray's data
     */
    void *getDataPtr()
    {
        return _data.ptr();
    }

    /*
     * Direct access to the utByteArray's size
     */
    UTsize getSize() const
    {
        return _data.size();
    }

    /*
     * Reserve the specified amount of memory for the utByteArray, this does
     * not resize the array, it allocates enough memory to hold the specified
     * amount.  This is most useful to avoid memory thrashing when writing to
     * the array
     */
    void reserve(UTsize size)
    {
        return _data.reserve(size);
    }

    /*
     * Set the utByteArray's size directly
     */
    void resize(UTsize size)
    {
        _position = _position > size ? size : _position;
        return _data.resize(size);
    }

    /*
     * returns true if file open/read was successful
     * with file contents in the ByteArray, addNullTerminator is useful
     * for reading string files, which may not be null terminated
     */
    static bool tryReadToArray(const utString& path, utByteArray& bytes, bool addNullTerminator = true);

    /*
     * Retrieve the low level array to avoid copies/etc in special situation
     */

    utArray<unsigned char> *getInternalArray() { return &_data; }

    /*
     * Compress the ByteArray data with the zlib compression algorithm.
     * The ByteArray gets resized to the compressed size of the data.
     */
    void compress();

    /*
     * Uncompress zlib or gzip compressed data. uncompressedSize is equivalent to
     * initialSize due to legacy code. If not specified, initialSize will be used
     * and ByteArray will be truncated to the actual size of the data. If the
     * uncompressed data size doesn't fit in initialSize bytes, the buffer gets
     * resized until it does.
     */
    void uncompress(int uncompressedSize = 0, int initialSize = 32768);
};
#endif // _PLATFORM_BYTEARRAY_H_
