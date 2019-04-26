#include <stdio.h>

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_attr.h"

#include "driver/mcpwm.h"
#include "soc/mcpwm_reg.h"
#include "soc/mcpwm_struct.h"

#define GPIO_PWM0A_OUT 26   //Set GPIO 15 as PWM0A left
#define GPIO_PWM0B_OUT 19   //Set GPIO 16 as PWM0B	right

#include <stdlib.h>
#include <math.h>
#include "driver/gpio.h"
#include "driver/adc.h"
#include "driver/uart.h"
#include "esp_adc_cal.h"
#include "time.h"

#include "string.h"
#include "freertos/queue.h"
#include "soc/rtc.h"

#include "esp_types.h"
#include "soc/timer_group_struct.h"
#include "driver/periph_ctrl.h"
#include "driver/timer.h"

#include "freertos/queue.h"
#include "soc/timer_group_struct.h"
#include "driver/periph_ctrl.h"
#include "driver/timer.h"

#include <string.h>
#include <sys/param.h>
#include "freertos/event_groups.h"
#include "esp_system.h"
#include "esp_wifi.h"
#include "esp_event_loop.h"
#include "esp_log.h"
#include "nvs_flash.h"

#include "lwip/err.h"
#include "lwip/sockets.h"
#include "lwip/sys.h"
#include <lwip/netdb.h>

#define Kp 0.8
#define Ki 0
#define Kd 0.4

#define TIMER_INTR_SEL TIMER_INTR_LEVEL  /*!< Timer level interrupt */
#define TIMER_GROUP    TIMER_GROUP_0     /*!< Test on timer group 0 */
#define TIMER_DIVIDER   80               /*!< Hardware timer clock divider, 80 to get 1MHz clock to timer */
#define TIMER_SCALE    (TIMER_BASE_CLK / TIMER_DIVIDER)  /*!< used to calculate counter value */
#define TIMER_FINE_ADJ   (0*(TIMER_BASE_CLK / TIMER_DIVIDER)/1000000) /*!< used to compensate alarm value */
#define TIMER_INTERVAL0_SEC   (0.1)   /*!< test interval for timer 0 */

#define DEFAULT_VREF    1100        //Use adc2_vref_to_gpio() to obtain a better estimate
#define NO_OF_SAMPLES   64

#define BUF_SIZE (100)
#define ECHO_TEST_TXD  (GPIO_NUM_17)
#define ECHO_TEST_RXD  (GPIO_NUM_16)
#define ECHO_TEST_RTS  (UART_PIN_NO_CHANGE)
#define ECHO_TEST_CTS  (UART_PIN_NO_CHANGE)

#define ECHO_TEST_TXD_SIDE  (GPIO_NUM_33)
#define ECHO_TEST_RXD_SIDE  (GPIO_NUM_15)
#define ECHO_TEST_RTS_SIDE  (UART_PIN_NO_CHANGE)
#define ECHO_TEST_CTS_SIDE  (UART_PIN_NO_CHANGE)

#define ECHO_TEST_TXD_B  (GPIO_NUM_4)
#define ECHO_TEST_RXD_B  (GPIO_NUM_4)
#define ECHO_TEST_RTS_B  (UART_PIN_NO_CHANGE)
#define ECHO_TEST_CTS_B  (UART_PIN_NO_CHANGE)

#define ID 0
#define EXAMPLE_WIFI_SSID "Group_7"
#define EXAMPLE_WIFI_PASS "smart-systems"
#define HOST_IP_ADDR "192.168.1.115"
#define PORT 3000

#define ID 1

static const char *TAG = "example";
static const char *payload = "whizzer.bu.edu";

char msg_send[10];
char msg_prev[10] = "asdfghjklz";

int len_out = 3;

char myID = (char)ID;

char command = 'n';
char rxID;
char start = 0x1B;

bool send_state = false;


//adc stuff for IR sensor
/*static esp_adc_cal_characteristics_t *adc_chars;
static const adc_channel_t channel0 = ADC_CHANNEL_6;     //GPIO34 
static const adc_atten_t atten = ADC_ATTEN_DB_0;
static const adc_unit_t unit = ADC_UNIT_1;*/

int dt_complete = 1;
double dt = 0.1;
double previous_error ;		// Set up PID loop
double integral ;
double error;
double integral;
double derivative;
double output;
double setpoint = 40; //setpoint is 50 cm
double PID(int measured_value) {
  error = setpoint - measured_value;
  integral = integral + error * dt;
  derivative = (error - previous_error) / dt;
  output = Kp * error + Ki * integral + Kd * derivative;
  previous_error = error;
  return output;
}


void IRAM_ATTR timer_isr()
{
    // Clear interrupt
    TIMERG0.int_clr_timers.t0 = 1;
    // Indicate timer has fired
    dt_complete = 1;
}

// Set up periodic timer for dt = 100ms
static void periodic_timer_init()
{
	int timer_group = TIMER_GROUP_0;
    int timer_idx = TIMER_0;
    // Basic parameters of the timer
    timer_config_t config;
    config.divider = TIMER_DIVIDER;
    config.counter_dir = TIMER_COUNT_UP;
    config.counter_en = TIMER_PAUSE;
    config.alarm_en = TIMER_ALARM_EN;
    config.intr_type = TIMER_INTR_LEVEL;
    config.auto_reload = 1;

    // register timer interrupt
    timer_init(TIMER_GROUP_0, timer_idx, &config);

    // Timer's counter will initially start from value below
    timer_set_counter_value(TIMER_GROUP_0, timer_idx, 0x00000000ULL);

    // Configure the alarm value and the interrupt on alarm.
    timer_set_alarm_value(TIMER_GROUP_0, timer_idx, TIMER_INTERVAL0_SEC * TIMER_SCALE);
    timer_enable_intr(TIMER_GROUP_0, timer_idx);
    timer_isr_register(TIMER_GROUP_0, timer_idx, timer_isr,
        (void *) timer_idx, ESP_INTR_FLAG_IRAM, NULL);

    // Start timer
    timer_start(TIMER_GROUP_0, timer_idx);
}

static void mcpwm_example_gpio_initialize()
{
    printf("initializing mcpwm gpio...\n");
    mcpwm_gpio_init(MCPWM_UNIT_0, MCPWM0A, GPIO_PWM0A_OUT);
    mcpwm_gpio_init(MCPWM_UNIT_0, MCPWM0B, GPIO_PWM0B_OUT);
}

uint16_t lidar_result;
	
uint16_t lidar_side;
	
int speed_r = 80;
int speed_l = 80;
 
 static EventGroupHandle_t wifi_event_group;

const int IPV4_GOTIP_BIT = BIT0;
const int IPV6_GOTIP_BIT = BIT1;


static esp_err_t event_handler(void *ctx, system_event_t *event)
{
    switch (event->event_id) {
    case SYSTEM_EVENT_STA_START:
        esp_wifi_connect();
        ESP_LOGI(TAG, "SYSTEM_EVENT_STA_START");
        break;
    case SYSTEM_EVENT_STA_CONNECTED:
        /* enable ipv6 */
        tcpip_adapter_create_ip6_linklocal(TCPIP_ADAPTER_IF_STA);
        break;
    case SYSTEM_EVENT_STA_GOT_IP:
        xEventGroupSetBits(wifi_event_group, IPV4_GOTIP_BIT);
        ESP_LOGI(TAG, "SYSTEM_EVENT_STA_GOT_IP");
        break;
    case SYSTEM_EVENT_STA_DISCONNECTED:
        /* This is a workaround as ESP32 WiFi libs don't currently auto-reassociate. */
        esp_wifi_connect();
        xEventGroupClearBits(wifi_event_group, IPV4_GOTIP_BIT);
        xEventGroupClearBits(wifi_event_group, IPV6_GOTIP_BIT);
        break;
    case SYSTEM_EVENT_AP_STA_GOT_IP6:
        xEventGroupSetBits(wifi_event_group, IPV6_GOTIP_BIT);
        ESP_LOGI(TAG, "SYSTEM_EVENT_STA_GOT_IP6");

        char *ip6 = ip6addr_ntoa(&event->event_info.got_ip6.ip6_info.ip);
        ESP_LOGI(TAG, "IPv6: %s", ip6);
    default:
        break;
    }
    return ESP_OK;
}

static void initialise_wifi(void)
{
    tcpip_adapter_init();
    wifi_event_group = xEventGroupCreate();
    ESP_ERROR_CHECK( esp_event_loop_init(event_handler, NULL) );
    wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
    ESP_ERROR_CHECK( esp_wifi_init(&cfg) );
    ESP_ERROR_CHECK( esp_wifi_set_storage(WIFI_STORAGE_RAM) );
    wifi_config_t wifi_config = {
        .sta = {
            .ssid = EXAMPLE_WIFI_SSID,
            .password = EXAMPLE_WIFI_PASS,
        },
    };
    ESP_LOGI(TAG, "Setting WiFi configuration SSID %s...", wifi_config.sta.ssid);
    ESP_ERROR_CHECK( esp_wifi_set_mode(WIFI_MODE_STA) );
    ESP_ERROR_CHECK( esp_wifi_set_config(ESP_IF_WIFI_STA, &wifi_config) );
    ESP_ERROR_CHECK( esp_wifi_start() );
}

static void wait_for_ip()
{
    uint32_t bits = IPV4_GOTIP_BIT | IPV6_GOTIP_BIT ;

    ESP_LOGI(TAG, "Waiting for AP connection...");
    xEventGroupWaitBits(wifi_event_group, bits, false, true, portMAX_DELAY);
    ESP_LOGI(TAG, "Connected to AP");
}
static void udp_server_task(void *pvParameters)
{	
	printf("UPD_SERVER_TASK is runnning\n");
    char rx_buffer[128];
    char addr_str[128];
    int addr_family;
    int ip_protocol;

    while (1) {
		printf("SERVer\n");
        struct sockaddr_in destAddr;
        destAddr.sin_addr.s_addr = htonl(INADDR_ANY);
        destAddr.sin_family = AF_INET;
        destAddr.sin_port = htons(PORT);
        addr_family = AF_INET;
        ip_protocol = IPPROTO_IP;
        inet_ntoa_r(destAddr.sin_addr, addr_str, sizeof(addr_str) - 1);
		
        int sock = socket(addr_family, SOCK_DGRAM, ip_protocol);
        if (sock < 0) {
            ESP_LOGE(TAG, "Unable to create socket: errno %d", errno);
            break;
        }
        ESP_LOGI(TAG, "Socket created");

        int err = bind(sock, (struct sockaddr *)&destAddr, sizeof(destAddr));
        if (err < 0) {
            ESP_LOGE(TAG, "Socket unable to bind: errno %d", errno);
        }
        ESP_LOGI(TAG, "Socket binded");

		int ii = 0;
        while (ii < 10) {
            ESP_LOGI(TAG, "Waiting for data");
            struct sockaddr_in6 sourceAddr; // Large enough for both IPv4 or IPv6
            socklen_t socklen = sizeof(sourceAddr);
            int len = recvfrom(sock, rx_buffer, sizeof(rx_buffer) - 1, 0, (struct sockaddr *)&sourceAddr, &socklen);

            // Error occured during receiving
            if (len < 0) {
                ESP_LOGE(TAG, "recvfrom failed: errno %d", errno);
                break;
            }
            // Data received
            else {
                // Get the sender's ip address as string
                if (sourceAddr.sin6_family == PF_INET) {
                    inet_ntoa_r(((struct sockaddr_in *)&sourceAddr)->sin_addr.s_addr, addr_str, sizeof(addr_str) - 1);
                } else if (sourceAddr.sin6_family == PF_INET6) {
                    inet6_ntoa_r(sourceAddr.sin6_addr, addr_str, sizeof(addr_str) - 1);
                }

                rx_buffer[len] = 0; // Null-terminate whatever we received and treat like a string...
                ESP_LOGI(TAG, "Received %d bytes from %s:", len, addr_str);
                ESP_LOGI(TAG, "MESSAGE RECEIVED = %s", rx_buffer);
				command = rx_buffer[0];

                int err = sendto(sock, rx_buffer, len, 0, (struct sockaddr *)&sourceAddr, sizeof(sourceAddr));
                if (err < 0) {
                    ESP_LOGE(TAG, "Error occured during sending: errno %d", errno);
                    break;
                }
            }
			
			printf("received sth\n");
			vTaskDelay(200);
        }
		

        /*if (sock != -1) {
            ESP_LOGE(TAG, "Shutting down socket and restarting...");
            shutdown(sock, 0);
            close(sock);
        }*/
		break;
    }
    vTaskDelete(NULL);
}


uart_config_t uart_config_beacon = {
        .baud_rate = 1200,
        .data_bits = UART_DATA_8_BITS,
        .parity    = UART_PARITY_DISABLE,
        .stop_bits = UART_STOP_BITS_1,
        .flow_ctrl = UART_HW_FLOWCTRL_DISABLE
	};


static void udp_client_task(void *pvParameters)
{
    char rx_buffer[128];
    char addr_str[128];
    int addr_family;
    int ip_protocol;

    while (1) {
        struct sockaddr_in destAddr;
        destAddr.sin_addr.s_addr = inet_addr(HOST_IP_ADDR);
        destAddr.sin_family = AF_INET;
        destAddr.sin_port = htons(PORT);
        addr_family = AF_INET;
        ip_protocol = IPPROTO_IP;
        inet_ntoa_r(destAddr.sin_addr, addr_str, sizeof(addr_str) - 1);

        int sock = socket(addr_family, SOCK_DGRAM, ip_protocol);
        if (sock < 0) {
            ESP_LOGE(TAG, "Unable to create socket: errno %d", errno);
            //break;
        }
        ESP_LOGI(TAG, "Socket created");

        while (1) {
			if (send_state == true)
			{
				send_state = false;
				int err = sendto(sock, msg_send, strlen(msg_send), 0, (struct sockaddr *)&destAddr, sizeof(destAddr));
				if (err < 0) {
					ESP_LOGE(TAG, "Error occured during sending: errno %d", errno);
					break;
				}
				printf("SENDING...\n");
			}
            //ESP_LOGI(TAG, "Message sent");
						
            /*struct sockaddr_in sourceAddr; // Large enough for both IPv4 or IPv6
            socklen_t socklen = sizeof(sourceAddr);
            int len = recvfrom(sock, rx_buffer, sizeof(rx_buffer) - 1, 0, (struct sockaddr *)&sourceAddr, &socklen);

            // Error occured during receiving
            if (len < 0) {
                ESP_LOGE(TAG, "recvfrom failed: errno %d", errno);
                break;
            }
            // Data received
            else {
                rx_buffer[len] = 0; // Null-terminate whatever we received and treat like a string
                ESP_LOGI(TAG, "Received %d bytes from %s:", len, addr_str);
                ESP_LOGI(TAG, "%s", rx_buffer);
            }*/
            vTaskDelay(200);
        }

        /*if (sock != -1) {
            ESP_LOGE(TAG, "Shutting down socket and restarting...");
            shutdown(sock, 0);
            close(sock);
        }
    }*/
    vTaskDelete(NULL);
	}
}

static void mcpwm_example_brushed_motor_control(void *arg)
{
	periodic_timer_init();
	
	uart_config_t uart_config = {
		.baud_rate = 115200,
		.data_bits = UART_DATA_8_BITS,
		.parity    = UART_PARITY_DISABLE,
		.stop_bits = UART_STOP_BITS_1,
		.flow_ctrl = UART_HW_FLOWCTRL_DISABLE
		};
		
	/*uart_config_t uart_config_side = {
		.baud_rate = 115200,
		.data_bits = UART_DATA_8_BITS,
		.parity    = UART_PARITY_DISABLE,
		.stop_bits = UART_STOP_BITS_1,
		.flow_ctrl = UART_HW_FLOWCTRL_DISABLE
		};*/
		
    //1. mcpwm gpio initialization
    mcpwm_example_gpio_initialize();

    //2. initial mcpwm configuration
    //printf("Configuring Initial Parameters of mcpwm...\n");
    mcpwm_config_t pwm_config;
    pwm_config.frequency = 2000;    //frequency = 500Hz,
    pwm_config.cmpr_a = 100.0;    //duty cycle of PWMxA = 0
    pwm_config.cmpr_b = 100.0;    //duty cycle of PWMxb = 0
    pwm_config.counter_mode = MCPWM_UP_COUNTER;
    pwm_config.duty_mode = MCPWM_DUTY_MODE_0;
    mcpwm_init(MCPWM_UNIT_0, MCPWM_TIMER_0, &pwm_config);    //Configure PWM0A & PWM0B with above settings
	
	//right wheel
	gpio_pad_select_gpio(25);
	gpio_pad_select_gpio(18);
	gpio_set_direction(25, GPIO_MODE_OUTPUT);
	gpio_set_direction(18, GPIO_MODE_OUTPUT);
	gpio_set_level(25, 1);
	gpio_set_level(18, 0);

	//left wheel
	gpio_pad_select_gpio(32);
	gpio_pad_select_gpio(14);
	gpio_set_direction(32, GPIO_MODE_OUTPUT);
	gpio_set_direction(14, GPIO_MODE_OUTPUT);
	gpio_set_level(32, 1);
	gpio_set_level(14, 0);
	
	mcpwm_set_duty_type(MCPWM_UNIT_0, MCPWM_TIMER_0, MCPWM_OPR_A, MCPWM_DUTY_MODE_0);
	mcpwm_set_duty(MCPWM_UNIT_0, MCPWM_TIMER_0, MCPWM_OPR_A, 100);
		
	mcpwm_set_duty_type(MCPWM_UNIT_0, MCPWM_TIMER_0, MCPWM_OPR_B, MCPWM_DUTY_MODE_0);
	mcpwm_set_duty(MCPWM_UNIT_0, MCPWM_TIMER_0, MCPWM_OPR_B, 100);
	
	
	uart_param_config(UART_NUM_0, &uart_config);
	uart_set_pin(UART_NUM_0, ECHO_TEST_TXD, ECHO_TEST_RXD, ECHO_TEST_RTS, ECHO_TEST_CTS);
	uart_driver_install(UART_NUM_0, BUF_SIZE * 2, 0, 0, NULL, 0);
	uint8_t *data = (uint8_t *) malloc(BUF_SIZE);
	uint8_t header = 0x59;
	
	/*uart_param_config(UART_NUM_1, &uart_config_side);
	uart_set_pin(UART_NUM_1, ECHO_TEST_TXD_SIDE, ECHO_TEST_RXD_SIDE, ECHO_TEST_RTS_SIDE, ECHO_TEST_CTS_SIDE);
	uart_driver_install(UART_NUM_1, BUF_SIZE * 2, 0, 0, NULL, 0);
	uint8_t *data_side = (uint8_t *) malloc(BUF_SIZE);*/
	
	uart_param_config(UART_NUM_1, &uart_config_beacon);
	uart_set_pin(UART_NUM_1, ECHO_TEST_TXD_B, ECHO_TEST_RXD_B, ECHO_TEST_RTS_B, ECHO_TEST_CTS_B);
	uart_driver_install(UART_NUM_1, BUF_SIZE * 2, 0, 0, NULL, 0);
	uint8_t *data_b = (uint8_t *) malloc(BUF_SIZE);
	
	uart_set_line_inverse(UART_NUM_1, UART_INVERSE_RXD);
	
	bool stop_state = false;
	bool beacon_state = false;
	
	
	mcpwm_set_duty(MCPWM_UNIT_0, MCPWM_TIMER_0, MCPWM_OPR_A, 0);
	mcpwm_set_duty(MCPWM_UNIT_0, MCPWM_TIMER_0, MCPWM_OPR_B, 0);
	
	while (1) {
		int output = uart_read_bytes(UART_NUM_0, data, BUF_SIZE, 100);
    	bool loop = true;
    	int j = 0;
    	while(loop == true)
    	{
    		//printf("data: %02hhx\n", data[j]);
    		if (data[j] == header && (data[j+1]) == header)
    		{
    			lidar_result = ((uint16_t)(data[j+3]) << 8) | (data[j+2]);
    			loop = false;
    		}
    		else{
    			j++;
    		}
    	}
		
		int output_beacon = uart_read_bytes(UART_NUM_1, data_b, BUF_SIZE, 20 / portTICK_RATE_MS);
		if (output_beacon > 0)
		{
			//printf("beacon is %d\n", output_beacon);
			if (output_beacon == 100){
				for (int i = 0; i < 20; i++) {
					
					if (data_b[i] == start) {

						rxID = data_b[i + 1];
						//char rxFragment[10];
						for (int j = 0; j < 10; j++) {
							//rxFragment[j] = (char)data_b[i + j + 2];
							msg_send[j] = (char)data_b[i + j + 2];
						}
						//printf("Received from device ID 0x%02X fragment: %s\n", rxID, rxFragment);
						/*if (msg_send == msg_prev){
							printf("SAME! DONT SEND!\n");
							break;}
						else{
							for (int j = 0; j < 10; j++) {
							//rxFragment[j] = (char)data_b[i + j + 2];
							msg_send[j] = (char)data_b[i + j + 2];
							}*/
						send_state = true;
						break;
						}
					}
			}	
		}
		
		//forward
		if (command == 'f')
		{
			if (lidar_result > 20)
			{
				printf("UU\n");
			mcpwm_set_duty(MCPWM_UNIT_0, MCPWM_TIMER_0, MCPWM_OPR_A, 100);
			mcpwm_set_duty(MCPWM_UNIT_0, MCPWM_TIMER_0, MCPWM_OPR_B, 100);
			vTaskDelay(100);
			mcpwm_set_duty(MCPWM_UNIT_0, MCPWM_TIMER_0, MCPWM_OPR_A, 0);
			mcpwm_set_duty(MCPWM_UNIT_0, MCPWM_TIMER_0, MCPWM_OPR_B, 0);
			command = 'n';
			}
		}
		//backward
		else if (command == 'b')
		{
			gpio_set_level(25, 0);
			gpio_set_level(18, 1);
			gpio_set_level(32, 0);
			gpio_set_level(14, 1);
			mcpwm_set_duty(MCPWM_UNIT_0, MCPWM_TIMER_0, MCPWM_OPR_A, 100);
			mcpwm_set_duty(MCPWM_UNIT_0, MCPWM_TIMER_0, MCPWM_OPR_B, 100);
			vTaskDelay(100);
			mcpwm_set_duty(MCPWM_UNIT_0, MCPWM_TIMER_0, MCPWM_OPR_A, 0);
			mcpwm_set_duty(MCPWM_UNIT_0, MCPWM_TIMER_0, MCPWM_OPR_B, 0);
			command = 'n';
			gpio_set_level(25, 1);
			gpio_set_level(18, 0);
			gpio_set_level(32, 1);
			gpio_set_level(14, 0);
		}
		//left
		else if (command == 'l')
		{
			mcpwm_set_duty(MCPWM_UNIT_0, MCPWM_TIMER_0, MCPWM_OPR_A, 100);
			mcpwm_set_duty(MCPWM_UNIT_0, MCPWM_TIMER_0, MCPWM_OPR_B, 0);
			vTaskDelay(50);
			mcpwm_set_duty(MCPWM_UNIT_0, MCPWM_TIMER_0, MCPWM_OPR_A, 0);
			mcpwm_set_duty(MCPWM_UNIT_0, MCPWM_TIMER_0, MCPWM_OPR_B, 0);
			command = 'n';
		}
		//right
		else if (command == 'r')
		{
			mcpwm_set_duty(MCPWM_UNIT_0, MCPWM_TIMER_0, MCPWM_OPR_A, 0);
			mcpwm_set_duty(MCPWM_UNIT_0, MCPWM_TIMER_0, MCPWM_OPR_B, 100);
			vTaskDelay(50);
			mcpwm_set_duty(MCPWM_UNIT_0, MCPWM_TIMER_0, MCPWM_OPR_A, 0);
			mcpwm_set_duty(MCPWM_UNIT_0, MCPWM_TIMER_0, MCPWM_OPR_B, 0);
			command = 'n';
		}
		else if (command == 's')
		{
			gpio_pad_select_gpio(12);
			gpio_set_direction(12, GPIO_MODE_OUTPUT);
			gpio_set_level(12, 1);
			vTaskDelay(50);
			gpio_set_level(12, 0);
			vTaskDelay(50);
			gpio_set_level(12, 1);
			vTaskDelay(50);
			gpio_set_level(12, 0);
			vTaskDelay(50);
			gpio_set_level(12, 1);
			vTaskDelay(50);
			gpio_set_level(12, 0);
			vTaskDelay(50);
			gpio_set_level(12, 1);
			vTaskDelay(50);
			gpio_set_level(12, 0);
			vTaskDelay(50);
			gpio_set_level(12, 1);
			vTaskDelay(50);
			gpio_set_level(12, 0);
		}
	}
}

void app_main()
{
    printf("Testing brushed motor...\n");
	ESP_ERROR_CHECK( nvs_flash_init() );
    initialise_wifi();
    wait_for_ip();
	
	
	printf("STARTING...\n");
	
    xTaskCreate(mcpwm_example_brushed_motor_control, "mcpwm_examlpe_brushed_motor_control", 4096, NULL, 5, NULL);
	xTaskCreate(udp_server_task, "udp_server", 4096, NULL, 5, NULL);
	xTaskCreate(udp_client_task, "udp_client", 4096, NULL, 5, NULL);
}