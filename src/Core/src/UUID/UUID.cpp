#include "UUID.h"

#include <ostream>



namespace DCore
{

#ifdef __linux__
UUIDWrapper::UUIDWrapper()
{
	uuid_clear(m_uuid);
}

UUIDWrapper::UUIDWrapper(const UUIDWrapper& other)
{
	uuid_copy(m_uuid, other.m_uuid);
}

UUIDWrapper::UUIDWrapper(const stringType& uuidString)
{
	uuid_parse(uuidString.c_str(), m_uuid);
}

std::ostream& operator<<(std::ostream& os, const UUIDWrapper& uuidWrapper)
{
	char uuidString[UUID_STR_LEN];
	uuidString[UUID_STR_LEN - 1] = '\0';
	uuid_unparse(uuidWrapper.m_uuid, uuidString);
	os << uuidString;
	return os;
}

void UUIDGenerator::GenerateUUID(UUIDWrapper& outUUID) const
{
	uuid_generate(outUUID.m_uuid);
}
#elif _WIN32
UUIDWrapper::UUIDWrapper()
{}

UUIDWrapper::UUIDWrapper(const UUIDWrapper& other)
	:
	m_uuid(other.m_uuid)
{}

UUIDWrapper::UUIDWrapper(const stringType& uuidString)
{
	stringType copy(uuidString);
	UuidFromString(reinterpret_cast<unsigned char*>(copy.data()), &m_uuid);
}

std::ostream& operator<<(std::ostream& os, const UUIDWrapper& uuidWrapper)
{
	unsigned char uuidString[UUIDWrapper::uuidStringLen];
	uuidString[UUIDWrapper::uuidStringLen - 1] = '\0';
	UuidToString(&uuidWrapper.m_uuid, reinterpret_cast<unsigned char**>(&uuidString));
	os << uuidString;
	return os;
}

void UUIDGenerator::GenerateUUID(UUIDWrapper& outUUID) const
{
	UuidCreate(&outUUID.m_uuid);
}

#endif

}
