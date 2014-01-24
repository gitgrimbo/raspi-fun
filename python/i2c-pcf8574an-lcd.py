import smbus
import time

# Access the i2c bus now.
bus = smbus.SMBus(0)

# PCF8574AN i2c address
address = 0x38

# I have the following pins set up
# LCD D4    -    PCF8574AN D0
# LCD D5    -    PCF8574AN D1
# LCD D6    -    PCF8574AN D2
# LCD D7    -    PCF8574AN D3
# LCD EN/RW -    PCF8574AN D4
# LCD RS    -    PCF8574AN D5

# So in this manner I write i2c bytes from the RasPi to the PCF8574AN,
# and the lowest bits 0-3 are the data, bit 4 is the EN/RW and bit 5 is the RS.
# Bits 6 and 7 are not used.

# Not sure this is required. Writing the PCF8574AN pins high.
bus.write_byte(address, 0xff)

class MyApp:

    # commands
    LCD_CLEARDISPLAY                = 0x01
    LCD_RETURNHOME                = 0x02
    LCD_ENTRYMODESET                = 0x04
    LCD_DISPLAYCONTROL                = 0x08
    LCD_CURSORSHIFT                = 0x10
    LCD_FUNCTIONSET                = 0x20
    LCD_SETCGRAMADDR                = 0x40
    LCD_SETDDRAMADDR                = 0x80

    # flags for display entry mode
    LCD_ENTRYRIGHT                = 0x00
    LCD_ENTRYLEFT                = 0x02
    LCD_ENTRYSHIFTINCREMENT        = 0x01
    LCD_ENTRYSHIFTDECREMENT        = 0x00

    # flags for display on/off control
    LCD_DISPLAYON                = 0x04
    LCD_DISPLAYOFF                = 0x00
    LCD_CURSORON                = 0x02
    LCD_CURSOROFF                = 0x00
    LCD_BLINKON                        = 0x01
    LCD_BLINKOFF                = 0x00

    # flags for display/cursor shift
    LCD_DISPLAYMOVE                = 0x08
    LCD_CURSORMOVE                = 0x00

    # flags for display/cursor shift
    LCD_DISPLAYMOVE                = 0x08
    LCD_CURSORMOVE                = 0x00
    LCD_MOVERIGHT                = 0x04
    LCD_MOVELEFT                = 0x00

    # flags for function set
    LCD_8BITMODE                = 0x10
    LCD_4BITMODE                = 0x00
    LCD_2LINE                        = 0x08
    LCD_1LINE                        = 0x00
    LCD_5x10DOTS                = 0x04
    LCD_5x8DOTS                        = 0x00

    rsMask = 0b00010000 # PCF8574AN D5
    enMask = 0b00100000 # PCF8574AN D6
    data = 0
    rsStatus = 0
    dataCount = 0

    def __init__(self):
        self.delayMicroseconds(40) # 40 microsecond sleep

        # http://en.wikipedia.org/wiki/Hitachi_HD44780_LCD_controller#Instruction_Set
        # and
        # http://lcd-linux.sourceforge.net/pdfdocs/hd44780.pdf
        # page 45 "Initializing by Instruction"

        # send 3, 3, 3, 2
        self.write4bits(0x33) # initialization
        self.write4bits(0x32) # initialization

        # [0,0,1,DL] [N,F,*,*]
        # DL(DATA_LENGTH): 0 = 4bit, 1 = 8bit
        # N:  0 = 1/8 or 1/11 duty (1 line), 1 = 1/16 duty (2 lines)
        # F: 0 = 5x8 dots, 1 = 5x10 dots
        # self.LCD_FUNCTIONSET = 0010 0000
        self.write4bits(self.LCD_FUNCTIONSET | 0b01000) # 2 line 5x7 matrix

        # [0,0,0,0] [1,D,C,B]
        # D(DISPLAY)
        # C(CURSOR)
        # B(BLINK)
        # self.LCD_DISPLAYCONTROL = 0000 1000
        self.write4bits(self.LCD_DISPLAYCONTROL | 0b100) # turn cursor off 0x0E to enable cursor

        # [0,0,0,1] [S/C,R/L,*,*]
        # S/C: 0 = move cursor, 1 = shift display
        # R/L: 0 = shift left, 1 = shift right
        # self.LCD_CURSORSHIFT = 0001 0000
        self.write4bits(self.LCD_CURSORSHIFT | 0b0000) # shift cursor right

        """ Initialize to default text direction (for romance languages) """
        # 0010 | 0000
        self.displaymode = self.LCD_ENTRYLEFT | self.LCD_ENTRYSHIFTDECREMENT

        # [0,0,0,0] [0,1,I/D,S]
        # I/D: 0 = decrement cursor position, 1 = increment cursor position
        # S: 0 = no display shift, 1 = display shift
        # self.LCD_ENTRYMODESET = 0000 0100
        self.write4bits(self.LCD_ENTRYMODESET | self.displaymode) #  set the entry mode

        self.clear()

    def log(self, s):
       print s

    def setdata(self, data):
        self.data = data
        return data

    def write(self, data):
        self.log('write    %03d %s' % (self.dataCount, format(data, '#010b')))
        self.dataCount += 1
        bus.write_byte(address, data)

    def write4bits(self, bits, char_mode=False):
        """ Send command to LCD """

        # toggle command?
        self.rsStatus = self.rsMask if char_mode else 0

        data = self.setdata((bits >> 4) & 0x0f)
        data = data | self.rsStatus
        self.write(data)

        self.pulseEnable()

        data = self.setdata(bits & 0x0f)
        data = data | self.rsStatus
        self.write(data)

        self.pulseEnable()

    def pulseEnable(self):
        self.log('PULSE START')
        #self.write(self.data | self.rsStatus)
        #self.delayMicroseconds(1)                # 1 microsecond pause - enable pulse must be > 450ns 
        self.write(self.data | self.enMask | self.rsStatus)
        self.delayMicroseconds(1)                # 1 microsecond pause - enable pulse must be > 450ns 
        self.write(self.data | self.rsStatus)
        self.delayMicroseconds(50)                # commands need > 37us to settle
        self.log('PULSE END')

    def delayMicroseconds(self, microseconds):
        seconds = microseconds / float(1000000)        # divide microseconds by 1 million for seconds
        time.sleep(seconds)

    def clear(self):
        self.write4bits(self.LCD_CLEARDISPLAY) # command to clear display
        self.delayMicroseconds(3000)        # 3000 microsecond sleep, clearing the display takes a long time

    def message(self, text):
        """ Send string to LCD. Newline wraps to second line"""

        for char in text:
            if char == '\n':
                self.write4bits(0xC0) # next line
            else:
                self.write4bits(ord(char),True)

if __name__ == '__main__':

    lcd = MyApp()

    lcd.clear()
    # On my LCD, the "are you?" string will appear at the start of line 2
    lcd.message("you know\nwhat to do")
    print "done"
