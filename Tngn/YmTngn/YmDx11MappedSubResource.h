#pragma once

#include "YmTngnFwd.h"
#include "YmBase/YmBase.h"

namespace Ymcpp {

class YmDx11MappedSubResource
{
public:
	YmDx11MappedSubResource() noexcept : YmDx11MappedSubResource(nullptr, nullptr, nullptr) {}
	YmDx11MappedSubResource(D3DDeviceContextPtr pDC, D3DResourcePtr pSubResource, void* pData) noexcept
		: m_pDC(std::move(pDC)), m_pSubResource(std::move(pSubResource)),
		m_rawData{ pData, 0, 0 }
	{}
	YmDx11MappedSubResource(D3DDeviceContextPtr pDC, D3DResourcePtr pSubResource, D3D11_MAPPED_SUBRESOURCE&& data) noexcept
		: m_pDC(std::move(pDC)), m_pSubResource(std::move(pSubResource)), m_rawData(data)
	{
		data.pData = nullptr;
	}

	/*virtual*/~YmDx11MappedSubResource();

	YmDx11MappedSubResource(const YmDx11MappedSubResource& other) = delete;
	YmDx11MappedSubResource(YmDx11MappedSubResource&& other) noexcept
		: m_pDC(std::move(other.m_pDC)), m_pSubResource(std::move(other.m_pSubResource)),
		m_rawData(other.m_rawData)
	{
		other.m_rawData.pData = nullptr;
	}

	YmDx11MappedSubResource& operator = (const YmDx11MappedSubResource& other) = delete;
	YmDx11MappedSubResource& operator = (YmDx11MappedSubResource&& other) noexcept
	{
		if (this != &other) {
			// move old value to destory it by noexcept destructor.
			// MoveToLeft() shall be noexcept in this case.
			YmDx11MappedSubResource oldValue(std::move(*this));
			MoveToLeft(m_pDC, other.m_pDC, nullptr);
			MoveToLeft(m_pSubResource, other.m_pSubResource, nullptr);
			m_rawData = other.m_rawData; other.m_rawData.pData = nullptr;
		}
		return *this;
	}

	bool IsMapped() const { return m_pDC && m_pSubResource; }
	void Unmap();

	template<class T>
	T* ToArray(size_t byteOffset) { return reinterpret_cast<T*>(ToByteArray() + byteOffset); }
	void Write(const void* pData, UINT dataByte);

	UINT GetRowPitch() const { return m_rawData.RowPitch; }

	UINT GetDepthPitch() const {return m_rawData.DepthPitch; }

private:
	char* ToByteArray() { return (char*)m_rawData.pData; }
private:
	D3DDeviceContextPtr m_pDC;
	D3DResourcePtr m_pSubResource;
	D3D11_MAPPED_SUBRESOURCE m_rawData;
};

}
