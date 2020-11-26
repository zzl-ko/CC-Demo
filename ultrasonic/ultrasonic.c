// #include <linux/cdev.h>
// #include <linux/init.h>
// #include <linux/module.h>
// #include <linux/device.h>
// #include <linux/ktime.h>
// #include <linux/fs.h>
// #include <linux/wait.h>
// #include <linux/sched.h>
// #include <linux/workqueue.h>
// #include <linux/irq.h>
// #include <linux/gpio.h>
// #include <linux/of.h>
// #include <linux/of_address.h>

#include <linux/version.h>
#include <linux/of_gpio.h>
#include <linux/interrupt.h>
#include <linux/platform_device.h>
#include <linux/delay.h>
#include <linux/uaccess.h>
#include <linux/miscdevice.h>
#include <linux/kthread.h>
#include <linux/slab.h>
#if 0
// #include "ultrasonic.h"
#else
#include <linux/pwm.h>
#include <linux/time.h>
#include <linux/mutex.h>
#include <linux/types.h>

//#define ULTRASONIC_USE_PWM

struct ultrasonic_data {
    u32 trig_gpio;              // trig pin, control start and stop.
    u32 echo_gpio;              // echo pin
    u32 irq_flags;
    int echo_irq;               // irq num
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
    struct task_struct *ptask;
};
 
#define ULTRASONIC_CMD_START        _IOW(0, 1, int)
#define ULTRASONIC_CMD_STOP         _IOW(0, 2, int)
#define ULTRASONIC_CMD_SET_DUTY     _IOW(0, 3, int)
#define ULTRASONIC_CMD_GET_DUTY     _IOW(0, 4, int)
#define ULTRASONIC_CMD_GET_DIS      _IOW(0, 5, int)
#endif

#define US_STATUS_OPEN              0
#define US_STATUS_START             1

#define US_RANGING_INTERVAL         (500) // unit ms

static struct platform_device *us_dev;
static struct ultrasonic_data *us_dat;
 
static int ultrasonic_open(struct inode *inode, struct file *file)
{
    struct ultrasonic_data *pdata = us_dat; //file->private_data;

    printk(KERN_INFO "%s %s line %d\n", __FILE__, __FUNCTION__, __LINE__);

    if (test_and_set_bit(US_STATUS_OPEN, &pdata->status))
        return -EBUSY;
    return 0;
}
 
static int ultrasonic_release(struct inode *inode, struct file *file)
{
    struct ultrasonic_data *pdata = us_dat; //file->private_data;

    printk(KERN_INFO "%s %s line %d\n", __FILE__, __FUNCTION__, __LINE__);

    clear_bit(US_STATUS_OPEN, &pdata->status);
    return 0;
}
 
static long ultrasonic_ioctl(struct file *file, unsigned int cmd, unsigned long arg)
{
    void __user *argp = (void __user *) arg;
    struct ultrasonic_data *pdata = us_dat; //file->private_data;

    printk(KERN_INFO "%s %s line %d\n", __FILE__, __FUNCTION__, __LINE__);
 
    switch (cmd) {
    case ULTRASONIC_CMD_START:
        if (!pdata->start) {
            do_gettimeofday(&pdata->l_time);
#ifdef ULTRASONIC_USE_PWM
            pwm_enable(pdata->pwm);
#endif
            pdata->start = 1;
        }
        break;
    case ULTRASONIC_CMD_STOP:
        if (pdata->start) {
            //disable_irq(pdata->echo_irq);
#ifdef ULTRASONIC_USE_PWM
            pwm_disable(pdata->pwm);
#endif
            pdata->start = 0;
        }
        break;
#ifdef ULTRASONIC_USE_PWM
    case ULTRASONIC_CMD_SET_DUTY:
        if (pdata->start)
            return -EFAULT;
        if(copy_from_user(&pdata->pwm_duty_ns, argp, sizeof(pdata->pwm_duty_ns)))
            return -EFAULT;
        pwm_config(pdata->pwm, pdata->pwm_duty_ns, pdata->pwm_period_ns);
        break;
    case ULTRASONIC_CMD_GET_DUTY:
        if(copy_to_user(argp, &pdata->pwm_duty_ns, sizeof(pdata->pwm_duty_ns)))
            return -EFAULT;
        break;
#endif
    case ULTRASONIC_CMD_GET_DIS:
        mutex_lock(&pdata->u_mutex);
        if(copy_to_user(argp, &pdata->dis_mm, sizeof(pdata->dis_mm))) {
            mutex_unlock(&pdata->u_mutex);
            return -EFAULT;
        }
        mutex_unlock(&pdata->u_mutex);
        break;
    default:
        printk("Unknown command!\n");
        return -EINVAL;
    }
 
    return 0;
}

static irqreturn_t ultrasonic_irq_handler(int irq, void *data)
{
    struct ultrasonic_data *pdata = data;

    // printk(KERN_INFO "%s %s line %d\n", __FILE__, __FUNCTION__, __LINE__);
 
    //Don't add printk(), or it will increase the time of the handler!!!
    if (test_bit(US_STATUS_START, &pdata->status)) {
        if (atomic_read(&pdata->count) == 0) {
            //Rising edge, record the start time.
            do_gettimeofday(&pdata->l_time);
            atomic_inc(&pdata->count);
        } else if (atomic_read(&pdata->count) == 1) {
            //Falling edge, record the stop time.
            do_gettimeofday(&pdata->c_time);
            //Following two filter other unuseful interrupts.
            atomic_inc(&pdata->count);
            clear_bit(US_STATUS_START, &pdata->status);
        }
    }
 
    return IRQ_HANDLED;
}

static int ultrasonic_thread(void *arg)
{
    struct ultrasonic_data *pdata = arg;
    long interval;

    printk(KERN_INFO "%s %s line %d: start...\n", __FILE__, __FUNCTION__, __LINE__);
 
    //enable interrupt before start test.
    enable_irq(pdata->echo_irq);
 
    while(!kthread_should_stop()) {
        //dev_dbg(&us_dev->dev, "test start!\n");
        atomic_set(&pdata->count, 0);
        set_bit(US_STATUS_START, &pdata->status);
 
        /*Following is the timing of starting ultrasonic.*/
        //Low active.
        gpio_set_value(pdata->trig_gpio, 0);
        //follow the spec, at least 10us.
        udelay(30);
        gpio_set_value(pdata->trig_gpio, 1);
        //Control test peroid.
        msleep(US_RANGING_INTERVAL);
        
        /*Calculate distance from time interval.*/
        //Max is 1000000
        if (pdata->c_time.tv_usec < pdata->l_time.tv_usec) 
            interval = 1000000 - pdata->l_time.tv_usec + pdata->c_time.tv_usec;
        else 
            interval = pdata->c_time.tv_usec - pdata->l_time.tv_usec;

        mutex_lock(&pdata->u_mutex);
        pdata->dis_mm = (170 * interval)/1000;
        mutex_unlock(&pdata->u_mutex);

        dev_dbg(&us_dev->dev, "c:%ld l:%ld, distance is :%ld mm\n", pdata->c_time.tv_usec, pdata->l_time.tv_usec, pdata->dis_mm);
        printk("c:%ld l:%ld, distance is :%ld mm\n", pdata->c_time.tv_usec, pdata->l_time.tv_usec, pdata->dis_mm);
    }

    printk(KERN_INFO "%s %s line %d: end\n", __FILE__, __FUNCTION__, __LINE__);
 
    return 0;
}
 
static const struct file_operations ultrasonic_fops = {
    .owner = THIS_MODULE,
    .open = ultrasonic_open,
    .release = ultrasonic_release,
    .unlocked_ioctl = ultrasonic_ioctl,
};

static struct miscdevice ultrasonic_dev = {
    .minor = MISC_DYNAMIC_MINOR,
    .name = "Ultrasonic",
    .fops = &ultrasonic_fops,
};

static int ultrasonic_probe(struct platform_device *pdev)
{
    int ret;
    struct file  *filp            = NULL;
	struct inode *inode           = NULL;
    struct device_node     *pnode = pdev->dev.of_node;
    struct ultrasonic_data *pdata = kzalloc(sizeof(struct ultrasonic_data), GFP_KERNEL);

    printk(KERN_INFO "%s %s line %d: start...\n", __FILE__, __FUNCTION__, __LINE__);
    
    if (pdata == NULL) {
        dev_err(&pdev->dev, "missing platform data\n");
        return -ENODEV;
    }
    platform_set_drvdata(pdev, pdata);

    //protect data to be read in user space.
    mutex_init(&pdata->u_mutex);

    pdata->status = 0;
    pdata->start  = 0;
    us_dat = pdata;
    us_dev = pdev;
 
#ifdef ULTRASONIC_USE_PWM
    pdata->pwm = pwm_request(pdata->pwm_id, "ultrasonic-pwm");
    if (IS_ERR(pdata->pwm)) {
        dev_err(&pdev->dev, "unable to request PWM for ultrasonic\n");
        ret = PTR_ERR(pdata->pwm);
        goto error0;
    } else
        dev_dbg(&pdev->dev, "got pwm for ultrasonic\n");
    //default config.
    pwm_config(pdata->pwm, pdata->pwm_duty_ns, pdata->pwm_period_ns);
    pwm_enable(pdata->pwm);
#endif
 
    //gpio init, control it to start and stop.
    pdata->trig_gpio = of_get_named_gpio(pnode, "firefly-trig-gpio", 0);
    if (!gpio_is_valid(pdata->trig_gpio))
    {
        dev_err(&pdev->dev, "trig_gpio [%d] is invalid\n", pdata->trig_gpio);
        goto error1;
    }
    ret = gpio_request(pdata->trig_gpio, "ultrasonic-trig");
    if (ret < 0) {
        dev_err(&pdev->dev, "request ultrasnoic-trig failed: %d\n", ret);
        goto error1;
    }
    gpio_direction_output(pdata->trig_gpio, 1);
    gpio_set_value(pdata->trig_gpio, 1);

    pdata->echo_gpio = of_get_named_gpio_flags(pnode, "firefly-echo-gpio", 0, &pdata->irq_flags);
    if (!gpio_is_valid(pdata->echo_gpio))
    {
        dev_err(&pdev->dev, "trig_gpio [%d] is invalid\n", pdata->trig_gpio);
        goto error2;
    }
    ret = gpio_request(pdata->echo_gpio, "ultrasonic-echo");
    if (ret < 0) {
        dev_err(&pdev->dev, "request ultrasnoic-echo failed: %d\n", ret);
        goto error2;
    }

    //request irq.
    pdata->echo_irq = gpio_to_irq(pdata->echo_gpio);
    // irq_flags = IRQF_TRIGGER_RISING|IRQF_TRIGGER_FALLING;
    // ret = devm_request_threaded_irq(&pdev->dev, pdata->echo_irq, ultrasonic_irq_handler, NULL, irq_flags, "ultrasonic-irq", pdata);
    ret = request_threaded_irq(pdata->echo_irq, ultrasonic_irq_handler, NULL, pdata->irq_flags, "ultrasonic-irq", pdata);
    if (ret < 0) {
        dev_err(&pdev->dev, "request ultrasonic-irq failed: %d\n", ret);
        goto error3;
    }
    //enable later.
    disable_irq(pdata->echo_irq);

    printk(KERN_INFO "%s: trig=%u, echo=%u, irq=%d, irq_flags=%u\n", __func__, pdata->trig_gpio, pdata->echo_gpio, pdata->echo_irq, pdata->irq_flags);

    //core thread to run and caculate.
    pdata->ptask = kthread_run(ultrasonic_thread, pdata, "us-kthread");
    if (IS_ERR(pdata->ptask)) {
        ret = PTR_ERR(pdata->ptask);
        dev_err(&pdev->dev, "create ultrasnoic core thread failed: %d\n", ret);
        goto error4;
    }

    //Used in user space.
    ret = misc_register(&ultrasonic_dev);
    if (ret) {
        dev_err(&pdev->dev, "ultrasonic_dev register failed\n");
        goto error5;
    }

    filp = filp_open("/dev/Ultrasonic", O_RDONLY|O_CREAT, 0);
    if(IS_ERR(filp)){
        dev_err(&pdev->dev, "Device file open error,process exit!\n");
        goto error5;
    }
    // https://blog.csdn.net/hn2zzzz1996/article/details/79496282?utm_source=blogxgwz4
#if LINUX_VERSION_CODE < KERNEL_VERSION(3,19,0)
    inode = filp->f_dentry->d_inode;
#else
    inode = file_inode(filp);
#endif
    inode->i_mode |= 0777;
    filp_close(filp, NULL);
  
    printk(KERN_INFO "%s %s line %d: finish!\n", __FILE__, __FUNCTION__, __LINE__);
    return 0;

error5:
    kthread_stop(pdata->ptask);
    mdelay(US_RANGING_INTERVAL * 2); // delay wait for kthread exit
error4:
    free_irq(pdata->echo_irq, pdata);
error3:
    gpio_free(pdata->echo_gpio);
error2:
    gpio_free(pdata->trig_gpio);
error1:
#ifdef ULTRASONIC_USE_PWM
error0:
    pwm_free(pdata->pwm);
#endif
    kfree(pdata);

    return ret;
}

static int ultrasonic_remove(struct platform_device *pdev)
{
    struct ultrasonic_data *pdata = platform_get_drvdata(pdev);

    if (NULL != pdata) {
        kthread_stop(pdata->ptask);
        mdelay(US_RANGING_INTERVAL * 2); // delay wait for kthread exit
        free_irq(pdata->echo_irq, pdata);
        gpio_free(pdata->echo_gpio);
        gpio_free(pdata->trig_gpio);
#ifdef ULTRASONIC_USE_PWM
        pwm_free(pdata->pwm);
#endif
        kfree(pdata);
    }
    misc_deregister(&ultrasonic_dev);
    printk(KERN_INFO "%s %s line %d: end\n", __FILE__, __FUNCTION__, __LINE__);

    return 0;
}

#ifdef CONFIG_OF
static const struct of_device_id of_ultrasonic_match[] = {
        { .compatible = "firefly,rk3399-ultrasonic" },
        { /* Sentinel */ }
};
#endif

static struct platform_driver ultrasonic_driver = {
    .probe              = ultrasonic_probe,
    .remove             = ultrasonic_remove,
    .driver             = {
        .name           = "ultrasonic",
        .owner          = THIS_MODULE,
        .of_match_table = of_ultrasonic_match,
    },
};

static int __init ultrasonic_init(void)
{
    printk(KERN_INFO "%s %s line %d\n", __FILE__, __FUNCTION__, __LINE__);
    return platform_driver_register(&ultrasonic_driver);
}
module_init(ultrasonic_init);

static void __exit ultrasonic_exit(void)
{
    printk(KERN_INFO "%s %s line %d\n", __FILE__, __FUNCTION__, __LINE__);
    platform_driver_unregister(&ultrasonic_driver);
}
module_exit(ultrasonic_exit);


MODULE_LICENSE("GPL");
MODULE_AUTHOR("Kevin Zhou");
MODULE_DESCRIPTION("Ultrasonic driver");

/**
参考如下: 
[[IMX6DL][Android4.4]超声波模块HC-SR04 Linux驱动源代码](https://blog.csdn.net/kris_fei/article/details/51161278)
[33 全志GPIO口的脚本配置及超声波测距模块的linux驱动](https://blog.csdn.net/jklinux/article/details/73457774)
[45 超声波测距模块的linux platform驱动模型实现](https://blog.csdn.net/jklinux/article/details/73742113)\
[AIO-3399C -- GPIO](http://wiki.t-firefly.com/AIO-3399C/driver_gpio.html)
[RK3399—中断](https://blog.csdn.net/qq_20553613/article/details/103970096)
[RK3399—pinctrl和gpio子系统](https://blog.csdn.net/qq_20553613/article/details/103931924)
[[Android] [RK3399] -- GPIO驱动与控制方式](https://blog.csdn.net/u014674293/article/details/93109720)
[[Android] [RK3399] -- 脉冲信号检测](https://blog.csdn.net/u014674293/article/details/105817230)

[解决驱动创建设备节点时权限为root只读只写的问题](https://blog.csdn.net/zq979999/article/details/48350097)
[设备节点创建以及驱动改权限](https://blog.csdn.net/u011850668/article/details/45363843)
*/