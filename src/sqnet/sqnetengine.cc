#include <sqnet/sqnet2.h>
#include "sqnetimp.h"

using namespace snqu::net2;
ISQServerEngine*
SNQU_CALL 
CreateSQNetworkEngine2(ISQServerSink *sink)
{
	return new snqu::net2::SQServerEngine(sink);
}