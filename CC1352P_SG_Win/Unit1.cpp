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
// ���j�[�N�����񌟍�
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
// �����������ϊ�
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
// Event���s
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
// PC�ɐڑ�����Ă���COM�|�[�g���A�f�o�C�X���𓾂�
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
			ShowMessage("Error�F[RegEnumValue()]");
			return;
		}
		wcstombs(&buf[0], (wchar_t *)byValueBuff, 256);
		sprintf(COMlist[k], "%s:", buf);
		wcstombs(&buf[0], (wchar_t *)cNameBuff, 256);
		strcat(COMlist[k], buf);
	}
}

//--------------------------------------------------------------------------
// VCP�|�[�g����
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
// USB�I�[�v��
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
	// �V���A���|�[�g���J����
	hCom = CreateFile(
		nam,					// �V���A���|�[�g�̕�����
		GENERIC_READ | GENERIC_WRITE,		// �A�N�Z�X���[�h�F�ǂݏ���
		0,					// ���L���[�h�F������̓A�N�Z�X�s��
		0,					// �Z�L�����e�B�����F�n���h���p������
		OPEN_EXISTING,				// �쐬�t���O
		FILE_ATTRIBUTE_NORMAL,			// ����
		0					// �e���v���[�g�̃n���h��
		);
	if(hCom == INVALID_HANDLE_VALUE) {
		sprintf(str, "Error Open_USB[%s]", ComNam);
		ShowMessage(str);
		return(INVALID_HANDLE_VALUE);
	}
	PurgeComm(hCom, PURGE_TXABORT | PURGE_RXABORT | PURGE_TXCLEAR | PURGE_RXCLEAR);
	// �ʐM������ݒ肷��
	GetCommState(hCom, &dcb);			// DCB ���擾
	// �f�[�^
	dcb.BaudRate = Speed;
	dcb.fBinary = TRUE;
	dcb.ByteSize = 8;
	dcb.fParity = NOPARITY;
	dcb.StopBits = ONESTOPBIT;
	// �n�[�h�E�G�A�E�t���[����
	dcb.fOutxCtsFlow = FALSE;			// CTS�n�[�h�E�F�A�t���[����FCTS������g�p���Ȃ��ꍇ��FLASE���w��
							// CTS ���������ꍇ��TRUE���w�肵��CTS�M�����Ď����܂�
	dcb.fOutxDsrFlow = FALSE;			// DSR�n�[�h�E�F�A�t���[����F�g�p���Ȃ��ꍇ��FALSE���w��
	dcb.fDtrControl = DTR_CONTROL_DISABLE;		// DTR�L��/�����F�@�����Ȃ�@DTR_CONTROL_DISABLE;ISABLE
	dcb.fRtsControl = RTS_CONTROL_DISABLE;		// RTS����FRTS��������Ȃ��ꍇ��RTS_CONTROL_DISABLE���w��
	// �\�t�g�E�F�A�t���[����
	dcb.fOutX = FALSE;				// ���M��XON/OFF����̗L���F�@�Ȃ���FLALSE
	dcb.fInX = FALSE;				// ��M��XON/XOFF����̗L���F�Ȃ���FALSE
	dcb.fTXContinueOnXoff = TRUE;			// ��M�o�b�t�@�[���t��XOFF��M��̌p�����M�ہF���M��TRUE
	dcb.XonLim = 512;				// XON��������܂łɊi�[�ł���ŏ��o�C�g���F
	dcb.XoffLim = 512;				// XOFF��������܂łɊi�[�ł���ŏ��o�C�g���F
	dcb.XonChar = 0x11;				// ���M��XON���� ( ���M�F�r�W�B���� ) �̎w��F
 							// ��ʂɁAXON�����Ƃ���11H ( �f�o�C�X����P�FDC1 )���悭�g���܂�
	dcb.XoffChar = 0x13;				// XOFF�����i���M�s�F�r�W�[�ʍ��j�̎w��F�Ȃ���FALSE
							// ��ʂɁAXOFF�����Ƃ���13H ( �f�o�C�X����3�FDC3 )���悭�g���܂�
	//���̑�
	dcb.fNull = FALSE;				// NULL�o�C�g�̔j���F �j�����遨TRUE
	dcb.fAbortOnError = TRUE;			// �G���[���̓ǂݏ�������I���F�@�I�����遨TRUE
	dcb.fErrorChar = FALSE;				// �p���e�B�G���[�������̃L�����N�^�iErrorChar�j�u���F�@�Ȃ���FLALSE
	dcb.ErrorChar = 0x00;				// �p���e�B�G���[�������̒u���L�����N�^
	dcb.EofChar = 0x03;				// �f�[�^�I���ʒm�L�����N�^�@�F�@��ʂ�0x03(ETX)���悭�g���܂��B
	dcb.EvtChar = 0x02;				// �C�x���g�ʒm�L�����N�^ �F�@��ʂ�0x02(STX)���悭�g���܂�

	SetCommState(hCom, &dcb); 			// DCB ��ݒ�
	Sleep(60);
	return(hCom);
}

//---------------------------------------------------------------------------
// USB�ɑ��M
//---------------------------------------------------------------------------
int USB_Tx(HANDLE hCom1, unsigned char *str, int len)
{
	DWORD	dwWritten; 	// �|�[�g�֏������񂾃o�C�g��

	WriteFile(hCom1, str, len, &dwWritten, NULL);
	if((int)dwWritten != len) return(-1);
	FlushFileBuffers(hCom1);
	return(1);
}

//---------------------------------------------------------------------------
// USB�����M
//---------------------------------------------------------------------------
int USB_Rx(HANDLE hCom1, unsigned int Wait)
{
	int	k;
	DWORD	dwErrors;	// �G���[���
	COMSTAT	ComStat;	// �f�o�C�X�̏��
static	DWORD	dwCount;	// ��M�f�[�^�̃o�C�g��
	DWORD	dwRead;		// �|�[�g����ǂݏo�����o�C�g��

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
// �I�����\��
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
// Form1 �E�C���h�E�E�N���G�C�g
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
// ���g���ёI��
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
	// VCP Poart �I�[�v��
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
	//�@���g��
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
	//�@�ϒ�
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
	// �o��������
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
	// �o�͎���
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
	// �r��������
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
// �k������
//---------------------------------------------------------------------------
void __fastcall TForm1::LoadClick(TObject *Sender)
{
	int	 k;
	double	Freq;
	float	oTime;
	int		Power;

	//--------------------------------------------------
	// VCP Poart �I�[�v��
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
// �r����
//---------------------------------------------------------------------------
void __fastcall TForm1::SaveClick(TObject *Sender)
{
	int	 k;
	double	Freq;
	float	oTime;
	int		Power;

	//--------------------------------------------------
	// VCP Poart �I�[�v��
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

