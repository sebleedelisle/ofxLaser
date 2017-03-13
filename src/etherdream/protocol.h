/* j4cDAC protocol definition
 *
 * Copyright 2011 Jacob Potter
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of either the GNU General Public License version 2
 * or 3, or the GNU Lesser General Public License version 3, as published
 * by the Free Software Foundation, at your option.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef PROTOCOL_H
#define PROTOCOL_H

#include <stdint.h>

#ifdef _MSC_VER
typedef unsigned int uint32_t;
typedef unsigned short uint16_t;
typedef short int16_t;
typedef unsigned char uint8_t;
#pragma pack(1)
#define __attribute__(x)
#else
#include <stdint.h>
#endif

typedef struct dac_point {
	uint16_t control;
	int16_t x;
	int16_t y;
	uint16_t r;
	uint16_t g;
	uint16_t b;
	uint16_t i;
	uint16_t u1;
	uint16_t u2;
} dac_point_t;

#define DAC_CTRL_RATE_CHANGE    0x8000

struct dac_status {
	uint8_t protocol;
	uint8_t light_engine_state;
	uint8_t playback_state;
	uint8_t source;
	uint16_t light_engine_flags;
	uint16_t playback_flags;
	uint16_t source_flags;
	uint16_t buffer_fullness;
	uint32_t point_rate;
	uint32_t point_count;
} __attribute__ ((packed));

struct dac_broadcast {
	uint8_t mac_address[6];
	uint16_t hw_revision;
	uint16_t sw_revision;
	uint16_t buffer_capacity;
	uint32_t max_point_rate;
        struct dac_status status;
} __attribute__ ((packed));

struct begin_command {
	uint8_t command;	/* 'b' (0x62) */
	uint16_t low_water_mark;
	uint32_t point_rate;
} __attribute__ ((packed));

struct queue_command {
	uint8_t command;	/* 'q' (0x74) */
	uint32_t point_rate;
} __attribute__ ((packed));

struct data_command {
	uint8_t command;	/* 'd' (0x64) */
	uint16_t npoints;
	struct dac_point data[];
} __attribute__ ((packed));

struct data_command_header {
	uint8_t command;	/* 'd' (0x64) */
	uint16_t npoints;
} __attribute__ ((packed));

struct dac_response {
	uint8_t response;
	uint8_t command;
	struct dac_status dac_status;
} __attribute__ ((packed));

#define CONNCLOSED_USER		(1)
#define CONNCLOSED_UNKNOWNCMD	(2)
#define CONNCLOSED_SENDFAIL	(3)
#define CONNCLOSED_MASK		(0xF)

#define RESP_ACK		'a'
#define RESP_NAK_FULL		'F'
#define RESP_NAK_INVL		'I'
#define RESP_NAK_ESTOP		'!'

#define PLUGIN_SIZE		600

#endif
