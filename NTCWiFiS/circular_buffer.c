/*
 * circular_buffer.c
 *
 *  Created on: Aug 2, 2021
 *      Author: anhtu
 */

/* Includes ------------------------------------------------------------------*/
#include "circular_buffer.h"

uint8_t Circ_Buff_Push(Circ_Buff_TypeDef *pCirc_Buff, uint8_t uc_Data)
{
	uint16_t Luc_Next_Index = 0;
	uint16_t Luc_tail = pCirc_Buff->uc_tail;
	uint16_t Luc_header = pCirc_Buff->uc_header;

	Luc_Next_Index = Luc_header + 1;
	if(Luc_Next_Index >= pCirc_Buff->uc_maxlen)
	{
		Luc_Next_Index = 0;
	}
	else
	{

	}
	if(Luc_Next_Index == Luc_tail)
	{
		return CIRC_BUFF_FULL;
	}
	else
	{

	}
	pCirc_Buff->uc_pBuff[Luc_header] = uc_Data;
	pCirc_Buff->uc_header = Luc_Next_Index;
	return CIRC_BUFF_E_OK;
}

uint8_t Circ_Buff_Pop(Circ_Buff_TypeDef *pCirc_Buff, uint8_t *uc_Data)
{
	uint16_t Luc_Next_Index = 0;
	uint16_t Luc_tail = pCirc_Buff->uc_tail;
	uint16_t Luc_header = pCirc_Buff->uc_header;

	if(Luc_header == Luc_tail)
	{
		return CIRC_BUFF_EMPTY;
	}
	else
	{

	}
	Luc_Next_Index = Luc_tail + 1;
	if(Luc_Next_Index >= pCirc_Buff->uc_maxlen)
	{
		Luc_Next_Index = 0;
	}
	else
	{

	}
	*uc_Data = pCirc_Buff->uc_pBuff[Luc_tail];
	pCirc_Buff->uc_tail = Luc_Next_Index;
	return CIRC_BUFF_E_OK;
}
