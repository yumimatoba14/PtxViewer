#pragma once

#include "YmTngnFwd.h"

namespace Ymcpp {
	class YmTngnShaderImpl;

	namespace YmTngnDrawUtil
	{
		// This class denies copy of YmTngnShaderImpl poitner.
		class ShaderHolder {
		public:
			explicit ShaderHolder(YmTngnShaderImpl* pShaderImpl) noexcept : m_pShaderImpl(pShaderImpl) {}
			~ShaderHolder() = default;
			ShaderHolder(const ShaderHolder&) = delete;
			ShaderHolder(ShaderHolder&& other) noexcept : m_pShaderImpl(other.m_pShaderImpl) { other.m_pShaderImpl = nullptr; }

			ShaderHolder& operator = (const ShaderHolder&) = delete;
			ShaderHolder& operator = (ShaderHolder&& other) = delete;
			ShaderHolder& operator = (nullptr_t) { m_pShaderImpl = nullptr; return *this; }

			YmTngnShaderImpl* Get() const { return m_pShaderImpl; }
			YmTngnShaderImpl* operator -> () const { return m_pShaderImpl; }

			operator bool() const { return m_pShaderImpl != nullptr; }
			bool operator !() const { return m_pShaderImpl == nullptr; }
		private:
			YmTngnShaderImpl* m_pShaderImpl;
		};
	}

	class YmTngnDrawModelMatrixSetter
	{
	public:
		YmTngnDrawModelMatrixSetter(YmTngnShaderImpl* pShaderImpl);
		YmTngnDrawModelMatrixSetter(YmTngnDrawModelMatrixSetter&& setter) = default;
		~YmTngnDrawModelMatrixSetter();

		/// <summary>
		/// 
		/// </summary>
		/// <param name="pMatrix"></param>
		/// <returns>true if model matrix has been changed.</returns>
		bool Set(const YmTngnModelMatrixPtr& pMatrix);
		/// <summary>
		/// 
		/// </summary>
		/// <param name="pMatrix"></param>
		/// <returns>true if model matrix has been changed.</returns>
		bool SetIdentity();

		static void SetIdentity(YmTngnShaderImpl* pShaderImpl);

	private:
		void SetIdentityImpl() { SetIdentity(m_pShader.Get()); }
	private:
		YmTngnDrawUtil::ShaderHolder m_pShader;
		DirectX::XMFLOAT4X4* m_pLastMatrix = nullptr;
	};
}
