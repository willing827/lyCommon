#pragma once

#include <sqstd/sqinc.h>
#include <Sddl.h>

namespace snqu { namespace common { 
class WinSAHelper
{
public:
	inline WinSAHelper()
	{
		m_sa.nLength = sizeof(SECURITY_ATTRIBUTES);
		m_sa.bInheritHandle = FALSE;
		LPCSTR szSD = "D:P"
			"(D;OICI;GA;;;BG)"
			"(A;OICI;GA;;;SY)"
			"(A;OICI;GA;;;BA)"
			"(A;OICI;GRGWGX;;;IU)";

		ConvertStringSecurityDescriptorToSecurityDescriptorA(szSD,SDDL_REVISION_1,&(m_sa.lpSecurityDescriptor),NULL);
	}
	
	virtual ~WinSAHelper()
	{
		if (m_sa.lpSecurityDescriptor != NULL)
		{
			LocalFree(m_sa.lpSecurityDescriptor);
			m_sa.lpSecurityDescriptor = NULL;
		}
	}

	inline operator LPSECURITY_ATTRIBUTES()
	{
		return &m_sa;
	}

private:
	SECURITY_ATTRIBUTES m_sa;

};

}}