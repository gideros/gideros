#ifndef BYTEBUFFER_H
#define BYTEBUFFER_H

#include <vector>
#include <string>

class ByteBuffer
{
public:
	ByteBuffer();
	ByteBuffer(const char* data, int size);

	void clear();
	void set(const char* data, int size);

	void append(char c);
	void append(unsigned char uc);
	void append(short s);
	void append(unsigned short us);
	void append(int i);
	void append(unsigned int ui);
	void append(float f);
	void append(const char* str);
	void append(const std::string& str);
	void append(const unsigned char* data, int size);

	void get(char& c);
	void get(unsigned char& uc);
	void get(short& s);
	void get(unsigned short& us);
	void get(int& i);
	void get(unsigned int& ui);
	void get(float& f);
	void get(std::string& str);
	void get(unsigned char* data, int  size);

	bool eob() const
	{
		return pos_ == buffer_.size();
	}

	int size() const;
	const char* data() const;

private:
	std::vector<char> buffer_;
    size_t pos_;
};

inline ByteBuffer& operator << (ByteBuffer& buffer, char c)
{
	buffer.append(c);
	return buffer;
}

inline ByteBuffer& operator << (ByteBuffer& buffer, unsigned char uc)
{
	buffer.append(uc);
	return buffer;
}

inline ByteBuffer& operator << (ByteBuffer& buffer, short s)
{
	buffer.append(s);
	return buffer;
}

inline ByteBuffer& operator << (ByteBuffer& buffer, unsigned short us)
{
	buffer.append(us);
	return buffer;
}


inline ByteBuffer& operator << (ByteBuffer& buffer, int i)
{
	buffer.append(i);
	return buffer;
}

inline ByteBuffer& operator << (ByteBuffer& buffer, unsigned int ui)
{
	buffer.append(ui);
	return buffer;
}

inline ByteBuffer& operator << (ByteBuffer& buffer, float f)
{
	buffer.append(f);
	return buffer;
}

inline ByteBuffer& operator << (ByteBuffer& buffer, const char* str)
{
	buffer.append(str);
	return buffer;
}

inline ByteBuffer& operator << (ByteBuffer& buffer, const std::string str)
{
	buffer.append(str);
	return buffer;
}

inline ByteBuffer& operator >> (ByteBuffer& buffer, char& c)
{
	buffer.get(c);
	return buffer;
}

inline ByteBuffer& operator >> (ByteBuffer& buffer, unsigned char& uc)
{
	buffer.get(uc);
	return buffer;
}

inline ByteBuffer& operator >> (ByteBuffer& buffer, short& s)
{
	buffer.get(s);
	return buffer;
}

inline ByteBuffer& operator >> (ByteBuffer& buffer, unsigned short& us)
{
	buffer.get(us);
	return buffer;
}

inline ByteBuffer& operator >> (ByteBuffer& buffer, int& i)
{
	buffer.get(i);
	return buffer;
}

inline ByteBuffer& operator >> (ByteBuffer& buffer, unsigned int& ui)
{
	buffer.get(ui);
	return buffer;
}

inline ByteBuffer& operator >> (ByteBuffer& buffer, float& f)
{
	buffer.get(f);
	return buffer;
}

inline ByteBuffer& operator >> (ByteBuffer& buffer, std::string& str)
{
	buffer.get(str);
	return buffer;
}

#endif
