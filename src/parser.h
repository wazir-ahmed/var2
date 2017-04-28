#ifndef _PARSER_H_INCLUDED
#define _PARSER_H_INCLUDED

uint8_t parse_cont_flag(uint8_t);
uint32_t parse_payload_len(uint8_t*, size_t);
uint8_t parse_packet_t(uint8_t);
uint8_t parse_version(uint8_t);
uint32_t parse_address(uint8_t*, size_t);

uint8_t change_packt(int, uint8_t);

#endif
