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

/***************************************************************
 * Name:      wxHexEditorngApp.h
 * Purpose:   Defines Application Class
 * Author:    Death Knight (spamjunkeater@gmail.com)
 * Created:   2008-05-12
 * Copyright: Erdem U. Altinyurt
 * License:   GPL v2
 **************************************************************/

#ifndef WXHEXEDITORNGAPP_H
#define WXHEXEDITORNGAPP_H

#ifndef _VERSION_
	#define _VERSION_ "0.25"
	#define _VERSION_S_ _VERSION_ " Beta Development"
#endif // _VERSION_

#ifdef __WXMSW__
	#define _VERSION_STR_ _VERSION_S_ " for Windows"
	#elif defined(__WXOSX__)
	#define _VERSION_STR_ _VERSION_S_ " for Mac"
	#elif defined(__WXGTK__) && defined(__linux__)
	#define _VERSION_STR_ _VERSION_S_ " for Linux"
	#elif defined(__WXGTK__) && defined(BSD)
	#define _VERSION_STR_ _VERSION_S_ " for BSD"
#else
	#define _VERSION_STR_ _VERSION_S_
#endif

#include <wx/app.h>
#include "HexEditorFrame.h"

class wxHexEditorngApp : public wxApp {
	public:
		virtual bool OnInit();// wxOVERRIDE;
		void PostAppInit(void);
		void SetLanguage(void);
#ifdef __WXMAC__	//for preferences and about
		DECLARE_EVENT_TABLE()
#endif // __WXMAC__
		void OnHelp(wxCommandEvent& evt);
#ifdef _DEBUG_EVENTS_
		int FilterEvent(wxEvent &evt);
		void OnMouseMove(wxMouseEvent &event);
#endif
	private:
		class HexEditorFrame* frame;
		wxLocale myLocale;

		// create the file system watcher here, because it needs an active loop
		virtual void OnEventLoopEnter(wxEventLoopBase* WXUNUSED(loop));// wxOVERRIDE;
	};

DECLARE_APP(wxHexEditorngApp)

uint64_t UnkFormatToUInt(wxString input );
bool kMGT_ToUInt( wxString user_input, uint64_t *size );
void SetStackLimit(void);
#endif // WXHEXEDITORNGAPP_H
