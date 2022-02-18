/*
 * infrastructure.c
 *
 *  Created on: 10 дек. 2021 г.
 *      Author: DROIK
 */
#include "infrastructure.h"

static float GetKoeff(float R)
{
  if(R < 802)//t <0
    return 0.88;

  else if( R >= 802 && R < 874)
  {// t>=0
    return 0.85;
  }
  else if( R >= 874 && R < 950)
  {//t=>10
    return 0.83;
  }
  else if( R >= 950 && R < 990)
  {//t=>20
    return 0.80;
  }
 /*else if( R >= 990 && R < 1029)
  {//t=>25
    return 0.83;
  }*/
  else if( R >= 1029 && R < 1108)
  {//t=>30
    return 0.78;
  }
  else if( R >= 1108 && R < 1192)
  {//t=>40
    return 0.75;
  }
  else if(R >= 1192)// t>50
    return 0.73;
  else
  {
    return 0.79;
  }
}

float GetTempValue(u32 adcValue)
{
	float U = adcValue/1000.0;
	float R = U/((3.3 - U)/RESISTOR);
	float d = (R - 1000.0)/(GetKoeff(R)*10.0);
	if( d > 199 ){
		return 199.0;
	}
	return 25.0 + d;
}
/*****************************************************************************
* Function:
*
* Description:
*
* Parameters:
*
* Return:
*
*****************************************************************************/
s32 GetInputValue(Enum_PinName *pin, s32 *cnt, u32 max_timeout, bool INV)
{
	s32 ret = -1;
	s32 st = Ql_GPIO_GetLevel(*pin);

	if(INV == TRUE){
		st = st == 0 ? 1 : 0;
	}

	if(st > 0){
		if( *cnt <  max_timeout)
			*(cnt) += 1;
		if(*cnt >= max_timeout){
			*(cnt) = max_timeout;
			ret = TRUE;
		}
	}
	else{
		if( *cnt >  0)
			*(cnt) -= 1;
		if(*cnt <= 0){
			*(cnt) = 0;
			ret = FALSE;
		}
	}
	return ret;
}

s32 ReadSerialPort(Enum_SerialPort port, /*[out]*/char* pBuffer, /*[in]*/u32 bufLen)
{

    s32 rdLen = 0;
    s32 rdTotalLen = 0;
    if (NULL == pBuffer || 0 == bufLen)
    {
        return -1;
    }
    memset(pBuffer, 0x0, bufLen);

    /*
    while (1)
    {
        rdLen = Ql_UART_Read(port, (u8*)(pBuffer + rdTotalLen), bufLen - rdTotalLen);
        if (rdLen <= 0)  // All data is read out, or Serial Port Error!
        {
            break;
        }
        rdTotalLen += rdLen;
        // Continue to read...
    }
    */
    if (rdLen < 0) // Serial Port Error!
    {
        APP_DEBUG("<--Fail to read from port[%d]-->\r\n", port);
        return -99;
    }
    return rdTotalLen;
}

#ifdef __PROJECT_SMART_BUTTON__

void reboot(sProgrammData *programmData)
{
	APP_DEBUG("<-- Rebooting -->\r\n");

	Ql_Sleep(2000);
	Ql_Reset(0);
}

#else

void reboot(sProgrammData *programmData)
{
    //u64 totalMS;
    //totalMS = Ql_GetMsSincePwrOn();
	APP_DEBUG("<-- Rebooting -->\r\n");

	programmData->needReboot = TRUE;
	//Ql_Sleep(5000);
	//Ql_Reset(0);
}

#endif
