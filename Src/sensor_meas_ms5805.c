/*
 * Copyright (c) 2006-2020, RT-Thread Development Team
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date           Author       Notes
 * 2020-08-19     Administrator       the first version
 */
#include "sensor_meas_ms5805.h"
#define DBG_ENABLE
#define DBG_SECTION_NAME "MS5805"
#define DBG_LEVEL DBG_LOG
#include <rtdbg.h>

static ms5805_device_t _ms5805_create(struct rt_sensor_intf *intf)
{
    ms5805_device_t dev;

    dev = ms5805_init(intf->dev_name, RT_NULL);

    return dev;
}

static rt_size_t ms5805_fetch_data(struct rt_sensor_device *sensor, void *buf, rt_size_t len)
{
    ms5805_device_t ms5805 = (ms5805_device_t)sensor->parent.user_data;
    struct rt_sensor_data *data = (struct rt_sensor_data *)buf;

    if (sensor->info.type == RT_SENSOR_CLASS_BARO)
    {
        ms5805_measure(ms5805);
        data->type = RT_SENSOR_CLASS_BARO;
        data->data.baro = ms5805->baro;
        data->timestamp = rt_sensor_get_ts();
    }
    else if (sensor->info.type == RT_SENSOR_CLASS_TEMP)
    {
        ms5805_measure(ms5805);
        data->type = RT_SENSOR_CLASS_TEMP;
        data->data.temp = ms5805->temp/10;
        data->timestamp = rt_sensor_get_ts();
    }
    else
        return 0;

    return 1;
}

static rt_err_t ms5805_control(struct rt_sensor_device *sensor, int cmd, void *arg)
{
    rt_err_t result = RT_EOK;

    return result;
}

static struct rt_sensor_ops sensor_ops =
{
    ms5805_fetch_data,
    ms5805_control
};

static int rt_hw_ms5805_baro_init(const char *name, struct rt_sensor_config *cfg, ms5805_device_t hdev)
{
    rt_sensor_t sensor = RT_NULL;
    int result;

    sensor = rt_calloc(1, sizeof(struct rt_sensor_device));
    if (sensor == RT_NULL)
        return -1;

    sensor->info.type       = RT_SENSOR_CLASS_BARO;
    sensor->info.vendor     = RT_SENSOR_VENDOR_UNKNOWN;
    sensor->info.model      = "ms5805_baro";
    sensor->info.unit       = RT_SENSOR_UNIT_PA;
    sensor->info.intf_type  = RT_SENSOR_INTF_I2C;
    sensor->info.range_max  = 200000;
    sensor->info.range_min  = 1000;
    sensor->info.period_min = 2;

    rt_memcpy(&sensor->config, cfg, sizeof(struct rt_sensor_config));
    sensor->ops = &sensor_ops;

    result = rt_hw_sensor_register(sensor, name, RT_DEVICE_FLAG_RDONLY, hdev);
    if (result != RT_EOK)
    {
        LOG_E("device register err code: %d", result);
        rt_free(sensor);
        return -RT_ERROR;
    }

    LOG_I("baro sensor init success");

    return RT_EOK;
}

static int rt_hw_ms5805_temp_init(const char *name, struct rt_sensor_config *cfg, ms5805_device_t hdev)
{
    rt_sensor_t sensor = RT_NULL;
    int result;

    sensor = rt_calloc(1, sizeof(struct rt_sensor_device));
    if (sensor == RT_NULL)
        return -1;

    sensor->info.type       = RT_SENSOR_CLASS_TEMP;
    sensor->info.vendor     = RT_SENSOR_VENDOR_UNKNOWN;
    sensor->info.model      = "ms5805_temp";
    sensor->info.unit       = RT_SENSOR_UNIT_DCELSIUS;
    sensor->info.intf_type  = RT_SENSOR_INTF_I2C ;
    sensor->info.range_max  = 85;
    sensor->info.range_min  = -40;
    sensor->info.period_min = 2;

    rt_memcpy(&sensor->config, cfg, sizeof(struct rt_sensor_config));
    sensor->ops = &sensor_ops;

    result = rt_hw_sensor_register(sensor, name, RT_DEVICE_FLAG_RDONLY, hdev);
    if (result != RT_EOK)
    {
        LOG_E("device register err code: %d", result);
        rt_free(sensor);
        return -RT_ERROR;
    }

    LOG_I("temp sensor init success");

    return RT_EOK;
}

int rt_hw_ms5805_init(const char *name, struct rt_sensor_config *cfg)
{
#if defined(MS5805_USING_TEMP) || defined(MS5805_USING_BARO)
    ms5805_device_t ms5805 = _ms5805_create(&cfg->intf);
    int tmp = 0;

    if (ms5805)
    {
#ifdef MS5805_USING_BARO
        tmp += rt_hw_ms5805_baro_init(name, cfg, ms5805);
#endif
#ifdef MS5805_USING_TEMP
        tmp += rt_hw_ms5805_temp_init(name, cfg, ms5805);
#endif
        if (tmp != 0)
        {
            ms5805_deinit(ms5805);
            return -1;
        }
        return 0;
    }
    ms5805_deinit(ms5805);
#endif

    return -1;
}
