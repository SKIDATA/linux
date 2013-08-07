/*
 * arch/arm/mach-tegra/board-skidata-carrier.c
 *
 * Copyright (C) 2013, Avionic Design GmbH
 *
 * This software is licensed under the terms of the GNU General Public
 * License version 2, as published by the Free Software Foundation, and
 * may be copied, distributed, and modified under those terms.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 */

#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/gpio.h>
#include <linux/i2c.h>
#include <linux/i2c/adnp.h>
#include <linux/regulator/machine.h>
#include <linux/regulator/fixed.h>

#include <asm/mach-types.h>
#include <asm/mach/arch.h>
#include <asm/mach/time.h>
#include <asm/setup.h>

#include <mach/dc.h>

#include "devices.h"
#include "board.h"
#include "board-skidata-carrier.h"
#include "gpio-names.h"

static struct tegra_gpio_table stc_com_gpio_table[] = {
	{ .gpio = SKIDATA_GPIO_SERVICE, .enable = true },
};

static void __init skidata_gpio_init(void)
{
	tegra_gpio_config(stc_com_gpio_table, ARRAY_SIZE(stc_com_gpio_table));
}

static struct adnp_platform_data stc_adnp_pdata = {
	.gpio_base = ADNP_GPIO(0),
	.nr_gpios = 64,
	.irq_base = ADNP_GPIO_TO_IRQ(ADNP_GPIO(0)),
	.names = NULL,
};

static struct i2c_board_info stc_i2c1_board_info[] __initdata = {
	{
		I2C_BOARD_INFO("gpio-adnp", 0x41),
		.platform_data = &stc_adnp_pdata,
		.irq = SKIDATA_IRQ_CPLD,
	},
};


static struct i2c_board_info stc_i2c2_gen2_board_info[] __initdata = {
	{
		I2C_BOARD_INFO("fm24cl64b", 0x51),
	},
	{
		I2C_BOARD_INFO("pcf8523", 0x68),
	},
};

static void __init skidata_i2c_init(void)
{
	i2c_register_board_info(0, stc_i2c1_board_info,
				ARRAY_SIZE(stc_i2c1_board_info));
	i2c_register_board_info(2, stc_i2c2_gen2_board_info,
				ARRAY_SIZE(stc_i2c2_gen2_board_info));
	tamonten_tsc2007_init(2, SKIDATA_GPIO_TOUCH_IRQ, SKIDATA_IRQ_TOUCH);
}

static struct tegra_dc_mode skidata_lvds_modes[] = {
	{
		.pclk = 33260000,
		.h_ref_to_sync = 0,
		.v_ref_to_sync = 0,
		.h_sync_width = 16,
		.v_sync_width = 15,
		.h_back_porch = 120,
		.v_back_porch = 15,
		.h_active = 800,
		.v_active = 480,
		.h_front_porch = 120,
		.v_front_porch = 15,
	},
};

static struct tegra_fb_data skidata_lvds_fb_data = {
	.win = 0,
	.xres = 800,
	.yres = 480,
	.bits_per_pixel = 32,
	.flags = TEGRA_FB_FLIP_ON_PROBE,
};

static int __init skidata_display_init(void)
{
	tamonten_lvds_disp_pdata.fb = &skidata_lvds_fb_data;
	tamonten_lvds_disp_pdata.default_out->modes =
		skidata_lvds_modes;
	tamonten_lvds_disp_pdata.default_out->n_modes =
		ARRAY_SIZE(skidata_lvds_modes);

	tamonten_lvds_init(&tegra_disp1_device.dev);
	tamonten_hdmi_init();

	return tamonten_display_init(&tamonten_lvds_disp_pdata,
				     &tamonten_hdmi_disp_pdata);
}

static void __init skidata_init(void)
{
	tamonten_init();
	tamonten_wm8903_init();

	skidata_gpio_init();
	skidata_i2c_init();
	skidata_display_init();
}

MACHINE_START(SKIDATA_CARRIER, "skidata-carrier")
	.boot_params    = 0x00000100,
	.fixup          = tamonten_fixup,
	.map_io         = tegra_map_common_io,
	.reserve        = tamonten_reserve,
	.init_early     = tegra_init_early,
	.init_irq       = tegra_init_irq,
	.timer          = &tegra_timer,
	.init_machine   = skidata_init,
MACHINE_END
