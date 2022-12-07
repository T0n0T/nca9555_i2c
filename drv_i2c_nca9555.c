// * Copyright (c) 2006-2021, RT-Thread Development Team
// *
// * SPDX-License-Identifier: Apache-2.0
// *
// * Change Logs:
// * Date           Author       Notes
// * 2022-10-12     liwentai       the first version
// *

#include    "drv_i2c_nca9555.h"
#include    "tool.h"

#define     DBG_TAG "nca9555"
#define     DBG_LVL DBG_INFO
#include    <rtdbg.h>

/**
 * @ingroup NCA9555基础设置
 */
struct rt_nca9555 nca9555_obj[] =
{
        NCA9555_config_0,
        NCA9555_config_1,
        NCA9555_config_2,
        NCA9555_config_3,
        NCA9555_config_4,
        NCA9555_config_5,
};

/**
 * @ingroup 引脚映射
 */

/**@{*/
#define ITEM_NUM(items) sizeof(items) / sizeof(items[0])
static const struct pin_list list[] =
{
        IO0_0,
        IO0_1,
        IO0_2,
        IO0_3,
        IO0_4,
        IO0_5,
        IO0_6,
        IO0_7,

        IO1_0,
        IO1_1,
        IO1_2,
        IO1_3,
        IO1_4,
        IO1_5,
        IO1_6,
        IO1_7,
};

rt_uint8_t set_pin[4] = { 0x00, 0x00, 0xff, 0xff };
/**@}*/

/**
 * @ingroup NCA9555设备读写
 */

/**@{*/

/**
 * @brief 用于读取NCA9555寄存器数据
 *        \li 寄存器读取长度可以为一字节或两字节，写入两字节的寄存器值时能够连续读取两个寄存器
 * @param[in] dev        NCA9555设备句柄
 * @param[in] reg        读取的寄存器
 * @param[in] len        寄存器数据读取长度
 * @param[out] buf       接受读取的寄存器值
 *
 * @return 返回读取状态，0为成功，-1为失败
 */
rt_err_t nca9555_read_data(nca_device_t dev, rt_uint8_t reg, rt_uint8_t len, rt_uint8_t* buf)
{
    struct rt_i2c_msg msgs[2] = { 0 };

    msgs[0].addr = dev->addr;
    msgs[0].flags = RT_I2C_WR;
    msgs[0].buf = &reg;
    msgs[0].len = 1;

    msgs[1].addr = dev->addr;
    msgs[1].flags = RT_I2C_RD;
    msgs[1].buf = buf;
    msgs[1].len = len;

    if (rt_i2c_transfer(dev->i2c_bus, msgs, 2) == 2)
    {
        LOG_D("Read successfully\n");
        return RT_EOK;        
    }
    else
    {
        LOG_D("NCA9555 on %s 0x%02X read failed!\n",dev->i2c_bus->parent.parent.name,dev->addr);
        return -RT_ERROR;
    }
}

/**
 * @brief 用于写入NCA9555寄存器数据
 *        \li 寄存器写入长度可以为一字节或两字节，写入两字节的寄存器值时能够连续写两个寄存器
 * @param[in] dev        NCA9555设备句柄
 * @param[in] reg        写入的寄存器
 * @param[in] len        数据写入长度
 * @param[in] buf        待写入的寄存器值
 *
 * @return 返回写入状态，0为成功，-1为失败
 */
rt_err_t nca9555_write_data(nca_device_t dev, rt_uint8_t reg, rt_uint8_t len, rt_uint8_t* buf)
{
    struct rt_i2c_msg msgs[2] = { 0 };

    msgs[0].addr = dev->addr;
    msgs[0].flags = RT_I2C_WR;
    msgs[0].buf = &reg;
    msgs[0].len = 1;

    msgs[1].addr = dev->addr;
    msgs[1].flags = RT_I2C_WR | RT_I2C_NO_START;
    msgs[1].buf = buf;
    msgs[1].len = len;

    if (rt_i2c_transfer(dev->i2c_bus, msgs, 2) == 2)
    {
        LOG_D("Read successfully\n");
        return RT_EOK;        
    }
    else
    {
        LOG_D("NCA9555 on %s 0x%02X read failed!\n",dev->i2c_bus->parent.parent.name,dev->addr);
        return -RT_ERROR;
    }
}
/**@}*/

/**
 * @ingroup 引脚映射
 */
/**@{*/
/**
 * @brief       用于使用字符串获取引脚
 *          \li 输入的字符串需要以 "IO1_1" 的格式
 *          \li 输入的字符串第一个数字为NCA9555的引脚分组，可选0,1
 *          \li 输入的字符串第二个数字为NCA9555的引脚编号，可选0到7
 *          \li 字符串可以用sprintf拼接
 * @param pin   代表引脚的字符串
 * @return      引脚对象变量
 */
static const struct pin_list* get_list(char* pin)
{
    const struct pin_list* index;
    index = RT_NULL;

    for (rt_uint8_t i = 0; i < ITEM_NUM(list); i++)
    {
        if (!rt_strncmp(list[i].name, pin, sizeof(list[i].name)+1))
        {
            index = &list[i];
            LOG_D("%s has been chosen.   ||||||||  %s source pin .   ||||||||  %d size.",
                    index->name, pin, sizeof(list[i].name));
            goto __exit;
        }
    }

    __exit:
    if (index == RT_NULL)
    {
        LOG_E("%s is not a usable pin, check the char whether conform to format like IO1_1 ", pin);
    }

    return index;
}
/**@}*/

/**
 * @ingroup NCA9555驱动的RTT接口
 */
/**@{*/


/**
 * @brief 用于写入RTT设备管理器的引脚写入接口
 * @details 输出的引脚电平可以为0或1，字符串输入 @see get_list，第四个参数size无效
 * @param[in] dev        NCA9555设备句柄
 * @param[in] pos        引脚输出的逻辑电平
 * @param[in] buffer     引脚信息
 *
 * @return 返回写入状态，0为成功，-1为失败
 */
static rt_size_t rt_nca9555_write_pin(rt_device_t dev, rt_off_t pos, void const* buffer, rt_size_t size)
{
    rt_uint8_t temp[2] = { 0 };
    nca_device_t nca9555 = RT_NULL;
    rt_off_t offset = 0;
    char* str = (char*) buffer;
    const struct pin_list* index;

    nca9555 = (nca_device_t) (dev->user_data);
    index = get_list(str);
    offset = index->offset;

    nca9555_read_data(nca9555, REGISTER_0_OUTPUT_PORT, 2, &temp[0]);

    switch(pos)
    {
    case PIN_LOW:
        if (offset < 8)
        {
            temp[0] &= ~(0x01 << offset);
        }
        else
        {
            temp[1] &= ~(0x01 << (offset - 8));
        }
        break;

    case PIN_HIGH:
        if (offset < 8)
        {
            temp[0] |= 0x01 << offset;
        }
        else
        {
            temp[1] |= 0x01 << (offset - 8);
        }
        break;
    default:
        rt_kprintf("put in a logical level for %s %s.\n", dev->parent.name, index->name);
        break;
    }
    nca9555_write_data(nca9555, REGISTER_0_OUTPUT_PORT, 2, &temp[0]);
    return size;
}

/**
 * @brief 用于写入RTT设备管理器的引脚读取接口
 * @details 读取到引脚电平为0或1，字符串输入 @see get_list，第二个参数pos和四个参数size无效
 * @param[in] dev        NCA9555设备句柄
 * @param[in] buffer     引脚信息
 * 
 * @return 引脚的逻辑电平
 * @retval 0 即该引脚为高电平
 * @retval 1 即改引脚为低电平
 */
static rt_size_t rt_nca9555_read_pin(rt_device_t dev, rt_off_t pos, void* buffer, rt_size_t size)
{
    rt_uint8_t temp[2] = { 0 };
    rt_size_t pin_status = 0;
    nca_device_t nca9555 = RT_NULL;
    char* str = (char*) buffer;
    const struct pin_list *index;
    rt_off_t offset = 0;

    nca9555 = (nca_device_t) (dev->user_data);
    index = get_list(str);
    offset = index->offset;

    nca9555_read_data(nca9555, REGISTER_0_INPUT_PORT, 2, &temp[0]);

    if (offset < 8)
    {
        pin_status = (temp[0] >> offset) & 0x01;
    }
    else
    {
        pin_status = (temp[1] >> (offset - 8)) & 0x01;
    }

    return pin_status;
}


/**
 * @brief 用于写入RTT设备管理器的引脚模式控制接口
 * @details 字符串输入 @see get_list，可以控制为输入，输出，极性反转，返回值无效
 * @param[in] dev      NCA9555设备句柄
 * @param[in] cmd      引脚模式
 * @param[in] args     引脚信息
 */
static rt_err_t rt_nca9555_control_pin(rt_device_t dev, int cmd, void* args)
{
    nca_device_t nca9555 = RT_NULL;
    nca9555 = (nca_device_t) (dev->user_data);

    rt_uint8_t buf[2] = { 0 };

    char* str = (char*) args;

    const struct pin_list* index;
    index = get_list(str);
    LOG_D("%s ",index->name);

    rt_off_t pos = index->offset;

    nca9555_read_data(nca9555, REGISTER_0_CONFIGURATION_PORT, 2, &buf[0]);
    switch (cmd)
    {
    case OUTPUT:
        LOG_D("OUTPUT mode\n");
        if (pos < 8)
        {
            buf[0] &= ~(0x01 << pos);
        }
        else
        {
            buf[1] &= ~(0x01 << (pos - 8));
        }
        nca9555_write_data(nca9555, REGISTER_0_CONFIGURATION_PORT, 2, &buf[0]);
        break;

    case INPUT:
        LOG_D("INPUT mode\n");
        if (pos < 8)
        {
            buf[0] |= 0x01 << pos;
        }
        else
        {
            buf[1] |= 0x01 << (pos - 8);
        }
        nca9555_write_data(nca9555, REGISTER_0_CONFIGURATION_PORT, 2, &buf[0]);
        break;

    case INVERT:
        LOG_D("INVERT!\n");
        if (pos < 8)
        {
            buf[0] |= 0x01 << pos;
        }
        else
        {
            buf[1] |= 0x01 << (pos - 8);
        }
        nca9555_write_data(nca9555, REGISTER_0_POLARITY_INVERSION_PORT, 2, &buf[0]);
        break;

    default:
        break;
    }
    return RT_EOK;
}

/**
 * @brief NCA9555设备初始化函数
 * 
 * @param i2c_bus_name 所在I2C总线名称
 * @param addr 所在地址
 * @return nca_device_t NCA9555设备句柄
 */
static nca_device_t nca9555_init(const char* i2c_bus_name, rt_uint8_t addr)
{
    //create a nca_deivce for writing various functions
    nca_device_t dev;
    rt_uint8_t ret = RT_EOK;

    RT_ASSERT(i2c_bus_name);

    dev = rt_calloc(1, sizeof(struct nca9555_device));

    if (dev == RT_NULL)
    {
        LOG_E("Can't allocate memory for NCA9555 device on '%s' ", i2c_bus_name);
        rt_free(dev);
        return RT_NULL;
    }

    dev->i2c_bus = rt_i2c_bus_device_find(i2c_bus_name);

    if (dev == RT_NULL)
    {
        LOG_E("Can't find NCA9555 device on '%s' ", i2c_bus_name);
        rt_free(dev);
        return RT_NULL;
    }

    dev->addr = addr;

    ret = nca9555_write_data(dev, REGISTER_0_CONFIGURATION_PORT, 2, &set_pin[2]);
    ret = nca9555_write_data(dev, REGISTER_0_OUTPUT_PORT, 2, &set_pin[0]);

    if (ret != RT_EOK) {
        return RT_NULL;
    }else {
        return dev;
    }
}

/**
 * @brief 用于将NCA9555写入到RTT的设备管理器中
 * 
 * @param dev NCA9555设备句柄
 * @param nca9555_name 写入设备管理器是使用的名字
 * @return rt_err_t 注册的结果
 */
rt_err_t rt_device_nca9555_register(nca_device_t dev, const char* nca9555_name)
{
    rt_device_t device = RT_NULL;
    device = rt_calloc(1, sizeof(struct rt_device));
    if (device == RT_NULL)
    {
        LOG_E("can't allocate memory for NCA9555 device");
        free(device);
    }

    /* register device */
    device->type = RT_Device_Class_Miscellaneous;

    device->init = RT_NULL;
    device->open = RT_NULL;
    device->close = RT_NULL;
    device->read = rt_nca9555_read_pin;
    device->write = rt_nca9555_write_pin;
    device->control = rt_nca9555_control_pin;

    device->user_data = (void*) dev;

    return rt_device_register(device, nca9555_name, RT_DEVICE_FLAG_RDWR);
}

/**
 * @brief 将NCA9555初始化，同时通过NCA9555的初始化情况自动判断主副班的使用情况
 * @return 0为注册成功，-1为注册有错误
 */
rt_err_t rt_hw_nca9555_init()
{
    int i;
    for (i = 0; i < ITEM_NUM(nca9555_obj); i++)
    {
        nca9555_obj[i].dev = nca9555_init(nca9555_obj[i].bus_name, nca9555_obj[i].addr);
        if (nca9555_obj[i].dev != RT_NULL) {
            if (rt_device_nca9555_register(nca9555_obj[i].dev, nca9555_obj[i].name) != RT_EOK)
            {
                LOG_E("NCA9555 device %s register failed.", nca9555_obj[i].name);
                return -RT_ERROR;
            }
        }else {
			LOG_E("Nca9555 device of master board on 0x%02X init fail.",nca9555_obj[i].addr);
			return -RT_ERROR;			
        }
    }
    return RT_EOK;
}
/**@}*/



/*
 * Copyright (c) 2006-2021, RT-Thread Development Team
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date           Author       Notes
 * 2022-10-12     liwentai       the first version
 */

#include    <rtdevice.h>
#include    <rtthread.h>
#include    <board.h>

#ifndef APPLICATIONS_I2C_MODULE_I2C_MODULE_H_
#define APPLICATIONS_I2C_MODULE_I2C_MODULE_H_

#define RT_I2C_NCA9555_NAME "i2c1"
#define RT_I2C_AMR_ADDR     0x20
#define RT_I2C_LED_ADDR     0x21
#define RT_I2C_ELEC_ADDR    0x22

#define     OUTPUT      0
#define     INPUT       1
#define     INVERT      2

//寄存器指令
#define  REGISTER_0_INPUT_PORT                  0X00
#define  REGISTER_1_INPUT_PORT                  0X01
#define  REGISTER_0_OUTPUT_PORT                 0X02
#define  REGISTER_1_OUTPUT_PORT                 0X03
#define  REGISTER_0_POLARITY_INVERSION_PORT     0X04
#define  REGISTER_1_POLARITY_INVERSION_PORT     0X05
#define  REGISTER_0_CONFIGURATION_PORT          0X06
#define  REGISTER_1_CONFIGURATION_PORT          0X07

//NCA9555_config
#define NCA9555_config_0            \
    {                               \
        .addr = RT_I2C_AMR_ADDR,    \
        .name = "AMR"               \
    }

#define NCA9555_config_1            \
    {                               \
        .addr = RT_I2C_LED_ADDR,    \
        .name = "LED"               \
    }

#define NCA9555_config_2            \
    {                               \
        .addr = RT_I2C_ELEC_ADDR,   \
        .name = "ELEC"              \
    }

//NCA9555IO_offset_config
#define IO0_0                \
    {                        \
        .name = "IO0_0",     \
        .offset = 0          \
    }

#define IO0_1                \
    {                        \
        .name = "IO0_1",     \
        .offset = 1          \
    }

#define IO0_2                \
    {                        \
        .name = "IO0_2",     \
        .offset = 2          \
    }

#define IO0_3                \
    {                        \
        .name = "IO0_3",     \
        .offset = 3          \
    }

#define IO0_4                \
    {                        \
        .name = "IO0_4",     \
        .offset = 4          \
    }

#define IO0_5                \
    {                        \
        .name = "IO0_5",     \
        .offset = 5          \
    }

#define IO0_6                \
    {                        \
        .name = "IO0_6",     \
        .offset = 6          \
    }

#define IO0_7                \
    {                        \
        .name = "IO0_7",     \
        .offset = 7          \
    }

#define IO1_0                \
    {                        \
        .name = "IO1_0",     \
        .offset = 8          \
    }

#define IO1_1                \
    {                        \
        .name = "IO1_1",     \
        .offset = 9          \
    }

#define IO1_2                \
    {                        \
        .name = "IO1_2",     \
        .offset = 10         \
    }

#define IO1_3                \
    {                        \
        .name = "IO1_3",     \
        .offset = 11         \
    }

#define IO1_4                \
    {                        \
        .name = "IO1_4",     \
        .offset = 12         \
    }

#define IO1_5                \
    {                        \
        .name = "IO1_5",     \
        .offset = 13         \
    }

#define IO1_6                \
    {                        \
        .name = "IO1_6",     \
        .offset = 14         \
    }

#define IO1_7                \
    {                        \
        .name = "IO1_7",     \
        .offset = 15         \
    }

struct pin_list
{
    const char* name;
    rt_off_t offset;
};

struct nca9555_device
{
    struct rt_i2c_bus_device* i2c_bus;
    rt_uint8_t addr;
};
typedef struct nca9555_device* nca_device_t;

struct rt_nca9555
{
    nca_device_t dev;
    rt_uint8_t addr;
    const char* name;
};

nca_device_t nca9555_init(const char* i2c_bus_name, rt_uint8_t addr);

rt_err_t nca9555_write_data(nca_device_t dev, rt_uint8_t reg, rt_uint8_t len, rt_uint8_t* buf);
rt_err_t nca9555_read_data(nca_device_t dev, rt_uint8_t reg, rt_uint8_t len, rt_uint8_t* buf);

#endif /* APPLICATIONS_I2C_MODULE_I2C_MODULE_H_ */
