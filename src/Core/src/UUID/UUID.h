#pragma once

#include <bitset>



#ifdef __linux__

#include <uuid/uuid.h>
#include <climits>



namespace DCore
{

class UUIDWrapper;

using UUIDType = UUIDWrapper;

class UUIDWrapper
{
	friend class UUIDGenerator;
	friend std::ostream& operator<<(std::ostream&, const UUIDWrapper&);
public:
	using stringType = std::string;
public:
	static constexpr size_t sizeBytes{ sizeof(uuid_t) };
	static constexpr size_t sizeBits{ sizeBytes * CHAR_BIT };
public:
	UUIDWrapper();
	UUIDWrapper(const UUIDWrapper&);
	UUIDWrapper(const stringType& uuidString);
	~UUIDWrapper() = default;
public:
	bool operator==(const UUIDWrapper& other) const
	{
		return uuid_compare(m_uuid, other.m_uuid) == 0;
	}

	bool operator!=(const UUIDWrapper& other) const
	{
		return uuid_compare(m_uuid, other.m_uuid) != 0;
	}

	UUIDWrapper& operator=(const UUIDWrapper& other)
	{
		uuid_copy(m_uuid, other.m_uuid);
		return *this;
	}

	operator std::string() const
	{
		char unparsed[UUID_STR_LEN];
		unparsed[UUID_STR_LEN - 1] = '\0';
		uuid_unparse(m_uuid, unparsed);
		return std::string(unparsed);
	}

	const unsigned char* Raw() const
	{
		return m_uuid;
	}
private:
	uuid_t m_uuid;
};

}

template <>
struct std::hash<DCore::UUIDType>
{
	using BitsType = std::bitset<DCore::UUIDType::sizeBits>;

	void GetUUIDBinaryRepresentation(const DCore::UUIDType& s, BitsType& outRepresentation) const
	{
		for (size_t i(0); i < DCore::UUIDType::sizeBytes; i++)
		{
			BitsType temp(s.Raw()[i]);
			temp <<= i * CHAR_BIT;
			outRepresentation |= temp;
		}
	}

	std::size_t operator()(const DCore::UUIDType& s) const noexcept
	{
		BitsType binary;
		GetUUIDBinaryRepresentation(s, binary);
		return std::hash<BitsType>{}(binary);
	}
};
#elif _WIN32
#pragma comment(lib, "rpcrt4.lib")

#include "DCoreAssert.h"

#include <rpc.h>



namespace DCore
{

class UUIDWrapper;

using UUIDType = UUIDWrapper;

class UUIDWrapper
{
	friend class UUIDGenerator;
	friend std::ostream& operator<<(std::ostream&, const UUIDWrapper&);
public:
	using stringType = std::string;
public:
	static constexpr size_t uuidStringLen{37};
public:
	UUIDWrapper();
	UUIDWrapper(const UUIDWrapper&);
	UUIDWrapper(const stringType& uuidString);
	~UUIDWrapper() = default;
public:
	bool operator==(const UUIDWrapper& other) const
	{
		UUID left(m_uuid);
		UUID right(other.m_uuid);
		RPC_STATUS status;
		signed int result(UuidCompare(&left, &right, &status));
		DASSERT_E(status == RPC_S_OK);
		return result == 0;
	}

	bool operator!=(const UUIDWrapper& other) const
	{
		UUID left(m_uuid);
		UUID right(other.m_uuid);	
		RPC_STATUS status;
		signed int result(UuidCompare(&left, &right, &status));
		DASSERT_E(status == RPC_S_OK);
		return result != 0;
	}

	UUIDWrapper& operator=(const UUIDWrapper& other)
	{
		m_uuid = other.m_uuid;
		return *this;
	}

	operator std::string() const
	{
		RPC_CSTR uuidString(nullptr);
		RPC_STATUS status(UuidToString(&m_uuid, &uuidString));
		DASSERT_E(status == RPC_S_OK);
		stringType returnValue(reinterpret_cast<char*>(uuidString));
		RpcStringFree(&uuidString);
		return returnValue;
	}

	const UUID& Raw() const
	{
		return m_uuid;
	}

	UUID& Raw()
	{
		return m_uuid;
	}
private:
	UUID m_uuid;
};

}

template <>
struct std::hash<DCore::UUIDType>
{
	size_t operator()(const DCore::UUIDType& s) const noexcept
	{
		DCore::UUIDType uuid(s);
		RPC_STATUS status;
		unsigned short result(UuidHash(&uuid.Raw(), &status));
		DASSERT_E(status == RPC_S_OK);
		return static_cast<size_t>(result);
	}
};
#endif 

namespace DCore
{

class UUIDGenerator
{
public:
	~UUIDGenerator() = default;
public:
	void GenerateUUID(UUIDWrapper& outUUID) const;
public:
	static UUIDGenerator& Get()
	{
		static UUIDGenerator generator;
		return generator;
	}
private:
	UUIDGenerator() = default;
};

}



