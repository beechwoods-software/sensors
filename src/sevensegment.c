/*
 * Copyright Beechwoods Software, Inc. 2023 brad@beechwoods.com
 * All Rights Reserved
 */

#ifdef CONFIG_USE_SEVEN_SEGMENT_DISPLAY

#include <zephyr/kernel.h>
#include <zephyr/device.h>
#
/**
 * @file sevensegment.c
 * @brief Seven-segment display implementation
 *
 * Implements the routines declared in `sevensegment.h` for controlling
 * a seven-segment display module.
 */
#include <zephyr/drivers/led.h>
#include <zephyr/drivers/gpio.h>
#include <stdio.h>
#include <string.h>

#include <zephyr/logging/log.h>
#include "sensors_logging.h"
LOG_MODULE_DECLARE( SENSORS_LOG_MODULE_NAME, CONFIG_SENSORS_LOG_LEVEL );

#define SEGMENT_A 0x01
#define SEGMENT_B 0x02
#define SEGMENT_C 0x04
#define SEGMENT_D 0x08
#define SEGMENT_E 0x10
#define SEGMENT_F 0x20
#define SEGMENT_G 0x40


#define SEG_OFF 0
#define SEG_ON 1


/*
 * The seven segment display is defined as follows
 *      1
 *     ____
 *    |    |
 *  32|    |2
 *    | 64 |
 *     ____
 *    |    |
 *  16|    | 4
 *    | 8  |
 *     ____
 */
uint8_t numeric_segment_table[] = {
  0x3F,  //0
  0x06,  //1
  0x5B,  //2
  0x4F,  //3
  0x66,  //4

  0x6D,  //5
  0x7C,  //6
  0x07,  //7
  0x7F,  //8
  0x67,  //9
  0x77,  //A (10)
  0x7C,  //b (11)
  0x39,  //C (12)
  0x5E,  //d (13)
  0x79,  //E (14)
  0x71   //F (15)
};

typedef struct char_map {
  char alpha;
  uint8_t map;
}char_map_t;

#define NUM_ALPHA 9
char_map_t alpha_segment_table [] = {
  { 'b', 0x7C },
  { 'u', 0x1C },
  { 't', 0x78 },
  { 'i', 0x30 },
  { 'm', 0x55 },
  { 'c', 0x58 },
  { 'y', 0x6E },
  { 'd', 0x5E },
  { 'r', 0x51 }
};

#define NUM_ELEMENTS 4
char elements[NUM_ELEMENTS] = {0};

int sevensegment_set_text(char * text)
{
  memset(elements, '\0', NUM_ELEMENTS);
  strncpy(elements, text, NUM_ELEMENTS);
  for(int i = 0; i< 4; i++) {
    if(elements[i] < 16) {
      LOG_DBG("7S set to %d",elements[i]);
    } else {
      LOG_DBG("7S set to %c",elements[i]);
    }
  }
  return 0;
}  
static int _7s_cur_element = 0;

int sevensegment_set_element(int element)
{
  if((element < 0) || (element >= NUM_ELEMENTS)) {
    LOG_ERR("Bad element %d", element);
    return -1;
  }
  _7s_cur_element = element;
  return 0;
}

int sevensegment_set_internal(int val);

static const struct gpio_dt_spec segment_a = GPIO_DT_SPEC_GET(DT_NODELABEL(segment_a),gpios);
static const struct gpio_dt_spec segment_b = GPIO_DT_SPEC_GET(DT_NODELABEL(segment_b),gpios);
static const struct gpio_dt_spec segment_c = GPIO_DT_SPEC_GET(DT_NODELABEL(segment_c),gpios);
static const struct gpio_dt_spec segment_d = GPIO_DT_SPEC_GET(DT_NODELABEL(segment_d),gpios);
static const struct gpio_dt_spec segment_e = GPIO_DT_SPEC_GET(DT_NODELABEL(segment_e),gpios);
static const struct gpio_dt_spec segment_f = GPIO_DT_SPEC_GET(DT_NODELABEL(segment_f),gpios);
static const struct gpio_dt_spec segment_g = GPIO_DT_SPEC_GET(DT_NODELABEL(segment_g),gpios);
#ifdef CONFIG_USE_FOUR_ELEMENT_SEVEN_SEGMENT_DISPLAY
static const struct gpio_dt_spec cell_select_1 = GPIO_DT_SPEC_GET(DT_NODELABEL(cell_select_1),gpios);
static const struct gpio_dt_spec cell_select_2 = GPIO_DT_SPEC_GET(DT_NODELABEL(cell_select_2),gpios);
static const struct gpio_dt_spec cell_select_3 = GPIO_DT_SPEC_GET(DT_NODELABEL(cell_select_3),gpios);
static const struct gpio_dt_spec cell_select_4 = GPIO_DT_SPEC_GET(DT_NODELABEL(cell_select_4),gpios);


static const struct gpio_dt_spec * cell_selects[4] = {
  &cell_select_1,
  &cell_select_2,
  &cell_select_3,
  &cell_select_4
};
#endif

void
sevensegment_post()
{
  LOG_INF("sevensegment post");
  if(gpio_pin_set_dt(&segment_a, 1) <0) {
    LOG_ERR("segment_a failed");
  }
  if(gpio_pin_set_dt(&segment_b, 1) <0) {
    LOG_ERR("segment_b failed");
  }
  if(gpio_pin_set_dt(&segment_c, 1) <0) {
    LOG_ERR("segment_c failed");
  }
  if(gpio_pin_set_dt(&segment_d, 1) <0) {
    LOG_ERR("segment_d failed");
  }
  if(gpio_pin_set_dt(&segment_e, 1) <0) {
    LOG_ERR("segment_e failed");
  }
  if(gpio_pin_set_dt(&segment_f, 1) <0) {
    LOG_ERR("segment_f failed");
  }
  if(gpio_pin_set_dt(&segment_g, 1) <0) {
    LOG_ERR("segment_g failed");
  }
  if(gpio_pin_set_dt(&segment_a, 0) <0) {
    LOG_ERR("segment_a failed");
  }
  if(gpio_pin_set_dt(&segment_b, 0) <0) {
    LOG_ERR("segment_b failed");
  }
  if(gpio_pin_set_dt(&segment_c, 0) <0) {
    LOG_ERR("segment_c failed");
  }
  if(gpio_pin_set_dt(&segment_d, 0) <0) {
    LOG_ERR("segment_d failed");
  }
  if(gpio_pin_set_dt(&segment_e, 0) <0) {
    LOG_ERR("segment_e failed");
  }
  if(gpio_pin_set_dt(&segment_f, 0) <0) {
    LOG_ERR("segment_f failed");
  }
  if(gpio_pin_set_dt(&segment_g, 0) <0) {
    LOG_ERR("segment_g failed");
  }

}

int
sevensegment_init()
{
  int rc = 0;
#ifdef CONFIG_USE_FOUR_ELEMENT_SEVEN_SEGMENT_DISPLAY
  LOG_INF("configure four element seven segment display\n");
  if(!device_is_ready(cell_select_1.port)) {
    rc = -1;
  }
  if(!device_is_ready(cell_select_2.port)) {
    rc = -1;
  }
  if(!device_is_ready(cell_select_3.port)) {
    rc = -1;
  }
  if(!device_is_ready(cell_select_4.port)) {
    rc = -1;
  }

#endif
  LOG_INF("Configure seven segment display\n");
  if(!device_is_ready(segment_a.port)) {
    rc = -1;
  }
  if(!device_is_ready(segment_b.port)) {
    rc = -1;
  }
  if(!device_is_ready(segment_c.port)) {
    rc = -1;
  }
  if(!device_is_ready(segment_d.port)) {
    rc = -1;
  }
  if(!device_is_ready(segment_e.port)) {
    rc = -1;
  }
  if(!device_is_ready(segment_f.port)) {
    rc = -1;
  }
  if(!device_is_ready(segment_g.port)) {
    rc = -1;
  }
  if(rc <0 ){
    return rc;
  }
#ifdef CONFIG_USE_FOUR_ELEMENT_SEVEN_SEGMENT_DISPLAY
  
  if(gpio_pin_configure_dt(&cell_select_1,GPIO_OUTPUT_INACTIVE) < 0) {
    rc = -1;
  }
  if(gpio_pin_configure_dt(&cell_select_2,GPIO_OUTPUT_INACTIVE) < 0) {
    rc = -1;
  }
  if(gpio_pin_configure_dt(&cell_select_3,GPIO_OUTPUT_INACTIVE) < 0) {
    rc = -1;
  }
  if(gpio_pin_configure_dt(&cell_select_4,GPIO_OUTPUT_INACTIVE) < 0) {
    rc = -1;
  }
#endif
  if(gpio_pin_configure_dt(&segment_a,GPIO_OUTPUT_ACTIVE) < 0) {
    rc = -1;
  }
  if(gpio_pin_configure_dt(&segment_b,GPIO_OUTPUT_ACTIVE) < 0) {
    rc = -1;
  }
  if(gpio_pin_configure_dt(&segment_c,GPIO_OUTPUT_ACTIVE) < 0) {
    rc = -1;
  }
  if(gpio_pin_configure_dt(&segment_d,GPIO_OUTPUT_ACTIVE) < 0) {
    rc = -1;
  }
  if(gpio_pin_configure_dt(&segment_e,GPIO_OUTPUT_ACTIVE) < 0) {
    rc = -1;
  }
  if(gpio_pin_configure_dt(&segment_f,GPIO_OUTPUT_ACTIVE) < 0) {
    rc = -1;
  }
  if(gpio_pin_configure_dt(&segment_g,GPIO_OUTPUT_ACTIVE) < 0) {
    rc = -1;
  }
  LOG_DBG("svn OK");
  return rc;
  
}

int
sevensegment_set_segment(int segment, bool on)
{
  int rc;
  uint8_t val;
  const struct gpio_dt_spec * pdt;
  switch(segment) {
  case 0:
  case '0':
  case 'a':
  case 'A':
    pdt= &segment_a;
    val = SEGMENT_A;
    break;
  case 1:
  case '1':
  case 'b':
  case 'B':
    pdt = &segment_b;
    val = SEGMENT_B;
    break;
  case 2:
  case '2':
  case 'c':
  case 'C':
    pdt = &segment_c;
    val = SEGMENT_C;
    break;

  default:
    LOG_ERR("Bad segment descriptor 0x%x", segment);
    return -1;
  }
  rc = gpio_pin_set_dt(pdt, val);
  return rc;
}
     
int
sevensegment_set_int(int number)
{
  uint8_t val;
  if(number > 0x0F) {
    return -1;
  }
  val = numeric_segment_table[number];
  elements[_7s_cur_element] = number;
  LOG_DBG("svn %d val %x",number,val);
  return(sevensegment_set_internal(val));
}
int sevensegment_set_internal(int val)
{
  int rc = 0;
  
  rc  = gpio_pin_set_dt(&segment_a, val & SEGMENT_A);

  rc |= gpio_pin_set_dt(&segment_b, val & SEGMENT_B);
  
  rc |= gpio_pin_set_dt(&segment_c, val & SEGMENT_C);
  
  rc |= gpio_pin_set_dt(&segment_d, val & SEGMENT_D);
  
  rc |= gpio_pin_set_dt(&segment_e, val & SEGMENT_E);
  
  rc |= gpio_pin_set_dt(&segment_f, val & SEGMENT_F);
  
  rc |= gpio_pin_set_dt(&segment_g, val & SEGMENT_G);

  return rc;
}

#ifdef CONFIG_USE_FOUR_ELEMENT_SEVEN_SEGMENT_DISPLAY
/* size of stack area used by each thread */
#define STACKSIZE 1024
/* scheduling priority used by each thread */
#define PRIORITY 7
int sevensegment_get_alpha(char alpha)
{
  int i;
  int val = 0;
  for(i = 0; i < NUM_ALPHA; i++) {
    if(alpha == alpha_segment_table[i].alpha) {
      val = alpha_segment_table[i].map;
      break;
    }
  }
  return val;
}

#define ELEMENT_SLEEP 1000
  
void scan(void)
{
  int i;
  int val;
  while(1) {
    for( i = 0; i < 4;i++)  {
      
      if (elements[i] < 16) {
        val = numeric_segment_table[(int)elements[i]];
      } else {
        val = sevensegment_get_alpha(elements[i]);
      }
      //      LOG_DBG("7s %d  0x%x 0x%x", i, elements[i], val);
      if(sevensegment_set_internal(val)) {
        LOG_ERR("seting 7s disp failed");
      }
      gpio_pin_set_dt(cell_selects[i], SEG_ON);
      
      k_msleep(ELEMENT_SLEEP);
      gpio_pin_set_dt(cell_selects[i], SEG_OFF);
      //      k_msleep(ELEMENT_SLEEP);
    }
  }
}
K_THREAD_DEFINE(scan_id, STACKSIZE, scan, NULL, NULL, NULL,
		PRIORITY, 0, 0);

#endif //CONFIG_USE_FOUR_ELEMENT_SEVEN_SEGMENT_DISPLAY
  
#endif // CONFIG_USE_SEVEN_SEGMENT_DISPLAY 
 
