/* Includes ------------------------------------------------------------------*/
#include "stm32f1xx_hal.h"

#include "string.h"
#include "can.h"
#include "canapi.h"

/* Private typedef -----------------------------------------------------------*/
/* Private define ------------------------------------------------------------*/
#define CRC8_F       /* activate the CRC8 integrity */
/* Private macro -------------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/
/* @note ATTENTION - please keep this variable 32bit alligned */
static CanTxMsgTypeDef transmitMsg;
static CanRxMsgTypeDef receiveMsg;

/* Private function prototypes -----------------------------------------------*/
uint16_t UpdateCRC16(uint16_t crc_in, uint8_t byte);
uint16_t Cal_CRC16(const uint8_t* p_data, uint32_t size);
uint8_t CalcChecksum(const uint8_t *p_data, uint32_t size);

/* Private functions ---------------------------------------------------------*/


/**
 * @brief  Receive a packet from sender
 * @param  data
 * @param  length
 *     0: end of transmission
 *     2: abort by sender
 *    >0: packet length
 * @param  timeout
 * @retval HAL_OK: normally return
 *         HAL_BUSY: abort by user
 */
PacketStatus_TypeDef CAN_Receive_Packet(uint8_t *p_data, uint32_t* len, uint32_t timeout, int skipAck = 0) {
	uint32_t crc;
	uint32_t received = 0;
	uint32_t packet_length = 0;
	HAL_StatusTypeDef status = HAL_TIMEOUT;
	PacketStatus_TypeDef result = RX_TIMEOUT;
	uint8_t char1;

	hcan.pTxMsg = &transmitMsg;
	hcan.pRxMsg = &receiveMsg;

	do {
		status = HAL_CAN_Receive(&hcan, CAN_FIFO0, timeout);
	}
	while ((status == HAL_OK) && (hcan.pRxMsg->DLC != 5));

	if ((status == HAL_OK) && (hcan.pRxMsg->DLC == 5) && (hcan.pRxMsg->Data[0] == START_OF_PACKET))
	{

				packet_length = *((uint32_t*) &hcan.pRxMsg->Data[1]);

				while ((received < packet_length) && (status == HAL_OK))
				{
					status = HAL_CAN_Receive(&hcan, CAN_FIFO0, timeout);

					memcpy(p_data, hcan.pRxMsg->Data, hcan.pRxMsg->DLC);
					p_data += hcan.pRxMsg->DLC;
					received += hcan.pRxMsg->DLC;

				}

				if (received == packet_length)
				{
					*len = packet_length;
					if (!skipAck)
					{
						hcan.pTxMsg->DLC = 1;
						hcan.pTxMsg->Data[0] = END_OF_PACKET_OK;
						HAL_Delay(100);
						status = HAL_CAN_Transmit(&hcan, timeout);
					}
					result = PACKET_OK;

				}
				else
				{
					result = PACKET_BAD;
				}


	}
	else
	{
		result = RX_TIMEOUT;
	}


	return result;
}


HAL_StatusTypeDef CAN_Ack_Packet() {
	hcan.pTxMsg->DLC = 1;
	hcan.pTxMsg->Data[0] = END_OF_PACKET_OK;
	HAL_Delay(100);
	int status = HAL_CAN_Transmit(&hcan, 1000);
}

HAL_StatusTypeDef CAN_Transmit_Packet(uint8_t *pdata, uint32_t len, uint32_t timeout) {

	    int sent_bytes = 0;
	    int nbytes = 0;
	    HAL_StatusTypeDef status = HAL_ERROR;
	    HAL_CAN_StateTypeDef state;

	    hcan.pTxMsg->Data[0] = START_OF_PACKET;
		hcan.pTxMsg->DLC = 5;
	    *((uint32_t*) &hcan.pTxMsg->Data[1]) = len;

	    status = HAL_CAN_Transmit(&hcan, timeout);

	    while ((sent_bytes < len) && (status == HAL_OK))
	    {
	        nbytes = ((len - sent_bytes) > 8)? 8 : (len - sent_bytes);
	        memcpy(hcan.pTxMsg->Data, pdata, nbytes);
	        pdata += nbytes;
	        sent_bytes += nbytes;
	        hcan.pTxMsg->DLC = nbytes;

//	        while ((state = HAL_CAN_GetState(&hcan)) != HAL_CAN_STATE_READY)
//	        	vTaskDelay(50/ portTICK_PERIOD_MS);

//	        vTaskDelay(800/ portTICK_PERIOD_MS);
	        status = HAL_CAN_Transmit(&hcan, timeout);
	    }

	    if (status == HAL_OK)
	    {
	    	status = HAL_CAN_Receive(&hcan, CAN_FIFO0, timeout);


	    	if ((status == HAL_OK) && (hcan.pRxMsg->DLC == 1) && (hcan.pRxMsg->Data[0] == END_OF_PACKET_OK))
	    	{

	    	}
	    	else {
	    		state = HAL_CAN_GetState(&hcan);
	    	}

	    }
	    else
	    {
	    	state = HAL_CAN_GetState(&hcan);
        	//vTaskDelay(50/ portTICK_PERIOD_MS);
	    }
	    return status;
}



/*******************(C)COPYRIGHT 2015 STMicroelectronics *****END OF FILE****/
