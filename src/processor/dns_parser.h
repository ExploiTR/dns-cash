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

	//I'm avoiding runtime re-alloc at the cost of stack
	//not sure if this was a de-optimization
	char qname[256];
	uint16_t qtype;
	uint16_t qclass;

	//Operator overload for using it in Cache key
	bool operator==(const DNSQuestion& other) const {
		return memcmp(qname, other.qname, 256) == 0 &&
			qtype == other.qtype &&
			qclass == other.qclass;
	}
};


/*
@note RFC 1035 - 4.1.3

  0  1  2  3  4  5  6  7  8  9 10 11 12 13 14 15
+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+
|                                               |
/                                               /
/                      NAME                     /
|                                               |
+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+
|                      TYPE                     |
+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+
|                     CLASS                     |
+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+
|                      TTL                      |
|                                               |
+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+
|                   RDLENGTH                    |
+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+
/                     RDATA                     /
/                                               /
+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+


@remarks
--> We don't necessarily need 'name', 'type', 'class' here if they are already in the Key.
*/

struct DNSAnswer {
	uint64_t expiry; // need to read ttl then add to UTC (steady clock)
	uint16_t rdlen;

	//For a modern server (even a laptop), wasting 500MB to save 
	//millions of malloc/free calls and gain raw speed is a fantastic trade-off.
	uint8_t rdata[512];
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


/*
* This function will read from input query, until a zero is found (end of name/data) and append to the buffer.
* Handles '.' addition and null-termination. See below note for details.
*
* @param query The input Query
* @param start_qry_idx Where to start reading in the query (!! PBR)
* @param sink_buf Where to write the read query
* @param start_sink_idx Where in the buffer to write the read query (!! PBR)
* @param jump_count For message compression, we need to track jumps for RFC 9267
*
* @returns bool If process is successful.
*
* @note
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
*/
bool parse_dnsq_internal_w_cmpr(const char* query, uint_fast16_t& start_qry_idx, char* sink_buf, uint_fast16_t& start_sink_idx, uint_fast8_t jump_count = 0);

/*
	For maps to work, we need a custom hash function for the key type.
	which has to be provided or otherwise, we'd get a similar error like mentioned below

	'std::hash<DNSQuestion>::hash(void)': attempting to reference a deleted function
*/
namespace std {
	template<>
	struct hash<DNSQuestion> {
		size_t operator()(const DNSQuestion& question) const {
			size_t seed = 0;

			auto hash_combine = [](size_t& seed, const size_t& value)
				{
					// 0x9e3779b97f4a7c15 -> 2^64 divided by golden ratio 1.618
					seed ^= value + 0x9e3779b97f4a7c15 + (seed << 6) + (seed >> 2);
				};

			hash_combine(seed, hash<string>()(question.qname));
			hash_combine(seed, hash<uint16_t>()(question.qtype));
			hash_combine(seed, hash<uint16_t>()(question.qclass));

			return seed;
		}
	};
}