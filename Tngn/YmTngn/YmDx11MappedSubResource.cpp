#include "pch.h"
#include "YmDx11MappedSubResource.h"

using namespace std;
using namespace Ymcpp;

////////////////////////////////////////////////////////////////////////////////

YmDx11MappedSubResource::~YmDx11MappedSubResource()
{
	YM_NOEXCEPT_BEGIN("YmDx11MappedSubResource::~YmDx11MappedSubResource");
	Unmap();
	YM_NOEXCEPT_END;
}

////////////////////////////////////////////////////////////////////////////////

void YmDx11MappedSubResource::Unmap()
{
	if (m_pDC && m_pSubResource) {
		m_pDC->Unmap(m_pSubResource.Get(), 0);
		m_pDC = nullptr;
		m_pSubResource = nullptr;
		m_aData = nullptr;
	}
	YM_ASSERT(!IsMapped());
}

void YmDx11MappedSubResource::Write(const void* pData, UINT dataByte)
{
	YM_IS_TRUE(m_aData != nullptr);
	memcpy(m_aData, pData, dataByte);
}

////////////////////////////////////////////////////////////////////////////////
