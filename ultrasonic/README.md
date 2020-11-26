1. 将 rk3399-ultrasonic.dtsi 复制到 kernel/arch/arm64/boot/dts/rockchip 目录下
2. 在 kernel/arch/arm64/boot/dts/rockchip/firefly-rk3399-aioc.dts 文件中包含 rk3399-ultrasonic.dtsi, 或者直接将其第一个括号中内容复制到 firefly-rk3399-aioc.dts
3. 重新编译kernel并烧录到 AIO-3399C
4. 执行 make 编译本工程中的驱动
5. 将编译生成的 ultrasonic.ko ultrasonic_test 拷贝到 AIO-3399C
6. sudo insmod ultrasonic.ko; sudo chmod 0666 /dev/Ultrasonic; ./ultrasonic_test
7. 测试完后执行 sudo rmmod ultrasonic
8. 调试过程中可以通过 tail -f /var/log/kern.log 查看驱动中日志打印