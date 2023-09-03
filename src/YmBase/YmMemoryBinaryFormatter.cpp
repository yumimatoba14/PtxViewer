#include "pch.h"
#include "YmMemoryBinaryFormatter.h"

using namespace std;
using namespace Ymcpp;

////////////////////////////////////////////////////////////////////////////////

YmMemoryBinaryFormatter::YmMemoryBinaryFormatter()
{
	// Initializer is not used because m_streamBuf is initialized after base class.
	SetStreamBuf(&m_streamBuf);
}

YmMemoryBinaryFormatter::~YmMemoryBinaryFormatter()
{
}

////////////////////////////////////////////////////////////////////////////////
