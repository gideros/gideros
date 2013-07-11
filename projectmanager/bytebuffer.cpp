#include "bytebuffer.h"
#include <memory.h>

ByteBuffer::ByteBuffer()
{
	pos_ = 0;
}

ByteBuffer::ByteBuffer(const char* data, int size)
{
	set(data, size);
}

#define MEMCPY_INTEGRAL(type, variable)	int size = (int)buffer_.size();					\
										buffer_.resize(size + sizeof(type));			\
										memcpy(&buffer_[size], &variable, sizeof(type));

#define MEMCPY_INTEGRAL_REVERSE(type, variable)	memcpy(&variable, &buffer_[pos_], sizeof(type));	\
												pos_ += sizeof(type);


void ByteBuffer::clear()
{
	buffer_.clear();
	pos_ = 0;
}

void ByteBuffer::set(const char* data, int size)
{
	buffer_.resize(size);
	memcpy(&buffer_[0], data, size);
	pos_ = 0;
}


void ByteBuffer::append(char c)
{
	buffer_.push_back(c);
}
void ByteBuffer::append(unsigned char uc)
{
	buffer_.push_back(uc);
}
void ByteBuffer::append(short s)
{
	MEMCPY_INTEGRAL(short, s)
}
void ByteBuffer::append(unsigned short us)
{
	MEMCPY_INTEGRAL(unsigned short, us)
}
void ByteBuffer::append(int i)
{
	MEMCPY_INTEGRAL(int, i)
}
void ByteBuffer::append(unsigned int ui)
{
	MEMCPY_INTEGRAL(unsigned int, ui)
}
void ByteBuffer::append(const char* str)
{
	int size = (int)buffer_.size();
	int length = strlen(str);
	buffer_.resize(size + length + 1);
	memcpy(&buffer_[size], str, length + 1);
}

void ByteBuffer::append(const std::string& str)
{
	append(str.c_str());
}

int ByteBuffer::size() const
{
	return (int)buffer_.size();
}

const char* ByteBuffer::data() const
{
	return &buffer_[0];
}

void ByteBuffer::get(char& c)
{
	c = buffer_[pos_];
	pos_++;
}

void ByteBuffer::get(unsigned char& uc)
{
	uc = buffer_[pos_];
	pos_++;
}

void ByteBuffer::get(short& s)
{
	MEMCPY_INTEGRAL_REVERSE(short, s)
}

void ByteBuffer::get(unsigned short& us)
{
	MEMCPY_INTEGRAL_REVERSE(unsigned short, us)
}

void ByteBuffer::get(int& i)
{
	MEMCPY_INTEGRAL_REVERSE(int, i)
}

void ByteBuffer::get(unsigned int& ui)
{
	MEMCPY_INTEGRAL_REVERSE(unsigned int, ui)
}

void ByteBuffer::get(std::string& str)
{
	str = &buffer_[pos_];
	pos_ += str.size() + 1;
}
