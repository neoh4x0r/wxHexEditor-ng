/***********************************(GPL)***********************************
*   wxHexEditor-ng is a hex edit tool for editing massive files in Linux   *
*   Copyright (C) 2022  Dario                                              *
*   Copyright (C) 2010  Erdem U. Altinyurt                                 *
*                                                                          *
*   This program is free software; you can redistribute it and/or          *
*   modify it under the terms of the GNU General Public License            *
*   as published by the Free Software Foundation; either version 2         *
*   of the License.                                                        *
*                                                                          *
*   This program is distributed in the hope that it will be useful,        *
*   but WITHOUT ANY WARRANTY; without even the implied warranty of         *
*   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the          *
*   GNU General Public License for more details.                           *
*                                                                          *
*   You should have received a copy of the GNU General Public License      *
*   along with this program;                                               *
*   if not, write to the Free Software Foundation, Inc.,                   *
*   51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA           *
*                                                                          *
*               home  : https://github.com/dariox86/wxHexEditor-ng         *
****************************************************************************/


#include "HexEditor.h"
#include "HexEditorGui.h"
#include "HexEditorFrame.h"
#include "HexEditorCtrl/HexEditorCtrl.h"
#include "HexEditorCtrl/wxHexCtrl/wxHexCtrl.h"
#include "../udis86/udis86.h"

#ifdef WX_GCH
#include <wx_pch.h>
#else
#include <wx/wx.h>
#endif

#ifndef HEXPANELS_H
#define HEXPANELS_H

#include <wx/filename.h>

#include <stdint.h>
#include <wx/memory.h>

#define HAS_A_TIME_MACHINEx
#define HAS_A_EXFAT_TIME
class DataInterpreter : public InterpreterGui{
	public:
		DataInterpreter(wxWindow* parent, int id = -1, wxPoint pos = wxDefaultPosition, wxSize size = wxSize( -1,-1 ), int style = wxTAB_TRAVERSAL )
		:InterpreterGui( parent, id, pos, size, style){
#if wxCHECK_VERSION( 2,9,0 ) && defined( __WXOSX__ )	//onOSX, 8 px too small.
			wxFont f = GetFont();
			f.SetPointSize(10);
			m_textctrl_binary->SetFont(f);
			m_textctrl_ascii->SetFont(f);
			m_textctrl_8bit->SetFont(f);
			m_textctrl_16bit->SetFont(f);
			m_textctrl_32bit->SetFont(f);
			m_textctrl_64bit->SetFont(f);
			m_textctrl_float->SetFont(f);
			m_textctrl_double->SetFont(f);
#endif
#ifdef HAS_A_TIME_MACHINE
            m_collapsiblePane_TimeMachine->Enable(true);
            m_collapsiblePane_TimeMachine->Show(true);
#ifdef HAS_A_EXFAT_TIME
            m_collapsiblePaneExFAT->Enable(true);
            m_collapsiblePaneExFAT->Show(true);
#endif
#endif

			unidata.raw = unidata.mraw = NULL;
			};
		void Set( wxMemoryBuffer buffer );
		void Clear( void );
		void OnUpdate( wxCommandEvent& event );
		void OnSpin( wxSpinEvent& event );
		void OnTextEdit( wxKeyEvent& event );
		void OnTextMouse( wxMouseEvent& event );
		void OnCheckEdit( wxCommandEvent& event );
		wxString AsciiSymbol( unsigned char a );

	protected:
		struct unidata{
			char *raw, *mraw;	//big endian and little endian
			struct endian{
				int8_t *bit8;
				int16_t *bit16;
				int32_t *bit32;
				int64_t *bit64;
				uint8_t  *ubit8;
				uint16_t *ubit16;
				uint32_t *ubit32;
				uint64_t *ubit64;
				float  *bitfloat;
				double *bitdouble;
				//_Float128 *bitf128;
				char *raw;
				} little, big;
			short size;
			char *mirrorbfr;
			}unidata;

#ifdef HAS_A_TIME_MACHINE
    #pragma pack (1)
        struct FATDate_t{
            unsigned Sec :5;
            unsigned Min :6;
            unsigned Hour :5;
            unsigned Day :5;
            unsigned Month :4;
            unsigned Year :7;
        }FATDate;


        enum TimeFormats{ UNIX32, UNIX64, FAT, NTFS, APFS, HFSp, exFAT_C,exFAT_M,exFAT_A, };

		wxString FluxCapacitor( unidata::endian *unit, TimeFormats format );
#endif //HAS_A_TIME_MACHINE
	};

class InfoPanel : public InfoPanelGui{
	public:
		InfoPanel(wxWindow* parent, int id = -1, wxPoint pos = wxDefaultPosition, wxSize size = wxSize( -1,-1 ), int style = wxTAB_TRAVERSAL )
		:InfoPanelGui( parent, id, pos, size, style){}
		void Set( wxFileName flnm, uint64_t lenght, wxString AccessMode, int FD, wxString XORKey );
		void OnUpdate( wxCommandEvent& event ){}
};

#define idDeleteTag 31001 //just random num
#define idEditTag 31002 //just random num
class TagPanel : public TagPanelGui{
	public:
		TagPanel(class HexEditorFrame* parent_, int id = -1, wxPoint pos = wxDefaultPosition, wxSize size = wxSize( -1,-1 ), int style = wxTAB_TRAVERSAL )
		:TagPanelGui( (wxWindow*) parent_, id, pos, size, style){
			parent = parent_;
			this->Connect( idDeleteTag, wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler( TagPanel::OnDeleteTag ) );
			this->Connect( idEditTag, wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler( TagPanel::OnEditTag ) );
			m_buttonClear->Show(true);
			m_buttonHide->Show(true);
			}
		class HexEditorFrame *parent;
		virtual void Set( ArrayOfTAG& TagArray );
		void Clear( void );
		void OnClear( wxCommandEvent& event );
		void OnHide( wxCommandEvent& event );
		void OnTagSelect( wxCommandEvent& event );
		//void OnUpdate( wxCommandEvent& event ){}
		void OnRightMouse( wxMouseEvent& event );
		void OnDeleteTag( wxCommandEvent& event );
		void OnEditTag( wxCommandEvent& event );
		void OnKeyDown( wxKeyEvent& event );
		~TagPanel(void){
			this->Disconnect( idDeleteTag, wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler( TagPanel::OnDeleteTag ) );
			this->Disconnect( idEditTag, wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler( TagPanel::OnEditTag ) );
			}
	protected:
		wxMutex mutextag;
};

class SearchPanel : public TagPanel{
	public:
	SearchPanel(class HexEditorFrame* parent_, int id = -1, wxPoint pos = wxDefaultPosition, wxSize size = wxSize( -1,-1 ), int style = wxTAB_TRAVERSAL )
	:TagPanel( parent_, id, pos, size, style){
	m_buttonClear->Show(true);
	m_buttonHide->Show(false);
	m_buttonTAG->Show(true);
	}
	void OnTagSelect( wxCommandEvent& event );
	void OnRightMouse( wxMouseEvent& event ){};
	void OnClear( wxCommandEvent& event );
	void OnTAG( wxCommandEvent& event );
	void Set( ArrayOfTAG& TagArray );
	};

class ComparePanel : public TagPanel{
	public:
	ComparePanel(class HexEditorFrame* parent_, int id = -1, wxPoint pos = wxDefaultPosition, wxSize size = wxSize( -1,-1 ), int style = wxTAB_TRAVERSAL )
	:TagPanel( parent_, id, pos, size, style){
        m_buttonClear->Show(false);
        m_buttonHide->Show(false);
        m_buttonTAG->Show(false);
        };
	void OnRightMouse( wxMouseEvent& event ){};
	void OnTagSelect( wxCommandEvent& event );
	};

class DisassemblerPanel : public DisassemblerPanelGUI{
	public:
	DisassemblerPanel( class HexEditorFrame* parent_, int id = -1, wxPoint pos = wxDefaultPosition, wxSize size = wxSize( -1,-1 ), int style = wxTAB_TRAVERSAL )
	:DisassemblerPanelGUI( (wxWindow*) parent_, id ){};
	void Set( wxMemoryBuffer buffer );
	void OnUpdate(wxCommandEvent& event);
	void Clear( void );
	wxMemoryBuffer mybuff;
	};
#endif
