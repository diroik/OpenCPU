/*
 * flash.h
 *
 *  Created on: 16 июня 2021 г.
 *      Author: Админ
 */

#ifndef FLASH_H_
#define FLASH_H_

#include "ql_type.h"
#include "ql_trace.h"
#include "ril_system.h"
#include "typedef.h"

 u16 	calc_settings_crc(sProgrammSettings *sett);
 bool 	init_flash(sProgrammSettings *sett_in_ram);
 bool 	write_to_flash_settings(sProgrammSettings *sett);
 bool 	restore_default_flash(sProgrammSettings *sett_in_ram);


#endif /* FLASH_H_ */
