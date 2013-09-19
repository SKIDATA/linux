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
#include "tamonten-tsc2007.h"

#define ADNP_GPIO(_x_)			(BOARD_GPIO_WM8903_LAST + (_x_))
#define ADNP_GPIO_TO_IRQ(_x_)		(INT_BOARD_BASE + 32 + ((_x_) - ADNP_GPIO(0)))
#define ADNP_IRQ(_x_)			ADNP_GPIO_TO_IRQ(ADNP_GPIO(_x_))

#define SKIDATA_GPIO_CPLD_IRQ		COM_GPIO_0
#define SKIDATA_GPIO_SERVICE		COM_GPIO_1

#define SKIDATA_GPIO_MMC_nCD_DEBOUNCED	ADNP_GPIO(15)
#define SKIDATA_GPIO_SOFT_RST_DEBOUNCED	ADNP_GPIO(16)

#define SKIDATA_GPIO_FAN_CONTROL	ADNP_GPIO(20)

#define SKIDATA_GPIO_OPT_B		ADNP_GPIO(27)
#define SKIDATA_GPIO_OPT_G		ADNP_GPIO(28)
#define SKIDATA_GPIO_OPT_R		ADNP_GPIO(29)

#define SKIDATA_GPIO_CCD_nRST		ADNP_GPIO(32)
#define SKIDATA_GPIO_CCD_nPWRDN		ADNP_GPIO(33)
#define SKIDATA_GPIO_SO_GPIO1		ADNP_GPIO(34)
#define SKIDATA_GPIO_TOUCH_IRQ		ADNP_GPIO(35)
#define SKIDATA_GPIO_RTC_IRQ		ADNP_GPIO(36)

#define SKIDATA_GPIO_PLATFORM_ID0	ADNP_GPIO(44)
#define SKIDATA_GPIO_PLATFORM_ID1	ADNP_GPIO(45)
#define SKIDATA_GPIO_PLATFORM_ID2	ADNP_GPIO(46)
#define SKIDATA_GPIO_PLATFORM_ID3	ADNP_GPIO(47)

#define SKIDATA_IRQ_CPLD		COM_GPIO_TO_IRQ(SKIDATA_GPIO_CPLD_IRQ)
#define SKIDATA_IRQ_TOUCH		ADNP_GPIO_TO_IRQ(SKIDATA_GPIO_TOUCH_IRQ)
#define SKIDATA_IRQ_RTC			ADNP_GPIO_TO_IRQ(SKIDATA_GPIO_RTC_IRQ)

#endif
