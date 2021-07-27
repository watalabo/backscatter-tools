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

#define	USB_BUF_SZ	32768
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
	if(str_unq_cmp(COMlist[0], "VCP") != 0) {
		Form1->VcpPort->Text = COMlist[0];
	} else {
		Form1->VcpPort->Text = "Select VCP Port";
	}
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
// �O���t������
//---------------------------------------------------------------------------
void InitGraph(TObject *Sender)
{
	int	k, gWid;
	TPoint	points_buf[2];

	gWid = Form1->Image1->Width;

	// Image�G���A�h��Ԃ�
	Form1->Image1->Canvas->Pen->Width = 2;
	Form1->Image1->Canvas->Pen->Color = clBtnHighlight;
	for(k = 0; k < gWid; k += 2) {
		points_buf[0] = Point(k, 0);
		points_buf[1] = Point(k, Form1->Image1->Height);
		Form1->Image1->Canvas->Polyline(points_buf, 1);
	}
	// ���ڐ���(10����)
	Form1->Image1->Canvas->Pen->Width = 1;
	Form1->Image1->Canvas->Pen->Color = clBlue;
	for(k = 0; k < 13; k++) {
		points_buf[0] = Point(0, (Form1->Image1->Height - 10) / 12 * k + 5);
		points_buf[1] = Point(gWid, (Form1->Image1->Height -10) / 12 * k + 5);
		Form1->Image1->Canvas->Polyline(points_buf, 1);
	}
	// �c�ڐ����
	//InitGraph(Sender);
	Form1->Image1->Canvas->Pen->Width = 1;
	Form1->Image1->Canvas->Pen->Color = clBlue;
	for(k = 0; k <= 1000; k += 100) {
		points_buf[0] = Point(k + 15, 0);
		points_buf[1] = Point(k + 15, Form1->Image1->Height);
		Form1->Image1->Canvas->Polyline(points_buf, 1);
	}
}

//---------------------------------------------------------------------------
// �I�����g�����\��
//---------------------------------------------------------------------------
void SetSelFrq()
{
	int		n;

	Form1->CenterFreq->Items->Clear();
	Form1->Bandwidth->Items->Clear();
	if(Form1->F2R4G->Checked == True) {
		for(n = 2360;  n <= 2500; n += 5) {
			sprintf(ustr, "%2.5f", float(n));
			Form1->CenterFreq->Items->Add(ustr);
		}
		Form1->CenterFreq->Text = "2440.00000";

		Form1->Bandwidth->Items->Add("   4.8_64");
		Form1->Bandwidth->Items->Add("   5.4_65");
		Form1->Bandwidth->Items->Add("   6.9_66");
		Form1->Bandwidth->Items->Add("   8.2_67");
		Form1->Bandwidth->Items->Add("   9.6_68");
		Form1->Bandwidth->Items->Add("  10.9_69");
		Form1->Bandwidth->Items->Add("  13.7_70");
		Form1->Bandwidth->Items->Add("  16.5_71");
		Form1->Bandwidth->Items->Add("  19.1_72");
		Form1->Bandwidth->Items->Add("  21.8_73");
		Form1->Bandwidth->Items->Add("  27.4_74");
		Form1->Bandwidth->Items->Add("  33.0_75");
		Form1->Bandwidth->Items->Add("  38.3_76");
		Form1->Bandwidth->Items->Add("  43.5_77");
		Form1->Bandwidth->Items->Add("  54.9_78");
		Form1->Bandwidth->Items->Add("  66.0_79");
		Form1->Bandwidth->Items->Add("  76.5_80");
		Form1->Bandwidth->Items->Add("  87.1_81");
		Form1->Bandwidth->Items->Add(" 109.8_82");
		Form1->Bandwidth->Items->Add(" 131.9_83");
		Form1->Bandwidth->Items->Add(" 153.1_84");
		Form1->Bandwidth->Items->Add(" 174.2_85");
		Form1->Bandwidth->Items->Add(" 219.6_86");
		Form1->Bandwidth->Items->Add(" 263.9_87");
		Form1->Bandwidth->Items->Add(" 306.1_88");
		Form1->Bandwidth->Items->Add(" 348.3_89");
		Form1->Bandwidth->Items->Add(" 439.1_90");
		Form1->Bandwidth->Items->Add(" 527.8_91");
		Form1->Bandwidth->Items->Add(" 612.2_92");
		Form1->Bandwidth->Items->Add(" 696.7_93");
		Form1->Bandwidth->Items->Add(" 878.2_94");
		Form1->Bandwidth->Items->Add("1055.6_95");
		Form1->Bandwidth->Items->Add("1224.4_96");
		Form1->Bandwidth->Items->Add("1393.3_97");
		Form1->Bandwidth->Items->Add("1756.4_98");
		Form1->Bandwidth->Items->Add("2111.1_99");
		Form1->Bandwidth->Items->Add("2448.9_100");
		Form1->Bandwidth->Items->Add("2786.7_101");
		Form1->Bandwidth->Items->Add("3512.9_102");
		Form1->Bandwidth->Items->Add("4222.2_103");
		Form1->Bandwidth->Text = " 109.8_82";
	} else {
		for(n = 865;  n <= 1050; n += 5) {
			sprintf(ustr, "%2.5f", float(n));
			Form1->CenterFreq->Items->Add(ustr);
		}
		Form1->CenterFreq->Text = "920.00000";

		Form1->Bandwidth->Items->Add("   4.5_64");
		Form1->Bandwidth->Items->Add("   5.1_65");
		Form1->Bandwidth->Items->Add("   6.5_66");
		Form1->Bandwidth->Items->Add("   7.8_67");
		Form1->Bandwidth->Items->Add("   9.0_68");
		Form1->Bandwidth->Items->Add("  10.2_69");
		Form1->Bandwidth->Items->Add("  12.9_70");
		Form1->Bandwidth->Items->Add("  15.5_71");
		Form1->Bandwidth->Items->Add("  18.0_72");
		Form1->Bandwidth->Items->Add("  20.5_73");
		Form1->Bandwidth->Items->Add("  25.8_74");
		Form1->Bandwidth->Items->Add("  31.0_75");
		Form1->Bandwidth->Items->Add("  36.0_76");
		Form1->Bandwidth->Items->Add("  41.0_77");
		Form1->Bandwidth->Items->Add("  51.6_78");
		Form1->Bandwidth->Items->Add("  62.1_79");
		Form1->Bandwidth->Items->Add("  72.0_80");
		Form1->Bandwidth->Items->Add("  81.9_81");
		Form1->Bandwidth->Items->Add(" 103.3_82");
		Form1->Bandwidth->Items->Add(" 124.1_83");
		Form1->Bandwidth->Items->Add(" 144.0_84");
		Form1->Bandwidth->Items->Add(" 163.8_85");
		Form1->Bandwidth->Items->Add(" 206.5_86");
		Form1->Bandwidth->Items->Add(" 248.2_87");
		Form1->Bandwidth->Items->Add(" 287.9_88");
		Form1->Bandwidth->Items->Add(" 327.6_89");
		Form1->Bandwidth->Items->Add(" 413.0_90");
		Form1->Bandwidth->Items->Add(" 496.4_91");
		Form1->Bandwidth->Items->Add(" 575.8_92");
		Form1->Bandwidth->Items->Add(" 655.3_93");
		Form1->Bandwidth->Items->Add(" 826.0_94");
		Form1->Bandwidth->Items->Add(" 992.8_95");
		Form1->Bandwidth->Items->Add("1151.7_96");
		Form1->Bandwidth->Items->Add("1310.5_97");
		Form1->Bandwidth->Items->Add("1652.1_98");
		Form1->Bandwidth->Items->Add("1985.7_99");
		Form1->Bandwidth->Items->Add("2303.4_100");
		Form1->Bandwidth->Items->Add("2621.1_101");
		Form1->Bandwidth->Items->Add("3304.2_102");
		Form1->Bandwidth->Items->Add("3971.4_103");
		Form1->Bandwidth->Text = " 103.3_82";
	}
	Form1->Span->Items->Clear();
	Form1->Span->Items->Add(1);
	for(n = 5;  n <= 193; n += 5) {
		Form1->Span->Items->Add(n);
	}
	Form1->Span->Text = 80;
}

//---------------------------------------------------------------------------
// Form1 �E�C���h�E�E�N���G�C�g
//---------------------------------------------------------------------------
void __fastcall TForm1::F1_Create(TObject *Sender)
{
	int		n;

	GetVcpPort(Sender);
	SetSelFrq();

	Form1->RSSI_Avg->Items->Clear();
	for(n = 1; n < 100; n++) {
		Form1->RSSI_Avg->Items->Add(n);
	}
	Form1->RSSI_Avg->Text = 1;

	InitGraph(Sender);
	time(&StartTime);
}

//---------------------------------------------------------------------------
// ���g���ёI��
//---------------------------------------------------------------------------
void __fastcall TForm1::F900MClick(TObject *Sender)
{
	SetSelFrq();
}
//---------------------------------------------------------------------------

void __fastcall TForm1::F2R4GClick(TObject *Sender)
{
	SetSelFrq();
}

//---------------------------------------------------------------------------
// PortSearch
//---------------------------------------------------------------------------
void __fastcall TForm1::PortSearchClick(TObject *Sender)
{
	GetVcpPort(Sender);
}

//---------------------------------------------------------------------------
//  �t�@�C��������
//---------------------------------------------------------------------------
void WriteFileRssi(char *Domain, int *Rssi, int num, int Frq)
{
	char	fnam[256];
	int		n;
	long	NowTime;
	FILE	*fp;
struct	tm	*i_tm;

	i_tm = localtime(&StartTime);
	sprintf(fnam, "%02d%02d%02d_%02d%02d_%s.txt",
		i_tm->tm_year - 100, i_tm->tm_mon + 1, i_tm->tm_mday,
		i_tm->tm_hour, i_tm->tm_min, Domain);
	if((fp = fopen(fnam, "a+")) == 0) return;
	time(&NowTime);
	fprintf(fp, "%02d%02d%02d:%02d%02d%02d\t",
		i_tm->tm_year - 100, i_tm->tm_mon + 1, i_tm->tm_mday,
		i_tm->tm_hour, i_tm->tm_min, i_tm->tm_min, i_tm->tm_sec);
	fprintf(fp, "%d\n", Frq);
	for(n = 0; n < num; n++) {
		fprintf(fp, "%d\t%d\n", n, Rssi[n]);
	}
	fclose(fp);

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

	double	CentorFreq;
	float	Span;
	int		rssi_avg;

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
	while(USB_Rx(ComCC, 10) > 0);


	//--------------------------------------------------
	// �Z���^�[���g��
	//--------------------------------------------------
	sprintf(ustr, "%s", (AnsiString)UTF8Encode(Form1->CenterFreq->Text));
	sscanf(ustr, "%lf", &CentorFreq);
	sprintf(buf, "freq %s\n", ustr);
	USB_Tx(ComCC, buf, strlen(buf));
	k = USB_Rx(ComCC, 16);
	if(str_unq_cmp(USB_RxBuf, "ACK_freq[") < 1) {
		sprintf(ustr, "Error [%s]", buf);
		ShowMessage(ustr);
		CloseHandle(ComCC);
		ComCC = INVALID_HANDLE_VALUE;
		return;
	}
	sprintf(ustr, "%6.3lfMHz", CentorFreq);
	Form1->CentorFreq->Caption = ustr;
	//--------------------------------------------------
	//  �X�p��
	//--------------------------------------------------
	sprintf(ustr, "%s", (AnsiString)UTF8Encode(Form1->Span->Text));
	sscanf(ustr, "%f", &Span);
	sprintf(buf, "span %s\n", ustr);
	USB_Tx(ComCC, buf, strlen(buf));
	k = USB_Rx(ComCC, 12);
	if(str_unq_cmp(USB_RxBuf, "ACK_span[") < 1) {
		sprintf(ustr, "Error [%s]", buf);
		ShowMessage(ustr);
		CloseHandle(ComCC);
		ComCC = INVALID_HANDLE_VALUE;
		return;
	}
	sprintf(ustr, "%6.3lfMHz", CentorFreq - Span / 2);
	Form1->StartFreq->Caption = ustr;
	sprintf(ustr, "%6.3lfMHz", CentorFreq + Span / 2);
	Form1->StopFreq->Caption = ustr;
	//--------------------------------------------------
	//  �o���h��
	//--------------------------------------------------
	sprintf(ustr, "%s", (AnsiString)UTF8Encode(Form1->Bandwidth->Text));
	for(n = 0; ustr[n] != '_' && n < 10; n++);
	sprintf(buf, "bw %s\n", &ustr[n+1]);
	USB_Tx(ComCC, buf, strlen(buf));
	k = USB_Rx(ComCC, 10);
	if(str_unq_cmp(USB_RxBuf, "ACK_bw[") < 1) {
		sprintf(ustr, "Error [%s]", buf);
		ShowMessage(ustr);
		CloseHandle(ComCC);
		ComCC = INVALID_HANDLE_VALUE;
		return;
	}
	//--------------------------------------------------
	//  RSSI���ω�
	//--------------------------------------------------
	sprintf(ustr, "%s", (AnsiString)UTF8Encode(Form1->RSSI_Avg->Text));
	sscanf(ustr, "%d", &rssi_avg);
	if(rssi_avg <= 0) {
		ShowMessage("Error RSSI_Avaraging");
		return;
	}
	sprintf(buf, "rssi %d\n", rssi_avg);
	USB_Tx(ComCC, buf, strlen(buf));
	k = USB_Rx(ComCC, 10);
	if(str_unq_cmp(USB_RxBuf, "ACK_rssi[") < 1) {
		sprintf(ustr, "Error [%s]", buf);
		ShowMessage(ustr);
		CloseHandle(ComCC);
		ComCC = INVALID_HANDLE_VALUE;
		return;
	}

	Form1->Cont_1->Enabled = False;
	Form1->M_Start->Enabled = False;

	Start = 1;
	nTime = 1;
	while(Start != 0) {
		//--------------------------------------------------
		//  �X�^�[�g
		//--------------------------------------------------
		if(Span == 0) {
			USB_Tx(ComCC, "stazs\n", 6);
		} else {
			USB_Tx(ComCC, "stafs\n", 6);
		}

		for(m = 0, RxN = 0; m < 20 && RxN < 1020; m++) {
			k = USB_Rx(ComCC, 1010);
			if(k > 0) {
				for(n = 0; n < k; n++, RxN++) RxDat[RxN] = USB_RxBuf[n];
				RxDat[RxN] = 0;
				m = 0;
			} else if(k == -1) {
				exit(1);
			}
			DoEvents();
		}

		k = str_unq_cmp(&RxDat[1000], "ElapsedTime=");
		if(k > 0) {
			n = str_int(&RxDat[1000 + k]);
			SweepTime = (1.0/48 * n) / 1000;
			sprintf(ustr, "[%d] SweepTime=%3.1fmS", nTime, SweepTime);
			Form1->Cmnt->Text = ustr;
		}
		nTime++;
		//--------------------------------------------------
		// �O���t�`��
		//--------------------------------------------------
		InitGraph(Sender);
		// RSSI�l
		Form1->Image1->Canvas->Pen->Color = clRed;
		for(n = 0; n < 1000; n++) {
			points_buf[0] = Point(n + 15,  Form1->Image1->Height - 5);
			points_buf[1] = Point(n + 15, (Form1->Image1->Height - 5) - ((120 + (signed char)RxDat[n]) * 3));
			Form1->Image1->Canvas->Polyline(points_buf, 1);
		}
		if(Span == 0) {
			Form1->StartFreq->Caption = "  0.0mS";
			sprintf(ustr, "   %3.1fmS", SweepTime / 2);
			Form1->CentorFreq->Caption = ustr;
			sprintf(ustr, "    %3.1fmS", SweepTime);
			Form1->StopFreq->Caption = ustr;
		}
		DoEvents();
		//Sleep(1000);
	}
	if(Span == 0) {
		USB_Tx(ComCC, "endzs\n", 6);
		k = USB_Rx(ComCC, 10);
		if(str_unq_cmp(USB_RxBuf, "ACK_endzs") < 1) {
			sprintf(ustr, "Error [%s]", buf);
			ShowMessage("Error [endzs]");
			CloseHandle(ComCC);
			ComCC = INVALID_HANDLE_VALUE;
			return;
		}
	}
	Form1->Cont_1->Enabled = True;
	Form1->M_Start->Enabled = True;
}


//---------------------------------------------------------------------------
// Stop
//---------------------------------------------------------------------------
void __fastcall TForm1::StopClick(TObject *Sender)
{
	Start = 0;

}


//---------------------------------------------------------------------------
// ZeroSpan
//---------------------------------------------------------------------------
void __fastcall TForm1::ZeroSpanClick(TObject *Sender)
{
	Form1->Span->Text = 0;
}
//---------------------------------------------------------------------------

