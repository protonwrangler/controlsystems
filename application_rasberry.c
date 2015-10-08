#include <stdio.h>
#include <math.h>
#include <string.h>
#include <errno.h>
#include <stdlib.h>
#include <wiringPi.h>
#include <bcm2835.h>

#define WORD_SIZE 2U
#define ENCODER_SIZE 3U
#define PWM       0b00000001
#define Encoder   0b00000010
#define Write     0b10000000
#define Timer     0

char EncoderIn[3];
char EncoderOut[3];
char DataOut[2];
char DataIn[2];
signed int speed = 1000;
volatile signed short PWMcheck;
volatile signed char Encoders;
volatile signed short Encoders1;
int data = 0;
int correct;
short control;
signed int PWMspeed;
char prompt;
int i;
int k;

void PWMwrite(char PWMspeed)
{

        control = PWM|Write;
        DataOut[0] = control;
        DataOut[1] = PWMspeed; //0b00101010;
        bcm2835_spi_transfernb(DataOut, DataIn, WORD_SIZE);
        PWMspeed = (0.5*((((speed)*1000/4.8)/5.9)-(Encoders*4.8/1000)))*128/12;
}

volatile signed short ReadEncode(void)
{
        control = Encoder;
        EncoderOut[0] = control;
        EncoderOut[1] = 0b00000000;
        EncoderOut[2] = 0b00000000;
        bcm2835_spi_transfernb(EncoderOut, EncoderIn, ENCODER_SIZE);
        Encoders1 = 0x0000;
        Encoders = EncoderIn[2];
        Encoders1 = (Encoders1|Encoders)*2.5;
        /*if((Encoders&0b1000000)==0b1000000){
                Encoders1 = ((Encoders&0x7F)|0x80)|0xFF00;
        }
        else{
                Encoders1 = ((Encoders&0x00FF));
        }*/
        printf("%d\n", Encoders1);

        return Encoders;
}

volatile signed short PWMread(void)
{
        control = PWM;
        DataOut[0] = control;
        DataOut[1] = 0b00000000;
        bcm2835_spi_transfernb(DataOut, DataIn, WORD_SIZE);
        PWMcheck = DataIn[1];
        return PWMcheck;
}

void timerInterrupt(void)
{
        Encoders = ReadEncode();
        PWMwrite(PWMspeed);
        PWMcheck = PWMread();
        data = 1;
}

int main(int argc, char **argv)
{
        if(!bcm2835_init())return 1;
        bcm2835_spi_begin();
        bcm2835_spi_setBitOrder(BCM2835_SPI_BIT_ORDER_MSBFIRST);
        bcm2835_spi_setDataMode(BCM2835_SPI_MODE1);
        bcm2835_spi_setClockDivider(BCM2835_SPI_CLOCK_DIVIDER_512);
        bcm2835_spi_chipSelect(BCM2835_SPI_CS0);
        bcm2835_spi_setChipSelectPolarity(BCM2835_SPI_CS0, LOW);

        if (wiringPiSetup() < 0)
        {
                fprintf(stderr, "Unable to setup wiringPi: %s\n", strerror(errno));
                return 1;
        }
        if (wiringPiISR (Timer, INT_EDGE_FALLING, &timerInterrupt) < 0)
        {
                fprintf(stderr, "Unable to setup wiringPi: %s\n", strerror(errno));
                return 1;
        }

        while(1)
        {

                bcm2835_delay(500);
                printf("\nPlease give an integer voltage between -10 and 10 for the
motor: ");
//                speed = getchar();
                scanf("%d", &speed);
                printf("%d\n", speed);
//                getchar();
                printf("\n");
                bcm2835_delay(1000);

                PWMspeed = speed*128/12;
                printf("%d",PWMcheck);


                if(data==1)
                {
                        bcm2835_delay(1000);
                        printf("\nThe motor's speed is now: ");
                        printf("%d\n", Encoders1);
                        printf("\n");
                        data = 0;
                }


        }
}


