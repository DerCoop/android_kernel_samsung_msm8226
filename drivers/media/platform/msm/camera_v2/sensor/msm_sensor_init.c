/* Copyright (c) 2013, The Linux Foundation. All rights reserved.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 and
 * only version 2 as published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 */

#define pr_fmt(fmt) "MSM-SENSOR-INIT %s:%d " fmt "\n", __func__, __LINE__

/* Header files */
#include <mach/gpiomux.h>
#include "msm_sensor_init.h"
#include "msm_sensor_driver.h"
#include "msm_sensor.h"
#include "msm_sd.h"

//#define CONFIG_MSMB_CAMERA_DEBUG
/* Logging macro */
#undef CDBG
#ifdef CONFIG_MSMB_CAMERA_DEBUG
#define CDBG(fmt, args...) pr_err(fmt, ##args)
#else
#define CDBG(fmt, args...) do { } while (0)
#endif

#if defined(CONFIG_SR352) || defined(CONFIG_SR200PC20) || defined(CONFIG_S5K4ECGX)
#define REAR_YUV_SENSOR
#endif

#if defined (CONFIG_SR130PC20) || defined(CONFIG_SR030PC50)
#define FRONT_YUV_SENSOR
#endif

struct class *camera_class;
uint16_t rear_vendor_id = 0;
/* Static function declaration */
static long msm_sensor_init_subdev_ioctl(struct v4l2_subdev *sd,
	unsigned int cmd, void *arg);

/* Static structure declaration */
static struct v4l2_subdev_core_ops msm_sensor_init_subdev_core_ops = {
	.ioctl = msm_sensor_init_subdev_ioctl,
};

static struct v4l2_subdev_ops msm_sensor_init_subdev_ops = {
	.core = &msm_sensor_init_subdev_core_ops,
};

static const struct v4l2_subdev_internal_ops msm_sensor_init_internal_ops;

/* Static function definition */
static long msm_sensor_driver_cmd(struct msm_sensor_init_t *s_init, void *arg)
{
	int32_t                      rc = 0;
	struct sensor_init_cfg_data *cfg = (struct sensor_init_cfg_data *)arg;

	/* Validate input parameters */
	if (!s_init || !cfg) {
		pr_err("failed: s_init %pK cfg %pK", s_init, cfg);
		return -EINVAL;
	}

	switch (cfg->cfgtype) {
	case CFG_SINIT_PROBE:
                pr_warn("%s : CFG_SINIT_PROBE", __func__);
		rc = msm_sensor_driver_probe(cfg->cfg.setting);
		if (rc < 0)
			pr_err("failed: msm_sensor_driver_probe rc %d", rc);
		break;
	default:
            pr_err("%s : default", __func__);
		break;
	}

	return rc;
}

static long msm_sensor_init_subdev_ioctl(struct v4l2_subdev *sd,
	unsigned int cmd, void *arg)
{
	int32_t rc = 0;
	struct msm_sensor_init_t *s_init = v4l2_get_subdevdata(sd);
	CDBG("Enter");

	/* Validate input parameters */
	if (!s_init) {
		pr_err("failed: s_init %pK", s_init);
		return -EINVAL;
	}

	switch (cmd) {
	case VIDIOC_MSM_SENSOR_INIT_CFG:
		rc = msm_sensor_driver_cmd(s_init, arg);
		break;

	default:
		pr_err("default");
		break;
	}
	return 0;

}

extern uint16_t back_cam_fw_version;

static ssize_t back_camera_type_show(struct device *dev,
			struct device_attribute *attr, char *buf)
{
#if defined(CONFIG_SR352)
	char type[] = "SILICONFILE_SR352\n";
#elif defined(CONFIG_SEC_ATLANTIC_PROJECT) || defined(CONFIG_MACH_VASTALTE_CHN_CTC) || defined (CONFIG_MACH_VICTOR3GDSDTV_LTN) || defined (CONFIG_MACH_MEGA2LTE_KTT)|| defined(CONFIG_SEC_HESTIA_PROJECT)  || defined (CONFIG_SEC_VASTALTE_CHN_CMMCC_DUOS_PROJECT)
	char type[] = "SONY_IMX219\n";
#elif defined (CONFIG_SR200PC20)
	char type[] = "SILICONFILE_SR200PC20\n";
#elif defined (CONFIG_MACH_VICTORLTE_CTC) || defined(CONFIG_S5K4ECGX)
    char type[] = "SLSI_S5K4ECGX\n";
#elif defined(CONFIG_MACH_B8080F_EUR) && defined(CONFIG_IMX175)
    char type[] = "SONY_IMX175\n";
#elif (defined(CONFIG_MACH_B8080F_EUR) || defined(CONFIG_SEC_MS01_PROJECT)) && defined(CONFIG_S5K4H5YB)
	char type[] = "SLSI_S5K4H5YB\n";
#else
    char type[] = "NULL\n";
#endif

	 return snprintf(buf, sizeof(type), "%s", type);
}

static ssize_t front_camera_type_show(struct device *dev,
			struct device_attribute *attr, char *buf)
{
#if defined(CONFIG_SR130PC20)
	char cam_type[] = "SILICONFILE_SR130PC20\n";
#elif defined(CONFIG_SEC_BERLUTI_PROJECT)
	char cam_type[] = "SLSI_S5K8B1YX\n";
#elif defined(CONFIG_MACH_ATLANTICLTE_USC)
	char cam_type[] = "SLSI_S5K6B2\n";
#elif defined(CONFIG_SEC_ATLANTIC_PROJECT) || defined(CONFIG_MACH_VASTALTE_CHN_CTC) || defined(CONFIG_MACH_MEGA2LTE_KTT)|| defined(CONFIG_SEC_HESTIA_PROJECT)
	char cam_type[] = "SLSI_S5K6B2YX\n";
#elif defined(CONFIG_MACH_VICTORLTE_CTC)
    char cam_type[] = "N\n";
#elif defined(CONFIG_SEC_VASTALTE_CHN_CMMCC_DUOS_PROJECT)
	char cam_type[] = "SLSI_S5K5E3YX\n";
#elif (defined(CONFIG_MACH_B8080F_EUR) || defined(CONFIG_SEC_MS01_PROJECT)) && defined(CONFIG_S5K6A3YX)
	char cam_type[] = "SLSI_S5K6A3YX\n";
#else
    char cam_type[] = "NULL\n";
#endif

	 return snprintf(buf, sizeof(cam_type), "%s", cam_type);
}

int back_camera_antibanding =  50; /*default*/
int back_camera_antibanding_get (void) {
	return back_camera_antibanding;
}

ssize_t back_camera_antibanding_show (struct device *dev, struct device_attribute *attr, char *buf) {
	int count;

	count = sprintf(buf, "%d", back_camera_antibanding);
	CDBG("show_antibanding is %d \n", back_camera_antibanding);

	return count;
}

ssize_t back_camera_antibanding_store (struct device *dev, struct device_attribute *attr, const char *buf, size_t size) {
	int tmp = 0;

	sscanf(buf, "%d", &tmp);
	if ((50 == tmp) || (60 == tmp)) {
		back_camera_antibanding = tmp;
		CDBG("store_antibanding is %d\n", back_camera_antibanding);
	}

	return size;
}

char cam_fw_ver[25] = "E08QLGI01CH E08QLGI01CH\n";/*VE,8mega_pixel,Qualcomm_isp,SLSI_sensor*/
static ssize_t back_camera_firmware_show(struct device *dev,
			struct device_attribute *attr, char *buf)
{
#if defined(CONFIG_MACH_KS01SKT) || defined(CONFIG_MACH_KS01KTT)\
	|| defined(CONFIG_MACH_KS01LGT)
	char cam_fw[] = "O13Q0SAGC01 O13Q0SAGC01\n";/*Camsys_module,13mega_pixel,Qualcomm_isp,Sony_sensor*/
	return snprintf(buf, sizeof(cam_fw), "%s", cam_fw);
#elif defined(CONFIG_MACH_B8080F_EUR) && defined(CONFIG_IMX175)
	char cam_fw[] = "B08QTGJ01MI B08QTGJ01MI\n";
	return snprintf(buf, sizeof(cam_fw), "%s", cam_fw);
#elif defined(CONFIG_MACH_B8080F_EUR) && defined(CONFIG_S5K4H5YB)
	char cam_fw[] = "E08QLHI01CI E08QLHI01CI\n";
	return snprintf(buf, sizeof(cam_fw), "%s", cam_fw);
#elif defined(CONFIG_MACH_VIENNAEUR) || defined(CONFIG_MACH_VASTALTE_CHN_CTC) || defined (CONFIG_MACH_VICTOR3GDSDTV_LTN)
	char cam_fw[] = "E08QSGG01OC E08QSGG01OC\n";/* Gumi, 8mega_pixel, Qualcomm_isp, Sony_sensor*/
	return snprintf(buf, sizeof(cam_fw), "%s", cam_fw);
#elif defined(CONFIG_SEC_ATLANTIC_PROJECT) || defined(CONFIG_MACH_MEGA2LTE_KTT) || defined(CONFIG_SEC_HESTIA_PROJECT) || defined(CONFIG_SEC_VASTALTE_CHN_CMMCC_DUOS_PROJECT)
	char cam_fw[] = "F08QUHI01SM F08QUHI01SM\n";/* Samsung Electro, 8mega_pixel, Qualcomm_isp, Sony_sensor IMX219*/
	return snprintf(buf, sizeof(cam_fw), "%s", cam_fw);
#elif defined(CONFIG_MACH_LT03EUR)
	char cam_fw[] = "E08QSGG01OE E08QSGG01OE\n";
	return snprintf(buf, sizeof(cam_fw), "%s", cam_fw);
#elif  defined(CONFIG_SR352)
	char cam_fw[] = "SR352 N\n";
	return snprintf(buf, sizeof(cam_fw), "%s", cam_fw);
#elif defined (CONFIG_SR200PC20)
	char cam_fw[] = "SR200PC20 N\n";
	return snprintf(buf, sizeof(cam_fw), "%s", cam_fw);
#elif defined(CONFIG_MACH_AFYONLTE_TMO) || defined(CONFIG_MACH_AFYONLTE_CAN) \
	|| defined(CONFIG_MACH_VICTORLTE_CTC) \
	|| defined (CONFIG_MACH_AFYONLTE_MTR)
	char cam_fw[] = "S5K4ECGX N\n";
	return snprintf(buf, sizeof(cam_fw), "%s", cam_fw);
#else // multi module case
	CDBG("[FW_DBG] cam_fw_ver : %s\n", cam_fw_ver);
	return snprintf(buf, sizeof(cam_fw_ver), "%s", cam_fw_ver);
#endif
}

static ssize_t back_camera_firmware_store(struct device *dev,
		struct device_attribute *attr, const char *buf, size_t size)
{
#if defined(CONFIG_SR352)
	char cam_fw_ver[25] = "SR352 N\n";
#elif defined (CONFIG_SR200PC20)
	char cam_fw_ver[25] = "SR200PC20 N\n";
#elif defined(CONFIG_MACH_B8080F_EUR) && defined(CONFIG_IMX175)
	char cam_fw_ver[25] = "B08QTGJ01MI B08QTGJ01MI\n";
#elif defined(CONFIG_MACH_B8080F_EUR) && defined(CONFIG_S5K4H5YB)
	char cam_fw_ver[25] = "E08QLHI01CI E08QLHI01CI\n";
#elif defined(CONFIG_MACH_AFYONLTE_TMO) || defined(CONFIG_MACH_AFYONLTE_CAN)\
	|| defined(CONFIG_MACH_VICTORLTE_CTC) \
	|| defined (CONFIG_MACH_AFYONLTE_MTR)
	char cam_fw_ver[25] = "S5K4ECGX N\n";
#endif
	CDBG("[FW_DBG] buf : %s\n", buf);
	snprintf(cam_fw_ver, sizeof(cam_fw_ver), "%s", buf);

	return size;
}

static ssize_t rear_camera_vendorid_show(struct device *dev,
			struct device_attribute *attr, char *buf)
{
	char vendor_id[16] = {0};
	if (rear_vendor_id)
	  sprintf(vendor_id, "0x0%x\n", rear_vendor_id);
	else
	  strncpy(vendor_id, "NULL\n", sizeof(vendor_id));
	return  snprintf(buf, sizeof(vendor_id), "%s", vendor_id);
}

static ssize_t front_camera_firmware_show(struct device *dev,
			struct device_attribute *attr, char *buf)
{
#if defined(CONFIG_SR130PC20)
	char cam_fw[] = "SR130PC20 N\n";
#elif defined(CONFIG_MACH_AFYONLTE_TMO) || defined(CONFIG_MACH_AFYONLTE_CAN) \
	|| defined (CONFIG_MACH_AFYONLTE_MTR)
	char cam_fw[] = "SR030PC50 N\n";
#elif defined(CONFIG_SEC_BERLUTI_PROJECT)
	char cam_fw[] = "S5K8B1YX S5K8B1YX\n";
#elif defined(CONFIG_MACH_ATLANTICLTE_USC)
	char cam_fw[] = "S5K6B2 S5K6B2\n";
#elif defined(CONFIG_MACH_VASTALTE_CHN_CTC)
	char cam_fw[] = "S5K6B2YX S5K6B2YX\n";
 #elif defined(CONFIG_SEC_HESTIA_PROJECT)
        char cam_fw[] = "S5K6B2YX N\n";
#elif defined(CONFIG_SEC_ATLANTIC_PROJECT) || defined(CONFIG_MACH_MEGA2LTE_KTT)
	char cam_fw[] = "S5K6B2YX N\n";
#elif defined(CONFIG_MACH_VICTORLTE_CTC)
	char cam_fw[] = "N N\n";
#elif defined(CONFIG_SEC_VASTALTE_CHN_CMMCC_DUOS_PROJECT)
	char cam_fw[] = "S5K5E3YX N\n";
#else
	char cam_fw[] = "S5K6A3YX S5K6A3YX\n";
#endif
	return  snprintf(buf, sizeof(cam_fw), "%s", cam_fw);
}

#if !defined(REAR_YUV_SENSOR)
#if defined(CONFIG_SEC_MS01_PROJECT)
char cam_load_fw[25] = "E08QLGI01CH E08QLGI01CH\n";
#elif defined(CONFIG_MACH_B8080F_EUR) && defined(CONFIG_IMX175)
char cam_load_fw[25] = "B08QTGJ01MI B08QTGJ01MI\n";
#elif defined(CONFIG_MACH_B8080F_EUR) && defined(CONFIG_S5K4H5YB)
char cam_load_fw[25] = "E08QLHI01CI E08QLHI01CI\n";
#else
char cam_load_fw[25] = "F08QUHE03SM F08QUHE03SM\n";
#endif
static ssize_t back_camera_firmware_load_show(struct device *dev,
					      struct device_attribute *attr, char *buf)
{
	CDBG("[FW_DBG] cam_load_fw : %s\n", cam_load_fw);
	return snprintf(buf, sizeof(cam_load_fw), "%s", cam_load_fw);
}

static ssize_t back_camera_firmware_load_store(struct device *dev,
					       struct device_attribute *attr, const char *buf, size_t size)
{
	CDBG("[FW_DBG] buf : %s\n", buf);
	snprintf(cam_load_fw, sizeof(cam_load_fw), "%s\n", buf);
	return size;
}
#ifndef EEPROM_CAM_FW
#if defined (CONFIG_MACH_B8080F_EUR) && defined(CONFIG_IMX175)
char cam_fw_full_ver[40] = "B08QTGJ01MI B08QTGJ01MI B08QTGJ01MI\n";
#elif defined(CONFIG_MACH_B8080F_EUR) && defined(CONFIG_S5K4H5YB)
char cam_fw_full_ver[40] = "E08QLHI01CI E08QLHI01CI E08QLHI01CI\n";
#elif defined(CONFIG_SEC_MS01_PROJECT)
char cam_fw_full_ver[40] = "E08QLGI01CH E08QLGI01CH E08QLGI01CH\n";
#else
char cam_fw_full_ver[40] = "F08QUHE03SM F08QUHE03SM F08QUHE03SM\n";//multi module
#endif
#else
char cam_fw_full_ver[40] = {0};//multi module
#if defined(CONFIG_SEC_ATLANTIC_PROJECT) || defined(CONFIG_MACH_MEGA2LTE_KTT) || defined(CONFIG_SEC_HESTIA_PROJECT) || defined(CONFIG_SEC_VASTALTE_CHN_CMMCC_DUOS_PROJECT)
/* Samsung Electro, 8mega_pixel, Qualcomm_isp, Sony_sensor IMX219*/
char load_fw[] = "F08QUHI01SM";
char phone_fw[] = "F08QUHI01SM";
#else
char load_fw[] = "F08QUHE03SM";
char phone_fw[] = "F08QUHE03SM";
#endif
extern char eeprom_fw[16];
#endif
static ssize_t back_camera_firmware_full_show(struct device *dev,
					 struct device_attribute *attr, char *buf)
{
	CDBG("[FW_DBG] cam_fw_ver : %s\n", cam_fw_full_ver);
#ifdef EEPROM_CAM_FW
	snprintf(cam_fw_full_ver, sizeof(cam_fw_full_ver), "%s %s %s\n",
			eeprom_fw, load_fw, phone_fw);
#endif
	return snprintf(buf, sizeof(cam_fw_full_ver), "%s", cam_fw_full_ver);
}
static ssize_t back_camera_firmware_full_store(struct device *dev,
					  struct device_attribute *attr, const char *buf, size_t size)
{
	CDBG("[FW_DBG] buf : %s\n", buf);
	snprintf(cam_fw_full_ver, sizeof(cam_fw_full_ver), "%s", buf);
	return size;
}
#endif
static DEVICE_ATTR(rear_camtype, S_IRUGO, back_camera_type_show, NULL);
static DEVICE_ATTR(rear_camfw, S_IRUGO|S_IWUSR|S_IWGRP,
    back_camera_firmware_show, back_camera_firmware_store);
#if !defined(REAR_YUV_SENSOR)
static DEVICE_ATTR(rear_camfw_full, S_IRUGO|S_IWUSR|S_IWGRP,
		   back_camera_firmware_full_show, back_camera_firmware_full_store);
static DEVICE_ATTR(rear_camfw_load, S_IRUGO | S_IWUSR | S_IWGRP,
		   back_camera_firmware_load_show, back_camera_firmware_load_store);
#endif
static DEVICE_ATTR(rear_camantibanding, S_IRUGO|S_IWUSR|S_IWGRP,
    back_camera_antibanding_show, back_camera_antibanding_store);
static DEVICE_ATTR(rear_vendorid, S_IRUGO, rear_camera_vendorid_show, NULL);
static DEVICE_ATTR(front_camtype, S_IRUGO, front_camera_type_show, NULL);
static DEVICE_ATTR(front_camfw, S_IRUGO, front_camera_firmware_show, NULL);
#if !defined(FRONT_YUV_SENSOR)
static DEVICE_ATTR(front_camfw_full, S_IRUGO, front_camera_firmware_show, NULL);
#endif

static int __init msm_sensor_init_module(void)
{
	struct msm_sensor_init_t *s_init = NULL;
	struct device			*cam_dev_back;
	struct device			*cam_dev_front;
	int rc = 0;
	camera_class = class_create(THIS_MODULE, "camera");
	if (IS_ERR(camera_class))
	    pr_err("failed to create device cam_dev_rear!\n");

	/* Allocate memory for msm_sensor_init control structure */
	s_init = kzalloc(sizeof(struct msm_sensor_init_t), GFP_KERNEL);
	if (!s_init) {
		class_destroy(camera_class);
		return -ENOMEM;
	}

	/* Initialize mutex */
	mutex_init(&s_init->imutex);

	/* Create /dev/v4l-subdevX for msm_sensor_init */
	v4l2_subdev_init(&s_init->msm_sd.sd, &msm_sensor_init_subdev_ops);
	snprintf(s_init->msm_sd.sd.name, sizeof(s_init->msm_sd.sd.name), "%s",
		"msm_sensor_init");
	v4l2_set_subdevdata(&s_init->msm_sd.sd, s_init);
	s_init->msm_sd.sd.internal_ops = &msm_sensor_init_internal_ops;
	s_init->msm_sd.sd.flags |= V4L2_SUBDEV_FL_HAS_DEVNODE;
	rc = media_entity_init(&s_init->msm_sd.sd.entity, 0, NULL, 0);
	if (rc < 0)
		goto entity_fail;
	s_init->msm_sd.sd.entity.type = MEDIA_ENT_T_V4L2_SUBDEV;
	s_init->msm_sd.sd.entity.group_id = MSM_CAMERA_SUBDEV_SENSOR_INIT;
	s_init->msm_sd.sd.entity.name = s_init->msm_sd.sd.name;
	s_init->msm_sd.close_seq = MSM_SD_CLOSE_2ND_CATEGORY | 0x6;
	rc = msm_sd_register(&s_init->msm_sd);
	if (rc < 0)
		goto msm_sd_register_fail;

	cam_dev_back = device_create(camera_class, NULL,
		1, NULL, "rear");
	if (IS_ERR(cam_dev_back)) {
		printk("Failed to create cam_dev_back device!\n");
		goto device_create_fail;
	}

	if (device_create_file(cam_dev_back, &dev_attr_rear_camtype) < 0) {
		printk("Failed to create device file!(%s)!\n",
			dev_attr_rear_camtype.attr.name);
		goto device_create_fail;
	}
	if (device_create_file(cam_dev_back, &dev_attr_rear_camfw) < 0) {
		printk("Failed to create device file!(%s)!\n",
			dev_attr_rear_camfw.attr.name);
		goto device_create_fail;
	}
#if !defined(REAR_YUV_SENSOR)
	if (device_create_file(cam_dev_back, &dev_attr_rear_camfw_full) < 0) {
		printk("Failed to create device file!(%s)!\n",
			dev_attr_rear_camfw_full.attr.name);
		goto device_create_fail;
	}
	if (device_create_file(cam_dev_back, &dev_attr_rear_camfw_load) < 0) {
		printk("Failed to create device file!(%s)!\n",
		       dev_attr_rear_camfw_load.attr.name);
		goto device_create_fail;
	}
#endif
	if (device_create_file(cam_dev_back, &dev_attr_rear_camantibanding) < 0) {
		printk("Failed to create device file!(%s)!\n",
			dev_attr_rear_camantibanding.attr.name);
		goto device_create_fail;
	}
	if (device_create_file(cam_dev_back, &dev_attr_rear_vendorid) < 0) {
		printk("Failed to create device file!(%s)!\n",
			dev_attr_rear_vendorid.attr.name);
		goto device_create_fail;
	}

	cam_dev_front = device_create(camera_class, NULL,
		2, NULL, "front");
	if (IS_ERR(cam_dev_front)) {
		printk("Failed to create cam_dev_front device!");
		goto device_create_fail;
	}

	if (device_create_file(cam_dev_front, &dev_attr_front_camtype) < 0) {
		printk("Failed to create device file!(%s)!\n",
			dev_attr_front_camtype.attr.name);
		goto device_create_fail;
	}
	if (device_create_file(cam_dev_front, &dev_attr_front_camfw) < 0) {
		printk("Failed to create device file!(%s)!\n",
			dev_attr_front_camfw.attr.name);
		goto device_create_fail;
	}
#if !defined(FRONT_YUV_SENSOR)
	if (device_create_file(cam_dev_front, &dev_attr_front_camfw_full) < 0) {
		printk("Failed to create device file!(%s)!\n",
			dev_attr_front_camfw_full.attr.name);
		goto device_create_fail;
	}
#endif
	return 0;

device_create_fail:
	msm_sd_unregister(&s_init->msm_sd);
msm_sd_register_fail:
	media_entity_cleanup(&s_init->msm_sd.sd.entity);
entity_fail:
	mutex_destroy(&s_init->imutex);
	kfree(s_init);
	class_destroy(camera_class);
	return rc;
}

static void __exit msm_sensor_exit_module(void)
{
	return;
}

module_init(msm_sensor_init_module);
module_exit(msm_sensor_exit_module);
MODULE_DESCRIPTION("msm_sensor_init");
MODULE_LICENSE("GPL v2");
