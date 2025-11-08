#include "dns_processor.h"
#include <exception>
#include <WinSock2.h>


void extract(const char* msg, int len) {
	if (len < 12)
		throw std::exception("Error: Message too short for DNS header");


	/*
	* RFC 1035 - 4.1.1 - Header section format
	* -----------------------------------------
	* ID              A 16 bit -> 2b
	* QR              A one bit -> 0.125b ??
	* OPCODE          A four bit -> 0.5b
	* RCODE           Response code - this 4 bit field -> 0.5b
	* QDCOUNT         an unsigned 16 bit -> 2b
	* ANCOUNT         an unsigned 16 bit -> 2b
	* NSCOUNT         an unsigned 16 bit -> 2b
	* ARCOUNT         an unsigned 16 bit -> 2b
	* -----------------------------------------
	* Bytes 2-3 (ONE 16-bit field):
	* |QR| Opcode  |AA|TC|RD|RA|   Z    |   RCODE   |
	*  1    4       1   1  1  1    3         4
	* = 16 bits total
	* -----------------------------------------
	* Total :
	* Bytes 0-1: ID (16 bits)
	* Bytes 2-3: FLAGS (16 bits, packed with: QR, Opcode, AA, TC, RD, RA, Z, RCODE)
	* Bytes 4-5: QDCOUNT (16 bits)
	* Bytes 6-7: ANCOUNT (16 bits)
	* Bytes 8-9: NSCOUNT (16 bits)
	* Bytes 10-11: ARCOUNT (16 bits)
	*/

	DNSHeader header;

	//read 16bit(2 bytes) starting from msg[0]'s address then 
	//dereference to get the value and use ntohs to convert network to host order.
	//header is pretty much straightforward
	header.id = ntohs((*(uint16_t*)&msg[0]));

	uint16_t flags = ntohs(*(uint16_t*)&msg[2]);
	header.qr = flags >> 15;
	header.opcode = flags >> 11 & 0b1111;
	header.aa = flags >> 10 & 0b1;
	header.tc = flags >> 9 & 0b1;
	header.rd = flags >> 8 & 0b1;
	header.ra = flags >> 7 & 0b1;
	header.z = flags >> 4 & 0b111;
	header.rcode = flags & 0b1111;

	header.qdcount = ntohs(*(uint16_t*)&msg[4]);
	header.ancount = ntohs(*(uint16_t*)&msg[6]);
	header.nscount = ntohs(*(uint16_t*)&msg[8]);
	header.arcount = ntohs(*(uint16_t*)&msg[10]);

	/*
	* For example, a datagram might need to use the domain names F.ISI.ARPA,
	* FOO.F.ISI.ARPA, ARPA, and the root.  Ignoring the other fields of the
	* message, these domain names might be represented as:
	*
	* 	   +--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+
	* 	20 |           1           |           F           |
	* 	   +--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+
	* 	22 |           3           |           I           |
	* 	   +--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+
	* 	24 |           S           |           I           |
	* 	   +--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+
	* 	26 |           4           |           A           |
	* 	   +--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+
	* 	28 |           R           |           P           |
	* 	   +--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+
	* 	30 |           A           |           0           |
	* 	   +--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+
	*
	* 	   +--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+
	* 	40 |           3           |           F           |
	* 	   +--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+
	* 	42 |           O           |           O           |
	* 	   +--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+
	* 	44 | 1  1|                20                       |
	* 	   +--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+
	*
	* 	   +--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+
	* 	64 | 1  1|                26                       |
	* 	   +--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+
	*
	* 	   +--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+
	* 	92 |           0           |                       |
	* 	   +--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+
	*
	* The domain name for F.ISI.ARPA is shown at offset 20.  The domain name
	* FOO.F.ISI.ARPA is shown at offset 40; this definition uses a pointer to
	* concatenate a label for FOO to the previously defined F.ISI.ARPA.  The
	* domain name ARPA is defined at offset 64 using a pointer to the ARPA
	* component of the name F.ISI.ARPA at 20; note that this pointer relies on
	* ARPA being the last label in the string at 20.  The root domain name is
	*
	* =======================================================================
	* RFC 1035 -  4.1.2. Question section format
	*
	*									1  1  1  1  1  1
	*	  0  1  2  3  4  5  6  7  8  9  0  1  2  3  4  5
	*	+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+
	*	|                                               |
	*	/                     QNAME                     /
	*	/                                               /
	*	+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+
	*	|                     QTYPE                     |
	*	+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+
	*	|                     QCLASS                    |
	*	+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+
	*/

	DNSQuestion question;
	uint_fast8_t qaddr = 12;

	std::string buffer;
	//should I use a unordered_map or a index-array for message-compression storage?

	while (*(uint8_t*)&msg[qaddr] != 0) {
		uint8_t len = *(uint8_t*)&msg[qaddr];

		buffer.append(&msg[qaddr + 1], len)
			.append(".");

		qaddr += len + 1;
	}

	question.qname = std::move(buffer);
	qaddr++; //move to next start

	question.qtype = (*(uint16_t*)&msg[qaddr]);
	qaddr++;

	question.qclass = (*(uint16_t*)&msg[qaddr]);
	qaddr++;


	//question.qname
}