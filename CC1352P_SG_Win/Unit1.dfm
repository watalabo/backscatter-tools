object Form1: TForm1
  Left = 0
  Top = 0
  AlphaBlendValue = 1
  Caption = 'SignalGenerator_CC1352P[861~1054MHz] [2360~2500MHz]'
  ClientHeight = 119
  ClientWidth = 671
  Color = clBtnFace
  Font.Charset = DEFAULT_CHARSET
  Font.Color = clWindowText
  Font.Height = -11
  Font.Name = 'Tahoma'
  Font.Style = []
  OldCreateOrder = False
  OnCreate = F1_Create
  PixelsPerInch = 96
  TextHeight = 13
  object Cont_0: TPanel
    Left = 0
    Top = 25
    Width = 671
    Height = 35
    Align = alTop
    TabOrder = 0
    object PortSearch: TButton
      AlignWithMargins = True
      Left = 210
      Top = 4
      Width = 66
      Height = 27
      Align = alLeft
      Caption = 'PortSearch'
      TabOrder = 0
      OnClick = PortSearchClick
    end
    object M_Start: TButton
      AlignWithMargins = True
      Left = 282
      Top = 4
      Width = 108
      Height = 27
      Align = alLeft
      Caption = 'Start'
      TabOrder = 1
      OnClick = M_StartClick
    end
    object VcpPort: TComboBox
      AlignWithMargins = True
      Left = 4
      Top = 4
      Width = 200
      Height = 22
      Align = alLeft
      Font.Charset = DEFAULT_CHARSET
      Font.Color = clWindowText
      Font.Height = -12
      Font.Name = 'Tahoma'
      Font.Style = []
      ParentFont = False
      TabOrder = 2
      Text = 'VcpPort'
    end
    object BoardSel: TComboBox
      AlignWithMargins = True
      Left = 550
      Top = 4
      Width = 117
      Height = 23
      Align = alRight
      BevelInner = bvLowered
      BevelKind = bkFlat
      BiDiMode = bdLeftToRight
      Color = clBtnFace
      DropDownCount = 30
      Font.Charset = ANSI_CHARSET
      Font.Color = clWindowText
      Font.Height = -15
      Font.Name = #65325#65331' '#12468#12471#12483#12463
      Font.Style = []
      ParentBiDiMode = False
      ParentFont = False
      TabOrder = 3
    end
    object Save: TButton
      AlignWithMargins = True
      Left = 468
      Top = 4
      Width = 66
      Height = 27
      Align = alLeft
      Caption = 'Save'
      TabOrder = 4
      OnClick = SaveClick
    end
    object Load: TButton
      AlignWithMargins = True
      Left = 396
      Top = 4
      Width = 66
      Height = 27
      Align = alLeft
      Caption = 'Load'
      TabOrder = 5
      OnClick = LoadClick
    end
  end
  object Cont_1: TPanel
    Left = 0
    Top = 60
    Width = 671
    Height = 35
    Align = alTop
    TabOrder = 1
    object Panel3: TPanel
      Left = 1
      Top = 1
      Width = 98
      Height = 33
      Align = alLeft
      TabOrder = 0
      object F2R4G: TRadioButton
        Left = 1
        Top = 1
        Width = 49
        Height = 31
        Align = alLeft
        Caption = '2.4G'
        Checked = True
        TabOrder = 0
        TabStop = True
        OnClick = F2R4GClick
      end
      object F900M: TRadioButton
        Left = 50
        Top = 1
        Width = 47
        Height = 31
        Align = alLeft
        Caption = '900M'
        TabOrder = 1
        OnClick = F900MClick
      end
    end
    object CenterFreq: TComboBox
      AlignWithMargins = True
      Left = 227
      Top = 4
      Width = 99
      Height = 23
      Align = alLeft
      BevelInner = bvLowered
      BevelKind = bkFlat
      BiDiMode = bdLeftToRight
      Color = clBtnFace
      DropDownCount = 30
      Font.Charset = ANSI_CHARSET
      Font.Color = clWindowText
      Font.Height = -15
      Font.Name = #65325#65331' '#65328#12468#12471#12483#12463
      Font.Style = []
      ParentBiDiMode = False
      ParentFont = False
      TabOrder = 1
    end
    object Panel1: TPanel
      Left = 99
      Top = 1
      Width = 125
      Height = 33
      Align = alLeft
      Caption = 'Frequency(MHz)'
      TabOrder = 2
    end
    object oTime: TPanel
      Left = 465
      Top = 1
      Width = 110
      Height = 33
      Align = alLeft
      Caption = 'OutputTime(Sec)'
      TabOrder = 3
    end
    object OutputTime: TComboBox
      AlignWithMargins = True
      Left = 578
      Top = 4
      Width = 87
      Height = 23
      Align = alLeft
      BevelInner = bvLowered
      BevelKind = bkFlat
      BiDiMode = bdLeftToRight
      Color = clBtnFace
      DropDownCount = 30
      Font.Charset = ANSI_CHARSET
      Font.Color = clWindowText
      Font.Height = -15
      Font.Name = #65325#65331' '#12468#12471#12483#12463
      Font.Style = []
      ParentBiDiMode = False
      ParentFont = False
      TabOrder = 4
    end
    object Panel5: TPanel
      Left = 329
      Top = 1
      Width = 80
      Height = 33
      Align = alLeft
      Caption = 'Power(dBm)'
      TabOrder = 5
    end
    object Power: TComboBox
      AlignWithMargins = True
      Left = 412
      Top = 4
      Width = 50
      Height = 23
      Align = alLeft
      BevelInner = bvLowered
      BevelKind = bkFlat
      BiDiMode = bdLeftToRight
      Color = clBtnFace
      DropDownCount = 30
      Font.Charset = ANSI_CHARSET
      Font.Color = clWindowText
      Font.Height = -15
      Font.Name = #65325#65331' '#12468#12471#12483#12463
      Font.Style = []
      ParentBiDiMode = False
      ParentFont = False
      TabOrder = 6
    end
  end
  object Panel2: TPanel
    Left = 0
    Top = 0
    Width = 671
    Height = 25
    Align = alTop
    TabOrder = 2
    object Cmnt: TEdit
      Left = 1
      Top = 1
      Width = 580
      Height = 21
      TabOrder = 0
    end
    object Mod: TCheckBox
      Left = 599
      Top = 1
      Width = 71
      Height = 23
      Align = alRight
      Caption = 'Modulate'
      TabOrder = 1
    end
  end
end
