#pragma once



namespace DEditor
{

class TypeNames
{
public:
	TypeNames() = delete;
	TypeNames(const TypeNames&) = delete;
	TypeNames(TypeNames&&) = delete;
public:
	static const char* physicsLayerNames[];
};

}
