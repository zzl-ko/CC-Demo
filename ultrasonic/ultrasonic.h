#ifndef __ULTRASONIC_H__
#define __ULTRASONIC_H__
 
#include <linux/pwm.h>
#include <linux/time.h>
#include <linux/mutex.h>
#include <linux/types.h>
#include <linux/cdev.h>

//#define ULTRASONIC_USE_PWM
 
struct ultrasonic_data {
	u32 trig_gpio;              // trig pin, control start and stop.
	u32 echo_gpio;	            // echo pin
	u32 irq_flags;
	int echo_irq;	            // irq num
#ifdef ULTRASONIC_USE_PWM
	struct pwm_device *pwm;
	u32    pwm_period_ns;
	u32    pwm_duty_ns;
	u8     pwm_id;
#endif
	u8                  start;
	atomic_t            count;
	long               dis_mm;
	unsigned long      status;
	struct timeval     l_time;  // last time
	struct timeval     c_time;  // current time
	struct mutex      u_mutex;
    dev_t               devno;
    struct cdev         usdev;
    struct class       *uscls;
    struct task_struct *ptask;
};
 
#define ULTRASONIC_CMD_START		_IOW(0, 1, int)
#define ULTRASONIC_CMD_STOP			_IOW(0, 2, int)
#define ULTRASONIC_CMD_SET_DUTY		_IOW(0, 3, int)
#define ULTRASONIC_CMD_GET_DUTY		_IOW(0, 4, int)
#define ULTRASONIC_CMD_GET_DIS		_IOW(0, 5, int)
 
#endif