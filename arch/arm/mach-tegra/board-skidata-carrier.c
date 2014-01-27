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
#include <linux/i2c/at24.h>
#include <linux/regulator/machine.h>
#include <linux/regulator/fixed.h>
#include <linux/mt9v126.h>

#include <media/soc_camera.h>
#include <media/tegra_v4l2_camera.h>

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

static struct at24_platform_data fram_pdata = {
	.byte_len = 8192,
	.page_size = 1,
	.flags = AT24_FLAG_ADDR16,
};

static struct i2c_board_info stc_i2c2_gen2_board_info[] __initdata = {
	{
		I2C_BOARD_INFO("at24", 0x51),
		.platform_data = &fram_pdata,
	},
	{
		I2C_BOARD_INFO("pcf8523", 0x68),
	},
};

static u32 stc_platform_id[] = {
	0xFDF,
};

static void __init skidata_i2c_init(void)
{
	i2c_register_board_info(COM_I2C_BUS_GEN2, stc_i2c2_gen2_board_info,
				ARRAY_SIZE(stc_i2c2_gen2_board_info));
	tamonten_adnp_init(COM_I2C_BUS_GEN1, SKIDATA_IRQ_CPLD,
			stc_platform_id, ARRAY_SIZE(stc_platform_id));
	tamonten_tsc2007_init(COM_I2C_BUS_GEN2, SKIDATA_GPIO_TOUCH_IRQ,
			SKIDATA_IRQ_TOUCH);
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

#ifdef CONFIG_VIDEO_TEGRA
static int skidata_camera_enable(struct nvhost_device *ndev)
{
	static int gpio_requested;
	if (!gpio_requested) {
		int err = gpio_request(SKIDATA_GPIO_CCD_nPWRDN, "ccd_npwrdn");
		if (err) {
			pr_err("Failed to request CCD power down gpio\n");
			return err;
		}
		gpio_requested = 1;
	}
	gpio_set_value_cansleep(SKIDATA_GPIO_CCD_nPWRDN, 1);
	return 0;
}

static void skidata_camera_disable(struct nvhost_device *ndev)
{
	gpio_set_value_cansleep(SKIDATA_GPIO_CCD_nPWRDN, 0);
}

static struct mt9v126_platform_data mt9v126_pdata = {
	.reset_gpio = SKIDATA_GPIO_CCD_nRST,
	.progressive = 1,
};

static struct i2c_board_info skidata_camera_bus_board_info[] = {
	{
		I2C_BOARD_INFO("mt9v126", 0x48),
	},
};

static struct soc_camera_link skidata_camera_iclink = {
	.bus_id = -1,
	.i2c_adapter_id = COM_I2C_BUS_CAM,
	.board_info = skidata_camera_bus_board_info,
	.priv = &mt9v126_pdata,
};

static struct platform_device skidata_soc_camera = {
	.name = "soc-camera-pdrv",
	.id = 0,
	.dev = {
		.platform_data = &skidata_camera_iclink,
	},
};

static struct tegra_camera_platform_data skidata_camera_platform_data = {
	.enable_camera = skidata_camera_enable,
	.disable_camera = skidata_camera_disable,
	.flip_v = 0,
	.flip_h = 0,
	.port = TEGRA_CAMERA_PORT_VIP,
};

static void __init skidata_camera_init(void)
{
	tegra_camera_device.dev.platform_data = &skidata_camera_platform_data;
	nvhost_device_register(&tegra_camera_device);
	platform_device_register(&skidata_soc_camera);
}
#else
static void __init skidata_camera_init(void) {}
#endif /* CONFIG_VIDEO_TEGRA */

static void __init skidata_init(void)
{
	tamonten_init();
	tamonten_wm8903_init();

	skidata_gpio_init();
	skidata_i2c_init();
	skidata_camera_init();
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
