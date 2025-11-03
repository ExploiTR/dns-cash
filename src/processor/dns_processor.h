#pragma once
#include <stdint.h>
/*
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
	uint16_t qname;
	uint16_t qtype;
	uint16_t qclass;
};