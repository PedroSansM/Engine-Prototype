#pragma once

#include "DCoreAssert.h"
#include "TemplateUtils.h"

#include <functional>
#include <iterator>
#include <type_traits>
#include <vector>



namespace DCore
{

// A vector that uses a implicit list to keep track of deleted elements.
template <class Type, class IdType = size_t, class VersionType = size_t>
class ReciclingVector
{
private:
	class Element;
public:
	using idType = IdType;
	using versionType = VersionType;
	using valueType = Type;
	using elementContainerType = std::vector<Element>;
public:
	ReciclingVector()
		:
		m_reciclingListHead(0),
		m_reciclingListSize(0)
	{}
	ReciclingVector(size_t initialCapacity)
		:
		m_reciclingListHead(0),
		m_reciclingListSize(0)
	{
		m_vector.reserve(initialCapacity);
	}
	ReciclingVector(const ReciclingVector& other)
		:
		m_vector(other.m_vector),
		m_reciclingListHead(other.m_reciclingListHead),
		m_reciclingListSize(other.m_reciclingListSize)
	{}
	ReciclingVector(ReciclingVector&& other) noexcept
		:
		m_vector(std::move(other.m_vector)),
		m_reciclingListHead(other.m_reciclingListHead),
		m_reciclingListSize(other.m_reciclingListSize)
	{}
	~ReciclingVector() = default;
public:
	class Ref
	{
		friend class ReciclingVector;
		friend class ConstRef;
	public:
		using valueType = Type;
	public:
		static constexpr bool isConstRef{false};
	public:
		Ref()
			:
			m_id(0),
			m_version(0),
			m_reciclingVector(nullptr)
		{}
		Ref(ReciclingVector& reciclingVector)
			:
			m_id(0),
			m_version(0),
			m_reciclingVector(&reciclingVector)
		{}
		Ref(IdType id, VersionType version, ReciclingVector& reciclingVector)
			:
			m_id(id),
			m_version(version),
			m_reciclingVector(&reciclingVector)
		{}
		Ref(const Ref& other)
			:
			m_id(other.m_id),
			m_version(other.m_version),
			m_reciclingVector(other.m_reciclingVector)
		{}
		Ref(const Ref& other, ReciclingVector& reciclingVector)
			:
			m_id(other.m_id),
			m_version(other.m_version),
			m_reciclingVector(&reciclingVector)
		{}
		~Ref() = default;
	public:
		IdType GetId() const
		{
			return m_id;
		}

		IdType GetIndex() const
		{
			return m_id - 1;
		}

		versionType GetVersion() const
		{
			return m_version;
		}

		ReciclingVector* GetReciclingVector()
		{
			return m_reciclingVector;
		}
		
		void SetIndex(idType index)
		{
			m_id = index + 1;
		}

		void SetVersion(versionType version)
		{
			m_version = version;
		}

		bool IsValid() const
		{
			return m_reciclingVector != nullptr && m_reciclingVector->IsValid(*this);
		}

		Type* Data()
		{
			DASSERT_E(IsValid());	
			return m_reciclingVector->TryGet(*this);
		}

		void Invalidate()
		{
			m_id = 0;
			m_version = 0;
			m_reciclingVector = nullptr;
		}
	public:
		Type* operator->()
		{
			DASSERT_E(IsValid());
			return m_reciclingVector->TryGet(*this);
		}
		
		const Type* operator->() const
		{
			DASSERT_E(IsValid());
			return m_reciclingVector->TryGet(*this);
		}

		Ref& operator=(const Ref& other)
		{
			m_id = other.m_id;
			m_version = other.m_version;
			m_reciclingVector = other.m_reciclingVector;
			return *this;
		}

		Ref& operator=(Ref&& other) noexcept
		{
			m_id = other.m_id;
			m_version = other.m_version;
			m_reciclingVector = other.m_reciclingVector;
			return *this;
		}

		Ref& operator=(valueType&& value) noexcept
		{
			DASSERT_E(IsValid());
			m_reciclingVector->Assign(*this, std::move(value));
			return *this;
		}

		bool operator==(const Ref& other) const
		{
			return m_reciclingVector == other.m_reciclingVector && m_id == other.m_id && m_version == other.m_version;
		}
		
		bool operator!=(const Ref& other) const
		{
			return !operator==(other);
		}
	private:
		IdType m_id;
		VersionType m_version;
		ReciclingVector* m_reciclingVector;
	};

	class ConstRef
	{
		friend class ReciclingVector;
	public:
		using valueType = Type;
		using idType = IdType;
	public:
		static constexpr bool isConstRef{true};
	public:
		ConstRef()
			:
			m_id(0),
			m_version(0),
			m_reciclingVector(nullptr)
		{}
		ConstRef(const ReciclingVector& reciclingVector)
			:
			m_id(0),
			m_version(0),
			m_reciclingVector(&reciclingVector)
		{}
		ConstRef(IdType id, VersionType version, const ReciclingVector& reciclingVector)
			:
			m_id(id),
			m_version(version),
			m_reciclingVector(&reciclingVector)
		{}
		ConstRef(const ConstRef& other)
			:
			m_id(other.m_id),
			m_version(other.m_version),
			m_reciclingVector(other.m_reciclingVector)
		{}
		ConstRef(const ConstRef& other, const ReciclingVector& reciclingVector)
			:
			m_id(other.m_id),
			m_version(other.m_version),
			m_reciclingVector(&reciclingVector)
		{}
		ConstRef(const Ref& other)
			:
			m_id(other.m_id),
			m_version(other.m_version),
			m_reciclingVector(other.m_reciclingVector)
		{}
		~ConstRef() = default;
	public:
		IdType GetId() const
		{
			return m_id;
		}

		IdType GetIndex() const
		{
			return m_id - 1;
		}

		versionType GetVersion() const
		{
			return m_version;
		}

		void SetIndex(idType index)
		{
			m_id = index + 1;
		}

		void SetVersion(versionType version)
		{
			m_version = version;
		}

		bool IsValid() const
		{
			return m_reciclingVector != nullptr && m_reciclingVector->IsValid(*this);
		}
		
		const Type* Data() const
		{
			DASSERT_E(IsValid());
			return m_reciclingVector->TryGet(*this);
		}

		void Invalidate()
		{
			m_id = 0;
			m_version = 0;
			m_reciclingVector = nullptr;
		}
	public:
		const Type* operator->() const
		{
			DASSERT_E(IsValid());
			return m_reciclingVector->TryGet(*this);
		}

		bool operator==(const ConstRef& other) const
		{
			return m_reciclingVector == other.m_reciclingVector && m_id == other.m_id && m_version == other.m_version;
		}
		
		bool operator!=(const ConstRef& other) const
		{
			return !operator==(other);
		}
		
		ConstRef& operator=(const ConstRef& other)
		{
			m_id = other.m_id;
			m_version = other.m_version;
			m_reciclingVector = other.m_reciclingVector;
			return *this;
		}
	private:
		IdType m_id;
		VersionType m_version;
		const ReciclingVector* m_reciclingVector;
	};
private:
	class Element
	{
		friend class ReciclingVector;
	public:
		Element()
			:
			m_version(0),
			m_next(0),
			m_busy(true)
		{}
		Element(const valueType& data)
			:
			m_data(data),
			m_version(0),
			m_next(0),
			m_busy(true)
		{}
		Element(valueType&& data) noexcept
			:
			m_data(std::move(data)),
			m_version(0),
			m_next(0),
			m_busy(true)
		{}
		Element(const Element& other)
			:
			m_data(other.m_data),
			m_version(other.m_version),
			m_next(other.m_next),
			m_busy(other.m_busy)
		{}
		Element(Element&& other) noexcept
			:
			m_data(std::move(other.m_data)),
			m_version(other.m_version),
			m_next(other.m_next),
			m_busy(other.m_busy)
		{
			other.m_version = 0;
			other.m_next = 0;
			other.m_busy = false;
		}

		template <
			class ...Args, 
			class FirstType = typename TypeList<Args...>::head,
			std::enable_if_t<!std::is_same_v<FirstType, const valueType&>, bool> = true, 
			std::enable_if_t<!std::is_same_v<FirstType, valueType&&>, bool> = true, 
			std::enable_if_t<!std::is_same_v<FirstType, Element&>, bool> = true,
			std::enable_if_t<!std::is_same_v<FirstType, const Element&>, bool> = true, 
			std::enable_if_t<!std::is_same_v<FirstType, Element&&>, bool> = true, 
			std::enable_if_t<std::is_constructible_v<valueType, Args...>, bool> = true> 
		Element(Args&&... args)
			:
			m_data(std::forward<Args>(args)...),
			m_version(0),
			m_next(0),
			m_busy(true)
		{}

		template <
			class ...Args, 
			class FirstType = typename TypeList<Args...>::head,
			std::enable_if_t<!std::is_same_v<FirstType, const valueType&>, bool> = true, 
			std::enable_if_t<!std::is_same_v<FirstType, valueType&&>, bool> = true, 
			std::enable_if_t<!std::is_same_v<FirstType, Element&>, bool> = true,
			std::enable_if_t<!std::is_same_v<FirstType, const Element&>, bool> = true,
			std::enable_if_t<!std::is_same_v<FirstType, Element&&>, bool> = true,
			std::enable_if_t<!std::is_constructible_v<valueType, Args...>, bool> = true> 
		Element(Args&&... args)
			:
			m_data{std::forward<Args>(args)...},
			m_version(0),
			m_next(0),
			m_busy(true)
		{}

		~Element() = default;
	public:
		valueType& GetData()
		{
			return m_data;
		}

		const valueType& GetData() const
		{
			return m_data;
		}

		void EraseData()
		{
			m_version++;
			m_busy = false;
		}

		void PlaceData(const valueType& data)
		{
			DASSERT_E(!m_busy);
			m_data.~valueType();
			new (&m_data) valueType(data);
			m_busy = true;
		}

		void PlaceData(valueType&& data) noexcept
		{
			DASSERT_E(!m_busy);
			m_data.~valueType();
			new (&m_data) valueType(std::move(data));
			m_busy = true;
		}
	public:
		template <
			class ...Args, 
			class FirstType = typename TypeList<Args...>::head,
			std::enable_if_t<!std::is_same_v<FirstType, const valueType&>, bool> = true,
			std::enable_if_t<!std::is_same_v<FirstType, valueType&&>, bool> = true>
		void PlaceData(Args&&... args)
		{
			DASSERT_E(!m_busy);
			m_data.~valueType();
			if constexpr (std::is_constructible_v<valueType, Args...>)
			{
				new (&m_data) valueType(std::forward<Args>(args)...);
			}
			else
			{
				new (&m_data) valueType{std::forward<Args>(args)...};
			}
			m_busy = true;
		}
	public:
		Element& operator=(valueType&& data) noexcept
		{
			m_data = std::move(data);
			return *this;
		}

		Element& operator=(Element&& other) noexcept
		{
			m_data = std::move(other.m_data);
			m_version = other.m_version;
			m_next = other.m_next;
			m_busy = other.m_busy;
			other.m_busy = false;
			return *this;
		}
	private:
		valueType m_data;
		VersionType m_version;
		IdType m_next;
		bool m_busy;
	};
public:
	Ref PushBack(const valueType& data)
	{
		if (m_reciclingListSize != 0)
		{
			Element& elementToRecicle(m_vector[m_reciclingListHead]);
			elementToRecicle.PlaceData(data);
			Ref ref(m_reciclingListHead + 1, elementToRecicle.m_version, *this);
			m_reciclingListHead = elementToRecicle.m_next;
			m_reciclingListSize--;
			return ref;
		}
		m_vector.push_back(data);
		return Ref((IdType)m_vector.size(), 0, *this);
	}

	Ref PushBack(valueType&& data) noexcept
	{
		if (m_reciclingListSize != 0)
		{
			Element& elementToRecicle(m_vector[m_reciclingListHead]);
			elementToRecicle.PlaceData(std::move(data));
			Ref ref(m_reciclingListHead + 1, elementToRecicle.m_version, *this);
			m_reciclingListHead = elementToRecicle.m_next;
			m_reciclingListSize--;
			return ref;
		}
		m_vector.push_back(std::move(data));
		return Ref((IdType)m_vector.size(), 0, *this);
	}

	ConstRef PushBackConst(valueType&& data) noexcept
	{
		if (m_reciclingListSize != 0)
		{
			Element& elementToRecicle(m_vector[m_reciclingListHead]);
			elementToRecicle.PlaceData(std::move(data));
			ConstRef ref(m_reciclingListHead + 1, elementToRecicle.m_version, *this);
			m_reciclingListHead = elementToRecicle.m_next;
			m_reciclingListSize--;
			return ref;
		}
		m_vector.emplace_back(std::move(data));
		return ConstRef((IdType)m_vector.size(), 0, *this);
	}

	void RemoveElementAtIndex(size_t index)
	{
		if (index > m_vector.size() || !m_vector[index].m_busy)
		{
			return;
		}
		Element& erasedElement(m_vector[index]);
		erasedElement.EraseData();
		if (m_reciclingListSize != 0)
		{
			erasedElement.m_next = m_reciclingListHead;
		}
		m_reciclingListHead = index;
		m_reciclingListSize++;
	}

	void Assign(Ref ref, valueType&& value) noexcept
	{
		DASSERT_E(IsValid(ref));
		m_vector[ref.GetIndex()] = std::move(value);
	}

	size_t Size() const
	{
		size_t size(0);
		for (const Element& element : m_vector)
		{
			if (element.m_busy)
			{
				size++;
			}
		}
		return size;
	}

	bool Empty() const
	{
		return Size() == 0;
	}

	Ref GetRefFromRefId(IdType refId)
	{
		DASSERT_E(refId > 0 && refId - 1 < m_vector.size());
		return Ref(refId, m_vector[refId - 1].m_version, *this);
	}

	Ref GetRefFromIndex(size_t index)
	{
		if (index < m_vector.size() && m_vector[index].m_busy)
		{
			return Ref(index + 1, m_vector[index].m_version, *this);
		}
		return Ref();
	}

	ConstRef GetRefFromIndex(size_t index) const
	{
		if (index < m_vector.size() && m_vector[index].m_busy)
		{
			return ConstRef(index + 1, m_vector[index].m_version, *this);
		}
		return Ref();
	}

	void Clear()
	{
		for (Element& element : m_vector)
		{
			if (element.m_busy)
			{
				element.EraseData();
			}
		}
		for (size_t i(0); i < m_vector.size(); i++)
		{
			m_vector[i].m_next = i + 1;
		}
		m_reciclingListHead = 0;
		m_reciclingListSize = m_vector.size();
	}
	
	void Reserve(size_t size)
	{
		m_vector.reserve(size);
	}

	std::vector<valueType> GetRawContent()
	{
		std::vector<valueType> content;
		for (Element& element : m_vector)
		{
			if (element.m_busy)
			{
				content.push_back(element.GetData());
			}
		}
		return content;
	}
public:
	template <
		class ...Args,
		class FirstType = typename TypeList<Args...>::head,
		std::enable_if_t<!std::is_same_v<FirstType, const valueType&>, bool> = true,
		std::enable_if_t<!std::is_same_v<FirstType, valueType&&>, bool> = true>
	Ref PushBack(Args&&... args)
	{
		if (m_reciclingListSize != 0)
		{
			Element& elementToRecicle(m_vector[m_reciclingListHead]);
			elementToRecicle.PlaceData(std::forward<Args>(args)...);
			Ref ref(m_reciclingListHead + 1, elementToRecicle.m_version, *this);
			m_reciclingListHead = elementToRecicle.m_next;
			m_reciclingListSize--;
			return ref;
		}
		m_vector.emplace_back(std::forward<Args>(args)...);
		return Ref((IdType)m_vector.size(), 0, *this);
	}

	template <
		class ...Args,
		class FirstType = typename TypeList<Args...>::head,
		std::enable_if_t<!std::is_same_v<FirstType, const valueType&>, bool> = true,
		std::enable_if_t<!std::is_same_v<FirstType, valueType&&>, bool> = true>
	ConstRef PushBackConst(Args&&... args)
	{
		if (m_reciclingListSize != 0)
		{
			Element& elementToRecicle(m_vector[m_reciclingListHead]);
			elementToRecicle.PlaceData(std::forward<Args>(args)...);
			Ref ref(m_reciclingListHead + 1, elementToRecicle.m_version, *this);
			m_reciclingListHead = elementToRecicle.m_next;
			m_reciclingListSize--;
			return ref;
		}
		m_vector.emplace_back(std::forward<Args>(args)...);
		return ConstRef((IdType)m_vector.size(), 0, *this);
	}


	template <class RefType>
	void Remove(RefType ref)
	{
		static_assert(std::is_same_v<RefType, Ref> || std::is_same_v<RefType, ConstRef>, "Invalid ref");
		if (!IsValid(ref))
		{
			return;
		}
		Element& erasedElement(m_vector[ref.m_id - 1]);
		erasedElement.EraseData();
		if (m_reciclingListSize != 0)
		{
			erasedElement.m_next = m_reciclingListHead;
		}
		m_reciclingListHead = ref.m_id - 1;
		m_reciclingListSize++;
	
	}

	template <class Func>
	void Iterate(Func func)
	{
		for (size_t id(0); id < m_vector.size(); id++)
		{
			if (!m_vector[id].m_busy)
			{
				continue;
			}
			bool toStop(std::invoke(func, Ref(id + 1, m_vector[id].m_version, *this)));
			if (toStop)
			{
				return;
			}
		}
	}

  	template <class Func>
	void Iterate(Func func) const
	{
		for (size_t id(0); id < m_vector.size(); id++)
		{
			if (!m_vector[id].m_busy)
			{
				continue;
			}
			bool toStop(std::invoke(func, ConstRef(id + 1, m_vector[id].m_version, *this)));
			if (toStop)
			{
				return;
			}
		}
	}

  	template <class Func>
	void IterateConstRef(Func func)
	{
		for (size_t id(0); id < m_vector.size(); id++)
		{
			if (!m_vector[id].m_busy)
			{
				continue;
			}
			bool toStop(std::invoke(func, ConstRef(id + 1, m_vector[id].m_version, *this)));
			if (toStop)
			{
				return;
			}
		}
	}

  	template <class Func>
	void IterateConstRef(Func func) const
	{
		for (size_t id(0); id < m_vector.size(); id++)
		{
			if (!m_vector[id].m_busy)
			{
				continue;
			}
			if (std::invoke(func, ConstRef(id + 1, m_vector[id].m_version, *this)))
			{
				return;
			}
		}
	}
public:
	Type& operator[](IdType index)
	{
		DASSERT_E(index >= 0 && index < m_vector.size());
		return m_vector[index].GetData();
	}

	const Type& operator[](IdType index) const
	{
		DASSERT_E(index >= 0 && index < m_vector.size());
		return m_vector[index].GetData();
	}

	ReciclingVector& operator=(ReciclingVector&& other) noexcept
	{
		m_vector = std::move(other.m_vector);
		m_reciclingListHead = other.m_reciclingListHead;
		m_reciclingListSize = other.m_reciclingListSize;
		return *this;
	}
private:
	elementContainerType m_vector;
	idType m_reciclingListHead;
	size_t m_reciclingListSize;
private:
	Type* TryGet(Ref ref)
	{
		if (!IsValid(ref))
		{
			return nullptr;
		}
		return &m_vector[ref.m_id - 1].GetData();
	}

private:
	template <class RefType>
	const Type* TryGet(RefType ref) const
	{
		if (!IsValid(ref))
		{
			return nullptr;
		}
		return &m_vector[ref.m_id - 1].GetData();
	}

	template <class RefType>
	bool IsValid(RefType ref) const
	{
		// Ref id 0 is invalid by default.
		return ref.m_id > 0 && ref.m_id - 1 < m_vector.size() && m_vector[ref.m_id - 1].m_version == ref.m_version;
	}
};

}
