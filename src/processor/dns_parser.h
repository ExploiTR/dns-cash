#pragma once
#include <stdint.h>
#include <string>

/*
@note
RFC 1035 - 4.1.1 - Header Section format
-----------------------------------------
									1  1  1  1  1  1
	  0  1  2  3  4  5  6  7  8  9  0  1  2  3  4  5
	+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+
	|                      ID                       |
	+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+
	|QR|   Opcode  |AA|TC|RD|RA|   Z    |   RCODE   |
	+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+
	|                    QDCOUNT                    |
	+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+
	|                    ANCOUNT                    |
	+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+
	|                    NSCOUNT                    |
	+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+
	|                    ARCOUNT                    |
	+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+
*/
struct DNSHeader {
	uint16_t id;        // Transaction ID
	uint8_t qr;         // Query (0) or Response (1) - 1 bit
	uint8_t opcode;     // Operation code - 4 bits
	uint8_t aa;         // Authoritative Answer - 1 bit
	uint8_t tc;         // Truncated - 1 bit
	uint8_t rd;         // Recursion Desired - 1 bit
	uint8_t ra;         // Recursion Available - 1 bit
	uint8_t z;          // Reserved - 1 bit
	uint8_t rcode;      // Response Code - 4 bits
	uint16_t qdcount;   // Question count
	uint16_t ancount;   // Answer count
	uint16_t nscount;   // Authority count
	uint16_t arcount;   // Additional count
};

/*
@note
RFC 1035 - 4.1.2 - Question Section format
-----------------------------------------
									1  1  1  1  1  1
	  0  1  2  3  4  5  6  7  8  9  0  1  2  3  4  5
	+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+
	|                                               |
	/                     QNAME                     / <-- this means variable length wow
	/                                               /
	+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+
	|                     QTYPE                     |
	+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+
	|                     QCLASS                    |
	+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+
*/
struct DNSQuestion {
	/*
	* QNAME     a domain name represented as a sequence of labels, where
                each label consists of a length octet followed by that
                number of octets.  The domain name terminates with the
                zero length octet for the null label of the root.  Note
                that this field may be an odd number of octets; no
                padding is used.
	*/

	//I'm not sure if using raw pointers will give
	//me any real gain here considering small string optimization
	//and domain name sizes so I'll keep it to string
	std::string qname;
	uint16_t qtype;
	uint16_t qclass;
};

struct DNSRequest {
	DNSHeader header;
	DNSQuestion question;
};

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
*/
bool parse_dns_query(const char* query, int qlen, DNSHeader& header, DNSQuestion& question);