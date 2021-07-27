//---------------------------------------------------------------------------

#ifndef Unit1H
#define Unit1H
//---------------------------------------------------------------------------
#include <Classes.hpp>
#include <Controls.hpp>
#include <StdCtrls.hpp>
#include <Forms.hpp>
#include <ExtCtrls.hpp>
//---------------------------------------------------------------------------
class TForm1 : public TForm
{
__published:	// IDE で管理されるコンポーネント
	TImage *Image1;
	TPanel *Cont_0;
	TButton *PortSearch;
	TButton *M_Start;
	TButton *Stop;
	TEdit *Cmnt;
	TLabel *Label12;
	TLabel *Label13;
	TLabel *Label14;
	TLabel *Label15;
	TLabel *Label16;
	TLabel *Label17;
	TLabel *Label18;
	TLabel *Label19;
	TLabel *Label20;
	TLabel *Label21;
	TLabel *Label22;
	TComboBox *VcpPort;
	TPanel *Panel3;
	TRadioButton *F2R4G;
	TRadioButton *F900M;
	TComboBox *CenterFreq;
	TPanel *Panel1;
	TPanel *Panel2;
	TComboBox *Bandwidth;
	TPanel *Panel5;
	TComboBox *Span;
	TLabel *Label1;
	TLabel *Label2;
	TLabel *StartFreq;
	TLabel *CentorFreq;
	TLabel *StopFreq;
	TButton *ZeroSpan;
	TPanel *Panel6;
	TComboBox *RSSI_Avg;
	TPanel *Cont_1;
	void __fastcall F1_Create(TObject *Sender);
	void __fastcall M_StartClick(TObject *Sender);
	void __fastcall PortSearchClick(TObject *Sender);
	void __fastcall StopClick(TObject *Sender);
	void __fastcall F900MClick(TObject *Sender);
	void __fastcall F2R4GClick(TObject *Sender);
	void __fastcall ZeroSpanClick(TObject *Sender);
	//void __fastcall TimeDomainClick(TObject *Sender);
	//void __fastcall FreqChange(TObject *Sender);
private:	// ユーザー宣言
public:		// ユーザー宣言
	__fastcall TForm1(TComponent* Owner);
};
//---------------------------------------------------------------------------
extern PACKAGE TForm1 *Form1;
//---------------------------------------------------------------------------
#endif
