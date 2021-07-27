/***** Includes *****/
#include <unistd.h>
#include <stdint.h>
#include <stddef.h>
#include <stdio.h>

/* TI Drivers */
#include <ti/drivers/Power.h>
#include <ti/drivers/UART.h>
#include <ti/drivers/PIN.h>
#include <ti/drivers/rf/RF.h>
#include <ti/drivers/GPIO.h>
#include <ti/drivers/timer/GPTimerCC26XX.h>

/* Board Header files */
#include "Board.h"

/* Application Header files */
#include "smartrf_settings.h"
#include "mac_settings.h"
#include "RFQueue.h"

#include <ti/drivers/NVS.h>


static RF_Object rfObject;
static RF_Handle rfHandle;

rfc_CMD_RADIO_SETUP_PA_t RF_cmdRadioSetup_HP =
{
    .commandNo = 0x0802,
    .status = 0x0000,
    .pNextOp = 0x00000000,
    .startTime = 0x00000000,
    .startTrigger.triggerType = 0x0,
    .startTrigger.bEnaCmd = 0x0,
    .startTrigger.triggerNo = 0x0,
    .startTrigger.pastTrig = 0x1,
    .condition.rule = 0x1,
    .condition.nSkip = 0x0,
    .mode = 0x01,                   // 0x01: IEEE 802.15.4
    .loDivider = 0x00,
    .config.frontEndMode = 0x0,     // 0x00: Differential mode
    .config.biasMode = 0x1,         // 1: External bias
    .config.analogCfgMode = 0x0,
    .config.bNoFsPowerUp = 0x0,     // 0: Power up frequency synthesizer.
    .txPower = 0xFFFF,
    .pRegOverride = pOverrides,
    .pRegOverrideTxStd = pOverridesTxStd,
    .pRegOverrideTx20 = pOverridesTx20
};

rfc_CMD_PROP_RADIO_DIV_SETUP_PA_t RF_cmdPropRadioDivSetup_HP =
{
    .commandNo = 0x3807,
    .status = 0x0000,
    .pNextOp = 0,
    .startTime = 0x00000000,
    .startTrigger.triggerType = 0x0,
    .startTrigger.bEnaCmd = 0x0,
    .startTrigger.triggerNo = 0x0,
    .startTrigger.pastTrig = 0x0,
    .condition.rule = 0x1,
    .condition.nSkip = 0x0,
    .modulation.modType = 0x1,
    .modulation.deviation = 0x64,
    .modulation.deviationStepSz = 0x0,
    .symbolRate.preScale = 0xF,
    .symbolRate.rateWord = 0x8000,
    .symbolRate.decimMode = 0x0,
    .rxBw = 0x52,
    .preamConf.nPreamBytes = 0x4,
    .preamConf.preamMode = 0x0,
    .formatConf.nSwBits = 0x20,
    .formatConf.bBitReversal = 0x0,
    .formatConf.bMsbFirst = 0x1,
    .formatConf.fecMode = 0x0,
    .formatConf.whitenMode = 0x0,
    .config.frontEndMode = 0x0,
    .config.biasMode = 0x1,
    .config.analogCfgMode = 0x0,
    .config.bNoFsPowerUp = 0x0,
    .txPower = 0xFFFF,
    .pRegOverride = pOverrides,
    .centerFreq = 0x0364,
    .intFreq = 0x8000,
    .loDivider = 0x05,
    .pRegOverrideTxStd = pOverridesTxStd,
    .pRegOverrideTx20 = pOverridesTx20
};


RF_TxPowerTable_Entry txPowerTable_900M[TX_POWER_TABLE_SIZE] =
{
    { 0, RF_TxPowerTable_DEFAULT_PA_ENTRY(11, 3, 0,  9) },  //0.2
    { 1, RF_TxPowerTable_DEFAULT_PA_ENTRY(12, 3, 0, 11) },  //1.1
    { 2, RF_TxPowerTable_DEFAULT_PA_ENTRY(14, 3, 0, 12) },  //2.5
    { 3, RF_TxPowerTable_DEFAULT_PA_ENTRY(15, 3, 0, 13) },  //3.2
    { 4, RF_TxPowerTable_DEFAULT_PA_ENTRY(17, 3, 0, 15) },  //4.2
    { 5, RF_TxPowerTable_DEFAULT_PA_ENTRY(19, 3, 0, 18) },  //5.1
    { 6, RF_TxPowerTable_DEFAULT_PA_ENTRY(22, 3, 0, 22) },  //6.2
    { 7, RF_TxPowerTable_DEFAULT_PA_ENTRY(26, 3, 0, 30) },  //7.1
    { 8, RF_TxPowerTable_DEFAULT_PA_ENTRY(19, 2, 0, 30) },  //8.2
    { 9, RF_TxPowerTable_DEFAULT_PA_ENTRY(28, 2, 0, 32) },  //9.0
    {10, RF_TxPowerTable_DEFAULT_PA_ENTRY(58, 0, 1,  0) },  //10.3
    {11, RF_TxPowerTable_HIGH_PA_ENTRY(13, 0, 0, 25,  0) }, //11.0
    {12, RF_TxPowerTable_HIGH_PA_ENTRY(17, 0, 0, 35,  0) }, //12.0
    {13, RF_TxPowerTable_HIGH_PA_ENTRY(21, 0, 0, 43,  0) }, //13.0
    {14, RF_TxPowerTable_HIGH_PA_ENTRY(26, 0, 0, 51,  2) }, //14.2
    {15, RF_TxPowerTable_HIGH_PA_ENTRY(29, 0, 0, 60,  4) }, //15.0
    {16, RF_TxPowerTable_HIGH_PA_ENTRY(18, 1, 0, 32,  4) }, //16.1
    {17, RF_TxPowerTable_HIGH_PA_ENTRY(15, 2, 0, 36,  4) }, //17.1
    {18, RF_TxPowerTable_HIGH_PA_ENTRY(22, 2, 0, 36,  4) }, //18.0
    {19, RF_TxPowerTable_HIGH_PA_ENTRY(40, 2, 0, 60,  4) }, //19.0
    {20, RF_TxPowerTable_HIGH_PA_ENTRY(30, 3, 0, 71, 27) }, //19.9
    RF_TxPowerTable_TERMINATION_ENTRY
};

RF_TxPowerTable_Entry txPowerTable_2R4G[TX_POWER_TABLE_SIZE] =
{
    { 0, RF_TxPowerTable_DEFAULT_PA_ENTRY(14, 0, 0, 32) },     //0.1
    { 1, RF_TxPowerTable_DEFAULT_PA_ENTRY(17, 0, 0, 33) },     //1.0
    { 2, RF_TxPowerTable_DEFAULT_PA_ENTRY(23, 0, 0, 35) },     //2.2
    { 3, RF_TxPowerTable_DEFAULT_PA_ENTRY(29, 0, 0, 45) },     //3.0
    { 4, RF_TxPowerTable_DEFAULT_PA_ENTRY(48, 0, 1, 73) },     //4.0
    { 5, RF_TxPowerTable_HIGH_PA_ENTRY(12, 3, 1, 32, 4) },     //5.0
    { 6, RF_TxPowerTable_HIGH_PA_ENTRY(17, 3, 1, 36, 4) },     //6.0
    { 7, RF_TxPowerTable_HIGH_PA_ENTRY(20, 3, 1, 40, 5) },     //7.1
    { 8, RF_TxPowerTable_HIGH_PA_ENTRY(34, 3, 1, 47, 5) },     //8.0
    { 9, RF_TxPowerTable_HIGH_PA_ENTRY(28, 3, 1, 45, 7) },     //9.0
    {10, RF_TxPowerTable_HIGH_PA_ENTRY(40, 3, 1, 45, 8) },     //10.1
    {11, RF_TxPowerTable_HIGH_PA_ENTRY(35, 3, 1, 45, 10) },    //11.1
    {12, RF_TxPowerTable_HIGH_PA_ENTRY(35, 3, 1, 45, 12) },    //12.1
    {13, RF_TxPowerTable_HIGH_PA_ENTRY(38, 3, 1, 45, 14) },    //13.1
    {14, RF_TxPowerTable_HIGH_PA_ENTRY(40, 3, 1, 45, 17) },    //14.1
    {15, RF_TxPowerTable_HIGH_PA_ENTRY(36, 3, 1, 45, 22) },    //15.0
    {16, RF_TxPowerTable_HIGH_PA_ENTRY(35, 3, 1, 45, 35) },    //16.1
    {17, RF_TxPowerTable_HIGH_PA_ENTRY(40, 3, 1, 45, 50) },    //17.0
    {18, RF_TxPowerTable_HIGH_PA_ENTRY(50, 3, 1, 50, 50) },    //18.0
    {19, RF_TxPowerTable_HIGH_PA_ENTRY(60, 3, 1, 60, 60) },    //18.6
    RF_TxPowerTable_TERMINATION_ENTRY
};

rfc_CMD_TX_TEST_t RF_cmdTxTest_kzk =
{
    .commandNo = 0x0808,
    .status = 0x0000,
    .pNextOp = 0,
    .startTime = 0x00000000,
    .startTrigger.triggerType = 0x0,
    .startTrigger.bEnaCmd = 0x0,
    .startTrigger.triggerNo = 0x0,
    .startTrigger.pastTrig = 0x0,
    .condition.rule = 0x1,
    .condition.nSkip = 0x0,
    .config.bUseCw = 0x1,            // Send continuous wave.
    //.config.bUseCw = 0x0,            // Send modulated signal.
    .config.bFsOff = 0x1,
    .config.whitenMode = 0x2,
    .__dummy0 = 0x00,
    .txWord = 0xABCD,
    .__dummy1 = 0x00,
    .endTrigger.triggerType = 0x3,     // endTime後終了
    .endTrigger.bEnaCmd = 0x0,
    .endTrigger.triggerNo = 0x3,       // endTime後終了
    .endTrigger.pastTrig = 0x1,
    .syncWord = 0x930B51DE,
    //.endTime = 0x00000000
    //.endTime = 100000000   //25S
    .endTime = 40000000   //5S
};

char    str[256], buf[256];

UART_Handle uart;

//---------------------------------------------------------------------------
// Char Code
//---------------------------------------------------------------------------
char    CharCode[] = {
//   0   1   2   3   4   5   6   7   8   9   A   B   C   D   E   F
     0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0, // 0
     0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0, // 1
     0,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1, // 2
     1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1, // 3
     1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1, // 4
     1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1, // 5
     1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1, // 6
     1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  0  // 7
};

//---------------------------------------------------------------------------
// Char->Int
//---------------------------------------------------------------------------
int str_int(char *str)
{
    char    buf[32];
register int     k;
static  int     n;

    for(k = 0, n = 0; str[k] != 0 && k < 32; k++) {
        if(str[k] != ' ' && str[k] != ',' && str[k] != '"') {
            buf[n] = str[k];
            n++;
        }
    }
    buf[n] = 0;
        if(buf[0] == '-') {
                for(k = 1, n = 0; buf[k] != 0; k++) if((buf[k] & 0xF0) == 0x30) n = (n * 10) + (buf[k] & 0x0F);
                n *= -1;
        } else  for(k = 0, n = 0; buf[k] != 0; k++) if((buf[k] & 0xF0) == 0x30)  n = (n * 10) + (buf[k] & 0x0F);
        return(n);
}

//---------------------------------------------------------------------------
// Frequency Set
//---------------------------------------------------------------------------
Set_Freq_Cmd(double freq)
{
    RF_cmdFs.frequency = freq;
    RF_cmdFs.fractFreq = (freq - RF_cmdFs.frequency) / ((double)1.0 / 0xFFFF);
    RF_postCmd(rfHandle, (RF_Op*)&RF_cmdFs, RF_PriorityNormal, NULL, 0);
}



void *mainThread(void *arg0)
{
    int         n, m, k;
    char        input;

    char        UART_buf[256];
    int         UART_p;

static  double  Freq = 920;
    int         nPower;
    float       fTime;
unsigned int    t_start, t_end;

    UART_Params uartParams;
    GPTimerCC26XX_Params tim_params;
    //----------------------------------------------------------------
    // Init
    //----------------------------------------------------------------
    RF_Params rfParams;
    RF_Params_init(&rfParams);
    //----------------------------------------------------------------
    // Red Green LED 消灯
    //----------------------------------------------------------------
    GPIO_init();
    GPIO_write(Board_GPIO0, 0);
    GPIO_write(Board_GPIO1, 0);
    sleep(1);
    //----------------------------------------------------------------
    // UART セットアップ
    //----------------------------------------------------------------
    UART_init();
    UART_Params_init(&uartParams);
    uartParams.writeDataMode = UART_DATA_BINARY;
    uartParams.readDataMode = UART_DATA_BINARY;
    uartParams.readReturnMode = UART_RETURN_FULL;
    uartParams.readEcho = UART_ECHO_OFF;
    uartParams.baudRate = 230400;
    uartParams.readTimeout =  10;            // 10=180uS, 入力有の場合=7uS
    //uartParams.readTimeout = 100;            // 100=1068uS, 入力有の場合=7uS
    uart = UART_open(Board_UART0, &uartParams);
    sprintf(str, "Backscatter_TAG_CW_Tx\r\n");
    UART_write(uart, str, strlen(str));
    //----------------------------------------------------------------
    // Timer セットアップ
    //----------------------------------------------------------------
    GPTimerCC26XX_Params_init(&tim_params);
    tim_params.width          = GPT_CONFIG_32BIT;
    tim_params.mode           = GPT_MODE_PERIODIC_UP;
    tim_params.debugStallMode = GPTimerCC26XX_DEBUG_STALL_OFF;
    GPTimerCC26XX_Handle hTimer = GPTimerCC26XX_open(Board_TIMER0, &tim_params);
    //----------------------------------------------------------------
    // Non-Volatile Storage
    //----------------------------------------------------------------
    NVS_Handle      nvsRegion;
    NVS_Attrs       regionAttrs;
    uint_fast16_t   status;
    NVS_init();

    nPower = 0;
    fTime  = 5;
    while(1) {
        //-------------------------------------------------------
        // UARTから入力されたコマンドを読込→実行
        //-------------------------------------------------------
        k = UART_read(uart, &input, 1);
        if(k != 0) {
            UART_buf[UART_p] = input;
            UART_p++;
            UART_p &= 0xFF;
            if(input == '\r' || input == '\n') {
                //UART_write(uart, UART_buf, UART_p);
                //UART_write(uart, "\r\n", 2);
                UART_buf[UART_p] = 0;
                //-------------------------------------------------------
                // freq
                //-------------------------------------------------------
                if(strncmp(UART_buf, "freq", 4) == 0) {
                    for(n = 0; CharCode[UART_buf[n]] != 0 && n < 8; n++);
                    for(     ; UART_buf[n] == ' ' || UART_buf[n] == '\t'; n++);
                    for(m = n; CharCode[UART_buf[m]] != 0; m++);
                    UART_buf[m] = 0;
                    sscanf(&UART_buf[n], "%lf", &Freq);
                    //sprintf(str, "ACK_freq[%7.4lf]\r\n", Freq);
                    //UART_write(uart, str, strlen(str));
                    UART_write(uart, "ACK_frq\r\n", 9);
                }
                //-------------------------------------------------------
                // Power
                //-------------------------------------------------------
                if(strncmp(UART_buf, "pow", 3) == 0) {
                    for(n = 0; CharCode[UART_buf[n]] != 0 && n < 8; n++);
                    for(     ; UART_buf[n] == ' ' || UART_buf[n] == '\t'; n++);
                    for(m = n; CharCode[UART_buf[m]] != 0; m++);
                    UART_buf[m] = 0;
                    sscanf(&UART_buf[n], "%d", &nPower);
                    //sprintf(str, "ACK_pow[%d]\r\n", nPower);
                    //UART_write(uart, str, strlen(str));
                    UART_write(uart, "ACK_pow\r\n", 9);
                }
                //-------------------------------------------------------
                // Time
                //-------------------------------------------------------
                if(strncmp(UART_buf, "tim", 3) == 0) {
                    for(n = 0; CharCode[UART_buf[n]] != 0 && n < 8; n++);
                    for(     ; UART_buf[n] == ' ' || UART_buf[n] == '\t'; n++);
                    for(m = n; CharCode[UART_buf[m]] != 0; m++);
                    UART_buf[m] = 0;
                    sscanf(&UART_buf[n], "%f", &fTime);
                    //sprintf(str, "ACK_tim[%3.2f]\r\n", fTime);
                    //UART_write(uart, str, strlen(str));
                    UART_write(uart, "ACK_tim\r\n", 9);
                }
                //-------------------------------------------------------
                // Parameters Read
                //-------------------------------------------------------
                if(strncmp(UART_buf, "rea", 3) == 0) {
                    nvsRegion = NVS_open(Board_NVS0, NULL);
                    NVS_getAttrs(nvsRegion, &regionAttrs);
                    status = NVS_read(nvsRegion, 0, buf, 128);
                    NVS_close(nvsRegion);
                    if(strncmp(buf, "freq=", 5) == 0) {
                        sscanf(buf, "freq=%lf pow=%d time=%f", &Freq, &nPower, &fTime);
                    }
                    sprintf(str, "ACK_rea[%s]\r\n", buf);
                    UART_write(uart, str, strlen(str));
                }
                //-------------------------------------------------------
                // Parameters Save
                //-------------------------------------------------------
                if(strncmp(UART_buf, "sav", 3) == 0) {
                    nvsRegion = NVS_open(Board_NVS0, NULL);
                    NVS_getAttrs(nvsRegion, &regionAttrs);
                    status = NVS_erase(nvsRegion, 0, regionAttrs.sectorSize);
                    sprintf(buf, "freq=%lf pow=%d time=%f", Freq, nPower, fTime);
                    status = NVS_write(nvsRegion, 0, buf, strlen(buf) + 1, NVS_WRITE_POST_VERIFY);
                    NVS_close(nvsRegion);
                    sprintf(str, "ACK_sav[%s]\r\n", buf);
                    UART_write(uart, str, strlen(str));
                }
                //-------------------------------------------------------
                // Modulate
                //-------------------------------------------------------
                if(strncmp(UART_buf, "mod", 3) == 0) {
                    for(n = 0; CharCode[UART_buf[n]] != 0 && n < 8; n++);
                    for(     ; UART_buf[n] == ' ' || UART_buf[n] == '\t'; n++);
                    if(UART_buf[n] == '1') {
                        RF_cmdTxTest_kzk.config.bUseCw = 0x0;            // Send modulated signal.
                    } else {
                        RF_cmdTxTest_kzk.config.bUseCw = 0x1;            // Send continuous wave.
                    }
                    sprintf(str, "ACK_mod[%c]\r\n", UART_buf[n]);
                    UART_write(uart, str, strlen(str));
                }
                //-------------------------------------------------------
                // start コマンド   [861_1054 2360_2500]
                //-------------------------------------------------------
                if(strncmp(UART_buf, "sta", 3) == 0) {
                    GPIO_write(Board_GPIO1, 1);
                    //-------------------------------------------------------
                    // RF_Open
                    //-------------------------------------------------------
                    if(Freq >= 861 && Freq <= 1054) {
                        rfHandle = RF_open(&rfObject, &RF_prop, (RF_RadioSetup*)&RF_cmdPropRadioDivSetup_HP, &rfParams);
                        RF_setTxPower(rfHandle, txPowerTable_900M[nPower].value);
                    }
                    if(Freq >= 2360 && Freq <= 2500) {
                        rfHandle = RF_open(&rfObject, &RF_prop, (RF_RadioSetup*)&RF_cmdRadioSetup_HP, &rfParams);
                        RF_setTxPower(rfHandle, txPowerTable_2R4G[nPower].value);
                    }
                    //sprintf(str, "Power=%d\r\n", nPower);
                    //UART_write(uart, str, strlen(str));
                    //nPower++;
                    //-------------------------------------------------------
                    // Frequency 設定
                    //-------------------------------------------------------
                    Set_Freq_Cmd(Freq);
                    //-------------------------------------------------------
                    // Timer Start
                    //-------------------------------------------------------
                    GPTimerCC26XX_start(hTimer);
                    t_start = GPTimerCC26XX_getValue(hTimer);
                    //-------------------------------------------------------
                    // CW送信
                    //-------------------------------------------------------
                    RF_cmdTxTest_kzk.endTime = fTime / 0.00000025;
                    RF_runCmd(rfHandle, (RF_Op*)&RF_cmdTxTest_kzk, RF_PriorityNormal, NULL, 0);
                    //-------------------------------------------------------
                    // Timer Stop
                    //-------------------------------------------------------
                    t_end = GPTimerCC26XX_getValue(hTimer);
                    GPTimerCC26XX_stop(hTimer);
                    //-------------------------------------------------------
                    //  RF_Quie フラッシュ
                    //-------------------------------------------------------
                    RF_flushCmd(rfHandle, RF_CMDHANDLE_FLUSH_ALL, 1);
                    //-------------------------------------------------------
                    //  RF_Close
                    //-------------------------------------------------------
                    RF_close(rfHandle);
                    //sprintf(str, "Output End Time[%3.0f]ms\r\n", ((t_end - t_start) * (1.0 / 48)) / 1000);
                    //UART_write(uart, str, strlen(str));
                    UART_write(uart, "End\r\n", 5);
                    GPIO_write(Board_GPIO1, 0);
                }
                UART_p = 0;
            }
        }
    }
}
