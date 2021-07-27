//---------------------------------------------------------------------------
#include	<vcl.h>
#include	<stdio.h>
#include	<Windows.h>
#include	<ustring.h>
#include	<time.h>
#include	<math.h>
#include	<dirent.h>
#include	<sys/stat.h>
#include	<locale.h>

#pragma hdrstop

#include "Unit1.h"

char	COMlist[32][64];
HANDLE	ComCC = INVALID_HANDLE_VALUE;

#define	USB_BUF_SZ	8192
unsigned char	USB_RxBuf[USB_BUF_SZ];

char	ustr[1024];
volatile int	Start = 0;
volatile int	Busy  = 0;

long	StartTime;

//---------------------------------------------------------------------------
#pragma package(smart_init)
#pragma resource "*.dfm"
TForm1 *Form1;
//---------------------------------------------------------------------------
__fastcall TForm1::TForm1(TComponent* Owner)
	: TForm(Owner)
{
}

//--------------------------------------------------------------------------
// ユニーク文字列検索
//--------------------------------------------------------------------------
int str_unq_cmp(char *stra, char *strb)
{
	int	k, n, m;

	n = strlen(strb);
	m = strlen(stra);
	for(k = 0; k < m; k++) {
		if(strncmp(&stra[k], strb, n) == 0) {
			if(m >= (k + n)) return(k + n);
			return(0);
		}
	}
	return(0);
}

//---------------------------------------------------------------------------
// 数字→整数変換
//---------------------------------------------------------------------------
int str_int(char *str)
{
	char	buf[32];
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
                for(k = 1, n = 0; buf[k] != 0; k++) if((buf[k] & 0x30) == 0x30) n = (n * 10) + (buf[k] & 0x0F);
                n *= -1;
        } else	for(k = 0, n = 0; buf[k] != 0; k++) if((buf[k] & 0x30) == 0x30)  n = (n * 10) + (buf[k] & 0x0F);
        return(n);
}

//--------------------------------------------------------------------------
// Event実行
//--------------------------------------------------------------------------
void DoEvents()
{
	int	k;
	MSG	msg;

	for(k = 0; ::PeekMessage(&msg, NULL, 0, 0, PM_REMOVE) != 0 && k < 100; k++) {
		::TranslateMessage(&msg);
		::DispatchMessage(&msg);
	}
}

//--------------------------------------------------------------------------
// PCに接続されているCOMポート名、デバイス名を得る
//--------------------------------------------------------------------------
void get_COM_port()
{
	HKEY	hKey = NULL;
	DWORD	dwCount = 0;
	TCHAR	cNameBuff[256];
unsigned char	byValueBuff[256];
	DWORD	dwNameBuffSize = 0;
	DWORD	dwValueBuffSize = 0;
	DWORD	dwType = 0;
	wchar_t	wbuf[256];
	char	buf[256];
	int	k;

	for(k = 0; k < 16; k++) COMlist[k][0] = 0;
	mbstowcs(&wbuf[0], "HARDWARE\\DEVICEMAP\\SERIALCOMM",  31);
	if(RegOpenKeyEx(HKEY_LOCAL_MACHINE, wbuf, 0, KEY_READ, &hKey) != ERROR_SUCCESS) {
		return;
	}
	if(RegQueryInfoKey(hKey, NULL, NULL, NULL, NULL, NULL, NULL,
				&dwCount, NULL, NULL, NULL, NULL) != ERROR_SUCCESS) {
		return;
	}
	for(k = 0; k < (int)dwCount && k < 32; k++) {
		dwNameBuffSize = sizeof(cNameBuff);
		dwValueBuffSize = sizeof(byValueBuff);
		if(RegEnumValue(hKey, k, &cNameBuff[0], &dwNameBuffSize, NULL,
			&dwType, &byValueBuff[0], &dwValueBuffSize) != ERROR_SUCCESS ) {
			ShowMessage("Error：[RegEnumValue()]");
			return;
		}
		wcstombs(&buf[0], (wchar_t *)byValueBuff, 256);
		sprintf(COMlist[k], "%s:", buf);
		wcstombs(&buf[0], (wchar_t *)cNameBuff, 256);
		strcat(COMlist[k], buf);
	}
}

//--------------------------------------------------------------------------
// VCPポート検索
//--------------------------------------------------------------------------
void GetVcpPort(TObject *Sender)
{
	int	k;
	wchar_t	wbuf[1000];

	get_COM_port();
	Form1->VcpPort->Items->Clear();
	for(k = 0; k < 32 && COMlist[k][0] != 0; k++) {
		Form1->VcpPort->Items->Add(COMlist[k]);
	}
	Form1->VcpPort->Text = "Select VCP Port";
}

//--------------------------------------------------------------------------
// USBオープン
//--------------------------------------------------------------------------
HANDLE Open_USB(char *ComNam, int Speed)
{
static	HANDLE	hCom;
	int	k;
	char	str[256];
	wchar_t	nam[64];
	DCB	dcb;

	sprintf(str, "\\\\.\\%s", ComNam);
	mbstowcs(nam, str, sizeof(str));
	// シリアルポートを開ける
	hCom = CreateFile(
		nam,					// シリアルポートの文字列
		GENERIC_READ | GENERIC_WRITE,		// アクセスモード：読み書き
		0,					// 共有モード：他からはアクセス不可
		0,					// セキュリティ属性：ハンドル継承せず
		OPEN_EXISTING,				// 作成フラグ
		FILE_ATTRIBUTE_NORMAL,			// 属性
		0					// テンプレートのハンドル
		);
	if(hCom == INVALID_HANDLE_VALUE) {
		sprintf(str, "Error Open_USB[%s]", ComNam);
		ShowMessage(str);
		return(INVALID_HANDLE_VALUE);
	}
	PurgeComm(hCom, PURGE_TXABORT | PURGE_RXABORT | PURGE_TXCLEAR | PURGE_RXCLEAR);
	// 通信属性を設定する
	GetCommState(hCom, &dcb);			// DCB を取得
	// データ
	dcb.BaudRate = Speed;
	dcb.fBinary = TRUE;
	dcb.ByteSize = 8;
	dcb.fParity = NOPARITY;
	dcb.StopBits = ONESTOPBIT;
	// ハードウエア・フロー制御
	dcb.fOutxCtsFlow = FALSE;			// CTSハードウェアフロー制御：CTS制御を使用しない場合はFLASEを指定
							// CTS 制御をする場合はTRUEを指定してCTS信号を監視します
	dcb.fOutxDsrFlow = FALSE;			// DSRハードウェアフロー制御：使用しない場合はFALSEを指定
	dcb.fDtrControl = DTR_CONTROL_DISABLE;		// DTR有効/無効：　無効なら　DTR_CONTROL_DISABLE;ISABLE
	dcb.fRtsControl = RTS_CONTROL_DISABLE;		// RTS制御：RTS制御をしない場合はRTS_CONTROL_DISABLEを指定
	// ソフトウェアフロー制御
	dcb.fOutX = FALSE;				// 送信時XON/OFF制御の有無：　なし→FLALSE
	dcb.fInX = FALSE;				// 受信時XON/XOFF制御の有無：なし→FALSE
	dcb.fTXContinueOnXoff = TRUE;			// 受信バッファー満杯＆XOFF受信後の継続送信可否：送信可→TRUE
	dcb.XonLim = 512;				// XONが送られるまでに格納できる最小バイト数：
	dcb.XoffLim = 512;				// XOFFが送られるまでに格納できる最小バイト数：
	dcb.XonChar = 0x11;				// 送信時XON文字 ( 送信可：ビジィ解除 ) の指定：
 							// 一般に、XON文字として11H ( デバイス制御１：DC1 )がよく使われます
	dcb.XoffChar = 0x13;				// XOFF文字（送信不可：ビジー通告）の指定：なし→FALSE
							// 一般に、XOFF文字として13H ( デバイス制御3：DC3 )がよく使われます
	//その他
	dcb.fNull = FALSE;				// NULLバイトの破棄： 破棄する→TRUE
	dcb.fAbortOnError = TRUE;			// エラー時の読み書き操作終了：　終了する→TRUE
	dcb.fErrorChar = FALSE;				// パリティエラー発生時のキャラクタ（ErrorChar）置換：　なし→FLALSE
	dcb.ErrorChar = 0x00;				// パリティエラー発生時の置換キャラクタ
	dcb.EofChar = 0x03;				// データ終了通知キャラクタ　：　一般に0x03(ETX)がよく使われます。
	dcb.EvtChar = 0x02;				// イベント通知キャラクタ ：　一般に0x02(STX)がよく使われます

	SetCommState(hCom, &dcb); 			// DCB を設定
	Sleep(60);
	return(hCom);
}

//---------------------------------------------------------------------------
// USBに送信
//---------------------------------------------------------------------------
int USB_Tx(HANDLE hCom1, unsigned char *str, int len)
{
	DWORD	dwWritten; 	// ポートへ書き込んだバイト数

	WriteFile(hCom1, str, len, &dwWritten, NULL);
	if((int)dwWritten != len) return(-1);
	FlushFileBuffers(hCom1);
	return(1);
}

//---------------------------------------------------------------------------
// USBから受信
//---------------------------------------------------------------------------
int USB_Rx(HANDLE hCom1, unsigned int Wait)
{
	int	k;
	DWORD	dwErrors;	// エラー情報
	COMSTAT	ComStat;	// デバイスの状態
static	DWORD	dwCount;	// 受信データのバイト数
	DWORD	dwRead;		// ポートから読み出したバイト数

	OVERLAPPED ovlpd;

	USB_RxBuf[0] = 0;
	Sleep(5);
	for(k = 0; k < 16; k++) {
		ovlpd.Offset = 0;
		ovlpd.OffsetHigh = 0;
		ovlpd.hEvent = NULL;
		dwCount = 0;
		ClearCommError(hCom1, &dwErrors, &ComStat);
		dwCount = ComStat.cbInQue;
		if(dwCount >= Wait) {
			if(dwCount > USB_BUF_SZ) {
				ShowMessage("ERROR:USB_Rx Over");
				return(-1);
			}
			ReadFile(hCom1, USB_RxBuf, dwCount, &dwRead, &ovlpd);
			if(dwCount != dwRead) {
				ShowMessage("ERROR:USB_Rx Receive");
				return(-1);
			}
			return(dwCount);
		}
		Sleep(50);
	}
	if(dwCount != 0) {
		ReadFile(hCom1, USB_RxBuf, dwCount, &dwRead, &ovlpd);
		if(dwCount != dwRead) {
			ShowMessage("ERROR:USB_Rx Receive");
			return(-1);
		}
		return(dwCount);
	}
	return(0);
}

//---------------------------------------------------------------------------
// 選択候補表示
//---------------------------------------------------------------------------
void SetSelVal()
{
	int		n, Max;

	Form1->CenterFreq->Items->Clear();
	Form1->Power->Items->Clear();
	if(Form1->F2R4G->Checked == True) {
		for(n = 2360;  n <= 2500; n += 5) {
			sprintf(ustr, "%2.5f", float(n));
			Form1->CenterFreq->Items->Add(ustr);
		}
		Form1->CenterFreq->Text = "2440.00000";
		sprintf(ustr, "%s", (AnsiString)UTF8Encode(Form1->BoardSel->Text));
		if(strcmp(ustr, "CC1352P-2") == 0) {
			Max = 19;
		} else {
			Max = 4;
		}
		for(n = 0; n <= Max; n++) {
			Form1->Power->Items->Add(n);
		}
		Form1->Power->Text = "0";


	} else {
		for(n = 865;  n <= 1050; n += 5) {
			sprintf(ustr, "%2.5f", float(n));
			Form1->CenterFreq->Items->Add(ustr);
		}
		Form1->CenterFreq->Text = "920.00000";
		sprintf(ustr, "%s", (AnsiString)UTF8Encode(Form1->BoardSel->Text));
		if(strcmp(ustr, "CC1352P1") == 0) {
			Max = 20;
		} else {
			Max = 10;
		}
		for(n = 0; n <= Max; n++) {
			Form1->Power->Items->Add(n);
		}
		Form1->Power->Text = "0";
	}
	Form1->OutputTime->Items->Clear();
	for(n = 1; n < 100; n++) {
		sprintf(ustr, "%2.2f", float(n));
		Form1->OutputTime->Items->Add(ustr);
	}
	Form1->OutputTime->Text = "3.00";
}

//---------------------------------------------------------------------------
// Form1 ウインドウ・クリエイト
//---------------------------------------------------------------------------
void __fastcall TForm1::F1_Create(TObject *Sender)
{
	int		n;

	GetVcpPort(Sender);

	Form1->BoardSel->Items->Clear();
	Form1->BoardSel->Items->Add("CC1352P1");
	Form1->BoardSel->Items->Add("CC1352P-2");
	Form1->BoardSel->Text = "CC1352P1";

	SetSelVal();


	time(&StartTime);
}

//---------------------------------------------------------------------------
// 周波数帯選択
//---------------------------------------------------------------------------
void __fastcall TForm1::F900MClick(TObject *Sender)
{
	SetSelVal();
}

void __fastcall TForm1::F2R4GClick(TObject *Sender)
{
	SetSelVal();
}

//---------------------------------------------------------------------------
// PortSearch
//---------------------------------------------------------------------------
void __fastcall TForm1::PortSearchClick(TObject *Sender)
{
	GetVcpPort(Sender);
}

//---------------------------------------------------------------------------
// Start
//---------------------------------------------------------------------------
char RxDat[8192];

void __fastcall TForm1::M_StartClick(TObject *Sender)
{
	int		k, n, m, RxN, nTime;
	char	buf[256];
	float	SweepTime;
	TPoint	points_buf[2];

	double	Freq;
	float	oTime;
	int		Power;

	Form1->Cmnt->Text = "";
	DoEvents();

	//--------------------------------------------------
	// VCP Poart オープン
	//--------------------------------------------------
	if(ComCC == INVALID_HANDLE_VALUE) {
		sprintf(ustr, "%s", (AnsiString)UTF8Encode(Form1->VcpPort->Text));
		for(k = 0; ustr[k] != ':' && k < 128; k++); ustr[k] = 0;
		ComCC = Open_USB(ustr, 115200);
		if(ComCC == INVALID_HANDLE_VALUE) {
			CloseHandle(ComCC);
			ComCC = INVALID_HANDLE_VALUE;
			return;
		}
	}
	//while(USB_Rx(ComCC, 10) > 0);
	//--------------------------------------------------
	//　周波数
	//--------------------------------------------------
	sprintf(ustr, "%s", (AnsiString)UTF8Encode(Form1->CenterFreq->Text));
	sscanf(ustr, "%lf", &Freq);
	sprintf(buf, "freq %s\n", ustr);
	USB_Tx(ComCC, buf, strlen(buf));
	k = USB_Rx(ComCC, 16);
	Form1->Cmnt->Text = (char *)USB_RxBuf;
	if(str_unq_cmp(USB_RxBuf, "ACK_freq[") < 1) {
		sprintf(ustr, "Error [%s]", buf);
		ShowMessage(ustr);
		CloseHandle(ComCC);
		ComCC = INVALID_HANDLE_VALUE;
		return;
	}
	//--------------------------------------------------
	//　変調
	//--------------------------------------------------
	if(Form1->Mod->Checked == True) {
		USB_Tx(ComCC, "mod 1\n", 6);
	} else {
		USB_Tx(ComCC, "mod 0\n", 6);
	}
	k = USB_Rx(ComCC, 8);
	if(str_unq_cmp(USB_RxBuf, "ACK_mod[") < 1) {
		ShowMessage("Error [mod]");
		CloseHandle(ComCC);
		ComCC = INVALID_HANDLE_VALUE;
		return;
	}
	//--------------------------------------------------
	// Ｐｏｗｅｒ
	//--------------------------------------------------
	sprintf(ustr, "%s", (AnsiString)UTF8Encode(Form1->Power->Text));
	sscanf(ustr, "%d", &Power);
	sprintf(buf, "pow %s\n", ustr);
	USB_Tx(ComCC, buf, strlen(buf));
	k = USB_Rx(ComCC, 10);
	//Form1->Cmnt->Text = (char *)USB_RxBuf;
	if(str_unq_cmp(USB_RxBuf, "ACK_pow[") < 1) {
		sprintf(ustr, "Error [%s]", buf);
		ShowMessage(ustr);
		CloseHandle(ComCC);
		ComCC = INVALID_HANDLE_VALUE;
		return;
	}
	//--------------------------------------------------
	// 出力時間
	//--------------------------------------------------
	sprintf(ustr, "%s", (AnsiString)UTF8Encode(Form1->OutputTime->Text));
	sscanf(ustr, "%f", &oTime);
	sprintf(buf, "tim %s\n", ustr);
	USB_Tx(ComCC, buf, strlen(buf));
	k = USB_Rx(ComCC, 10);
	//Form1->Cmnt->Text = (char *)USB_RxBuf;
	if(str_unq_cmp(USB_RxBuf, "ACK_tim[") < 1) {
		sprintf(ustr, "Error [%s]", buf);
		ShowMessage(ustr);
		CloseHandle(ComCC);
		ComCC = INVALID_HANDLE_VALUE;
		return;
	}
	Form1->M_Start->Enabled = False;
	DoEvents();
	//--------------------------------------------------
	// Ｓｔａｒｔ
	//--------------------------------------------------
	USB_Tx(ComCC, "sta\n", 4);
	for(n = 0; n < int(oTime * 1000); n += 500) {
		Sleep(500);
		sprintf(ustr, "ElapsedTime=%2.1f", (float)n / 1000);
		Form1->Cmnt->Text = ustr;
		DoEvents();
	}
	k = USB_Rx(ComCC, 20);
	if(str_unq_cmp(USB_RxBuf, "Output End") < 1) {
		ShowMessage("Error [sta]");
	}
	Form1->Cmnt->Text = "Output End";
	Form1->M_Start->Enabled = True;
}

//---------------------------------------------------------------------------
// Ｌｏａｄ
//---------------------------------------------------------------------------
void __fastcall TForm1::LoadClick(TObject *Sender)
{
	int	 k;
	double	Freq;
	float	oTime;
	int		Power;

	//--------------------------------------------------
	// VCP Poart オープン
	//--------------------------------------------------
	if(ComCC == INVALID_HANDLE_VALUE) {
		sprintf(ustr, "%s", (AnsiString)UTF8Encode(Form1->VcpPort->Text));
		for(k = 0; ustr[k] != ':' && k < 128; k++); ustr[k] = 0;
		ComCC = Open_USB(ustr, 115200);
		if(ComCC == INVALID_HANDLE_VALUE) {
			CloseHandle(ComCC);
			ComCC = INVALID_HANDLE_VALUE;
			return;
		}
	}
	USB_Tx(ComCC, "rea\n", 4);
	k = USB_Rx(ComCC, 45);
	if(str_unq_cmp(USB_RxBuf, "ACK_rea[") < 1) {
		ShowMessage("Error [rea]");
		CloseHandle(ComCC);
		ComCC = INVALID_HANDLE_VALUE;
		return;
	}
	//Form1->Cmnt->Text = (char *)USB_RxBuf;
	sscanf(USB_RxBuf, "ACK_rea[freq=%lf pow=%d time=%f]", &Freq, &Power, &oTime);
	if(Freq >= 2360) {
		Form1->F2R4G->Checked = True;
	} else {
		Form1->F900M->Checked = True;
	}
	SetSelVal();
	sprintf(ustr, "%2.5lf", Freq);
	Form1->CenterFreq->Text = ustr;
	sprintf(ustr, "%d", Power);
	Form1->Power->Text = ustr;
	sprintf(ustr, "%2.2f", oTime);
	Form1->OutputTime->Text = ustr;
}

//---------------------------------------------------------------------------
// Ｓａｖ
//---------------------------------------------------------------------------
void __fastcall TForm1::SaveClick(TObject *Sender)
{
	int	 k;
	double	Freq;
	float	oTime;
	int		Power;

	//--------------------------------------------------
	// VCP Poart オープン
	//--------------------------------------------------
	if(ComCC == INVALID_HANDLE_VALUE) {
		sprintf(ustr, "%s", (AnsiString)UTF8Encode(Form1->VcpPort->Text));
		for(k = 0; ustr[k] != ':' && k < 128; k++); ustr[k] = 0;
		ComCC = Open_USB(ustr, 115200);
		if(ComCC == INVALID_HANDLE_VALUE) {
			CloseHandle(ComCC);
			ComCC = INVALID_HANDLE_VALUE;
			return;
		}
	}
	USB_Tx(ComCC, "sav\n", 4);
	k = USB_Rx(ComCC, 45);
	Form1->Cmnt->Text = (char *)USB_RxBuf;
	if(str_unq_cmp(USB_RxBuf, "ACK_sav[") < 1) {
		ShowMessage("Error [rea]");
		CloseHandle(ComCC);
		ComCC = INVALID_HANDLE_VALUE;
	}
}
//---------------------------------------------------------------------------

