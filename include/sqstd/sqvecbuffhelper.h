#ifndef VECTOR_BUFF_HELPER_H
#define VECTOR_BUFF_HELPER_H
#include <vector>
#include <algorithm>

namespace snqu{ namespace buffer{

inline int read(const std::vector<char>& buffer, 
	             int offset, char* data, int data_size)
{
	int copy_size = std::min<int>(data_size, buffer.size() - offset);
	memcpy(data, &buffer[offset], copy_size);
    return copy_size;
}

template<typename T>
inline bool read(const std::vector<char>& buffer, int offset, T& value)
{
	T* value_ptr = &value;
	if (read(buffer, offset, (char*)value_ptr, sizeof(T)))
	{
		value = *value_ptr;
		return true;
	}

	return false;
}

inline void write(std::vector<char>& buffer, int offset,
	              const char* data, int data_size)
{
	if (data_size > 0)
	{
        buffer.resize(offset + data_size, 0);
        memcpy_s(&buffer[offset], buffer.size(), data, data_size);
	}
}

template<typename T>
inline void write(std::vector<char>& buffer, int offset, const T& value)
{
	return write(buffer, offset, (const char*)&value, sizeof(T));
}

}}

#endif // VECTOR_BUFF_HELPER_H