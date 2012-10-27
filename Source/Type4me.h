#include <18F14K50.h>

#FUSES HS                       // High speed Osc
#FUSES NOWDT                    // No Watch Dog Timer
#FUSES NOPROTECT                // No code protection 
#FUSES NOLVP                    // No low voltage prgming
#FUSES NODEBUG                  // No debugging
#FUSES NOBROWNOUT               // No brownout reset
#FUSES NOMCLR                   // No MCLR
#FUSES PLLEN                    // PLL Enabled
#FUSES CPUDIV1                  // System Clock by 1

#device PASS_STRINGS = IN_RAM   // Useful for strcmp() with constant strings

#ROM int8 0xF00000={0x00}       // Init internal EEPROM

#define LED                     PIN_C5
#define BUTTON                  PIN_C0
#define RX_BUFFER_SIZE          100
#define KB_MODIFIER_LEFT_SHIFT  2

#use delay(clock=48000000)

void get_password_from_eeprom();
void store_string_to_eeprom();
int1 check_button();
void send_char(char c);
unsigned int16 ToHidKeyboardScancode(char c);

