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

#include "Tag.h"
#include "wxHexCtrl.h"
int TagElementSort( TagElement **a, TagElement **b ){
	return (*a)->start - (*b)->start;
	}
TagElement::TagElement(){
	visible = false;
	start = end = 0;
	tag.Clear();
	FontClrData.SetColour( *wxBLACK );//Default wxBlack
	NoteClrData.SetColour( *wxRED );
	wxP = NULL;
	}

TagElement::TagElement( TagElementData data ){
	start = data.start;
	end=data.end;
	FontClrData=data.FontClrData;
	NoteClrData=data.NoteClrData;
	visible = false;
	wxP = NULL;
	}

TagElement::TagElement( uint64_t _start, uint64_t _end, wxString _tag, wxColourData fntclr, wxColourData noteclr):
			start( _start < _end ? _start : _end), end(_start < _end ? _end : _start), tag(_tag), FontClrData(fntclr), NoteClrData(noteclr){
	visible = false;
	wxP = NULL;
	}

TagElement::TagElement( uint64_t _start, uint64_t _end, wxString _tag, wxColour fntclr, wxColour noteclr):
			start( _start < _end ? _start : _end), end(_start < _end ? _end : _start), tag(_tag){
	FontClrData.SetColour(fntclr);
	NoteClrData.SetColour(noteclr);
	visible = false;
	wxP = NULL;
	}


TagElement::~TagElement(){
	if( visible )
		Hide();
	if(wxP != NULL)
		wxP->Destroy();
	tag.Clear();
	}

int TagElement::TagCompare(TagElement **a, TagElement **b){
	if( (*a)->start == (*b)->start )
		return 0;
	if( (*a)->start > (*b)->start )
		return 1;
	return -1;
	}

wxColour TagElement::SoftColour( wxColour col ){
	//fake alpha highlighting for tags.
	unsigned char r,g,b;
	wxColour back=col;
	r=back.Red();
	g=back.Green();
	b=back.Blue();
	r +=  (0xff - r) / 2;
	g +=  (0xff - g) / 2;
	b +=  (0xff - b) / 2;
	back.Set(r,g,b,0xFF);
	return back;
	}

void TagElement::Show( const wxPoint& pos, wxWindow *parent ){
	if( !visible && !tag.IsEmpty() ){
#ifdef _DEBUG_TAG_
		std::cout << "Show tag element " << this << std::endl;
#endif
		visible = true;
#ifdef __WXMAC__
		wxP = new wxPopupTransientWindow( parent );
#else
		wxP = new wxPopupWindow( parent );  //OSX doesn't like that. Gives wxNonOwnedWindow::Show(bool) error...
#endif
		wxP->SetBackgroundColour( NoteClrData.GetColour() );
		wxP->SetForegroundColour( FontClrData.GetColour() );
		wxStaticText *text = new wxStaticText( wxP, wxID_ANY, tag );
		wxBoxSizer *topSizer = new wxBoxSizer( wxVERTICAL );
		topSizer->Add( text, 0, wxALL, 5 );
		wxP->SetAutoLayout( true );
		wxP->SetSizer( topSizer );
		topSizer->Fit(wxP);
		wxP->Position( pos, wxSize(0,-50)); //topSizer->GetSize() ); //size put gap here
		wxP->Show();
// TODO (death#1#): Auto hide on popup menus. Modularized!
		}
	}

uint64_t TagElement::Size( void ){
	return end-start+1;
	}

void TagElement::Hide( void ){
	if( visible )
		{
#ifdef _DEBUG_TAG_
		std::cout << "Hide tag element " << this << std::endl;
#endif
		visible=false;
        wxP->Hide();
		wxP->Destroy();
		wxP=NULL;
		}
	}

bool TagElement::isCover( uint64_t position ){
	if(( start <= position && end >= position )
		|| ( (start > end) && ( start >= position ) && ( end <= position )))	// for inversed start - end)
		return true;
	else
		return false;
	}

void TagElement::print( void ){
	std::cout << "TAD start:" << start << " end: "<< end << " Tag:" << tag.ToAscii() << std::endl;
	}

//----------TAG DIALOG-----------//
TagDialog::TagDialog(TagElement& TagE, wxWindow* parent):TagDialogGui( parent ),Tag(TagE){
	TmpTag = Tag;
	TagTextCtrl->SetValue( TmpTag.tag );
	m_FontColourPicker->SetColour( TmpTag.FontClrData.GetColour() );
	m_NoteColourPicker->SetColour( TmpTag.NoteClrData.GetColour() );
//	TagTextCtrl->SetBinValue( TmpTag.tag );
//	wxTextAttr attr;
//	attr.SetTextColour( TmpTag.FontClrData.GetColour() );
//	attr.SetBackgroundColour( TmpTag.NoteClrData.GetColour() );
//	attr.SetBackgroundColour( *wxRED );
//	TagTextCtrl->SetDefaultStyle( attr );
	}

void TagDialog::OnSave( wxCommandEvent& event ){
	TmpTag.tag = TagTextCtrl->GetValue();
	TmpTag.FontClrData.SetColour( m_FontColourPicker->GetColour() );
	TmpTag.NoteClrData.SetColour( m_NoteColourPicker->GetColour() );
	Tag = TmpTag;
	EndModal(wxID_SAVE);
	}

void TagDialog::OnDelete( wxCommandEvent& event ){
	EndModal(wxID_DELETE);
	}
