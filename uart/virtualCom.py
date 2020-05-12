#!/usr/bin/env python
# coding=utf-8

'''
	Author: KevinOscar
    File: virtualCom.py
    Date: 2014-11-15
    Desc: 这是一个用Python写成的Linux下的虚拟终端，
		  可在本机模拟串口进行调试,
		  在命令行输入 python virtualCom.py &
		  即可创建两个虚拟终端，之后可用send,receive测试
'''

import pty
import os
import select

def mkpty():
	# 打开伪终端
	master1, slave = pty.openpty()
	slaveName1 = os.ttyname(slave)
	master2, slave = pty.openpty()
	slaveName2 = os.ttyname(slave)
	print '\nslave device names:',slaveName1,slaveName2
	return (master1,master2) # 以元组数据类型返回
	

if __name__ == "__main__":
	master1,master2 = mkpty()
	while True:
		rl,wl,el = select.select([master1,master2],[],[],1)
		for master in rl:
			data = os.read(master,128)
			print "virtualCom: read %d data." %len(data)
			if master == master1:
				os.write(master2,data)
			else:
				os.write(master1,data)
