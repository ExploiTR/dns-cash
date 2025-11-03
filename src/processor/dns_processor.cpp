#include "dns_processor.h"
#include <exception>
#include <WinSock2.h>

/*
RFC 1035 - 4.1.1 - Header section format
-----------------------------------------
ID              A 16 bit -> 2b
QR              A one bit -> 0.125b ??
OPCODE          A four bit -> 0.5b
RCODE           Response code - this 4 bit field -> 0.5b
QDCOUNT         an unsigned 16 bit -> 2b
ANCOUNT         an unsigned 16 bit -> 2b
NSCOUNT         an unsigned 16 bit -> 2b
ARCOUNT         an unsigned 16 bit -> 2b
-----------------------------------------
Bytes 2-3 (ONE 16-bit field):
|QR| Opcode  |AA|TC|RD|RA|   Z    |   RCODE   |
 1    4       1   1  1  1    3         4
= 16 bits total
-----------------------------------------
Total :
Bytes 0-1: ID (16 bits)
Bytes 2-3: FLAGS (16 bits, packed with: QR, Opcode, AA, TC, RD, RA, Z, RCODE)
Bytes 4-5: QDCOUNT (16 bits)
Bytes 6-7: ANCOUNT (16 bits)
Bytes 8-9: NSCOUNT (16 bits)
Bytes 10-11: ARCOUNT (16 bits)
*/

void extract(const char* msg, int len) {
	if (len < 12)
		throw std::exception("Error: Message too short for DNS header");

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
	* 3.1 - Name Space Defn
	* Domain names in messages are expressed in terms of a sequence of labels.
	* Each label is represented as a one octet length field followed by that
	* number of octets. Since every domain name ends with the null label of
	* the root, a domain name is terminated by a length byte of zero.
	* Sample : \x06google\x03com\x00
	* 
	*/
	DNSQuestion question;

	//question.qname
}