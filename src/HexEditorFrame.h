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


#ifndef __HexEditorFrame__
#define __HexEditorFrame__

#include "HexEditorGui.h"
#include "HexEditor.h"
#include "HexEditorApp.h"
#include "HexDialogs.h"
#include "HexPanels.h"
#include "../resources/wxhex.xpm"
#include <wx/filename.h>
#include <wx/dir.h>
#include <wx/dnd.h>
#include <wx/aboutdlg.h>
#include <wx/artprov.h>
#include <wx/aui/aui.h>
#include <wx/aui/auibar.h>
#include <wx/aui/auibook.h>
#include <wx/config.h>
#include <wx/url.h>
#include <wx/textdlg.h>
#include <wx/mstream.h>
#include <wx/docview.h>
#include <wx/regex.h>

#if _FSWATCHER_
#include <wx/fswatcher.h>
#endif // _FSWATCHER_

#if defined __WXMAC__
#include "../resources/osx/png2c.h"
#elif defined __WXMSW__
#include "../resources/win/png2c.h"
#endif


#define wxGetBitmapFromMemory(name) _wxGetBitmapFromMemory(name ## _png, sizeof(name ## _png))
inline wxBitmap _wxGetBitmapFromMemory(const unsigned char *data, int length) {
	wxMemoryInputStream is(data, length);
	return wxBitmap(wxImage(is, wxBITMAP_TYPE_ANY, -1), -1);
	}

class IPCServer;
class DnDFile;
class copy_maker;
class HexEditorFrame : public HexEditorGui {
	public:
		HexEditorFrame();
		HexEditorFrame(	wxWindow* parent, wxWindowID id = wxID_ANY );
		~HexEditorFrame();
		void TagHideAll();
		class HexEditor* OpenFile(wxFileName flname, bool openAtRight=false);
		class HexEditor* GetActiveHexEditor(void);
		copy_maker *MyCopyMark;
#if _FSWATCHER_
		wxFileSystemWatcher *file_watcher;
#endif // _FSWATCHER_
		//this two need public for About and Preferences at Mac
      void OnHelpMenu( wxCommandEvent& event );
		void OnOptionsMenu( wxCommandEvent& event );
	protected:
		void OnMenuEvent( wxCommandEvent& event );
		void OnToolsMenu( wxCommandEvent& event );
		void OnUpdateUI( wxUpdateUIEvent& event );
		void OnViewMenu( wxCommandEvent& event );
  		void OnDevicesMenu( wxCommandEvent& event );
		void OnOptionsFileMode( wxCommandEvent& event );
		void OnClose( wxCloseEvent& event );
		void OnActivate( wxActivateEvent& event );

		void OpenMemorySPDDevice( void );

		void ActionEnabler( void );
		void ActionDisabler( void );
		void OnNotebookTabSelection( wxAuiNotebookEvent& event );
		void OnNotebookTabClose( wxAuiNotebookEvent& event );
		void OnFloatingPaneClosed( wxAuiManagerEvent& event );

	private:
		void PrepareAUI( void );
		//wxAuiNotebook *MyNotebook;
		wxAuiManager *MyAUI;
		DataInterpreter *MyInterpreter;
		InfoPanel *MyInfoPanel;
		TagPanel *MyTagPanel;
		TagPanel *MySearchPanel;
		TagPanel *MyComparePanel;
		DisassemblerPanel *MyDisassemblerPanel;
		wxFileHistory *MyFileHistory;
		wxString license;
#ifdef _WX_AUIBAR_H_
		wxAuiToolBar* Toolbar;
#else
		wxToolBar* Toolbar;
#endif
		friend class DnDFile;
		IPCServer* m_server;
	};

class HexEditorArtProvider : public wxArtProvider {
	protected:
		virtual wxBitmap CreateBitmap(const wxArtID& id, const wxArtClient& client,
		                              const wxSize& size);
	};


class DnDFile : public wxFileDropTarget {
	public:
		DnDFile( HexEditorFrame* myHexFramework) {
			HexFramework = myHexFramework;
			}
		virtual bool OnDropFiles(wxCoord x, wxCoord y, const wxArrayString& filenames);
	private:
		wxAuiNotebook *m_pOwner;
		HexEditorFrame *HexFramework;
	};

class VersionChecker : public UpdateDialogGui {
	public:
		VersionChecker( wxString _url, wxString current_version, wxWindow *parent = NULL, wxWindowID id = 1  );
		void OnChkDisplay( wxCommandEvent& event );
	};

#include <wx/ipc.h>
//For open files with wxHexEditor-ng mime
class IPCServer : public wxServer {
	public:
		IPCServer(HexEditorFrame *parent_): parent(parent_) {};
		virtual wxConnectionBase *OnAcceptConnection(const wxString& topic);
	private:
		HexEditorFrame *parent;
	};

class copy_maker {
	public:
		bool copied;		//copy in action or not
		uint64_t start;		//copy start position
		uint64_t size;		//size of copy
		wxMemoryBuffer m_buffer; //uses RAM, for small data
		//wxFile *tempfile;	//uses Temp HDD File and delete after.
		FAL *sourcefile;	//uses HDD File and NOT delete after.
		copy_maker( ) {
			copied = false;
			start = size = 0;
			//	tempfile = NULL;
			sourcefile = NULL;
			}
		~copy_maker( ) {
			///if( m_buffer.GetDataLen() )
			///	m_buffer.Clear();
			//if(tempfile != NULL)
			//if(sourcefile != NULL)
			}
		bool SetClipboardData( wxString& CopyString) {
			if(wxTheClipboard->Open()) {
//					if (wxTheClipboard->IsSupported( wxDF_TEXT )){
				wxTheClipboard->Clear();
				int isok = wxTheClipboard->SetData( new wxTextDataObject( CopyString ));
				wxTheClipboard->Flush();
				wxTheClipboard->Close();
				return isok;
				}
			else {
				wxMessageBox( wxString(_( "Clipboard could not be opened.")) + wxT("\n") + _("Operation cancelled!"), _("Copy To Clipboard Error"), wxOK|wxICON_ERROR);
				return false;
				}
			}

		wxString GetClipboardData( void ) {
			if(wxTheClipboard->Open()) {
				if (wxTheClipboard->IsSupported( wxDF_TEXT )) {
					wxTextDataObject data;
					wxTheClipboard->GetData( data );
					wxTheClipboard->Close();
					return data.GetText();
					}
				else {
					wxBell();
					wxTheClipboard->Close();
					return wxString();
					}
				}
			else {
				wxMessageBox( wxString(_( "Clipboard could not be opened.")) + wxT("\n") + _("Operation cancelled!"), _("Copy To Clipboard Error"), wxOK|wxICON_ERROR);
				return wxString();
				}
			}
	};

#endif
