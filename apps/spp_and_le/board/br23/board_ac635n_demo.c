#include "app_config.h"

#ifdef CONFIG_BOARD_AC635N_DEMO

#include "system/includes.h"
#include "asm/charge.h"
#include "device/key_driver.h"
#include "asm/power/p33.h"
#include "asm/iic_soft.h"
#ifdef CONFIG_LITE_AUDIO
#include "media/includes.h"
#endif/*CONFIG_LITE_AUDIO*/

#include "rtc_alarm.h"
#include "asm/power/power_port.h"

#define LOG_TAG_CONST       BOARD
#define LOG_TAG             "[BOARD]"
#define LOG_ERROR_ENABLE
#define LOG_DEBUG_ENABLE
#define LOG_INFO_ENABLE
/* #define LOG_DUMP_ENABLE */
#define LOG_CLI_ENABLE
#include "debug.h"

void board_power_init(void);
const struct soft_iic_config soft_iic_cfg[] = {
    //iic0 data
    {
        .scl = IO_PORTC_04,                             //IIC CLK脚
        .sda = IO_PORTC_05,                             //IIC DAT脚
        .delay = 50,                                    //软件IIC延时参数，影响通讯时钟频率
        .io_pu = 1,                                     //是否打开上拉电阻，如果外部电路没有焊接上拉电阻需要置1
    },
};

/************************** LOW POWER config ****************************/
const struct low_power_param power_param = {
    .config         = TCFG_LOWPOWER_LOWPOWER_SEL,          //0：sniff时芯片不进入低功耗  1：sniff时芯片进入powerdown
    .btosc_hz       = TCFG_CLOCK_OSC_HZ,                   //外接晶振频率
    .delay_us       = TCFG_CLOCK_SYS_HZ / 1000000L,        //提供给低功耗模块的延时(不需要需修改)
    .btosc_disable  = TCFG_LOWPOWER_BTOSC_DISABLE,         //进入低功耗时BTOSC是否保持
    .vddiom_lev     = TCFG_LOWPOWER_VDDIOM_LEVEL,          //强VDDIO等级,可选：2.0V  2.2V  2.4V  2.6V  2.8V  3.0V  3.2V  3.6V
    .vddiow_lev     = TCFG_LOWPOWER_VDDIOW_LEVEL,          //弱VDDIO等级,可选：2.1V  2.4V  2.8V  3.2V
    .osc_type       = OSC_TYPE_LRC,//OSC_TYPE_LRC,
    .dcdc_port      = TCFG_DCDC_PORT_SEL,
#if TCFG_RTC_ALARM_ENABLE
    .rtc_clk    	= CLK_SEL_32K,//CLK_SEL_32K,
#endif

#if TCFG_USE_VIRTUAL_RTC
    .virtual_rtc    = 1,
    .vir_rtc_trim_time = 60 * 20,
    .nv_timer_interval = 500,                           //s,lp_timer�жϼ��
#else
    .user_nv_timer_en  = 0,
#endif

};


/************************** KEY MSG****************************/
/*������������Ϣ���ã����USER_CFG��������USE_CONFIG_KEY_SETTINGΪ1�����������ļ���ȡ��Ӧ�����������Ľṹ��*/
u8 key_table[KEY_NUM_MAX][KEY_EVENT_MAX] = {
    // SHORT           LONG              HOLD              UP              DOUBLE           TRIPLE
};


// *INDENT-OFF*
/************************** UART config****************************/
#if TCFG_UART0_ENABLE
UART0_PLATFORM_DATA_BEGIN(uart0_data)
    .tx_pin = TCFG_UART0_TX_PORT,                             //���ڴ�ӡTX����ѡ��
    .rx_pin = TCFG_UART0_RX_PORT,                             //���ڴ�ӡRX����ѡ��
    .baudrate = TCFG_UART0_BAUDRATE,                          //���ڲ�����

    .flags = UART_DEBUG,                                      //����������ӡ��Ҫ�ѸĲ�������ΪUART_DEBUG
UART0_PLATFORM_DATA_END()
#endif //TCFG_UART0_ENABLE

/************************** CHARGE config****************************/
#if TCFG_CHARGE_ENABLE
CHARGE_PLATFORM_DATA_BEGIN(charge_data)
    .charge_en              = TCFG_CHARGE_ENABLE,              //���ó��ʹ��
    .charge_poweron_en      = TCFG_CHARGE_POWERON_ENABLE,      //�Ƿ�֧�ֳ�翪��
    .charge_full_V          = TCFG_CHARGE_FULL_V,              //����ֹ��ѹ
    .charge_full_mA			= TCFG_CHARGE_FULL_MA,             //����ֹ����
    .charge_mA				= TCFG_CHARGE_MA,                  //������
/*ldo5v�γ�����ֵ������ʱ�� = (filter*2 + 20)ms,ldoin<0.6V��ʱ����ڹ���ʱ�����Ϊ�γ�
 ���ڳ���ֱ�Ӵ�5V����0V�ĳ��֣���ֵ�������ó�0�����ڳ�����5V�ȵ���0V֮������ѹ��xV��
 ���֣���Ҫ����ʵ��������ø�ֵ��С*/
	.ldo5v_off_filter		= 100,
/*ldo5v��10k��������ʹ��,��������Ҫ����ĸ��ز��ܼ�⵽����ʱ���뽫�ñ�����1,Ĭ��ֵ����Ϊ1
  ���ڳ�����Ҫ������ѹ,��ά�ֵ�ѹ�Ǵӳ��վ����������赽���ڵĲգ��뽫��ֵ��Ϊ0*/
	.ldo5v_pulldown_en		= 0,
CHARGE_PLATFORM_DATA_END()
#endif//TCFG_CHARGE_ENABLE

    /************************** AD KEY ****************************/
#if TCFG_ADKEY_ENABLE
const struct adkey_platform_data adkey_data = {
    .enable = TCFG_ADKEY_ENABLE,                              //AD����ʹ��
    .adkey_pin = TCFG_ADKEY_PORT,                             //AD������Ӧ����
    .ad_channel = TCFG_ADKEY_AD_CHANNEL,                      //ADͨ��ֵ
    .extern_up_en = TCFG_ADKEY_EXTERN_UP_ENABLE,              //�Ƿ�ʹ�������������
    .ad_value = {                                             //���ݵ���������ĵ�ѹֵ
        TCFG_ADKEY_VOLTAGE0,
        TCFG_ADKEY_VOLTAGE1,
        TCFG_ADKEY_VOLTAGE2,
        TCFG_ADKEY_VOLTAGE3,
        TCFG_ADKEY_VOLTAGE4,
        TCFG_ADKEY_VOLTAGE5,
        TCFG_ADKEY_VOLTAGE6,
        TCFG_ADKEY_VOLTAGE7,
        TCFG_ADKEY_VOLTAGE8,
        TCFG_ADKEY_VOLTAGE9,
    },
    .key_value = {                                             //AD�������������ļ�ֵ
        TCFG_ADKEY_VALUE0,
        TCFG_ADKEY_VALUE1,
        TCFG_ADKEY_VALUE2,
        TCFG_ADKEY_VALUE3,
        TCFG_ADKEY_VALUE4,
        TCFG_ADKEY_VALUE5,
        TCFG_ADKEY_VALUE6,
        TCFG_ADKEY_VALUE7,
        TCFG_ADKEY_VALUE8,
        TCFG_ADKEY_VALUE9,
    },
};
#endif

#if TCFG_IRKEY_ENABLE
const struct irkey_platform_data irkey_data = {
	    .enable = TCFG_IRKEY_ENABLE,                              //IR����ʹ��
	    .port = TCFG_IRKEY_PORT,                                       //IR������
};
#endif

/************************** IO KEY ****************************/
#if TCFG_IOKEY_ENABLE
const struct iokey_port iokey_list[] = {
	{
		.connect_way = TCFG_IOKEY_POWER_CONNECT_WAY,          // IO连接方式
		.key_type.one_io.port = TCFG_IOKEY_POWER_ONE_PORT,    // IO端口
		.key_value = TCFG_IOKEY_POWER_ONE_PORT_VALUE,         // 按键值
	},

	/* { */
		/* .connect_way = TCFG_IOKEY_PREV_CONNECT_WAY, */
		/* .key_type.one_io.port = TCFG_IOKEY_PREV_ONE_PORT, */
		/* .key_value = TCFG_IOKEY_PREV_ONE_PORT_VALUE, */
	/* }, */

	/* { */
		/* .connect_way = TCFG_IOKEY_NEXT_CONNECT_WAY, */
		/* .key_type.one_io.port = TCFG_IOKEY_NEXT_ONE_PORT, */
		/* .key_value = TCFG_IOKEY_NEXT_ONE_PORT_VALUE, */
	/* }, */

    {
        .connect_way = TCFG_IOKEY_AIR_CONNECT_WAY,          // IO连接方式
        .key_type.one_io.port = TCFG_IOKEY_AIR_ONE_PORT,    // IO端口
        .key_value = TCFG_IOKEY_AIR_ONE_PORT_VALUE,         // 按键值
    },

    {
        .connect_way = TCFG_IOKEY_AIR_KEY_WAY,          // IO连接方式
        .key_type.one_io.port = TCFG_IOKEY_AIR_KEY_PORT,    // IO端口
        .key_value = TCFG_IOKEY_AIR_ONE_PORT_VALUE,         // 按键值
    }
};
const struct iokey_platform_data iokey_data = {
	.enable = TCFG_IOKEY_ENABLE,                              //是否使能IO按键
	.num = ARRAY_SIZE(iokey_list),                            //IO按键的个数
	.port = iokey_list,                                       //IO按键端口列表
};

#if MULT_KEY_ENABLE
//组合按键消息映射表
//配置注意事项:单个按键按键值需要按照顺序编号,如power:0, prev:1, next:2
//bit_value = BIT(0) | BIT(1) 指按键值为0和按键值为1的两个按键被同时按下,
//remap_value = 3指当这两个按键被同时按下后重新映射的按键值;
const struct key_remap iokey_remap_table[] = {
	{.bit_value = BIT(0) | BIT(1), .remap_value = 3},
	{.bit_value = BIT(0) | BIT(2), .remap_value = 4},
	{.bit_value = BIT(1) | BIT(2), .remap_value = 5},
};

const struct key_remap_data iokey_remap_data = {
	.remap_num = ARRAY_SIZE(iokey_remap_table),
	.table = iokey_remap_table,
};
#endif

#endif

#if TCFG_RTC_ALARM_ENABLE
const struct sys_time def_sys_time = {  //默认时间
    .year = 2025,
    .month = 1,
    .day = 1,
    .hour = 8,
    .min = 12,
    .sec = 0,
};
const struct sys_time def_alarm = {     //默认闹钟时间
    .year = 2025,
    .month = 1,
    .day = 1,
    .hour = 0,
    .min = 0,
    .sec = 7,
};

extern void alarm_isr_user_cbfun(u8 index);
RTC_DEV_PLATFORM_DATA_BEGIN(rtc_data)
	.default_sys_time = &def_sys_time,
	.default_alarm = &def_alarm,
    /* .cbfun = NULL,                      //    默认无闹钟 */
    .cbfun = alarm_isr_user_cbfun,
RTC_DEV_PLATFORM_DATA_END()
#endif

void debug_uart_init(const struct uart_platform_data *data)
{
#if TCFG_UART0_ENABLE
    if (data) {
        uart_init(data);
    } else {
        uart_init(&uart0_data);
    }
#endif
}


/*
 * @brief      获取开机状态
 * @return     0:关机 1:开机
 */
u8 get_power_on_status(void)
{
#if TCFG_IOKEY_ENABLE
    struct iokey_port *power_io_list = NULL;
    power_io_list = iokey_data.port;

    if (iokey_data.enable) {
        if (gpio_read(power_io_list->key_type.one_io.port) == power_io_list->connect_way){
            return 1;
        }
    }
#endif

#if TCFG_ADKEY_ENABLE
    if (adkey_data.enable) {
        return 1;
    }
#endif

#if TCFG_LP_TOUCH_KEY_ENABLE
    return lp_touch_key_power_on_status();
#endif

    return 0;
}

/*
 * @brief      获取按键状态
 * @return     按键值
 */
u8 get_check_status(void)
{
    for (int i = 1; i < iokey_data.num; i++) {
        struct iokey_port *key_port = &iokey_data.port[i];
        if (key_port->key_value == TCFG_IOKEY_AIR_ONE_PORT_VALUE) {
            if (gpio_read(key_port->key_type.one_io.port) == key_port->connect_way) {
                return i; // 按键按下
            }
        } else if (key_port->key_value == TCFG_IOKEY_AIR_ONE_PORT_VALUE) {
            if (gpio_read(key_port->key_type.one_io.port) == key_port->connect_way) {
                return i; // 按键按下
            }
        }
    }

    return 0; // 无按键按下
}

static void alm_wakeup_isr()
{
    printf("alarm_wakeup_isr!!!!!");
}
/**
 * 设置RTC默认时间（备用时间）
 * 
 * 该函数将系统时间结构体初始化为预设的默认时间值。
 * 默认时间设置为：2020年2月28日 23时59分40秒
 * 
 * @param t 指向系统时间结构体的指针，用于存储默认时间值
 */
void  set_rtc_default_time(struct sys_time *t)
{
    // 设置默认时间为2020年2月28日23时59分40秒
    t->year = 2020;
    t->month = 2;
    t->day = 28;
    t->hour = 23;
    t->min = 59;
    t->sec = 40;
}

static void board_devices_init(void)
{
#if TCFG_PWMLED_ENABLE
    pwm_led_init(&pwm_led_data);
#endif

#if (TCFG_IOKEY_ENABLE || TCFG_ADKEY_ENABLE || TCFG_IRKEY_ENABLE || TCFG_TOUCH_KEY_ENABLE)
	key_driver_init();
#endif

#if TCFG_USE_VIRTUAL_RTC
    vir_rtc_simulate_init(NULL, NULL);
#endif

#if TCFG_RTC_ALARM_ENABLE
    alarm_init(&rtc_data);
#endif
}

extern void set_dcdc_ctl_port(u8 port);
extern void cfg_file_parse(u8 idx);
void board_init()
{
    board_power_init();
    adc_vbg_init();
    adc_init();
    cfg_file_parse(0);
    devices_init();

	board_devices_init();

#if TCFG_CHARGE_ENABLE
    charge_api_init((void *)&charge_data);
#if TCFG_HANDSHAKE_ENABLE
    if(get_charge_online_flag()){
        handshake_app_start(0, NULL);
    }
#endif
#else
	/*close FAST CHARGE */
	CHARGE_EN(0);
	CHGBG_EN(0);
#endif

	log_info("board_init");

	if(get_charge_online_flag()){
		log_info("charge...select LDO15");
		power_set_mode(PWR_LDO15);
	}else{
		power_set_mode(TCFG_LOWPOWER_POWER_SEL);
	}
}

/************************** DAC ****************************/
#if TCFG_AUDIO_DAC_ENABLE
struct dac_platform_data dac_data = {
    .ldo_volt       = TCFG_AUDIO_DAC_LDO_VOLT,                   //DACVDD�ȼ�.��Ҫ���ݾ���Ӳ�������ã��ߵ�ѹ����ѡ:1.2V/1.3V/2.35V/2.5V/2.65V/2.8V/2.95V/3.1V
#if ((TCFG_AUDIO_DAC_CONNECT_MODE == DAC_OUTPUT_FRONT_LR_REAR_LR) || (TCFG_AUDIO_DAC_CONNECT_MODE == DAC_OUTPUT_DUAL_LR_DIFF))
    .vcmo_en        = 0,                                         //��������˫������ֹر�VCOMO
#else
    .vcmo_en        = 0,                                         //�Ƿ��VCOMO
#endif
    .output         = TCFG_AUDIO_DAC_CONNECT_MODE,               //DAC������ã��;���Ӳ�������йأ������Ӳ��������
    .ldo_isel       = 3,
    .ldo_fb_isel    = 2,
    .lpf_isel       = 0x8,
};
#endif

/************************** ADC ****************************/
#if TCFG_AUDIO_ADC_ENABLE
const struct ladc_port ladc_list[] = {
	{// 0
		.channel = TCFG_AUDIO_ADC_LINE_CHA0,
	},
	{// 1
		.channel = TCFG_AUDIO_ADC_LINE_CHA1,
	},
	// total must < 4
};
struct adc_platform_data adc_data = {
	.mic_channel    = TCFG_AUDIO_ADC_MIC_CHA,                   //MICͨ��ѡ�񣬶���693x��MICֻ��һ��ͨ�����̶�ѡ��������
/*MIC LDO������λ���ã�
    0:0.625ua    1:1.25ua    2:1.875ua    3:2.5ua*/
	.mic_ldo_isel   = TCFG_AUDIO_ADC_LDO_SEL,
/*MIC �Ƿ�ʡ��ֱ���ݣ�
    0: ��ʡ����  1: ʡ���� */
#if ((TCFG_AUDIO_DAC_CONNECT_MODE == DAC_OUTPUT_FRONT_LR_REAR_LR) || (TCFG_AUDIO_DAC_CONNECT_MODE == DAC_OUTPUT_DUAL_LR_DIFF))
	.mic_capless    = 0,//��������˫�������ʹ�ã���ʡ���ݽӷ�
#else
	.mic_capless    = 0,
#endif
/*MIC�ڲ��������赲λ���ã�Ӱ��MIC��ƫ�õ�ѹ
    21:1.18K	20:1.42K 	19:1.55K 	18:1.99K 	17:2.2K 	16:2.4K 	15:2.6K		14:2.91K	13:3.05K 	12:3.5K 	11:3.73K
	10:3.91K  	9:4.41K 	8:5.0K  	7:5.6K		6:6K		5:6.5K		4:7K		3:7.6K		2:8.0K		1:8.5K				*/
    .mic_bias_res   = 16,
/*MIC LDO��ѹ��λ����,Ҳ��Ӱ��MIC��ƫ�õ�ѹ
    0:2.3v  1:2.5v  2:2.7v  3:3.0v */
	.mic_ldo_vsel  = 2,
/*MIC���ݸ�ֱģʽʹ���ڲ�micƫ��(PC7)*/
	.mic_bias_inside = 1,
/*�����ڲ�micƫ�����*/
	.mic_bias_keep = 0,

	// ladc ͨ��
    .ladc_num = ARRAY_SIZE(ladc_list),
    .ladc = ladc_list,
};
#endif

/************************** PWR config ****************************/
struct port_wakeup port0 = {
	.pullup_down_enable = ENABLE,                            //配置I/O 内部上下拉是否使能
	.edge               = FALLING_EDGE,                      //唤醒方式选择,可选：上升沿\下降沿
	.attribute          = BLUETOOTH_RESUME,                  //保留参数
#if TCFG_ADKEY_ENABLE
	.iomap              = TCFG_ADKEY_PORT,                   //唤醒口选择
#endif
#if TCFG_IOKEY_ENABLE
	.iomap              = TCFG_IOKEY_POWER_ONE_PORT,                   //IO唤醒口选择
#endif
    .filter_enable      = ENABLE,
};

const struct sub_wakeup sub_wkup = {
	.attribute  = BLUETOOTH_RESUME,
};

const struct charge_wakeup charge_wkup = {
	.attribute  = BLUETOOTH_RESUME,
};

const struct wakeup_param wk_param = {
#if TCFG_ADKEY_ENABLE || TCFG_IOKEY_ENABLE
	.port[1] = &port0,
#endif
	.sub = &sub_wkup,
	.charge = &charge_wkup,
};

//-----------------------------------------------
static struct port_wakeup port1 = {
	.pullup_down_enable = ENABLE,                            //����I/O �ڲ��������Ƿ�ʹ��
	.edge               = FALLING_EDGE,                      //���ѷ�ʽѡ��,��ѡ��������\�½���
	.attribute          = BLUETOOTH_RESUME,                  //��������
	.iomap              = 0,                                 //���ѿ�ѡ��
    .filter_enable      = DISABLE,
};

/*
note:
1.sleep���̽�ֹ��ʱ������
2.�ϵ��ʼ��������io����Ϊ���裬�͹��������뽫io����Ϊ����/ȷ��״̬��
 */
extern void dac_power_off(void);
u32 spi_get_port(void);
void board_set_soft_poweroff(void)
{
    u16 port_group[] = {
        [PORTA_GROUP] = 0xffff,
        [PORTB_GROUP] = 0xffff,
        [PORTC_GROUP] = 0xffff,
		[PORTD_GROUP] = 0Xffff,
    };

	if(TCFG_LOWPOWER_POWER_SEL == PWR_DCDC15){
		power_set_mode(PWR_LDO15);
	}

	//flash��Դ
	if(spi_get_port()==0){
		port_protect(port_group, SPI0_PWR_A);
		port_protect(port_group, SPI0_CS_A);
		port_protect(port_group, SPI0_CLK_A);
		port_protect(port_group, SPI0_DO_D0_A);
		port_protect(port_group, SPI0_DI_D1_A);
		if(get_sfc_bit_mode()==4){
			port_protect(port_group, SPI0_WP_D2_A);
			port_protect(port_group, SPI0_HOLD_D3_A);
		}
	}else{
		port_protect(port_group, SPI0_PWR_B);
		port_protect(port_group, SPI0_CS_B);
		port_protect(port_group, SPI0_CLK_B);
		port_protect(port_group, SPI0_DO_D0_B);
		port_protect(port_group, SPI0_DI_D1_B);
		if(get_sfc_bit_mode()==4){
			port_protect(port_group, SPI0_WP_D2_B);
			port_protect(port_group, SPI0_HOLD_D3_B);
		}
	}

//adkey / io������Ϊ���ѿڱ���
#if TCFG_IOKEY_ENABLE
	port_protect(port_group, TCFG_IOKEY_POWER_ONE_PORT);
#endif

#if TCFG_ADKEY_ENABLE
    port_protect(port_group,TCFG_ADKEY_PORT);
#endif

    //< close gpio
    gpio_dir(GPIOA, 0, 16, port_group[PORTA_GROUP], GPIO_OR);
    gpio_set_pu(GPIOA, 0, 16, ~port_group[PORTA_GROUP], GPIO_AND);
    gpio_set_pd(GPIOA, 0, 16, ~port_group[PORTA_GROUP], GPIO_AND);
    gpio_die(GPIOA, 0, 16, ~port_group[PORTA_GROUP], GPIO_AND);
    gpio_dieh(GPIOA, 0, 16, ~port_group[PORTA_GROUP], GPIO_AND);

    gpio_dir(GPIOB, 0, 16, port_group[PORTB_GROUP], GPIO_OR);
    gpio_set_pu(GPIOB, 0, 16, ~port_group[PORTB_GROUP], GPIO_AND);
    gpio_set_pd(GPIOB, 0, 16, ~port_group[PORTB_GROUP], GPIO_AND);
    gpio_die(GPIOB, 0, 16, ~port_group[PORTB_GROUP], GPIO_AND);
    gpio_dieh(GPIOB, 0, 16, ~port_group[PORTB_GROUP], GPIO_AND);

    gpio_dir(GPIOC, 0, 16, port_group[PORTC_GROUP], GPIO_OR);
    gpio_set_pu(GPIOC, 0, 16, ~port_group[PORTC_GROUP], GPIO_AND);
    gpio_set_pd(GPIOC, 0, 16, ~port_group[PORTC_GROUP], GPIO_AND);
    gpio_die(GPIOC, 0, 16, ~port_group[PORTC_GROUP], GPIO_AND);
    gpio_dieh(GPIOC, 0, 16, ~port_group[PORTC_GROUP], GPIO_AND);

	gpio_dir(GPIOD, 0, 16, port_group[PORTD_GROUP], GPIO_OR);
    gpio_set_pu(GPIOD, 0, 16, ~port_group[PORTD_GROUP], GPIO_AND);
    gpio_set_pd(GPIOD, 0, 16, ~port_group[PORTD_GROUP], GPIO_AND);
    gpio_die(GPIOD, 0, 16, ~port_group[PORTD_GROUP], GPIO_AND);
    gpio_dieh(GPIOD, 0, 16, ~port_group[PORTD_GROUP], GPIO_AND);

    usb_iomode(1);

	gpio_set_pull_up(IO_PORT_DP, 0);
    gpio_set_pull_down(IO_PORT_DP, 0);
    gpio_set_direction(IO_PORT_DP, 1);
    gpio_set_die(IO_PORT_DP, 0);
    gpio_set_dieh(IO_PORT_DP, 0);

    gpio_set_pull_up(IO_PORT_DM, 0);
    gpio_set_pull_down(IO_PORT_DM, 0);
    gpio_set_direction(IO_PORT_DM, 1);
    gpio_set_die(IO_PORT_DM, 0);
    gpio_set_dieh(IO_PORT_DM, 0);
}

void sleep_exit_callback(u32 usec)
{
	putchar('>');
}

void sleep_enter_callback(u8  step)
{
	if (step == 1) {
		putchar('<');
#if TCFG_AUDIO_ENABLE
        dac_power_off();
#endif/*TCFG_AUDIO_ENABLE*/
    } else {
	   	usb_iomode(1);

		gpio_set_pull_up(IO_PORT_DP, 0);
        gpio_set_pull_down(IO_PORT_DP, 0);
        gpio_set_direction(IO_PORT_DP, 1);
        gpio_set_die(IO_PORT_DP, 0);
        gpio_set_dieh(IO_PORT_DP, 0);

        gpio_set_pull_up(IO_PORT_DM, 0);
        gpio_set_pull_down(IO_PORT_DM, 0);
        gpio_set_direction(IO_PORT_DM, 1);
        gpio_set_die(IO_PORT_DM, 0);
        gpio_set_dieh(IO_PORT_DM, 0);
	}
}

void board_power_init(void)
{
    log_info("Power init : %s", __FILE__);

    power_init(&power_param);

    gpio_longpress_pin0_reset_config(IO_PORTB_01, 0, 0);
    gpio_shortpress_reset_config(0);//1--enable 0--disable

    power_set_callback(TCFG_LOWPOWER_LOWPOWER_SEL, sleep_enter_callback, sleep_exit_callback, board_set_soft_poweroff);

	power_keep_dacvdd_en(0);

	power_wakeup_init(&wk_param);

#if (!TCFG_IOKEY_ENABLE && !TCFG_ADKEY_ENABLE)
    charge_check_and_set_pinr(0);
#endif
#if USER_UART_UPDATE_ENABLE
	{
#include "uart_update.h"
		uart_update_cfg update_cfg = {
			.rx = UART_UPDATE_RX_PORT,
			.tx = UART_UPDATE_TX_PORT,
			.output_channel = CH1_UT1_TX,
			.input_channel = INPUT_CH0,
		};
		uart_update_init(&update_cfg);
	}
#endif
}
#endif
