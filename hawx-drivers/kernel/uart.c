//
// low-level driver routines for 16550a UART.
//
#include "types.h"
#include "memlayout.h"
#include "riscv.h"
#include "console.h"
#include "port.h"
#include "string.h"

// the UART control registers are memory-mapped
// at address UART0. this macro returns the
// address of one of the registers.
#define Reg(reg) ((volatile unsigned char *)(UART0 + reg))

static int             uartgetc(void);

// the UART control registers.
// some have different meanings for
// read vs write.
// see http://byterunner.com/16550.html
#define RHR 0                 // receive holding register (for input bytes)
#define THR 0                 // transmit holding register (for output bytes)
#define IER 1                 // interrupt enable register
#define IER_RX_ENABLE (1<<0)
#define IER_TX_ENABLE (1<<1)
#define FCR 2                 // FIFO control register
#define FCR_FIFO_ENABLE (1<<0)
#define FCR_FIFO_CLEAR (3<<1) // clear the content of the two FIFOs
#define ISR 2                 // interrupt status register
#define LCR 3                 // line control register
#define LCR_EIGHT_BITS (3<<0)
#define LCR_BAUD_LATCH (1<<7) // special mode to set baud rate
#define LSR 5                 // line status register
#define LSR_RX_READY (1<<0)   // input is waiting to be read from RHR
#define LSR_TX_IDLE (1<<5)    // THR can accept another character to send

#define ReadReg(reg) (*(Reg(reg)))
#define WriteReg(reg, v) (*(Reg(reg)) = (v))


extern volatile int panicked; // from printf.c


// Initialize the UART driver 
void
uartinit(void)
{
  // Initialize the UART driver to operate at 38.4K baud, with 8-bit
  // words and no parity. Enable and reset FIFOs and enable TX and RX 
  // interrupts. There is basically only one way to accomplish this, 
  // so you should probably go looking for code to copy from xv6!
  // YOUR CODE HERE
}


// If the UART is idle, and a character is waiting in the 
// console output port, send it.
void 
uartstart(void)
{
    // This function starts the UART for interrupt driven output, which 
    // allows the system to continue doing other things while characters
    // are being sent out the UART. In our kernel, characters are printed
    // from the console output port (ports[PORT_CONSOLEOUT]). What this
    // function does is the following:
    // 1. Check if the PORT_CONSOLEOUT port has any characters in it.
    //    If it does not, return immediately. (NOTE: You want to do this
    //    by looking at the "count" field in port. DO NOT read the character
    //    yet as you may not be sending this character.)
    // 2. Check if the UART transmit holding register is full. If it is,
    //    return immediately. The UART will interrupt when it is ready for
    //    a new byte.
    // 3. Read the next character from the PORT_CONSOLEOUT port using 
    //    port_read.
    // 4. Send the character out the UART by writing it to the THR register.
    // YOUR CODE HERE
}


// Transmit a character via the UART, waiting for it to transmit completely
void
uartputc(int c)
{
  // If we are panicked, this function should hard lock and spin for
  // ever. Otherwise, write the chracter c immediately to the uart
  // port bypassing all interrupt schemes.
  // YOUR CODE HERE
}


// Transmit all of the characters in the PORT_CONSOLEOUT port.
// Send them one character at a time using uartputc, waiting for
// the UART to be ready to accept the next character.
void
uartflush()
{
  // While there is a character in the PORT_CONSOLEOUT port, 
  // fetch one charater and send it out the uart.
  // This function will use port_read and uartputc
  // YOUR CODE HERE
}


// read one input character from the UART.
// return -1 if none is waiting.
static int
uartgetc(void)
{
  // Read a character from the UART and return it as an integer.
  // If the UART has now character available, return -1 instead.
  // YOUR CODE HERE
  return -1;
}


// Handle a uart interrupt, raised because input has
// arrived, or the uart is ready for more output, or
// both. called from trap.c.
void
uartintr(void)
{
  // Read all incoming characters and add them to the PORT_CONSOLEIN
  // port. Echo the characters to the screen by adding them to
  // PORT_CONSOLEOUT as well. Note that some characters require extra
  // work. The terminal will send carriage return (\r) when enter is
  // pressed, these should be translated into newline (\n) characters
  // in order to work as expected. 
  // HINT: uartgetc and its return values are your friend...
  //
  // After all input has been processed, call uartstart to begin
  // character echo.
  // BONUS OPPORTUNITY: Make backspace work properly for extra credit.
  // YOUR CODE HERE
}
