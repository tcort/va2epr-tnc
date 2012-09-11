/*
 * This file provides Transmit Buffer using an external RAM chip (23LC1024-I/SN)
 *
 * The host PC will be sending us data at 9600 baud via USART, but we can
 * only send AFSK encoded data at 1200 baud. Additionally, the computer could
 * send us data while another station is transmitting. Therefore, we need
 * to buffer outgoing data. The goal is to flush this buffer (i.e. empty
 * it by sending data to the radio every time there is a complete frame
 * in it AND there are no other stations sending.
 *
 * Ports and Peripherals Used
 * --------------------------
 *
 * SPI
 *
 * PB4 $SS$
 * PB5 MOSI
 * PB6 MISO
 * PB7 SCK
 */

void sram_init(void) {

}
