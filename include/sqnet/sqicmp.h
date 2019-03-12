#ifndef SQICMP_H
#define SQICMP_H

#include <sqstd/sqinc.h>
#include <sqstd/sqtaskthread.h>

/* TCP header */
typedef u_int tcp_seq;

typedef struct _sniff_tcp {
	unsigned short tcp_source_port;
	unsigned short tcp_dest_port;
	unsigned int tcp_sequence;
	unsigned int tcp_acknowledge;
	unsigned char tcp_ns :1;
	unsigned char tcp_reserved_part1:3; 
	unsigned char tcp_data_offset:4;

	unsigned char tcp_fin :1;
	unsigned char tcp_syn :1;
	unsigned char tcp_rst :1;
	unsigned char tcp_psh :1;
	unsigned char tcp_ack :1;
	unsigned char tcp_urg :1;
	unsigned char tcp_ecn :1;
	unsigned char tcp_cwr :1;

	unsigned short tcp_window;
	unsigned short tcp_checksum;
	unsigned short tcp_urgent;
} sniff_tcp_t;

/* IP header */
typedef struct _sniff_ip {
	unsigned char ip_header_len:4; 
	unsigned char ip_version :4;
	unsigned char ip_tos;
	unsigned short ip_total_length; 
	unsigned short ip_id;
	unsigned char ip_frag_offset :5; 
	unsigned char ip_more_fragment :1;
	unsigned char ip_dont_fragment :1;
	unsigned char ip_reserved_zero :1;
	unsigned char ip_frag_offset1; 
	unsigned char ip_ttl;
	unsigned char ip_protocol;
	unsigned short ip_checksum;
	unsigned int ip_srcaddr;
	unsigned int ip_destaddr;
} sniff_ip_t;


typedef struct icmphdr 
{ 
	BYTE i_type; 
	BYTE i_code; /* type sub code */ 
	USHORT i_cksum; 
	USHORT i_id; 
	USHORT i_seq; 
	/* This is not the std header, but we reserve space for time */ 
	ULONG timestamp; 
}IcmpHeader; 


/* pseudo header used for tcp checksuming
* a not so well documented fact ... in public
*/
typedef u_long	u_int32_t;
typedef u_short u_int16_t;

typedef struct _pseudo_hdr {
	unsigned long src;
	unsigned long dst;
	unsigned char mbz;
	unsigned char proto;
	unsigned short len;
} pseudo_hdr_t;


#define IPHDRSIZE sizeof(sniff_ip_t)
#define TCPHDRSIZE sizeof(sniff_tcp_t)
#define PSEUDOHDRSIZE sizeof(pseudo_hdr_t)


enum 
{
	IP32_STEP = 0x1000000,
	HOST_COUNT = 253,
};


#define DEF_ICMP_PACKET_SIZE		32
#define DEF_ICMP_PACKET_NUMBER		2
#define ICMP_ECHO					8 
#define ICMP_ECHOREPLY				0 
#define ICMP_MIN					8 // minimum 8 byte icmp packet (just header)

namespace snqu { namespace net2 { 
	class SQ_ICMP
	{
	public:
		SQ_ICMP();
		virtual ~SQ_ICMP();

	public:
		bool ping(const string& host_ip);

	protected:
		u_short check_sum(u_short *buffer, int size);
		void send_icmp_packet(ulong32 host_ip);
		void fill_icmp_data(char * icmp_data, int datasize);
		bool decode_icmp_packet(char *buf, int bytes, struct sockaddr_in *from);

	private:
		SOCKET  m_icmp_sock;
		bool    m_result;
	};
}}
#endif //SQICMP_H