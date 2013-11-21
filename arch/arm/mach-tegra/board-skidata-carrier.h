/*
 * arch/arm/mach-tegra/board-skidata-carrier.h
 *
 * Copyright (C) 2010 Google, Inc.
 * Copyright (C) 2013 Avionic Design GmbH
 *
 * This software is licensed under the terms of the GNU General Public
 * License version 2, as published by the Free Software Foundation, and
 * may be copied, distributed, and modified under those terms.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 */

#ifndef _MACH_TEGRA_BOARD_MEDCOM_WIDE_H
#define _MACH_TEGRA_BOARD_MEDCOM_WIDE_H

#include "com-tamonten.h"
#include "tamonten-wm8903.h"
#include "tamonten-adnp.h"
#include "tamonten-tsc2007.h"

#define SKIDATA_GPIO_CPLD_IRQ		COM_GPIO_0
#define SKIDATA_GPIO_SERVICE		COM_GPIO_1

#define SKIDATA_GPIO_MMC_nCD_DEBOUNCED	BOARD_GPIO(ADNP, 15)
#define SKIDATA_GPIO_SOFT_RST_DEBOUNCED	BOARD_GPIO(ADNP, 16)

#define SKIDATA_GPIO_FAN_CONTROL	BOARD_GPIO(ADNP, 20)

#define SKIDATA_GPIO_OPT_B		BOARD_GPIO(ADNP, 27)
#define SKIDATA_GPIO_OPT_G		BOARD_GPIO(ADNP, 28)
#define SKIDATA_GPIO_OPT_R		BOARD_GPIO(ADNP, 29)

#define SKIDATA_GPIO_CCD_nRST		BOARD_GPIO(ADNP, 32)
#define SKIDATA_GPIO_CCD_nPWRDN		BOARD_GPIO(ADNP, 33)
#define SKIDATA_GPIO_SO_GPIO1		BOARD_GPIO(ADNP, 34)
#define SKIDATA_GPIO_TOUCH_IRQ		BOARD_GPIO(ADNP, 35)
#define SKIDATA_GPIO_RTC_IRQ		BOARD_GPIO(ADNP, 36)

#define SKIDATA_GPIO_PLATFORM_ID0	BOARD_GPIO(ADNP, 44)
#define SKIDATA_GPIO_PLATFORM_ID1	BOARD_GPIO(ADNP, 45)
#define SKIDATA_GPIO_PLATFORM_ID2	BOARD_GPIO(ADNP, 46)
#define SKIDATA_GPIO_PLATFORM_ID3	BOARD_GPIO(ADNP, 47)

#define SKIDATA_IRQ_CPLD		COM_GPIO_TO_IRQ(SKIDATA_GPIO_CPLD_IRQ)
#define SKIDATA_IRQ_TOUCH		BOARD_GPIO_TO_IRQ(ADNP, SKIDATA_GPIO_TOUCH_IRQ)
#define SKIDATA_IRQ_RTC			BOARD_GPIO_TO_IRQ(ADNP, SKIDATA_GPIO_RTC_IRQ)

#endif
