文件说明:
virtualCom.py -- 创建虚拟终端(可用于模拟串口)
gplSerial.py  -- Python实现的一个串口操作API
uart.c        -- C实现的unix下的串口操作API
Makefile      -- 用于编译uart.c


使用说明：
1.先用如下命令后台运行虚拟串口
python virtualCom.py &
终端将打印的类似如下信息，
slave device names: /dev/pts/7 /dev/pts/9

2.执行make, 生成rx_uart, tx_uart

3.新打开一个终端执行: ./rx_uart /dev/pts/7

5.打开另一个终端执行: ./tx_uart /dev/pts/8