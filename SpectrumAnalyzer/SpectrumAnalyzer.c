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

// CMD_RADIO_SETUP for IEEE 15.4
/* Use dynamic PA struct type for 13x2 */
rfc_CMD_RADIO_SETUP_PA_t RF_cmdRadioSetup_kzk_2R4G =
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

    //.txPower = 0x7217,              // 1.19dBm
    //.txPower = 0x723F,              // 3.76dBm
    .txPower = 0x013F,              // 3.81dBm

    .pRegOverride = NULL,
    .pRegOverrideTxStd = NULL,
    .pRegOverrideTx20 = NULL,

    /**********************************************
    // +19.5dBm
    .txPower = 0xFFFF,
    .pRegOverride = pOverrides,
    .pRegOverrideTxStd = pOverridesTxStd,
    .pRegOverrideTx20 = pOverridesTx20
    ******************************************/
};

// CMD_TX_TEST
// Transmitter Test Command
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
    .config.bUseCw = 0x1,
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
    .endTime = 10000000
};

/* IEEE TX Command */
rfc_CMD_IEEE_TX_t RF_cmdIEEETx_kzk =
{
    .commandNo = CMD_IEEE_TX,           // 0x2C01
    .status = 0,
    .pNextOp = 0,
    .startTime = 0,
    .startTrigger.triggerType = TRIG_NOW,
    .startTrigger.bEnaCmd = 0,
    .startTrigger.triggerNo = 0,
    .startTrigger.pastTrig = 0,
    .condition.rule = COND_NEVER,
    .condition.nSkip = 0,
    .txOpt.bIncludePhyHdr = 0,
    .txOpt.bIncludeCrc = 0,
    .txOpt.payloadLenMsb = 0,
    .payloadLen = 0,
    .pPayload = 0,
    .timeStamp = 0,
};


static RF_Object rfObject;
static RF_Handle rfHandle;
static dataQueue_t dataQueue;
static uint8_t rxDataEntryBuffer[512];


// CMD_IEEE_RX
// IEEE 802.15.4 Receive Command
rfc_CMD_IEEE_RX_t RF_cmdIeeeRx_kzk =
{
    .commandNo = 0x2801,
    .status = 0x0000,
    .pNextOp = 0,
    .startTime = 0x00000000,
    .startTrigger.triggerType = 0x0,
    .startTrigger.bEnaCmd = 0x0,
    .startTrigger.triggerNo = 0x0,
    .startTrigger.pastTrig = 0x0,
    .condition.rule = 0x0,
    .condition.nSkip = 0x0,
    .channel = 0x00,
    .rxConfig.bAutoFlushCrc = 0x0,
    .rxConfig.bAutoFlushIgn = 0x1,
    .rxConfig.bIncludePhyHdr = 0x1,
    .rxConfig.bIncludeCrc = 0x0,
    .rxConfig.bAppendRssi = 0x1,
    .rxConfig.bAppendCorrCrc = 0x1,
    .rxConfig.bAppendSrcInd = 0x0,
    .rxConfig.bAppendTimestamp = 0x0,
    .pRxQ = 0,
    .pOutput = 0,
    .frameFiltOpt.frameFiltEn = 0x0,
    .frameFiltOpt.frameFiltStop = 0x0,
    .frameFiltOpt.autoAckEn = 0x0,
    .frameFiltOpt.slottedAckEn = 0x0,
    .frameFiltOpt.autoPendEn = 0x0,
    .frameFiltOpt.defaultPend = 0x0,
    .frameFiltOpt.bPendDataReqOnly = 0x0,
    .frameFiltOpt.bPanCoord = 0x0,
    .frameFiltOpt.maxFrameVersion = 0x3,
    .frameFiltOpt.fcfReservedMask = 0x0,
    .frameFiltOpt.modifyFtFilter = 0x0,
    .frameFiltOpt.bStrictLenFilter = 0x0,
    .frameTypes.bAcceptFt0Beacon = 0x1,
    .frameTypes.bAcceptFt1Data = 0x1,
    .frameTypes.bAcceptFt2Ack = 0x1,
    .frameTypes.bAcceptFt3MacCmd = 0x1,
    .frameTypes.bAcceptFt4Reserved = 0x1,
    .frameTypes.bAcceptFt5Reserved = 0x1,
    .frameTypes.bAcceptFt6Reserved = 0x1,
    .frameTypes.bAcceptFt7Reserved = 0x1,
    .ccaOpt.ccaEnEnergy = 0x0,
    .ccaOpt.ccaEnCorr = 0x0,
    .ccaOpt.ccaEnSync = 0x0,
    .ccaOpt.ccaCorrOp = 0x1,
    .ccaOpt.ccaSyncOp = 0x1,
    .ccaOpt.ccaCorrThr = 0x0,
    .ccaRssiThr = 0x64,
    .__dummy0 = 0x00,
    .numExtEntries = 0x00,
    .numShortEntries = 0x00,
    .pExtEntryList = 0,
    .pShortEntryList = 0,
    .localExtAddr = 0x12345678,
    .localShortAddr = 0xABBA,
    .localPanID = 0x0000,
    .__dummy1 = 0x0,
    .endTrigger.triggerType = 0x1,
    .endTrigger.bEnaCmd = 0x0,
    .endTrigger.triggerNo = 0x0,
    .endTrigger.pastTrig = 0x0,
    .endTime = 0x00000000
};


rfc_CMD_PROP_RADIO_DIV_SETUP_PA_t RF_cmdPropRadioSetup =
{
    .commandNo = 0x3806,
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
    //.txPower = 0xFFFF,
    .txPower = 0x013F,              // 3.81dBm
    .pRegOverride = pOverrides,
    //.centerFreq = 0x0364, // modified (default: 0x0393)
    //.intFreq = 0x8000,
    //.loDivider = 0x05,
    //.pRegOverrideTxStd = pOverridesTxStd,
    //.pRegOverrideTx20 = pOverridesTx20
};


/***** Defines *****/

/***** Prototypes *****/

/***** Variable declarations *****/

/***** Function definitions *****/

static rfc_dataEntryGeneral_t* currentDataEntry;
static uint8_t packetLength;
static uint8_t* packetDataPointer;

char    packet[128];
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

void callback(RF_Handle h, RF_CmdHandle ch, RF_EventMask e)
{
    int     n;

    if (e & RF_EventRxEntryDone)
    {

        /* Get current unhandled data entry */
        currentDataEntry = RFQueue_getDataEntry();

        packetLength      = *(uint8_t*)(&currentDataEntry->data) & 0x7F;
        packetDataPointer = (uint8_t*)(&currentDataEntry->data + 1);

        memcpy(packet, packetDataPointer, (packetLength + 1));

        RFQueue_nextEntry();
    }
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
    int         n, m, k, i;
    char        input;

    char        UART_buf[256];
    int         UART_p;

    static  double  centor_freq = 2440, centor_freq_bck = 0;
    static  int     bandwide = 80, rssi_avg = 1;
    static  float   span = 80;
    int             MAX_sample = 1000;
    char            sampled_rssi[10000];
    double          set_freq;
    float           step_freq;
    unsigned int    t_start, t_end;
    int             FreqBand, Band_OK;

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
    uartParams.baudRate = 115200;
    uartParams.readTimeout =  10;            // 10=180uS, 入力有の場合=7uS
    //uartParams.readTimeout = 100;            // 100=1068uS, 入力有の場合=7uS
    uart = UART_open(Board_UART0, &uartParams);
    sprintf(str, "SpectrumAnalyzer:\r\n");
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
    // Rx Queue 設定
    //----------------------------------------------------------------
    if(RFQueue_defineQueue(&dataQueue, rxDataEntryBuffer, sizeof(rxDataEntryBuffer), 2, 128)) {
        /* Failed to allocate space for all data entries */
        while(1);
    }

    RF_cmdPropRx.pQueue = &dataQueue;
    RF_cmdPropRx.rxConf.bAutoFlushIgnored = 1;
    RF_cmdPropRx.rxConf.bAutoFlushCrcErr = 1;
    RF_cmdPropRx.maxPktLen = 30;
    RF_cmdPropRx.pktConf.bRepeatOk = 1;
    RF_cmdPropRx.pktConf.bRepeatNok = 1;

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
                // Centor freq
                //-------------------------------------------------------
                if(strncmp(UART_buf, "freq", 4) == 0) {
                    for(n = 0; CharCode[UART_buf[n]] != 0 && n < 8; n++);
                    for(     ; UART_buf[n] == ' ' || UART_buf[n] == '\t'; n++);
                    for(m = n; CharCode[UART_buf[m]] != 0; m++);
                    UART_buf[m] = 0;
                    sscanf(&UART_buf[n], "%lf", &centor_freq);
                    sprintf(str, "ACK_freq[%7.4lf]\r\n", centor_freq);
                    UART_write(uart, str, strlen(str));
                }
                //-------------------------------------------------------
                // bandwideth
                //-------------------------------------------------------
                if(strncmp(UART_buf, "bw", 2) == 0) {
                    for(n = 0; CharCode[UART_buf[n]] != 0 && n < 8; n++);
                    for(     ; UART_buf[n] == ' ' || UART_buf[n] == '\t'; n++);
                    for(m = n; CharCode[UART_buf[m]] != 0; m++);
                    UART_buf[m] = 0;
                    sscanf(&UART_buf[n], "%d", &bandwide);
                    sprintf(str, "ACK_bw[%d]\r\n", bandwide);
                    UART_write(uart, str, strlen(str));
                }
                //-------------------------------------------------------
                // span
                //-------------------------------------------------------
                if(strncmp(UART_buf, "span", 4) == 0) {
                    for(n = 0; CharCode[UART_buf[n]] != 0 && n < 8; n++);
                    for(     ; UART_buf[n] == ' ' || UART_buf[n] == '\t'; n++);
                    for(m = n; CharCode[UART_buf[m]] != 0; m++);
                    UART_buf[m] = 0;
                    sscanf(&UART_buf[n], "%f", &span);
                    sprintf(str, "ACK_span[%3.1f]\r\n", span);
                    UART_write(uart, str, strlen(str));
                }
                //-------------------------------------------------------
                // RSSI Avaraging
                //-------------------------------------------------------
                if(strncmp(UART_buf, "rssi", 4) == 0) {
                    for(n = 0; CharCode[UART_buf[n]] != 0 && n < 8; n++);
                    for(     ; UART_buf[n] == ' ' || UART_buf[n] == '\t'; n++);
                    for(m = n; CharCode[UART_buf[m]] != 0; m++);
                    UART_buf[m] = 0;
                    sscanf(&UART_buf[n], "%d", &rssi_avg);
                    sprintf(str, "ACK_rssi[%d]\r\n", rssi_avg);
                    UART_write(uart, str, strlen(str));
                }
                //-------------------------------------------------------
                // start コマンド   [861_1054 2360_2500]
                //-------------------------------------------------------
                set_freq   = centor_freq - (span / 2);
                step_freq = span / MAX_sample;

                if(strncmp(UART_buf, "stafs", 5) == 0) {
                    //-------------------------------------------------------
                    // RF_Open
                    //-------------------------------------------------------
                    if(centor_freq >= 861 && centor_freq <= 1054) {
                        RF_cmdPropRadioDivSetup.rxBw = bandwide;
                        //RF_cmdPropRadioDivSetup.rxBw = 65;
                        rfHandle = RF_open(&rfObject, &RF_prop, (RF_RadioSetup*)&RF_cmdPropRadioDivSetup, &rfParams);
                        FreqBand = 1;
                    }
                    if(centor_freq >= 2360 && centor_freq <= 2500) {
                        RF_cmdPropRadioSetup.rxBw = bandwide;
                        //RF_cmdPropRadioSetup.rxBw = 64;
                        rfHandle = RF_open(&rfObject, &RF_prop, (RF_RadioSetup*)&RF_cmdPropRadioSetup, &rfParams);
                        FreqBand = 2;
                    }
                    //-------------------------------------------------------
                    // Timer Start
                    //-------------------------------------------------------
                    GPIO_write(Board_GPIO1, 1);                 // Green LED ON
                    GPTimerCC26XX_start(hTimer);
                    t_start = GPTimerCC26XX_getValue(hTimer);
                    //-------------------------------------------------------
                    // Frequency スイープ　RSSI　測定
                    //-------------------------------------------------------
                    for(k = 0; k < MAX_sample; k++) {
                        //-------------------------------------------------------
                        //  RF_Quie フラッシュ(これが無いと動作しない)
                        //-------------------------------------------------------
                        RF_flushCmd(rfHandle, RF_CMDHANDLE_FLUSH_ALL, 1);
                        //-------------------------------------------------------
                        // Frequency 設定　バンド幅を外れた場合は センター周波数
                        //-------------------------------------------------------
                        if(FreqBand == 1) {
                            if(set_freq >= 861 && set_freq <= 1054) {
                                Set_Freq_Cmd(set_freq);
                                Band_OK = 1;
                            } else {
                                Set_Freq_Cmd(centor_freq);
                                Band_OK = 0;
                            }
                        }
                        if(FreqBand == 2) {
                            if(set_freq >= 2360 && set_freq <= 2500) {
                                Set_Freq_Cmd(set_freq);
                                Band_OK = 1;
                            } else {
                                Set_Freq_Cmd(centor_freq);
                                Band_OK = 0;
                            }
                        }
                        //-------------------------------------------------------
                        // Rx スタート
                        //-------------------------------------------------------
                        RF_EventMask terminationReason = RF_postCmd(rfHandle, (RF_Op*)&RF_cmdPropRx, RF_PriorityNormal, &callback, RF_EventRxEntryDone);
                        //RF_EventMask terminationReason = RF_postCmd(rfHandle, (RF_Op*)&RF_cmdPropRx, RF_PriorityNormal, &callback, RF_EventCmdDone );
                        //-------------------------------------------------------
                        // RSSI 測定
                        // RxがスタートするまでRSSI値-128が出る。Rxスタート後スタート時間の1.2倍待つ
                        //-------------------------------------------------------
                        i = RF_getRssi(rfHandle);
                        for(n = 0; n < 1000 && i == -128; n++) {
                            i = RF_getRssi(rfHandle);
                        }
                        for(m = 0; m < n * 1.2; m++) {
                        //for(m = 0; m < n * 2.0; m++) {
                            i = RF_getRssi(rfHandle);
                        }
                        if(Band_OK == 1) {
                            for(n = 0, i = 0; n < rssi_avg; n++) {
                                i += RF_getRssi(rfHandle);
                            }
                            sampled_rssi[k] = i / rssi_avg;
                        } else {
                            sampled_rssi[k] = -128;
                        }
                        //-------------------------------------------------------
                        // Next Frequency
                        //-------------------------------------------------------
                        set_freq += step_freq;
                    }
                    //-------------------------------------------------------
                    // Timer Stop
                    //-------------------------------------------------------
                    t_end = GPTimerCC26XX_getValue(hTimer);
                    GPIO_write(Board_GPIO1, 0);
                    GPTimerCC26XX_stop(hTimer);
                    //-------------------------------------------------------
                    //  RF_Quie フラッシュ
                    //-------------------------------------------------------
                    RF_flushCmd(rfHandle, RF_CMDHANDLE_FLUSH_ALL, 1);
                    //-------------------------------------------------------
                    //  RF_Close
                    //-------------------------------------------------------
                    RF_close(rfHandle);
                    //-------------------------------------------------------
                    // Result Tx
                    //-------------------------------------------------------
                    UART_write(uart, sampled_rssi, MAX_sample);
                    sprintf(str, "ElapsedTime=%d\r\n", t_end - t_start);
                    UART_write(uart, str, strlen(str));
                }
                if(strncmp(UART_buf, "stazs", 5) == 0) {
                    if(centor_freq != centor_freq_bck) {
                        centor_freq_bck = centor_freq;
                        //-------------------------------------------------------
                        // RF_Open
                        //-------------------------------------------------------
                        if(centor_freq >= 861 && centor_freq <= 1054) {
                            RF_cmdPropRadioDivSetup.rxBw = bandwide;
                            rfHandle = RF_open(&rfObject, &RF_prop, (RF_RadioSetup*)&RF_cmdPropRadioDivSetup, &rfParams);
                            FreqBand = 1;
                        }
                        if(centor_freq >= 2360 && centor_freq <= 2500) {
                            RF_cmdPropRadioSetup.rxBw = bandwide;
                            rfHandle = RF_open(&rfObject, &RF_prop, (RF_RadioSetup*)&RF_cmdPropRadioSetup, &rfParams);
                            FreqBand = 2;
                        }
                        //-------------------------------------------------------
                        //  RF_Quie フラッシュ
                        //-------------------------------------------------------
                        RF_flushCmd(rfHandle, RF_CMDHANDLE_FLUSH_ALL, 1);
                        //-------------------------------------------------------
                        // Frequency 設定
                        //-------------------------------------------------------
                        Set_Freq_Cmd(centor_freq);
                        //-------------------------------------------------------
                        // Rx スタート
                        //-------------------------------------------------------
                        RF_EventMask terminationReason = RF_postCmd(rfHandle, (RF_Op*)&RF_cmdPropRx, RF_PriorityNormal, &callback, RF_EventRxEntryDone);
                    }
                    //-------------------------------------------------------
                    // Timer Start
                    //-------------------------------------------------------
                    GPTimerCC26XX_start(hTimer);
                    GPIO_write(Board_GPIO0, 0);
                    GPIO_write(Board_GPIO1, 0);
                    t_start = GPTimerCC26XX_getValue(hTimer);
                    //-------------------------------------------------------
                    // RSSI 測定
                    //-------------------------------------------------------
                    for(k = 0, m = 0; k < MAX_sample; k++) {
                        if(k % 100 == 0) {
                            if(m == 0) {
                                m = 1;
                                GPIO_write(Board_GPIO0, 1);
                                GPIO_write(Board_GPIO1, 0);
                            } else {
                                m = 0;
                                GPIO_write(Board_GPIO0, 0);
                                GPIO_write(Board_GPIO1, 1);
                            }
                        }
                        for(n = 0, i = 0; n < rssi_avg; n++) {
                            i += RF_getRssi(rfHandle);
                        }
                        sampled_rssi[k] = i / rssi_avg;
                    }
                    //-------------------------------------------------------
                    // Timer Stop
                    //-------------------------------------------------------
                    t_end = GPTimerCC26XX_getValue(hTimer);
                    GPIO_write(Board_GPIO1, 0);
                    GPTimerCC26XX_stop(hTimer);
                    //-------------------------------------------------------
                    // Result Tx
                    //-------------------------------------------------------
                    UART_write(uart, sampled_rssi, MAX_sample);
                    sprintf(str, "ElapsedTime=%d\r\n", t_end - t_start);
                    UART_write(uart, str, strlen(str));
                }
                if(strncmp(UART_buf, "endzs", 5) == 0) {
                    centor_freq_bck = 0;
                    //-------------------------------------------------------
                    //  RF_Quie フラッシュ
                    //-------------------------------------------------------
                    RF_flushCmd(rfHandle, RF_CMDHANDLE_FLUSH_ALL, 1);
                    //-------------------------------------------------------
                    //  RF_Close
                    //-------------------------------------------------------
                    RF_close(rfHandle);
                    UART_write(uart, "ACK_endzs\r\n", 11);
                }
                UART_p = 0;
            }
        }
    }
}
