
/***** Includes *****/
/* For usleep() */
#include <unistd.h>
#include <stdint.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>

/* TI Drivers */
#include <ti/drivers/Power.h>
#include <ti/drivers/UART.h>
#include <ti/drivers/PIN.h>
#include <ti/drivers/rf/RF.h>
#include <ti/drivers/GPIO.h>

/* Board Header files */
#include "Board.h"

/* Application Header files */
#include "smartrf_settings.h"
#include "mac_settings.h"
#include "RFQueue.h"

#include <ti/drivers/NVS.h>

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
    .endTime = 1000000                  // 254mS
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


/***** Defines *****/

/***** Prototypes *****/

/***** Variable declarations *****/

static RF_Object rfObject;
static RF_Handle rfHandle;


/***** Function definitions *****/

char packet[128];

static rfc_dataEntryGeneral_t* currentDataEntry;
static uint8_t packetLength;
static uint8_t* packetDataPointer;

int MakeIeeePkt(int DstPAN, int DstMAC, int SrcPAN, int SrcMAC, int Seq, char *PayLoad, int Len)
{
    int     n;

    packet[ 0] = 0x41;
    packet[ 1] = 0x88;
    packet[ 2] = Seq & 0xFF;
    packet[ 3] =  DstPAN & 0x00FF;
    packet[ 4] = (DstPAN & 0xFF00) >> 8;
    packet[ 5] =  DstMAC & 0x00FF;
    packet[ 6] = (DstMAC & 0xFF00) >> 8;
    packet[ 7] =  SrcPAN & 0x00FF;
    packet[ 8] = (SrcPAN & 0xFF00) >> 8;
    packet[ 9] =  SrcMAC & 0x00FF;
    packet[10] = (SrcMAC & 0xFF00) >> 8;
    for(n = 0; n < Len; n++) {
        packet[11 + n] = PayLoad[n];
    }
    return(5 + 11 + n + 2);
}

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

char    CharHex[] = {
//   0   1   2   3   4   5   6   7   8   9   A   B   C   D   E   F
     0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0, // 0
     0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0, // 1
     0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0, // 2
     0,  1,  2,  3,  4,  5,  6,  7,  8,  9,  0,  0,  0,  0,  0,  0, // 3
     0, 10, 11, 12, 13, 14, 15,  0,  0,  0,  0,  0,  0,  0,  0,  0, // 4
     0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0, // 5
     0, 10, 11, 12, 13, 14, 15,  0,  0,  0,  0,  0,  0,  0,  0,  0, // 6
     0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0  // 7
};

//---------------------------------------------------------------------------
// Frequency Set
//---------------------------------------------------------------------------
Set_Freq_Cmd(double freq)
{
    RF_cmdFs.frequency = freq;
    RF_cmdFs.fractFreq = (freq - RF_cmdFs.frequency) / ((double)1.0 / 0xFFFF);
    RF_postCmd(rfHandle, (RF_Op*)&RF_cmdFs, RF_PriorityNormal, NULL, 0);
}

//---------------------------------------------------------------------------
// Common variables
//---------------------------------------------------------------------------
UART_Handle uart;
UART_Params uartParams;

char    str[256];
double  Freq = 2440.0;
int     nPower = 0;
int     DstMAC = 0x0000, SrcMAC = 0x0000;
int     ReceiveStart = 0;
int     UARTspeed = 0;
char    DumpMode = 'g';
volatile int    ReceiveISR = 0;
volatile int    PacketGood = 0, PacketError = 0, PacketReceive = 0;

//---------------------------------------------------------------------------
// Print Variables
//---------------------------------------------------------------------------
void PrintVariables()
{
    sprintf(str, "------------------------------------------------------------\r\n"); UART_write(uart, str, strlen(str));
    sprintf(str, "freq[%7.4lf]MHz\r\n", Freq); UART_write(uart, str, strlen(str));
    sprintf(str, "DstMAC[%04X]  ", DstMAC); UART_write(uart, str, strlen(str));
    sprintf(str, "SrcMAC[%04X]\r\n", SrcMAC); UART_write(uart, str, strlen(str));
    if(UARTspeed == 0 ) {
        sprintf(str, "UART speed[9600]BPS  "); UART_write(uart, str, strlen(str));
    } else {
        sprintf(str, "UART speed[230400]BPS  "); UART_write(uart, str, strlen(str));
    }
    sprintf(str, "Receive Packet Dump Mode[%c]\r\n", DumpMode); UART_write(uart, str, strlen(str));
    sprintf(str, "------------------------------------------------------------\r\n"); UART_write(uart, str, strlen(str));
}

//---------------------------------------------------------------------------
// Help
//---------------------------------------------------------------------------
void PrintHelp()
{
    sprintf(str, "------------------------------------------------------------\r\n"); UART_write(uart, str, strlen(str));
    sprintf(str, "fre  RF_Frequency(MHz)     float\r\n"); UART_write(uart, str, strlen(str));
    sprintf(str, "dstm Destnation MAC Addr   HEX(16bit)\r\n"); UART_write(uart, str, strlen(str));
    sprintf(str, "srcm Source     MAC Addr   HEX(16bit)\r\n"); UART_write(uart, str, strlen(str));
    sprintf(str, "sta  Start Receive\r\n"); UART_write(uart, str, strlen(str));
    sprintf(str, "sto  Stop  Receive(Display Statistics)\r\n"); UART_write(uart, str, strlen(str));
    sprintf(str, "cw   CW Generate           30__600Sec\r\n"); UART_write(uart, str, strlen(str));
    sprintf(str, "sp   UART speed l:9600 h:230400 BPS\r\n"); UART_write(uart, str, strlen(str));
    sprintf(str, "mo   Receive Packet Dump Mode\r\n"); UART_write(uart, str, strlen(str));
    sprintf(str, "     g (G:Good/E:Error)\r\n"); UART_write(uart, str, strlen(str));
    sprintf(str, "     o (Dump Packet Overview)\r\n"); UART_write(uart, str, strlen(str));
    sprintf(str, "     d (Dump Packet Details)\r\n"); UART_write(uart, str, strlen(str));
    sprintf(str, "sav  Parameters Save (Load when power is on)\r\n"); UART_write(uart, str, strlen(str));
    sprintf(str, "------------------------------------------------------------\r\n"); UART_write(uart, str, strlen(str));
}

void callback(RF_Handle h, RF_CmdHandle ch, RF_EventMask e)
{
    if (e & RF_EventRxEntryDone)
    {

        /* Get current unhandled data entry */
        currentDataEntry = RFQueue_getDataEntry();

        packetLength      = *(uint8_t*)(&currentDataEntry->data) & 0x7F;
        packetDataPointer = (uint8_t*)(&currentDataEntry->data + 1);

        memcpy(packet, packetDataPointer, (packetLength + 1));
        ReceiveISR = 1;

        RFQueue_nextEntry();
    }
}

//---------------------------------------------------------------------------
// DumpPacket
//---------------------------------------------------------------------------
int CheckPacket()
{
unsigned short int  dMAC, sMAC;

    PacketReceive++;
    dMAC  = packet[5];
    dMAC |= packet[6] << 8;
    sMAC  = packet[9];
    sMAC |= packet[10] << 8;
    if(DstMAC != 0) {
        if(DstMAC != dMAC) return(-1);
    }
    if(SrcMAC != 0) {
        if(SrcMAC != sMAC) return(-1);
    }
    if((packet[packetLength -1] & 0x80) == 0) {
        PacketGood++;
    } else {
        PacketError++;
    }
    return(1);
}

void DumpPacketDetail()
{
    int     n;

    if((packet[packetLength -1] & 0x80) == 0) {
        UART_write(uart, "<GO>", 4);
    } else {
        UART_write(uart, "<NG>", 4);
    }
    sprintf(str, " RSSI[%3d]", (signed char)packet[packetLength - 2]); UART_write(uart, str, strlen(str));
    sprintf(str, " FCF[%02X%02X] Seq[%02X] Dst[%02X%02X][%02X%02X] Src[%02X%02X][%02X%02X]\r\n",
            packet[1], packet[0], packet[2], packet[4], packet[3], packet[6], packet[5], packet[8], packet[7], packet[10], packet[9]);
    UART_write(uart, str, strlen(str));
    UART_write(uart, "  payload[", 10);
    for(n = 11; n < packetLength - 2; n++) {
        sprintf(str, "%02X ", packet[n]); UART_write(uart, str, strlen(str));
    }
    sprintf(str, "]\r\n"); UART_write(uart, str, strlen(str));
}

void DumpPacketOverview()
{
    if((packet[packetLength -1] & 0x80) == 0) {
        UART_write(uart, "<GO>", 4);
    } else {
        UART_write(uart, "<NG>", 4);
    }
    sprintf(str, "RSSI[%3d]dBm ", (signed char)packet[packetLength - 2]); UART_write(uart, str, strlen(str));
    sprintf(str, "Seq[%02X] SrcPAN[%02X%02X] SrcMAC[%02X%02X] Payload[%d]byte\r\n",
            packet[2], packet[8], packet[7], packet[10], packet[9], packetLength - 13);
    UART_write(uart, str, strlen(str));
}

//---------------------------------------------------------------------------
// MainThread
//---------------------------------------------------------------------------
void *mainThread(void *arg0)
{
    int         k, n, m;
    char        input;
    char        UART_buf[256];
    int         UART_p;

    /* Call driver init functions */
    GPIO_init();
    UART_init();

    sleep(1);
    GPIO_write(Board_GPIO0, 0);
    GPIO_write(Board_GPIO1, 0);
    sleep(1);

    /* Configure the radio for Proprietary mode */
    RF_Params rfParams;
    /* Create a UART with data processing off. */
    UART_Params_init(&uartParams);
    uartParams.writeDataMode = UART_DATA_BINARY;
    uartParams.readDataMode = UART_DATA_BINARY;
    uartParams.readReturnMode = UART_RETURN_FULL;
    uartParams.readEcho = UART_ECHO_OFF;
    uartParams.baudRate = 9600;
    uartParams.readTimeout =  10;            // 10=180uS, 入力有の場合=7uS
    //uartParams.readTimeout = 100;            // 100=1068uS, 入力有の場合=7uS

    //----------------------------------------------------------------
    // Non-Volatile Storage
    //----------------------------------------------------------------
    NVS_Handle      nvsRegion;
    NVS_Attrs       regionAttrs;
    uint_fast16_t   status;
    NVS_init();
    //-------------------------------------------------------
    // Parameters Load
    //-------------------------------------------------------
    nvsRegion = NVS_open(Board_NVS0, NULL);
    NVS_getAttrs(nvsRegion, &regionAttrs);
    status = NVS_read(nvsRegion, 0, str, 128);
    if(strncmp(str, "Freq=", 5) == 0) {
        sscanf(str, "Freq=%lf DstMAC=%04X SrcMAC=%04X", &Freq, &DstMAC, &SrcMAC);
    }
    NVS_close(nvsRegion);

    uart = UART_open(Board_UART0, &uartParams);

    PrintHelp();
    PrintVariables();

    UART_p = 0;

    RF_Params_init(&rfParams);

    if(RFQueue_defineQueue(&dataQueue, rxDataEntryBuffer, sizeof(rxDataEntryBuffer), 2, 128)) {
        /* Failed to allocate space for all data entries */
        while(1);
    }

    while (1) {
        //-------------------------------------------------------
        // UARTから入力されたコマンドを読込→実行
        //-------------------------------------------------------
        k = UART_read(uart, &input, 1);
        if(k != 0) {

            if(input == '\r' || input == '\n') {
                //UART_write(uart, UART_buf, UART_p);
                UART_write(uart, "\r\n", 2);
                UART_buf[UART_p] = 0;
                UART_p = 0;
                //-------------------------------------------------------
                // freq
                //-------------------------------------------------------
                if(strncmp(UART_buf, "freq", 3) == 0) {
                    for(n = 0; CharCode[UART_buf[n]] != 0 && n < 8; n++);
                    for(     ; UART_buf[n] == ' ' || UART_buf[n] == '\t'; n++);
                    for(m = n; CharCode[UART_buf[m]] != 0; m++);
                    UART_buf[m] = 0;
                    sscanf(&UART_buf[n], "%lf", &Freq);
                    PrintVariables();
                }
                //-------------------------------------------------------
                // Dst MAC
                //-------------------------------------------------------
                if(strncmp(UART_buf, "dstm", 4) == 0) {
                    for(n = 0; CharCode[UART_buf[n]] != 0 && n < 8; n++);
                    for(     ; UART_buf[n] == ' ' || UART_buf[n] == '\t'; n++);
                    DstMAC = 0;
                    DstMAC = CharHex[UART_buf[n    ]] << 12 | CharHex[UART_buf[n + 1]] << 8 |
                             CharHex[UART_buf[n + 2]] <<  4 | CharHex[UART_buf[n + 3]];
                    PrintVariables();
                }
                //-------------------------------------------------------
                // Src MAC
                //-------------------------------------------------------
                if(strncmp(UART_buf, "srcm", 4) == 0) {
                    for(n = 0; CharCode[UART_buf[n]] != 0 && n < 8; n++);
                    for(     ; UART_buf[n] == ' ' || UART_buf[n] == '\t'; n++);
                    SrcMAC = 0;
                    SrcMAC = CharHex[UART_buf[n    ]] << 12 | CharHex[UART_buf[n + 1]] << 8 |
                             CharHex[UART_buf[n + 2]] <<  4 | CharHex[UART_buf[n + 3]];
                    PrintVariables();
                }
                //-------------------------------------------------------
                // Start
                //-------------------------------------------------------
                if(strncmp(UART_buf, "sta", 3) == 0) {
                    if(ReceiveStart != 0) {
                        //-------------------------------------------------------
                        //  RF_Quie フラッシュ
                        //-------------------------------------------------------
                        RF_flushCmd(rfHandle, RF_CMDHANDLE_FLUSH_ALL, 1);

                        RF_close(rfHandle);
                        ReceiveStart = 0;
                    }
                    //-------------------------------------------------------
                    // RF_Open
                    //-------------------------------------------------------
                    rfHandle = RF_open(&rfObject, &RF_prop, (RF_RadioSetup*)&RF_cmdRadioSetup_HP, &rfParams);
                    RF_setTxPower(rfHandle, txPowerTable_2R4G[nPower].value);
                    //-------------------------------------------------------
                    // Frequency 設定
                    //-------------------------------------------------------
                    Set_Freq_Cmd(Freq);
                    //-------------------------------------------------------
                    // IEEE802.15.4 Rx
                    //-------------------------------------------------------
                    RF_cmdIeeeRx_kzk.pRxQ = &dataQueue;
                    RF_EventMask terminationReason = RF_postCmd(rfHandle, (RF_Op*)&RF_cmdIeeeRx_kzk,
                                                                RF_PriorityNormal, &callback,
                                                                RF_EventRxEntryDone);

                    PacketReceive = 0; PacketGood = 0; PacketError = 0;
                    ReceiveISR = 0;
                    ReceiveStart = 1;
                    UART_write(uart, "Receive Start\r\n", 15);
                }
                //-------------------------------------------------------
                // Stop
                //-------------------------------------------------------
                if(strncmp(UART_buf, "sto", 3) == 0) {
                    //-------------------------------------------------------
                    //  RF_Quie フラッシュ
                    //-------------------------------------------------------
                    RF_flushCmd(rfHandle, RF_CMDHANDLE_FLUSH_ALL, 1);

                    RF_close(rfHandle);
                    ReceiveStart = 0;
                    sprintf(str, "Receive[%d] Good[%d] Error[%d]\r\n", PacketReceive, PacketGood, PacketError);
                    UART_write(uart, str, strlen(str));
                }
                //-------------------------------------------------------
                // Send CW
                //-------------------------------------------------------
                if(strncmp(UART_buf, "cw", 2) == 0) {
                    if(ReceiveStart != 0) {
                        //-------------------------------------------------------
                        //  RF_Quie フラッシュ
                        //-------------------------------------------------------
                        RF_flushCmd(rfHandle, RF_CMDHANDLE_FLUSH_ALL, 1);

                        RF_close(rfHandle);
                        ReceiveStart = 0;
                    }
                    for(n = 0; CharCode[UART_buf[n]] != 0 && n < 8; n++);
                    for(     ; UART_buf[n] == ' ' || UART_buf[n] == '\t'; n++);
                    for(m = n; CharCode[UART_buf[m]] != 0; m++);
                    UART_buf[m] = 0;
                    sscanf(&UART_buf[n], "%d", &k);
                    if(k <=  30) k = 30;
                    if(k >= 600) k = 600;
                    //-------------------------------------------------------
                    // RF_Open
                    //-------------------------------------------------------
                    rfHandle = RF_open(&rfObject, &RF_prop, (RF_RadioSetup*)&RF_cmdRadioSetup_HP, &rfParams);
                    RF_setTxPower(rfHandle, txPowerTable_2R4G[nPower].value);
                    //-------------------------------------------------------
                    // Frequency 設定
                    //-------------------------------------------------------
                    Set_Freq_Cmd(Freq);
                    //-------------------------------------------------------
                    // CW送信
                    //-------------------------------------------------------
                    RF_cmdTxTest_kzk.endTime = k / 0.00000025;
                    RF_runCmd(rfHandle, (RF_Op*)&RF_cmdTxTest_kzk, RF_PriorityNormal, NULL, 0);
                    //-------------------------------------------------------
                    //  RF_Quie フラッシュ
                    //-------------------------------------------------------
                    RF_flushCmd(rfHandle, RF_CMDHANDLE_FLUSH_ALL, 1);
                    //-------------------------------------------------------
                    //  RF_Close
                    //-------------------------------------------------------
                    RF_close(rfHandle);
                    sprintf(str, "cw[%d]Sec END\r\n", k); UART_write(uart, str, strlen(str));
                }
                //-------------------------------------------------------
                // Help
                //-------------------------------------------------------
                if(UART_buf[0] == 'h' || UART_buf[0] == 'H' || UART_buf[0] == '?') {
                    PrintHelp();
                    PrintVariables();
                }
                //-------------------------------------------------------
                // UART Speed
                //-------------------------------------------------------
                if(strncmp(UART_buf, "speed", 2) == 0) {
                    for(n = 0; CharCode[UART_buf[n]] != 0 && n < 8; n++);
                    for(     ; UART_buf[n] == ' ' || UART_buf[n] == '\t'; n++);
                    if(UART_buf[n] == 'H' || UART_buf[n] == 'h') {
                        uartParams.baudRate = 230400;
                        UARTspeed = 1;
                    } else {
                        uartParams.baudRate = 9600;
                        UARTspeed = 0;
                    }
                    UART_close(uart);
                    uart = UART_open(Board_UART0, &uartParams);
                    PrintVariables();
                }
                //-------------------------------------------------------
                // Receive Packet Dump Mode
                //-------------------------------------------------------
                if(strncmp(UART_buf, "mode", 2) == 0) {
                    for(n = 0; CharCode[UART_buf[n]] != 0 && n < 8; n++);
                    for(     ; UART_buf[n] == ' ' || UART_buf[n] == '\t'; n++);
                    if(UART_buf[n] == 'g') DumpMode = 'g';
                    if(UART_buf[n] == 'o') DumpMode = 'o';
                    if(UART_buf[n] == 'd') DumpMode = 'd';
                    PrintVariables();
                }
                //-------------------------------------------------------
                // Parameters Save
                //-------------------------------------------------------
                if(strncmp(UART_buf, "sav", 3) == 0) {
                    nvsRegion = NVS_open(Board_NVS0, NULL);
                    NVS_getAttrs(nvsRegion, &regionAttrs);
                    status = NVS_erase(nvsRegion, 0, regionAttrs.sectorSize);
                    sprintf(str, "Freq=%lf DstMAC=%04X SrcMAC=%04X", Freq, DstMAC, SrcMAC);
                    status = NVS_write(nvsRegion, 0, str, strlen(str) + 1, NVS_WRITE_POST_VERIFY);
                    NVS_close(nvsRegion);
                    UART_write(uart, "Parameters Save\r\n", 17);
                }
            } else {
                UART_buf[UART_p] = input;
                UART_p++;
            }
        }
        if(ReceiveStart != 0) {
            if(ReceiveISR != 0) {
                GPIO_write(Board_GPIO1, 1);
                if(CheckPacket() > 0) {
                    switch(DumpMode) {
                        case    'g':
                            if((packet[packetLength -1] & 0x80) == 0) {
                                UART_write(uart, "G", 1);
                            } else {
                                UART_write(uart, "E", 1);
                            }
                            break;
                        case    'o':
                            DumpPacketOverview();
                            break;
                        case    'd':
                            DumpPacketDetail();
                            break;
                    }
                }
                ReceiveISR = 0;
                GPIO_write(Board_GPIO1, 0);
            }
        }
    }
}
