#include "Type4me.h"

#include <string.h>
#include <usb_desc_key_cdc.h>
#include <usb_cdc.h>
#include <usb_bootloader.h>


int8 rx_position;
char rx_buffer[RX_BUFFER_SIZE];
char mystring[RX_BUFFER_SIZE];

int8 mystring_length;
int1 notify_enumeration;

void main() {
   
   output_low(LED);
   notify_enumeration = TRUE;
      
   rx_position = 0;
   get_password_from_eeprom();   
   
   // init USB device without waiting for connection
   usb_init_cs();

   // main loop
   while(TRUE) {
      
      // needed for USB operation
      usb_task();
      
      // device has been enumerated by the host (PC)?
      if(usb_enumerated()) {

         // the first time, blink led: device is ready
         if(notify_enumeration) {
            output_high(LED);
            delay_ms(200);
            output_low(LED);
            notify_enumeration = FALSE;
         }
         
         // button pressed? type string using USB keyboard
         if(check_button()) printf(send_char, mystring);
         
         // new char from USB serial port?
         if(usb_cdc_kbhit()) {
        
            // save incoming char
            delay_ms(10);            
            char incoming_char = usb_cdc_getc();
            
            // new line?
            if(incoming_char == '\n') {
            
               // end string
               rx_buffer[rx_position] = '\0';
               
               // clear command
               if(strcmp(rx_buffer, "!CLEAR!") == 0) {
                  mystring[0] = '\0';
                  store_string_to_eeprom();
                  printf(usb_cdc_putc, "\r\nCLEARED!\r\n");
               }
               
               // save command
               else if(strcmp(rx_buffer, "!SAVE!") == 0) {
                  store_string_to_eeprom();
                  printf(usb_cdc_putc, "\r\nSAVED!\r\n");
               }
               
               // normal string, save rx buffer content and ack
               else {
                  for(int i = 0; i <= rx_position; i++)
                     mystring[i] = rx_buffer[i];
                  printf(usb_cdc_putc, "\r\nOK!\r\n");
               }
               
               // reset rx buffer
               rx_position = 0;
            }
            
            // "normal" char?
            else if(incoming_char != '\r') {
            
               // save it in rx buffer and echo to user
               rx_buffer[rx_position] = incoming_char;
               usb_cdc_putc(incoming_char);
               rx_position++;
               
               // end of buffer reached? restart
               if(rx_position == RX_BUFFER_SIZE)
                  rx_position = 0;
            }
         }
      } 
   }
}

void get_password_from_eeprom() {

   mystring_length = read_eeprom(0);
   if(mystring_length == 0) {
      mystring[0] = '\0';
      return;
   }
   
   for(int i = 0; i < mystring_length; i++) 
      mystring[i] = read_eeprom(i + 1);
}

void store_string_to_eeprom() {

   int position = 0;
   char password_char;
   do {
      password_char = mystring[position];
      write_eeprom(position + 1, password_char);
      position++;
   } while(password_char != '\0');
   write_eeprom(0, position);
}

int1 check_button() {

   if(input(BUTTON)) return FALSE;
   delay_ms(50);
   if(input(BUTTON)) return FALSE;
   return TRUE;
}

void send_char(char c) {

   unsigned int8 tx_msg[7];
   unsigned int8 leds;
   unsigned int16 scancode;
   
   // is host able to receive data?
   if (usb_tbe(USB_HID_ENDPOINT)) {
      
      memset(tx_msg, 0x00, sizeof(tx_msg));

      // convert char to scancode and send it
      scancode = ToHidKeyboardScancode(c);
      if(bit_test(scancode, 8)) tx_msg[0] = KB_MODIFIER_LEFT_SHIFT;
      tx_msg[2] = scancode;
      usb_put_packet(USB_HID_ENDPOINT, tx_msg, sizeof(tx_msg), USB_DTS_TOGGLE);
      delay_ms(50);
      
      // release key     
      memset(tx_msg, 0x00, sizeof(tx_msg));
      usb_put_packet(USB_HID_ENDPOINT, tx_msg, sizeof(tx_msg), USB_DTS_TOGGLE);
      delay_ms(50);
   }
}

// Convert char to HID scan code, from CCS examples
unsigned int16 ToHidKeyboardScancode(char c)
{
   unsigned int16 shift = 0;
   
   if ((c>='A')&&(c<='Z'))
   {
      c = tolower(c);
      shift = 0x100;
   }
   if ((c>='a')&&(c<='z'))
   {
      return(((c-='a')+4)|shift);
   }
   if ((c>='1')&&(c<='9'))
   {
      return((c-='0')+0x1D);
   }
   switch(c)
   {
      case '!':   return(0x11E);
      case '@':   return(0x11F);
      case '#':   return(0x120);
      case '$':   return(0x121);
      case '%':   return(0x122);
      case '^':   return(0x123);
      case '&':   return(0x124);
      case '*':   return(0x125);
      case '(':   return(0x126);
      case ')':   return(0x127);
      case '0':   return(0x27);
      case '\n':  return(0x28);  //enter
      case '\r':  return(0x28);  //enter
      case '\b':  return(0x2A);  //backspace
      case '\t':  return(0x2B);  //tab
      case ' ':   return(0x2C);  //space
      case '_':   return(0x12D);
      case '-':   return(0x2D);
      case '+':   return(0x12E);
      case '=':   return(0x2E);
      case '{':   return(0x12F);
      case '[':   return(0x2F);
      case '}':   return(0x130);
      case ']':   return(0x30);
      case '|':   return(0x131);
      case '\\':   return(0x31);
      case ':':   return(0x133);
      case ';':   return(0x33);
      case '"':   return(0x134);
      case '\'':   return(0x34);
      case '~':   return(0x135);
      case '`':   return(0x35);
      case '<':   return(0x136);
      case ',':   return(0x36);
      case '>':   return(0x137);
      case '.':   return(0x37);
      case '?':   return(0x138);
      case '/':   return(0x38);
   }
}

