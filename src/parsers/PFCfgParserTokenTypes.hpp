#ifndef INC_PFCfgParserTokenTypes_hpp_
#define INC_PFCfgParserTokenTypes_hpp_

/* $ANTLR 2.7.7 (20090306): "pf.g" -> "PFCfgParserTokenTypes.hpp"$ */

#ifndef CUSTOM_API
# define CUSTOM_API
#endif

#ifdef __cplusplus
struct CUSTOM_API PFCfgParserTokenTypes {
#endif
	enum {
		EOF_ = 1,
		NEWLINE = 4,
		LINE_COMMENT = 5,
		WORD = 6,
		EQUAL = 7,
		ANTISPOOF = 8,
		ALTQ = 9,
		QUEUE = 10,
		SET = 11,
		SCRUB = 12,
		TABLE = 13,
		LESS_THAN = 14,
		GREATER_THAN = 15,
		PERSIST = 16,
		CONST = 17,
		COUNTERS = 18,
		FILE = 19,
		STRING = 20,
		OPENING_BRACE = 21,
		COMMA = 22,
		CLOSING_BRACE = 23,
		EXLAMATION = 24,
		COLON = 25,
		NETWORK = 26,
		BROADCAST = 27,
		PEER = 28,
		INT_CONST = 29,
		SELF = 30,
		IPV4 = 31,
		SLASH = 32,
		NAT = 33,
		PASS = 34,
		MINUS = 35,
		STATIC_PORT = 36,
		OPENING_PAREN = 37,
		CLOSING_PAREN = 38,
		PORT = 39,
		STAR = 40,
		BITMASK = 41,
		RANDOM = 42,
		SOURCE_HASH = 43,
		HEX_KEY = 44,
		STRING_KEY = 45,
		ROUND_ROBIN = 46,
		STICKY_ADDRESS = 47,
		BINAT = 48,
		RDR = 49,
		TIMEOUT = 50,
		BLOCK = 51,
		DROP = 52,
		RETURN = 53,
		RETURN_RST = 54,
		TTL = 55,
		RETURN_ICMP = 56,
		RETURN_ICMP6 = 57,
		IN = 58,
		OUT = 59,
		LOG = 60,
		ALL = 61,
		USER = 62,
		TO = 63,
		QUICK = 64,
		ON = 65,
		INET = 66,
		INET6 = 67,
		PROTO = 68,
		IP = 69,
		ICMP = 70,
		IGMP = 71,
		TCP = 72,
		UDP = 73,
		RDP = 74,
		RSVP = 75,
		GRE = 76,
		ESP = 77,
		AH = 78,
		EIGRP = 79,
		OSPF = 80,
		IPIP = 81,
		VRRP = 82,
		L2TP = 83,
		ISIS = 84,
		FROM = 85,
		URPF_FAILED = 86,
		ANY = 87,
		NO_ROUTE = 88,
		IPV6 = 89,
		ROUTE_TO = 90,
		REPLY_TO = 91,
		FLAGS = 92,
		ICMP_TYPE = 93,
		ICMP_CODE = 94,
		ICMP6_TYPE = 95,
		TAGGED = 96,
		TAG = 97,
		NO = 98,
		KEEP = 99,
		MODULATE = 100,
		SYNPROXY = 101,
		STATE = 102,
		LABEL = 103,
		EXIT = 104,
		QUIT = 105,
		INTRFACE = 106,
		ICMP6 = 107,
		IGRP = 108,
		IPSEC = 109,
		NOS = 110,
		PCP = 111,
		PIM = 112,
		PPTP = 113,
		RIP = 114,
		SNP = 115,
		HOST = 116,
		RANGE = 117,
		LOG_LEVEL_ALERTS = 118,
		LOG_LEVEL_CRITICAL = 119,
		LOG_LEVEL_DEBUGGING = 120,
		LOG_LEVEL_EMERGENCIES = 121,
		LOG_LEVEL_ERRORS = 122,
		LOG_LEVEL_INFORMATIONAL = 123,
		LOG_LEVEL_NOTIFICATIONS = 124,
		LOG_LEVEL_WARNINGS = 125,
		LOG_LEVEL_DISABLE = 126,
		LOG_LEVEL_INACTIVE = 127,
		Whitespace = 128,
		HEX_CONST = 129,
		NUMBER = 130,
		NEG_INT_CONST = 131,
		HEX_DIGIT = 132,
		DIGIT = 133,
		NUM_3DIGIT = 134,
		NUM_HEX_4DIGIT = 135,
		NUMBER_ADDRESS_OR_WORD = 136,
		PIPE_CHAR = 137,
		NUMBER_SIGN = 138,
		PERCENT = 139,
		AMPERSAND = 140,
		APOSTROPHE = 141,
		PLUS = 142,
		DOT = 143,
		SEMICOLON = 144,
		QUESTION = 145,
		COMMERCIAL_AT = 146,
		OPENING_SQUARE = 147,
		CLOSING_SQUARE = 148,
		CARET = 149,
		UNDERLINE = 150,
		TILDE = 151,
		DOUBLE_QUOTE = 152,
		NULL_TREE_LOOKAHEAD = 3
	};
#ifdef __cplusplus
};
#endif
#endif /*INC_PFCfgParserTokenTypes_hpp_*/