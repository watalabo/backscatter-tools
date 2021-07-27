object Form1: TForm1
  Left = 0
  Top = 0
  AlphaBlendValue = 1
  Caption = 'SpectrumAnalyzer_CC1352P[861~1054MHz] [2360~2500MHz]'
  ClientHeight = 491
  ClientWidth = 1081
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
  object Image1: TImage
    Left = 0
    Top = 21
    Width = 1030
    Height = 375
  end
  object Label12: TLabel
    Left = 1036
    Top = 21
    Width = 37
    Height = 13
    Caption = '0 (dbm)'
  end
  object Label13: TLabel
    Left = 1036
    Top = 50
    Width = 16
    Height = 13
    Caption = '-10'
  end
  object Label14: TLabel
    Left = 1036
    Top = 80
    Width = 16
    Height = 13
    Caption = '-20'
  end
  object Label15: TLabel
    Left = 1036
    Top = 110
    Width = 16
    Height = 13
    Caption = '-30'
  end
  object Label16: TLabel
    Left = 1036
    Top = 140
    Width = 16
    Height = 13
    Caption = '-40'
  end
  object Label17: TLabel
    Left = 1036
    Top = 170
    Width = 16
    Height = 13
    Caption = '-50'
  end
  object Label18: TLabel
    Left = 1036
    Top = 200
    Width = 16
    Height = 13
    Caption = '-60'
  end
  object Label19: TLabel
    Left = 1036
    Top = 230
    Width = 16
    Height = 13
    Caption = '-70'
  end
  object Label20: TLabel
    Left = 1036
    Top = 260
    Width = 16
    Height = 13
    Caption = '-80'
  end
  object Label21: TLabel
    Left = 1036
    Top = 290
    Width = 16
    Height = 13
    Caption = '-90'
  end
  object Label22: TLabel
    Left = 1036
    Top = 320
    Width = 22
    Height = 13
    Caption = '-100'
  end
  object Label1: TLabel
    Left = 1036
    Top = 350
    Width = 22
    Height = 13
    Caption = '-110'
  end
  object Label2: TLabel
    Left = 1036
    Top = 380
    Width = 22
    Height = 13
    Caption = '-120'
  end
  object StartFreq: TLabel
    Left = 5
    Top = 396
    Width = 66
    Height = 13
    Caption = '2400.000MHz'
  end
  object CentorFreq: TLabel
    Left = 490
    Top = 396
    Width = 66
    Height = 13
    Caption = '2400.000MHz'
  end
  object StopFreq: TLabel
    Left = 970
    Top = 396
    Width = 66
    Height = 13
    Caption = '2480.000MHz'
  end
  object Cont_0: TPanel
    Left = 0
    Top = 421
    Width = 1081
    Height = 35
    Align = alBottom
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
    object Stop: TButton
      AlignWithMargins = True
      Left = 396
      Top = 4
      Width = 100
      Height = 27
      Align = alLeft
      Caption = 'Stop'
      TabOrder = 2
      OnClick = StopClick
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
      TabOrder = 3
      Text = 'VcpPort'
    end
  end
  object Cmnt: TEdit
    Left = 0
    Top = 0
    Width = 1081
    Height = 21
    Align = alTop
    TabOrder = 1
  end
  object Cont_1: TPanel
    Left = 0
    Top = 456
    Width = 1081
    Height = 35
    Align = alBottom
    Caption = 'Cont_1'
    TabOrder = 2
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
      Caption = 'CenterFrequency(MHz)'
      TabOrder = 2
    end
    object Panel2: TPanel
      Left = 586
      Top = 1
      Width = 110
      Height = 33
      Align = alLeft
      Caption = 'Bandwidth(KHz)_ID'
      TabOrder = 3
    end
    object Bandwidth: TComboBox
      AlignWithMargins = True
      Left = 699
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
      Font.Name = #65325#65331' '#12468#12471#12483#12463
      Font.Style = []
      ParentBiDiMode = False
      ParentFont = False
      TabOrder = 4
    end
    object Panel5: TPanel
      Left = 329
      Top = 1
      Width = 65
      Height = 33
      Align = alLeft
      Caption = 'Span(MHz)'
      TabOrder = 5
    end
    object Span: TComboBox
      AlignWithMargins = True
      Left = 397
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
    object ZeroSpan: TButton
      AlignWithMargins = True
      Left = 453
      Top = 4
      Width = 130
      Height = 27
      Align = alLeft
      Caption = 'ZeroSpan(TimeDomain)'
      TabOrder = 7
      OnClick = ZeroSpanClick
    end
    object Panel6: TPanel
      Left = 801
      Top = 1
      Width = 100
      Height = 33
      Align = alLeft
      Caption = 'RSSI_Avaraging'
      TabOrder = 8
    end
    object RSSI_Avg: TComboBox
      AlignWithMargins = True
      Left = 904
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
      TabOrder = 9
    end
  end
end
