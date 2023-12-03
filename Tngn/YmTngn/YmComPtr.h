#pragma once

#include "YmTngnFwd.h"

namespace Ymcpp {

namespace Detail {
	template<class T>
	class ComPtrPtr
	{
	public:
		ComPtrPtr() : m_pp(nullptr) {}
		explicit ComPtrPtr(T** pp) : m_pp(pp) {}

		T* operator * () const { return *m_p; }
		operator T** () const { return m_pp; }
		operator IUnknown** () const { return reinterpret_cast<IUnknown**>(m_pp); }

		/// <summary>
		/// This function is added to support IID_PPV_ARGS by YmComPtr.
		/// </summary>
		/// <param name="pp"></param>
		/// <returns></returns>
		friend void** IID_PPV_ARGS_Helper(const ComPtrPtr& pp) { return reinterpret_cast<void**>(pp.m_pp); }

	private:
		T** m_pp;
	};
}

/// <summary>
/// Alternative of Microsoft::WRL::ComPtr because ComPtr cannot be used under the CLR option.
/// </summary>
/// <typeparam name="T"></typeparam>
template<class T>
class YmComPtr
{
public:
	using InterfaceType = T;

public:
	YmComPtr() noexcept : m_p(nullptr) {}
	YmComPtr(nullptr_t)noexcept  : m_p(nullptr) {}
	template<class T2>
	YmComPtr(const YmComPtr<T2>& other) : m_p(other.Get()) { AddRef(); }

	~YmComPtr() { Release(); }

	YmComPtr(YmComPtr&& other) noexcept : m_p(other.m_p) { other.m_p = nullptr; }
	YmComPtr(const YmComPtr& other) : m_p(other.m_p) { AddRef(); }

	YmComPtr& operator = (YmComPtr&& other) noexcept
	{
		if (other.GetThis() != GetThis()) {
			Release();
			m_p = other.m_p;
			other.m_p = nullptr;
		}
		return *this;
	}

	YmComPtr& operator = (const YmComPtr& other)
	{
		YmComPtr pTmp(other);
		*this = std::move(pTmp);
		return *this;
	}

	YmComPtr& operator = (nullptr_t)
	{
		Release();
		return *this;
	}

	void Reset() noexcept { Release(); }

	InterfaceType* Get() const { return m_p; }
	InterfaceType& operator * () const { return *m_p; }
	InterfaceType* operator -> () const { return m_p; }

	Detail::ComPtrPtr<InterfaceType> operator &()
	{
		if (*this) {
			Release();
		}
		return Detail::ComPtrPtr<InterfaceType>(&m_p);
	}

	operator bool() const { return m_p != nullptr; }

	bool operator == (InterfaceType* other) const { return m_p == other; }
	bool operator != (InterfaceType* other) const { return m_p != other; }
	bool operator < (InterfaceType* other) const { return m_p < other; }
private:
	const YmComPtr* GetThis() const { return this; }
	void AddRef()
	{
		if (m_p) {
			m_p->AddRef();
		}
	}

	void Release() noexcept
	{
		YM_NOEXCEPT_BEGIN("YmComPtr::Release");
		if (m_p) {
			m_p->Release();
			m_p = nullptr;
		}
		YM_NOEXCEPT_END;
	}

private:
	InterfaceType* m_p;
};

}
