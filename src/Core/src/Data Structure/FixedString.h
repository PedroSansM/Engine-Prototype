#pragma once

#include <cstddef>
#include <cstring>
#include <ostream>
#include <string>



namespace DCore
{

// A fixed size string.
template <size_t Size>
class FixedString
{
	template <size_t OtherSize>
	friend std::ostream& operator<<(std::ostream&, const FixedString<OtherSize>&);
public:
	FixedString()
	{
		std::memset(m_string, '\0', Size);
	}
	FixedString(const char* string)
	{
		std::memset(m_string, '\0', Size);
		if (string != nullptr && std::strlen(string) <= Size)
		{
			std::strcpy(m_string, string);
		}
	}
	FixedString(const std::string& string)
	{
		std::memset(m_string, '\0', Size);
		if (string.size() <= Size)
		{
			std::strcpy(m_string, string.c_str());
		}
	}
	FixedString(const FixedString& other) noexcept
	{
		std::memset(m_string, '\0', Size);
		if (Size >= other.Length())
		{
			std::strcpy(m_string, other.m_string);
		}
	}
	~FixedString() = default;
public:
	FixedString& Append(const char* string)
	{
		std::strcat(m_string, string);
		return *this;
	}

	bool TryAppend(const char* string)
	{
		if (Size - Length() < std::strlen(string))
		{
			return false;
		}
		std::strcat(m_string. string);
		return true;
	}
	
	void Clear()
	{
		std::memset(m_string, 0, Size);
	}

	size_t Length() const
	{
		return std::strlen(m_string);
	}

	bool Empty() const
	{
		return std::strlen(m_string) == 0;
	}

	const char* Data() const
	{
		return m_string;
	}

	char* Data()
	{
		return m_string;
	}
	
	size_t (GetFreeSpace)() const
	{
		return Size - Length();
	}
public:
	FixedString& operator=(const char* string)
	{
		if (string != nullptr && std::strlen(string) <= Size)
		{
			std::strcpy(m_string, string);
		}
		return *this;
	}
	
	bool operator==(const std::string& string) const
	{
		return operator==(string.c_str());
	}

	bool operator==(const char* string) const
	{
		return std::strcmp(m_string, string) == 0;
	}

	bool operator!=(const char* string) const
	{
		return !operator==(string);
	}

	operator const char*() const
	{
		return m_string;
	}
public:
	template <size_t OtherSize>
	void Append(const FixedString<OtherSize>& other)
	{
		std::strcat(m_string, other.Data());
	}

	template <size_t OtherSize>
	bool TryAppend(const FixedString<OtherSize>& other)
	{
		if (Size - Length() < other.Length())
		{
			return false;
		}
		std::strcat(m_string, other.Data());
		return true;
	}
public:
	template <size_t OtherSize>
	FixedString& operator=(const FixedString<OtherSize>& other)
	{
		if (Size >= other.Length())
		{
			std::strcpy(m_string, other.Data());
		}
		return *this;
	}
private:
	char m_string[Size + 1];
};

template <size_t Size>
std::ostream& operator<<(std::ostream& stream, const FixedString<Size>& fixedString)
{
	stream << fixedString.m_string;
	return stream;
}

}
