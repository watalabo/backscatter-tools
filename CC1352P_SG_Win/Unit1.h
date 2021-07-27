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
	TPanel *Cont_0;
	TButton *PortSearch;
	TButton *M_Start;
	TComboBox *VcpPort;
	TPanel *Panel3;
	TRadioButton *F2R4G;
	TRadioButton *F900M;
	TComboBox *CenterFreq;
	TPanel *Panel1;
	TPanel *oTime;
	TComboBox *OutputTime;
	TPanel *Panel5;
	TComboBox *Power;
	TPanel *Cont_1;
	TComboBox *BoardSel;
	TButton *Save;
	TButton *Load;
	TPanel *Panel2;
	TEdit *Cmnt;
	TCheckBox *Mod;
	void __fastcall F1_Create(TObject *Sender);
	void __fastcall M_StartClick(TObject *Sender);
	void __fastcall PortSearchClick(TObject *Sender);
	//void __fastcall StopClick(TObject *Sender);
	void __fastcall F900MClick(TObject *Sender);
	void __fastcall F2R4GClick(TObject *Sender);
	void __fastcall LoadClick(TObject *Sender);
	void __fastcall SaveClick(TObject *Sender);
	//void __fastcall ZeroSpanClick(TObject *Sender);
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
