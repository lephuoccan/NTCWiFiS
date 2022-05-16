/*
 * circular_buffer.h
 *
 *  Created on: Aug 2, 2021
 *      Author: anhtu
 */

#ifndef __CIRCULAR_BUFFER_H__
#define __CIRCULAR_BUFFER_H__

/* Include -------------------------------------------------------------------*/
#include <stdint.h>

/* Macro ---------------------------------------------------------------------*/
#define CIRC_BUFF_EMPTY						0x00
#define CIRC_BUFF_FULL						0xFF
#define CIRC_BUFF_E_OK						0x01

#define CIRC_BUFF_INIT(buff,size)			\
	uint8_t uc_a##buff[size];				\
	Circ_Buff_TypeDef buff = {				\
			.uc_header = 0,					\
			.uc_tail = 0,					\
			.uc_maxlen = size,				\
			.uc_pBuff = uc_a##buff			\
	}										\

typedef struct _Circ_Buff_TypeDef {
	uint16_t uc_header;
	uint16_t uc_tail;
	const uint16_t uc_maxlen;
	uint8_t *uc_pBuff;
}Circ_Buff_TypeDef;

#ifdef __cplusplus
extern "C" {
#endif
  uint8_t Circ_Buff_Push(Circ_Buff_TypeDef *pCirc_Buff, uint8_t uc_Data);
  uint8_t Circ_Buff_Pop(Circ_Buff_TypeDef *pCirc_Buff, uint8_t *uc_Data);

#ifdef __cplusplus
} // extern "C"
#endif

#endif /* INC_CIRCULAR_BUFFER_H_ */
