/*
 * This file is part of GreatFET
 */

#include "usb_api_rfhax.h"

#include "usb.h"
#include "usb_queue.h"
#include "usb_endpoint.h"
#include <greatfet_core.h>
#include <clocks.h>
#include "pins.h"

#define RFHAX_OFF 0
#define RFHAX_CW 1
#define RFHAX_ASK 2
#define RFHAX_FSK 3

volatile bool rfhax_enabled = true;

static uint8_t rfhax_mode = RFHAX_FSK;
uint16_t f1;
uint16_t f2;

usb_request_status_t usb_vendor_request_rfhax(
	usb_endpoint_t* const endpoint, const usb_transfer_stage_t stage)
{
	rfhax_mode = endpoint->setup.index;
	if (stage == USB_TRANSFER_STAGE_SETUP) {
		f1 = endpoint->setup.value;
		pll0audio_config(f1);
		switch(rfhax_mode) {
			case RFHAX_OFF:
				rfhax_enabled = false;
				pll0audio_off();
				usb_transfer_schedule_ack(endpoint->in);
				break;
			case RFHAX_CW:
				rfhax_enabled = false;
				pll0audio_on();
				usb_transfer_schedule_ack(endpoint->in);
				break;
			case RFHAX_ASK:
				rfhax_enabled = true;
				usb_transfer_schedule_ack(endpoint->in);
				break;
			case RFHAX_FSK:
				rfhax_enabled = true;
				f2 = 4319;
				// usb_transfer_schedule_block(endpoint->out, &f2, 2, NULL, NULL);
				usb_transfer_schedule_ack(endpoint->in);
				break;
		}
	} else if (stage == USB_TRANSFER_STAGE_DATA) {
		usb_transfer_schedule_ack(endpoint->in);
	}
	return USB_REQUEST_STATUS_OK;
}


void rfhax(void)
{
	f1 = 4320;
	f2 = 4330;
	bool phase = false;
	pll0audio_config(f1);
	pll0usb_config(f2);
	while(rfhax_enabled) {
		led_toggle(LED3);
		// pll0audio_on();
		if(phase) {
			// pll0audio_on();
			// pll0usb_on();
			switch_clocks(0);
			phase = false;
		} else {
			// pll0audio_off();
			// pll0usb_off();
			switch_clocks(1);
			phase = true;
		}
		delay(10000000);
	}
}
