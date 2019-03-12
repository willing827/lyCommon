#include <sqnet/sqnethelper2.h>


info_msg* info_msg::Construct(CONNID dwConnID, LPCTSTR lpszEvent, int iContentLength, LPCTSTR lpszContent)
{
	return new info_msg(dwConnID, lpszEvent, iContentLength, lpszContent);
}

void info_msg::Destruct(info_msg* pMsg)
{
	delete pMsg;
}

info_msg::info_msg(CONNID dwConnID, LPCTSTR lpszEvent, int iContentLength, LPCTSTR lpszContent)
	: connID(dwConnID), evt(lpszEvent), contentLength(iContentLength), content(lpszContent)
{
}

info_msg::~info_msg()
{
	if(contentLength > 0)
		delete[] content;
}
