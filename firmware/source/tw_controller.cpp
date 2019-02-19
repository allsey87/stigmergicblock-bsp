#include "tw_controller.h"

#include <avr/io.h>
#include <util/twi.h>

#define TW_CLOCK  400000L

/****************************************/
/****************************************/

CTWController::CTWController() {
   TWSR = 0;
   TWBR = ((F_CPU / TW_CLOCK) - 16) / 2;
}

/****************************************/
/****************************************/

void CTWController::Enable() {
   TWCR = (1 << TWEN);
}

/****************************************/
/****************************************/

void CTWController::Disable() {
   TWCR = 0;
}

/****************************************/
/****************************************/

bool CTWController::Start(uint8_t un_device, EMode e_mode) {
   /* send START condition */
   TWCR = (1 << TWINT) | (1 << TWSTA) | (1 << TWEN);
   /* wait until transmission completed */
   Wait();
   /* check value of TWI Status Register */
   uint8_t unStatus = TW_STATUS;
   if ( (unStatus != TW_START) && (unStatus != TW_REP_START)) return false;
   /* send device address */
   TWDR = (un_device << 1) | static_cast<uint8_t>(e_mode);
   TWCR = (1 << TWINT) | (1 << TWEN);
   /* wait until transmission completed and ACK/NACK has been received */
   Wait();
   /* check value of TWI Status Register */
   unStatus = TW_STATUS;
   return ((unStatus == TW_MT_SLA_ACK) || (unStatus == TW_MR_SLA_ACK));
}

/****************************************/
/****************************************/

void CTWController::StartWait(uint8_t un_device, EMode e_mode) {
   for(;;) {
      /* send START condition */
      TWCR = (1 << TWINT) | (1 << TWSTA) | (1 << TWEN);
      /* wait until transmission completed */
      Wait();
      /* check value of TWI Status Register */
      uint8_t unStatus = TW_STATUS;
      if ( (unStatus != TW_START) && (unStatus != TW_REP_START)) continue;
      /* send device address */
      TWDR = (un_device << 1) | static_cast<uint8_t>(e_mode);
      TWCR = (1 << TWINT) | (1 << TWEN);
      /* wail until transmission completed */
      Wait();
      /* check value of TWI Status Register */
      unStatus = TW_STATUS;
      if((unStatus == TW_MT_SLA_NACK) || (unStatus == TW_MR_DATA_NACK)) {    	    
         /* device busy, send stop condition to terminate write operation */
         TWCR = (1 << TWINT) | (1 << TWEN) | (1 << TWSTO);
         /* wait until stop condition is executed and bus released */
         while(TWCR & (1 << TWSTO));
         continue;
      }
      else break;
   }
}

/****************************************/
/****************************************/

void CTWController::Stop() {
   /* send stop condition */
   TWCR = (1 << TWINT) | (1 << TWEN) | (1 << TWSTO);
   /* wait until stop condition is executed and bus released */
   while(TWCR & (1 << TWSTO));
}

/****************************************/
/****************************************/

bool CTWController::Transmit(uint8_t un_data) {
   /* send data to the previously addressed device */
   TWDR = un_data;
   TWCR = (1 << TWINT) | (1 << TWEN);
   /* wait until transmission completed */
   Wait();
   return (TW_STATUS == TW_MT_DATA_ACK);
}

/****************************************/
/****************************************/

uint8_t CTWController::Receive(bool b_continue) {
   TWCR = (1 << TWINT) |
          (1 << TWEN)  |  
          (b_continue ? (1 << TWEA) : 0);
   /* wait until done */
   Wait();
   /* return received byte */
   return TWDR;
}

/****************************************/
/****************************************/

void CTWController::Wait() {
   while(!(TWCR & (1 << TWINT)));
}

/****************************************/
/****************************************/



