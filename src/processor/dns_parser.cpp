#include "dns_parser.h"
#include <WinSock2.h>
#include <iostream>


/*
* This function will run in a thread to parse DNS Query information from the message itself.
* @param query Raw DNS Query Message from Client
* @param qlen Query length
* @return header DNS Header Information
* @return question DNS Question Information
* @return bool If function is safely executed.
*
* @remarks
*
*	4.1. Format
*
*	All communications inside of the domain protocol are carried in a single
*	format called a message.  The top level format of message is divided
*	into 5 sections (some of which are empty in certain cases) shown below:
*
*		+---------------------+
*		|        Header       |
*		+---------------------+
*		|       Question      | the question for the name server
*		+---------------------+
*		|        Answer       | RRs answering the question
*		+---------------------+
*		|      Authority      | RRs pointing toward an authority
*		+---------------------+
*		|      Additional     | RRs holding additional information
*		+---------------------+
*
* @note : Important Consideration
*
* Anti-Patters : https://www.rfc-editor.org/rfc/rfc9267.txt
* Original 1035 : https://datatracker.ietf.org/doc/html/rfc1035
*
*/
uint_fast16_t parse_dns_query(const char* query, const int query_len, DNSHeader& header, DNSQuestion& question) {
	if (query_len < 12) {
		std::cerr << "Error: Message too short for DNS header - Silently dropping the request." << std::endl;
		return false;
	}

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

	//read 16bit(2 bytes) starting from query[0]'s address then
	//dereference to get the value and use ntohs to convert network to host order.
	//header is pretty much straightforward
	header.id = ntohs((*(uint16_t*)&query[0]));

	const uint16_t flags = ntohs(*(uint16_t*)&query[2]);
	header.qr = flags >> 15;
	header.opcode = flags >> 11 & 0b1111;
	header.aa = flags >> 10 & 0b1;
	header.tc = flags >> 9 & 0b1;
	header.rd = flags >> 8 & 0b1;
	header.ra = flags >> 7 & 0b1;
	header.z = flags >> 4 & 0b111;
	header.rcode = flags & 0b1111;

	header.qdcount = ntohs(*(uint16_t*)&query[4]);
	header.ancount = ntohs(*(uint16_t*)&query[6]);
	header.nscount = ntohs(*(uint16_t*)&query[8]);
	header.arcount = ntohs(*(uint16_t*)&query[10]);

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
	* ARPA being the last label in the string at 20.
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

	//RFC 1035 DNS over UDP is limited to 512 bytes, but for safety
	uint_fast16_t qaddr = 12;

	//is this efficient enough? need profiling
	auto& buffer = question.qname;
	uint_fast16_t buffer_pos = 0;

	if (!parse_dnsq_internal_w_cmpr(query, qaddr, buffer, buffer_pos, 0)) return false;

	buffer[buffer_pos] = '\0';

	if (qaddr + 4 > query_len) return false;  // Need 4 bytes for QTYPE+QCLASS

	question.qtype = ntohs(*(uint16_t*)&query[qaddr]);
	qaddr += 2;

	question.qclass = ntohs(*(uint16_t*)&query[qaddr]);
	qaddr += 2;

	return qaddr;
}

bool parse_dnsq_internal_w_cmpr(const char* query, uint_fast16_t& qaddr, char* sink_buf, uint_fast16_t& bufaddr, uint_fast8_t jump_count)
{
	// RFC 9267: Prevent compression pointer loops
	if (jump_count > 5) return false;

	while (*(uint8_t*)&query[qaddr] != 0) {
		//read length from the first byte at the index
		const uint8_t len = *(uint8_t*)&query[qaddr];

		// Validate label length (max 63 per 4.1.4. Message compression - RFC 1035)
		if (len > 63 || bufaddr + len + 1 >= 255)
			return false;

		// message compression handling - 4.1.4
		// compression pointer check first 2 as 1
		if ((len & 0b11000000) == 0b11000000) {
			//take rest except first 2 in the 16bit
			uint_fast16_t cmpr_offset = 0b0011111111111111 & ntohs(*(uint16_t*)&query[qaddr]);

			//offset should not exceed qaddr
			if (cmpr_offset >= qaddr) return false;

			//parse compression
			if (const bool res = parse_dnsq_internal_w_cmpr(query, cmpr_offset, sink_buf, bufaddr, jump_count + 1); !res)
				return false;

			//skip to the next line (next to next byte)
			qaddr += 2;

			//a sequence of labels ending with a pointer
			return true;
		}

		// Copy label, write to the sink index from next byte of query skipping the length specififer byte
		memcpy(&sink_buf[bufaddr], &query[qaddr + 1], len);

		//update sink index with length
		bufaddr += len;

		//skip to the next byte
		qaddr += len + 1;

		// Add dot only if next byte not zero
		if (*(uint8_t*)&query[qaddr] != 0)
			sink_buf[bufaddr++] = '.';
	}

	++qaddr; //move to next start

	return true;
}
