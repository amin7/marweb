/*
 * sdcard_ex.h
 *
 *  Created on: 22 ����. 2019 �.
 *      Author: ominenko
 */

#ifndef SDCARD_EX_H_
#define SDCARD_EX_H_

#include <SdFat.h>

void sdcard_info(sdfat::SdFat &sd, Stream &out_stream);


#endif /* SDCARD_EX_H_ */
