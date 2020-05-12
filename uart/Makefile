all: rx tx
	@echo "everything is ok!"

rx:
	gcc uart.c -Wall -lpthread -DUART_RD_WR_FLAG=0 -o rx_uart

tx:
	gcc uart.c -Wall -lpthread -DUART_RD_WR_FLAG=1 -o tx_uart

clean:
	rm -f rx_uart tx_uart
