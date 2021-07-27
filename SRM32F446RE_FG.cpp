#include    "mbed.h"
//#include    "stm32f4xx.h"

AnalogOut   DA0(PA_4);
DigitalOut  TRG(PC_0);

//---------------------------------------------------------------------------
// SetI2sClock(freq KHz)
//---------------------------------------------------------------------------
void SetI2sClock(int frq)
{
    switch(frq) {
        case    16:     // 160kHz
                // PLLI2Sclock = 160MHz   { (XTAL=8MHz * (PLLI2SN=320 / PLLM=8)) / PLLI2SR=2 }
                RCC->PLLI2SCFGR  = 2 << 28;         // PLLI2SR = 2
                RCC->PLLI2SCFGR |= 2 << 24;         // PLLI2SQ = 2
                RCC->PLLI2SCFGR |= 320 << 6;        // PLLI2SN
                // (160Mhz / I2SDIV=250) / 4
                SPI1->I2SPR      = 0x0000;          // MCKOE=0, I2SODD=0
                SPI1->I2SPR     |= 250;              // I2SDIV  = 250
                return;
        case    160:     // 1.6MHz
                // PLLI2Sclock = 160MHz   { (XTAL=8MHz * (PLLI2SN=320 / PLLM=8)) / PLLI2SR=2 }
                RCC->PLLI2SCFGR  = 2 << 28;         // PLLI2SR = 2
                RCC->PLLI2SCFGR |= 2 << 24;         // PLLI2SQ = 2
                RCC->PLLI2SCFGR |= 320 << 6;        // PLLI2SN
                // (160Mhz / I2SDIV=25) / 4
                SPI1->I2SPR      = 0x0000;          // MCKOE=0, I2SODD=0
                SPI1->I2SPR     |= 25;              // I2SDIV  = 25
                return;
        case    800:
                // PLLI2Sclock = 128MHz   { (XTAL=8MHz * (PLLI2SN=256 / PLLM=8)) / PLLI2SR=2 }
                RCC->PLLI2SCFGR  = 2 << 28;         // PLLI2SR = 2
                RCC->PLLI2SCFGR |= 2 << 24;         // PLLI2SQ = 2
                RCC->PLLI2SCFGR |= 256 << 6;        // PLLI2SN
                // (128Mhz / I2SDIV=4) / 4
                SPI1->I2SPR      = 0x0000;          // MCKOE=0, I2SODD=0
                SPI1->I2SPR     |= 4;               // I2SDIV  = 4
                return;
        case    1600:       // 16MHz
                // PLLI2Sclock = 128MHz   { (XTAL=8MHz * (PLLI2SN=256 / PLLM=8)) / PLLI2SR=2 }
                RCC->PLLI2SCFGR  = 2 << 28;         // PLLI2SR = 2
                RCC->PLLI2SCFGR |= 2 << 24;         // PLLI2SQ = 2
                RCC->PLLI2SCFGR |= 256 << 6;        // PLLI2SN
                // (128Mhz / I2SDIV=2) / 4
                SPI1->I2SPR      = 0x0000;          // MCKOE=0, I2SODD=0
                SPI1->I2SPR     |= 2;               // I2SDIV  = 2
                return;
        default:
                return;
    }
}

//---------------------------------------------------------------------------
// I2S1 Tx End ISR
//---------------------------------------------------------------------------
#define MAX_WAV 20000
#define MAX_REC 32768
short int       WAV[MAX_WAV];
short int       REC[MAX_REC];
volatile float  WAV_F = 0;
volatile int    REC_P = 0;
volatile int    Freq = 30;
volatile int    Stop = 0;

static void I2S1_IRQ_Handler(void)
{
    //TRG = 1;
    SPI1->DR = 0x5555;              // DAC Clock
    
    DAC->DHR12RD = WAV[(int)WAV_F];
    DAC->SWTRIGR = 0x01;
    WAV_F += (float)MAX_WAV / (1000000.0f / Freq);
    if(WAV_F >= MAX_WAV) WAV_F = 0;
    
    if(Stop != 0) return;
    
    REC[REC_P] = ADC1->DR;
    ADC1->CR2   = 0x40000001;       // ADC0:SWSTART:
    //while((ADC1->SR & 0x02) == 0);  // Wait ADC Done: EOC=1
    REC_P++;
    REC_P &= 0x07FFF;
    //TRG = 0;
}

//---------------------------------------------------------------------------
// Main
//---------------------------------------------------------------------------
int main()
{
    int     n, k, avg;
    double  Level;
    char    str[256], c, Spc, wav;
    
    //------------------------------------------
    // Port_A SetUp
    //------------------------------------------
    RCC->AHB1ENR  |= 0x00000001;        // GPIOA Clock Enable
    GPIOA->MODER  &= 0xFFFFFFFC;
    GPIOA->MODER  |= 3;                 // MODER0 =Ana PA_0 = ADC123_IN0
    //------------------------------------------
    // SPI/I2S Control
    //------------------------------------------
    RCC->APB2ENR    |= 0x00001000;      // SPI1 Clock Enable
    RCC->CR         |= 0x04000000;      // PLLI2SON = 1
    SPI1->CR1        = 0x0000;          // Not use I2S
    SPI1->CR2        = 0x0000;
    SPI1->I2SCFGR  = 0x0800;            // I2SMOD =I2S Mode
    SPI1->I2SCFGR |= 0x0400;            // I2SE   =I2S Enable
    SPI1->I2SCFGR |= 2 << 8;            // I2SCFG 2=MasterTx 3=MasterRx
    SPI1->I2SCFGR |= 0 << 7;            // PCMSYNC=ShortFrameSync
    SPI1->I2SCFGR |= 1 << 4;            // I2SSTD =MSB(LeftStart)
    SPI1->I2SCFGR |= 0 << 3;            // CKPOL  =Low to High
    SPI1->I2SCFGR |= 0 << 1;            // DATLEN =16bit
    SPI1->I2SCFGR |= 0;                 // CHLEN  =16bit
    SetI2sClock(800);
    //SetI2sClock(16);
    Spc = 'H';
    //Spc = 'L';
    //------------------------------------------
    // ADC1 SetUp
    //------------------------------------------
    RCC->APB2ENR  |= 0x00000100;        // ADC1EN:   ADC1 clock enable
    //ADC->CCR       = 0x00800000;        // Temp Sensor Enable
    ADC1->CR1      = 0;
    ADC1->CR1     |= 0 << 24;           // RES[1:0]: 0=12bit 1= 10bit 2=8bit 3=6bit
    ADC1->CR2      = 0x00000001;        // ADON:
    ADC1->SQR3     = 0x00000000;        // SEQ1[4:0]=ADC123_IN0
    ADC1->SMPR2    = 0x00000000;
    //------------------------------------------
    // I2S1 Interupt Enable
    //------------------------------------------
    NVIC_SetVector(SPI1_IRQn, (uint32_t)I2S1_IRQ_Handler);
    NVIC_EnableIRQ(SPI1_IRQn);
    SPI1->CR2 |= 0x0080;                // TXEIE Interupt Enable
    
    
    for(k = 0; k < MAX_WAV; k++) {
        WAV[k] = sin(((3.14159265359f * 2) / MAX_WAV) * k) * 1800 + 2048;
    }
    wav = 'i';
    
    wait(1.0);

    printf("\nSTM32 Function Generator\r\n");
    n = 0;
    while(1) {
        c = getchar();
        if(c == '\n' || c == '\r') {
            str[n] = 0;
            if(n == 0) printf("\r\n");
            n = 0;
            if((str[0] & 0xF0) == 0x30) {
                sscanf(str, "%d", &Freq);
                switch(Spc) {
                    case    'H': 
                            Freq *= 2;
                            if(Freq > 100000) Freq = 100000;
                            printf("[%d]Hz\r\n", Freq / 2);
                            break;
                    case    'M':
                            Freq *= 10;
                            if(Freq > 100000) Freq = 100000;
                            printf("[%d]Hz\r\n", Freq / 10);
                            break;
                    case    'L':
                            Freq *= 100;
                            if(Freq > 100000) Freq = 100000;
                            printf("[%d]Hz\r\n", Freq / 100);
                            break;
                }
                if(Freq == 0) {
                    Stop = 1;
                    DAC->DHR12RD = 2048;
                    DAC->SWTRIGR = 0x01;
                } else {
                    Stop = 0;
                }
            }
            if(strncmp(str, "si", 2) == 0) {
                for(k = 0; k < MAX_WAV; k++) {
                    WAV[k] = sin(((3.14159265359f * 2) / MAX_WAV) * k) * 1800 + 2048;
                }
                wav = 'i';
                printf("Sine Wave\r\n");
            }
            if(strncmp(str, "sq", 2) == 0) {
                for(k = 0; k < MAX_WAV / 2; k++) {
                    WAV[k] = 0;
                }
                for( ; k < MAX_WAV; k++) {
                    WAV[k] = 4095;
                }
                wav = 'q';
                printf("Square Wave\r\n");
            }
            if(strncmp(str, "sa", 2) == 0) {
                for(k = 0; k < MAX_WAV / 2; k++) {
                    WAV[k] = (float)4096 / (MAX_WAV / 2) * k;
                }
                for( ; k < MAX_WAV; k++) {
                    WAV[k] = (float)4096 / (MAX_WAV / 2) * (k - (MAX_WAV / 2));
                }
                wav = 'a';
                printf("Sawtooth Wave\r\n");
            }
            if(strncmp(str, "tr", 2) == 0) {
                for(k = 0; k < MAX_WAV / 2; k++) {
                    WAV[k] = (float)4096 / (MAX_WAV / 2) * k;
                }
                for( ; k < MAX_WAV; k++) {
                    WAV[k] = 4095 - ((float)4096 / (MAX_WAV / 2) * (k - (MAX_WAV / 2)));
                }
                wav = 'r';
                printf("Triangle Wave\r\n");
            }
            if(strncmp(str, "hs", 2) == 0) {
                SetI2sClock(800);
                switch(Spc) {
                    case    'M': Freq /= 5;   break;
                    case    'L': Freq /= 50;  break;
                }
                Spc = 'H';
                printf("hs [500k]SPS [%d]Hz\r\n", Freq / 2);
            }
            if(strncmp(str, "ms", 2) == 0) {
                SetI2sClock(160);
                switch(Spc) {
                    case    'H': Freq *= 5;   break;
                    case    'L': Freq /= 10;   break;
                }
                Spc = 'M';
                if(Freq > 100000) Freq = 100000;
                printf("ms [100k]SPS [%d]Hz\r\n", Freq / 10);
            }
            if(strncmp(str, "ls", 2) == 0) {
                SetI2sClock(16);
                switch(Spc) {
                    case    'H': Freq *= 50;   break;
                    case    'M': Freq *= 10;    break;
                }
                Spc = 'L';
                if(Freq > 100000) Freq = 100000;
                printf("ls [10k]SPS [%d]Hz\r\n", Freq / 100);
            }
            if(strncmp(str, "lv", 2) == 0) {
                Stop = 1;
                for(k = 0, Level = 0; k < MAX_REC; k++) {
                    Level += ((3.3f / 4096) * (REC[k] - 2078)) * ((3.3f / 4096) * (REC[k] - 2078));
                }
                for(k = 0, avg = 0; k < MAX_REC; k++) {
                    avg += REC[k];
                }
                Stop = 0;
                printf("Level[%2.3f]Vrms Avg[%d]\r\n", float(sqrt(Level / MAX_REC)), avg / MAX_REC);
            }
            if((str[0] == 'h' || str[0] == 'H') && str[1] == 0) {
                printf("---------------------------------------------------------\r\n");
                printf("PA0:Level Input  PA4:Function Generator Output\r\n");
                printf("NNNN     Frequency(Hz)\r\n");
                printf("si       Sine     Wave\r\n");
                printf("sq       Square   Wave\r\n");
                printf("sa       Sawtooth Wave\r\n");
                printf("tr       Triangle Wave\r\n");
                printf("hs       [500k]SPS  ADC(  2uS * 32768Sample =   65.536mS)\r\n");
                printf("ms       [100k]SPS  ADC( 10uS * 32768Sample =  327.680mS)\r\n");
                printf("ls       [10k]SPS   ADC(100uS * 32768Sample = 3276.800mS)\r\n");
                printf("lv       Level Measure (rms)\r\n");
                printf("---------------------------------------------------------\r\n");
                switch(wav) {
                    case    'i':
                        printf("Sine Wave  ");
                        break;
                    case    'q':
                        printf("Square Wave  ");
                        break;
                    case    'a':
                        printf("Sawtooth Wave  ");
                        break;
                    case    'r':
                        printf("Triangle Wave  ");
                        break;
                }
                switch(Spc) {
                    case    'H':
                        printf("hs[500k]SPS  Freq[%d]Hz\r\n", Freq / 2);
                        break;
                    case    'M':
                        printf("ms[100k]SPS  Freq[%d]Hz\r\n", Freq / 10);
                        break;
                    case    'L':
                        printf("ls[ 10k]SPS  Freq[%d]Hz\r\n", Freq / 100);
                        break;
                }
                printf("---------------------------------------------------------\r\n");
            }
        } else {
            str[n] = c;
            n++;
        }
    }
}