#include<stdint.h>
#include<stddef.h>

#include "parser.h"

uint8_t parse_cont_flag(uint8_t byte) {
    return byte & 0b00000001;
}

uint32_t parse_payload_len(uint8_t* bytes, size_t field_len) {
    int i;
    uint32_t payload_len = 0;
    for(i = 0; i < field_len; i++) {
        payload_len = payload_len << 8;
        payload_len = payload_len | bytes[i];
    }
    return payload_len;
}

uint8_t parse_packet_t(uint8_t byte) {
    return byte & 0b00001111;
}

uint8_t parse_version(uint8_t byte) {
    return byte >> 4;
}

uint32_t parse_address(uint8_t *bytes, size_t field_len) {
    int i;
    uint32_t address = 0;
    for(i=0; i < field_len; i++ ) {
        address = address << 8;
        address = address | bytes[i];
    }
    return address >> 1;
}

uint8_t change_packt(int type, uint8_t byte) {
    uint8_t ver = byte & 0b11110000; 
    uint8_t packt = type & 0b00001111;

    return ver | packt;
}
