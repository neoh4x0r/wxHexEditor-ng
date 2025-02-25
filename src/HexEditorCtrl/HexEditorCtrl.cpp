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
extern int fake_block_size;
#include "HexEditorCtrl.h"
//???
//BEGIN_EVENT_TABLE(,wxScrolledWindow )
//	EVT_CHAR( wxHexCtrl::OnChar )
//  EVT_PAINT(wxHexCtrl::OnPaint )
//    EVT_SIZE( HexEditorCtrl::OnResize )
//    EVT_RIGHT_DOWN( wxHexCtrl::OnMouseRight )
//    EVT_SET_FOCUS( wxHexCtrl::OnFocus )
//    EVT_KILL_FOCUS( wxHexCtrl::OnKillFocus )
//END_EVENT_TABLE()

#include <wx/arrimpl.cpp> // this is a magic incantation which must be done!
WX_DEFINE_OBJARRAY(wxArrayUINT64);

HexEditorCtrl::HexEditorCtrl(wxWindow* parent, int id, const wxPoint& pos, const wxSize& size, long style):
	HexEditorCtrlGui(parent, id, pos, size, wxTAB_TRAVERSAL) {
	select = new class Select( GetEventHandler() );

	SetAutoLayout(true);

	m_static_offset->SetLabel( wxString(wxT("+"))+_("Offset")+wxT("-") );
	SetFont();

	Dynamic_Connector();
	offset_scroll = new wxHugeScrollBar( offset_scroll_real );
	TAGMutex = false;
	hex_ctrl->TagMutex = &TAGMutex;
	text_ctrl->TagMutex = &TAGMutex;

	//Using hex_ctrl ZebraStriping pointer for all 3 panels.
	delete text_ctrl->ZebraStriping;
	delete offset_ctrl->ZebraStriping;
	ZebraStriping = hex_ctrl->ZebraStriping;
	text_ctrl->ZebraStriping = ZebraStriping;
	offset_ctrl->ZebraStriping = ZebraStriping;
	myConfigBase::Get()->Read( _T("ZebraStriping"), &ZebraEnable, true);
	sector_size=0;
	}

HexEditorCtrl::~HexEditorCtrl( void ) {
	Dynamic_Disconnector();
	Clear();

	WX_CLEAR_ARRAY(MainTagArray);
	WX_CLEAR_ARRAY(HighlightArray);
	WX_CLEAR_ARRAY(CompareArray);

	MainTagArray.Shrink();
	HighlightArray.Shrink();
	CompareArray.Shrink();

	delete select;
	delete offset_scroll;
	}

void HexEditorCtrl::Dynamic_Connector() {
	this->Connect( idTagAddSelection, wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler( HexEditorCtrl::OnTagAddSelection ), NULL, this );
	this->Connect( idTagQuick, wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler( HexEditorCtrl::OnTagQuick ), NULL, this );
	this->Connect( idTagEdit, wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler( HexEditorCtrl::OnTagEdit ), NULL, this );
	this->Connect( __idOffsetHex__, wxEVT_UPDATE_UI, wxUpdateUIEventHandler( HexEditorCtrl::UpdateUI ) );
	this->Connect( wxEVT_KILL_FOCUS, wxFocusEventHandler(HexEditorCtrl::OnKillFocus),NULL, this);
	hex_ctrl->Connect( wxEVT_KILL_FOCUS, wxFocusEventHandler(HexEditorCtrl::OnKillFocus),NULL, this);
	text_ctrl->Connect( wxEVT_KILL_FOCUS, wxFocusEventHandler(HexEditorCtrl::OnKillFocus),NULL, this);
	hex_ctrl->Connect( wxEVT_SET_FOCUS, wxFocusEventHandler(HexEditorCtrl::OnFocus),NULL, this);
	text_ctrl->Connect( wxEVT_SET_FOCUS, wxFocusEventHandler(HexEditorCtrl::OnFocus),NULL, this);


	offset_ctrl->Connect( wxEVT_RIGHT_DOWN,	wxMouseEventHandler(HexEditorCtrl::OnMouseRight),NULL, this);
	offset_ctrl->Connect( wxEVT_LEFT_DOWN,	wxMouseEventHandler(HexEditorCtrl::OnMouseLeft),NULL, this);

	hex_ctrl ->Connect( wxEVT_LEFT_DOWN,	wxMouseEventHandler(HexEditorCtrl::OnMouseLeft),NULL, this);
	text_ctrl->Connect( wxEVT_LEFT_DOWN,	wxMouseEventHandler(HexEditorCtrl::OnMouseLeft),NULL, this);
	hex_ctrl ->Connect( wxEVT_LEFT_UP,	wxMouseEventHandler(HexEditorCtrl::OnMouseSelectionEnd),NULL, this);
	text_ctrl->Connect( wxEVT_LEFT_UP,	wxMouseEventHandler(HexEditorCtrl::OnMouseSelectionEnd),NULL, this);
	hex_ctrl ->Connect( wxEVT_RIGHT_DOWN,wxMouseEventHandler(HexEditorCtrl::OnMouseRight),NULL, this);
	text_ctrl->Connect( wxEVT_RIGHT_DOWN,wxMouseEventHandler(HexEditorCtrl::OnMouseRight),NULL, this);
	hex_ctrl ->Connect( wxEVT_MOTION,	wxMouseEventHandler(HexEditorCtrl::OnMouseMove),NULL, this);
	text_ctrl->Connect( wxEVT_MOTION,	wxMouseEventHandler(HexEditorCtrl::OnMouseMove),NULL, this);
	}

void HexEditorCtrl::Dynamic_Disconnector() {
	this->Disconnect( idTagAddSelection, wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler( HexEditorCtrl::OnTagAddSelection ), NULL, this );
	this->Disconnect( idTagQuick, wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler( HexEditorCtrl::OnTagQuick ), NULL, this );
	this->Disconnect( idTagEdit, wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler( HexEditorCtrl::OnTagEdit ), NULL, this );
	this->Disconnect( __idOffsetHex__, wxEVT_UPDATE_UI, wxUpdateUIEventHandler( HexEditorCtrl::UpdateUI ) );
	this->Disconnect( wxEVT_KILL_FOCUS, wxFocusEventHandler(HexEditorCtrl::OnKillFocus),NULL, this);
	hex_ctrl->Disconnect( wxEVT_KILL_FOCUS, wxFocusEventHandler(HexEditorCtrl::OnKillFocus),NULL, this);
	text_ctrl->Disconnect( wxEVT_KILL_FOCUS, wxFocusEventHandler(HexEditorCtrl::OnKillFocus),NULL, this);

	offset_ctrl->Disconnect( wxEVT_RIGHT_DOWN,	wxMouseEventHandler(HexEditorCtrl::OnMouseRight),NULL, this);
	offset_ctrl->Disconnect( wxEVT_LEFT_DOWN,	wxMouseEventHandler(HexEditorCtrl::OnMouseLeft),NULL, this);
	hex_ctrl ->Disconnect( wxEVT_LEFT_DOWN,	wxMouseEventHandler(HexEditorCtrl::OnMouseLeft),NULL, this);
	text_ctrl->Disconnect( wxEVT_LEFT_DOWN,	wxMouseEventHandler(HexEditorCtrl::OnMouseLeft),NULL, this);
	hex_ctrl ->Disconnect( wxEVT_LEFT_UP,	wxMouseEventHandler(HexEditorCtrl::OnMouseSelectionEnd),NULL, this);
	text_ctrl->Disconnect( wxEVT_LEFT_UP,	wxMouseEventHandler(HexEditorCtrl::OnMouseSelectionEnd),NULL, this);
	hex_ctrl ->Disconnect( wxEVT_RIGHT_DOWN,	wxMouseEventHandler(HexEditorCtrl::OnMouseRight),NULL, this);
	text_ctrl->Disconnect( wxEVT_RIGHT_DOWN,	wxMouseEventHandler(HexEditorCtrl::OnMouseRight),NULL, this);
	hex_ctrl ->Disconnect( wxEVT_MOTION,	wxMouseEventHandler(HexEditorCtrl::OnMouseMove),NULL, this);
	text_ctrl->Disconnect( wxEVT_MOTION,	wxMouseEventHandler(HexEditorCtrl::OnMouseMove),NULL, this);
	}

//-----READ/WRITE FUNCTIONS-------//
void HexEditorCtrl::ReadFromBuffer( uint64_t position, unsigned lenght, char *buffer, bool cursor_reset, bool paint ) {
	if( lenght==4294967295LL ) {
		std::cout << "Buffer has no data!" << std::endl;
		return;
		}
	static wxMutex MyBufferMutex;
	MyBufferMutex.Lock();
	page_offset = position;
	if( lenght != ByteCapacity() ) {
		//last line could be NULL;
		}
	Clear( false, cursor_reset );
	wxString text_string;

// Optimized Code
//	for( unsigned i=0 ; i<lenght ; i++ )
	//text_string << text_ctrl->Filter(buffer[i]);
//		text_string << static_cast<wxChar>((unsigned char)(buffer[i]));
//		text_string << CP473toUnicode((unsigned char)(buffer[i]));

	//Painting Zebra Stripes, -1 means no stripe. 0 means start with normal, 1 means start with zebra
	*ZebraStriping=(ZebraEnable ? position/BytePerLine()%2 : -1);
	if(sector_size > 1) {
		offset_ctrl->sector_size=sector_size;
		unsigned draw_line=sector_size-(page_offset%sector_size);
		hex_ctrl->ThinSeparationLines.Clear();
		text_ctrl->ThinSeparationLines.Clear();
		do {
			hex_ctrl->ThinSeparationLines.Add( 2*draw_line );
			text_ctrl->ThinSeparationLines.Add( draw_line );
			draw_line += sector_size;
			}
		while (draw_line < lenght );
		}
	else if(fake_block_size>1){
		offset_ctrl->sector_size=fake_block_size;
		unsigned draw_line=fake_block_size-(page_offset%fake_block_size);
		hex_ctrl->ThinSeparationLines.Clear();
		text_ctrl->ThinSeparationLines.Clear();
		do {
			hex_ctrl->ThinSeparationLines.Add( 2*draw_line );
			text_ctrl->ThinSeparationLines.Add( draw_line );
			draw_line += fake_block_size;
			}
		while (draw_line < lenght );
		}
	else if(hex_ctrl->ThinSeparationLines.Count() ){
		hex_ctrl->ThinSeparationLines.Clear();
		text_ctrl->ThinSeparationLines.Clear();
		}

	if(ProcessRAMMap.Count()) {
		hex_ctrl->ThinSeparationLines.Clear();
		text_ctrl->ThinSeparationLines.Clear();
		//Notice that, ProcessRAMMap is SORTED.
		for( unsigned i=0; i < ProcessRAMMap.Count(); i++ ) {
			int64_t M = ProcessRAMMap.Item(i);
			if( M > page_offset + ByteCapacity() )
				break;

			if(    (M > page_offset)
			       && (M <= page_offset + ByteCapacity()) ) {

				int draw_line = M - page_offset;
				hex_ctrl->ThinSeparationLines.Add( 2*draw_line );
				text_ctrl->ThinSeparationLines.Add( draw_line );
				}
			}
		}

	hex_ctrl->SetBinValue(buffer, lenght, false );
	//text_ctrl->ChangeValue(text_string, false);
	text_ctrl->SetBinValue(buffer, lenght, false);

	offset_ctrl->SetValue( position, BytePerLine() );

	if( offset_scroll->GetThumbPosition() != (page_offset / BytePerLine()) )
		offset_scroll->SetThumbPosition( page_offset / BytePerLine() );

	if( paint ) {
		PaintSelection();
		}

	MyBufferMutex.Unlock();
	}

void HexEditorCtrl::TextCharReplace( long char_location, const wxChar chr) {
	text_ctrl->Replace( char_location, chr, true );
	char_location *= 2; //Converting Byte location to Hex Location;
	wxString temp = wxString::Format(wxT("%02X"),chr);
	hex_ctrl->Replace(char_location, char_location+2, temp);
	}

void HexEditorCtrl::HexCharReplace(long hex_location, const wxChar chr) {
	hex_ctrl->Replace( hex_location, chr, true );
	hex_location /=2;	// Hex location is now Byte location
	char rdchr = hex_ctrl->ReadByte(hex_location);
	text_ctrl->Replace(	hex_location, rdchr, true );
	}
//-----MENUS--------//

void HexEditorCtrl::ShowContextMenu( const wxMouseEvent& event ) {
	//Non operational code... HexEditor::ShowContextMenu() used instead
	wxMenu menu;
	uint64_t TagPosition=0;
	if( event.GetEventObject() == hex_ctrl )
		TagPosition = page_offset + (hex_ctrl->PixelCoordToInternalPosition( event.GetPosition() ) / 2);
	if( event.GetEventObject() == text_ctrl )
		TagPosition = page_offset + text_ctrl->PixelCoordToInternalPosition( event.GetPosition() );

	TagElement *TAG;
	for( unsigned i = 0 ; i < MainTagArray.Count() ; i++ ) {
		TAG = MainTagArray.Item(i);
		if( TAG->isCover( TagPosition ) ) {	//end not included!
			menu.Append(idTagEdit, _T("Tag Edit"));
			break;
			}
		}

	if( select->GetState() ) {
		menu.Append(idTagAddSelection, _T("Tag Selection"));
		menu.Append(idTagQuick, _T("Tag Selection"));
		}
//  menu.AppendSeparator();
	wxPoint pos = event.GetPosition();
	wxWindow *scr = static_cast<wxWindow*>( event.GetEventObject() );
	pos += scr->GetPosition();
	PopupMenu(&menu, pos);
	}

//-----VISUAL FUNCTIONS------//
void HexEditorCtrl::SetFont( wxFont f ) {
	stdfont = f;
	m_static_offset->SetFont( stdfont );
	m_static_address->SetFont( stdfont );
	m_static_byteview->SetFont( stdfont );
	SetStyle();
	}

void HexEditorCtrl::SetFont( ) {
	wxFont newfont;
	int FontSize=10;
	myConfigBase::Get()->Read( wxT("FontSize"), &FontSize, 10 );

#if defined( __WXOSX__ )
	newfont = wxFont(FontSize, wxFONTFAMILY_MODERN, wxFONTSTYLE_NORMAL, wxFONTWEIGHT_NORMAL, 0, wxT("Monaco"), wxFONTENCODING_ISO8859_1);// Fonts are too small on wxOSX 2.9.x series.
#elif defined( __WXMSW__ )
	newfont = wxFont(FontSize, wxFONTFAMILY_MODERN, wxFONTSTYLE_NORMAL, wxFONTWEIGHT_NORMAL, 0, wxT("Courier New"), wxFONTENCODING_ISO8859_1);
#else
	newfont = wxFont(FontSize, wxFONTFAMILY_MODERN, wxFONTSTYLE_NORMAL, wxFONTWEIGHT_NORMAL, 0, wxT("Monospace"), wxFONTENCODING_ISO8859_1);
#endif
	SetFont( newfont );
	}

void HexEditorCtrl::SetStyle( ) {
	wxString Colour;
	wxColour Foreground,Background;
	wxTextAttr Style;

	//Normal style set
	if( wxConfig::Get()->Read( _T("ColourHexForeground"), &Colour) )
		Foreground.Set( Colour );
	else
		Foreground = wxSystemSettings::GetColour( wxSYS_COLOUR_WINDOWTEXT ) ;

	if( wxConfig::Get()->Read( _T("ColourHexBackground"), &Colour) )
		Background.Set( Colour );
	else
		Background = wxSystemSettings::GetColour( wxSYS_COLOUR_WINDOW ) ;

	Style = wxTextAttr( Foreground, Background, stdfont );
	offset_ctrl->SetDefaultStyle( Style );
	hex_ctrl->SetDefaultStyle( Style );
	wxString cp;
	myConfigBase::Get()->Read( _T("CharacterEncoding"), &cp, wxT("DOS OEM") );
	text_ctrl->PrepareCodepageTable(cp);
	text_ctrl->SetDefaultStyle( Style );

	//Selection style set

	if(wxConfig::Get()->Read( _T("ColourHexSelectionForeground"), &Colour) )
		Foreground.Set( Colour );
	else
		Foreground = wxSystemSettings::GetColour( wxSYS_COLOUR_HIGHLIGHTTEXT );

	if( wxConfig::Get()->Read( _T("ColourHexSelectionBackground"), &Colour) )
		Background.Set( Colour );
	else
		Background = wxSystemSettings::GetColour( wxSYS_COLOUR_HIGHLIGHT );

	Style = wxTextAttr( Foreground, Background,	stdfont );
	offset_ctrl->SetSelectionStyle( Style );
	hex_ctrl->SetSelectionStyle( Style );
	text_ctrl->SetSelectionStyle( Style );
	}

//Handles selection operations.
bool HexEditorCtrl::Selector() {
	wxWindow* FocusAt=FindFocus();
	if( !(FocusAt == hex_ctrl || FocusAt == text_ctrl) ) {		//Checks If selecton from hex or text control
#ifdef _DEBUG_
		std::cout << "Selector without focus captured." << std::endl;
		std::cout << "use last focus as a work-a-round." << std::endl;
#endif
		FocusAt=LastFocused;
		//return false;
		}
#ifdef _DEBUG_SELECT_
	std::cout << "Selector captured at CursorOffset()=" << std::dec << CursorOffset() << "\t select->StartOffset:" <<  select->StartOffset << std::endl;
#endif
	select->EndOffset = CursorOffset();	//Than make selection
	if( !select->GetState() ) {	// If no select available,
		select->SetState( true );	// then set start selection procedure
		select->OriginalStartOffset = select->StartOffset = select->EndOffset;
		return true;
		}

	if( FocusAt==text_ctrl ) { //TextCtrl capture mechanism is different since cursor is a slim line "|"
		if( select->OriginalStartOffset > select->EndOffset ) { //Backward selection at text panel
			select->StartOffset = select->OriginalStartOffset-1;
			}
		else if(select->OriginalStartOffset < select->EndOffset) { //forward selection on text panel
			select->StartOffset = select->OriginalStartOffset;
			select->EndOffset = select->EndOffset-1;
			}
		else {
			select->SetState( false );
			}
		}

#ifdef _DEBUG_SELECT_
	std::cout << "Selector Result : " << select->StartOffset << ":" << select->EndOffset << std::endl;
#endif
	return select->GetState();
	}

bool HexEditorCtrl::Select ( uint64_t start_offset, uint64_t end_offset ) {
	/*
	if( start_offset < 0 || end_offset < 0
	//	|| start_offset > myfile->Length() ||  end_offset > myfile->Length() //??
		){
		wxBell();
		return false;
	}
	*/
#ifdef _DEBUG_SELECT_
	std::cout << "HexEditorCtrl::Select( " << std::dec << start_offset << "," <<  end_offset << ")" << std::endl;
#endif
	select->StartOffset = start_offset;
	select->EndOffset  = end_offset;
	select->SetState( true );
	PaintSelection();
	return true;
	}

void inline HexEditorCtrl::ClearPaint( void ) {
	hex_ctrl ->ClearSelection();
	text_ctrl->ClearSelection();
	}

void HexEditorCtrl::PreparePaintTAGs( void ) { //TagElement& TAG ){
	TagHideAll();
	WX_CLEAR_ARRAY(hex_ctrl->TagArray);
	WX_CLEAR_ARRAY(text_ctrl->TagArray);

	if(!TagInvisible)
		PaintTAGsPrefilter( MainTagArray );

	PaintTAGsPrefilter( HighlightArray );

	PaintTAGsPrefilter( CompareArray );
	}

void HexEditorCtrl::PaintTAGsPrefilter( ArrayOfTAG& Arr ) {
	unsigned c = Arr.Count();
	if( c==0 )
		return;

	unsigned s = 0;
	if (c>0)
		for( ; static_cast<uint64_t>( page_offset ) > (Arr.Item(s))->end && s<c-1; s++ );
	unsigned e=s;
	if (c>0)
		for( ; page_offset + GetByteCount() > static_cast<int>( Arr.Item(e)->start ) && e<c-1 ; e++ );
#ifdef _DEBUG_PAINT_
	std::cout << "Tags needed between : " << s << " - " << e << std::endl;
#endif //_DEBUG_PAINT_
	for( unsigned i=s; i<=e ; i++)
		PushTAGToControls( Arr.Item(i) );
	}

//This functions move tags to local hex and text ctrls.
void HexEditorCtrl::PushTAGToControls( TagElement* TAG) {
	int64_t start_byte = TAG->start;
	int64_t end_byte = TAG->end;

	if(start_byte > end_byte) {								// swap if start > end
		int64_t temp = start_byte;
		start_byte = end_byte;
		end_byte = temp;
		}

	if( start_byte >= page_offset + GetByteCount() )	// ...[..].TAG...
		return;
	else if( end_byte < page_offset )						// ..TAG..[...]...
		return;

	if( start_byte <= page_offset )							// ...TA[G..]....
		start_byte = page_offset;

	if( end_byte >= page_offset + GetByteCount() )		//...[..T]AG...
		end_byte = GetByteCount() + page_offset;

	start_byte	-= page_offset;
	end_byte		-= page_offset;

	TagElement *TAX;//For debugging

	TAX = new TagElement( start_byte/(GetCharToHexSize()/2), end_byte/(GetCharToHexSize()/2)+1, TAG->tag, TAG->FontClrData, TAG->NoteClrData );
	text_ctrl->TagArray.Add( TAX );
	TAX = new TagElement( start_byte*2, (end_byte+1)*2, TAG->tag, TAG->FontClrData, TAG->NoteClrData );
	hex_ctrl->TagArray.Add( TAX );
	}

void HexEditorCtrl::SetHex2ColorMode( bool mode ) {
	hex_ctrl->Hex2ColorMode = mode;
	text_ctrl->Hex2ColorMode = mode;
	}

bool HexEditorCtrl::GetHex2ColorMode( void ) {
	return hex_ctrl->Hex2ColorMode;
	}

void HexEditorCtrl::PaintSelection( void ) {
	PreparePaintTAGs();
	if( select->GetState() ) {
		int64_t start_byte = select->StartOffset;
		int64_t end_byte = select->EndOffset;

		if(start_byte > end_byte) {								// swap if start > end
			int64_t temp = start_byte;
			start_byte = end_byte;
			end_byte = temp;
			}

		if( start_byte >= page_offset + GetByteCount() ) {	// ...[..].TAG...
			ClearPaint();
			return;
			}
		else if( start_byte <= page_offset )					// ...TA[G..]....
			start_byte = page_offset;

		if( end_byte < page_offset ) {							// ..TAG..[...]...
			ClearPaint();
			return;
			}
		else if( end_byte >= page_offset + GetByteCount() )//...[..T]AG...
			end_byte = GetByteCount() + page_offset;

		start_byte -= page_offset;
		end_byte   -= page_offset;

		text_ctrl->SetSelection(start_byte/(GetCharToHexSize()/2), end_byte/(GetCharToHexSize()/2)+1);
		hex_ctrl ->SetSelection(start_byte*2, (end_byte+1)*2);
		}
	else
		ClearPaint();
	}

void inline HexEditorCtrl::MyFreeze() {
	hex_ctrl->Freeze();
	text_ctrl->Freeze();
	offset_ctrl->Freeze();
	}

void inline HexEditorCtrl::MyThaw() {
	hex_ctrl->Thaw();
	text_ctrl->Thaw();
	offset_ctrl->Thaw();
	}

void HexEditorCtrl::Clear( bool RePaint, bool cursor_reset ) {
	hex_ctrl->Clear( RePaint, cursor_reset );
	text_ctrl->Clear( RePaint, cursor_reset );
	offset_ctrl->Clear( RePaint, cursor_reset );
	}

void HexEditorCtrl::RePaint( void ) {
	*ZebraStriping=(ZebraEnable ? page_offset/BytePerLine()%2 : -1);
	hex_ctrl->RePaint( );
	text_ctrl->RePaint( );
	offset_ctrl->RePaint( );
	}

void HexEditorCtrl::ControlShow( panels control, bool show ) {
	if( control == OFFSET_CTRL ) {
		offset_ctrl->Show( show );
		m_static_offset->Show(show);
		}
	else if(control == HEX_CTRL ) {
		hex_ctrl->Show( show );
		m_static_address->Show(show);
		}
	else if(control == TEXT_CTRL ) {
		text_ctrl->Show( show );
		m_static_byteview->Show(show);
		}
	wxYieldIfNeeded();
	Layout();
	Fit();
	}
bool HexEditorCtrl::ControlIsShown(panels control) {
	switch(control) {
		case OFFSET_CTRL:
			return offset_ctrl->IsShown();
		case HEX_CTRL	:
			return hex_ctrl->IsShown();
		case TEXT_CTRL	:
			return text_ctrl->IsShown();
		}
	return false;
	}

void HexEditorCtrl::OnResize( wxSizeEvent &event ) {
	int x = event.GetSize().GetX();
	int y = event.GetSize().GetY();
	int charx = hex_ctrl->GetCharSize().GetX();
	int chartx = text_ctrl->GetCharSize().GetX();
	int offset_x = offset_ctrl->GetCharSize().GetX()*offset_ctrl->GetLineSize();// + 4;
	offset_x = offset_ctrl->IsShown() ? offset_x : 0;


	x -= offset_x;								//Remove Offset Control box X because its changeable
	x -= offset_scroll_real->GetSize().GetX();  //Remove Offset scroll size
	x -= 4*2;									//+x 4 pixel external borders (dark ones, 2 pix each size)
	x = wxMax(0,x);								//Avoid X being negative
	y -= m_static_byteview->GetSize().GetY();	//Remove Head Text Y
	//AutoFill:
	bool custom_hex_format;
	wxConfig::Get()->Read( wxT("UseCustomHexFormat"), &custom_hex_format, false );
	wxString fmt(wxT("xx "));
	if( custom_hex_format )
		wxConfig::Get()->Read( wxT("CustomHexFormat"), &fmt, wxT("xx "));

// TODO (death#1#): Move style engine somewhere else to speedy resizing.
	hex_ctrl->SetFormat( fmt );

	int cnt_chr=0; //Counted character at current format
	for( unsigned i = 0 ; i <  fmt.Len() ; i++ ) {
		if( fmt[i]!=' ' )
			cnt_chr++;
		}
	cnt_chr/=2; // divide 2 for find byte per hex representation.

	int hexchr=0,textchr = 0;
	//Recalculate available area due hidden panels.
	hexchr+=hex_ctrl->IsShown() ? fmt.Len() : 0;
	textchr+=text_ctrl->IsShown() ? cnt_chr : 0;
	int available_space=0;
	available_space=x/(hexchr*charx+textchr*chartx/(GetCharToHexSize()/2));

	//Limiting Bytes Per Line
	bool use_BytesPerLineLimit;
	wxConfig::Get()->Read( wxT("UseBytesPerLineLimit"), &use_BytesPerLineLimit, false );
	if( use_BytesPerLineLimit ) {
		int BytesPerLineLimit;
		wxConfig::Get()->Read( wxT("BytesPerLineLimit"), reinterpret_cast<int*>(&BytesPerLineLimit), 16);

		//Downsizing is available
		if( available_space*cnt_chr > BytesPerLineLimit )
			available_space = BytesPerLineLimit/cnt_chr;
		}

	//Calculation of available area for Hex and Text panels.
	int text_x = chartx*available_space*cnt_chr/(GetCharToHexSize()/2)  +2 +4;
	int hex_x = charx*available_space*fmt.Len()  +2 +4 - charx ; //no need for last gap;
	int ByteShownPerLine=available_space*cnt_chr;

	text_x = text_ctrl->IsShown() ? text_x : 0;
	hex_x = hex_ctrl->IsShown() ? hex_x : 0;

#ifdef _DEBUG_SIZE_
	std::cout<< "HexEditorCtrl::OnResize()" << std::endl
	         << "HexEditorCtrl SizeEvent ReSize Command=(" << event.GetSize().GetX() << ',' << event.GetSize().GetY() << ")\n"
	         << "Offset Scrll: \t(" << offset_scroll->GetSize().GetX() << ',' << event.GetSize().GetY() <<")\n"
	         << "Offset Ctrl: \t(" << offset_ctrl->GetSize().GetX() << ',' << event.GetSize().GetY() <<")\n"
	         << "Hex Ctrl: \t(" << hex_x << ',' << event.GetSize().GetY() << ")\n"
	         << "Text Ctrl: \t(" << text_x << ',' << event.GetSize().GetY() << ")\n"
	         << "Hex Char: \t" << charx << std::endl
	         << "ByteShownPerLine: \t" << ByteShownPerLine << std::endl;
#endif

	offset_ctrl->SetMinSize( wxSize( offset_x, y ) );
//	offset_ctrl->SetSize( wxSize( offset_x , y ) ); //Not needed, Layout() Makes the job well.
	m_static_offset->SetMinSize( wxSize(offset_x, m_static_offset->GetSize().GetY()) );

	hex_ctrl->SetMinSize( wxSize( hex_x, y ));
//	hex_ctrl->SetSize( wxSize( hex_x, y ));
	m_static_address->SetMinSize( wxSize(hex_x, m_static_offset->GetSize().GetY()) ) ;

	text_ctrl->SetMinSize( wxSize( text_x, y ));
//	text_ctrl->SetSize( wxSize( text_x, y ));
	m_static_byteview->SetMinSize( wxSize( text_x, m_static_offset->GetSize().GetY()) );

	// Destroy the sizer created by the form builder before adding the windows
	// managed by it to another sizer, otherwise we would crash later when
	// destroying the sizer as it wouldn't know about the windows it contains.
	SetSizer(NULL);

	//Preparing Sizer
	wxFlexGridSizer* fgSizer1 = new wxFlexGridSizer( 2, 4, 0, 0 );
#if 1
	fgSizer1->Add( m_static_offset, 0, wxALIGN_CENTER|wxLEFT, 5 );
	fgSizer1->Add( m_static_address, 0, wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL|wxLEFT, 2 );
	fgSizer1->Add( m_static_byteview, 0, wxALIGN_CENTER|wxALL, 0 );
	fgSizer1->Add( m_static_null, 0, wxALIGN_CENTER, 3 );
	fgSizer1->Add( offset_ctrl, 0, wxALIGN_CENTER_VERTICAL|wxALL|wxEXPAND, 0 );
	fgSizer1->Add( hex_ctrl, 0, wxALIGN_CENTER_VERTICAL|wxALL|wxEXPAND, 0 );
	fgSizer1->Add( text_ctrl, 0, wxALIGN_CENTER_VERTICAL|wxALL|wxEXPAND, 0 );
	fgSizer1->Add( offset_scroll_real, 0, wxEXPAND, 0 );

#else
	fgSizer1->Add( m_static_offset, 0, wxALIGN_CENTER|wxLEFT, 0 );
	fgSizer1->Add( m_static_address, 0, wxALIGN_CENTER|wxALIGN_CENTER_VERTICAL|wxLEFT, 0 );
	fgSizer1->Add( m_static_byteview, 0, wxALIGN_CENTER|wxALL, 0 );
	fgSizer1->Add( m_static_null, 0, wxALIGN_CENTER, 0 );
	fgSizer1->Add( offset_ctrl, 0, wxALIGN_CENTER|wxALL|wxEXPAND, 0 );
	fgSizer1->Add( hex_ctrl, 0, wxALIGN_CENTER|wxALL, 0 );
	fgSizer1->Add( text_ctrl, 0, wxALIGN_CENTER|wxALL|wxEXPAND, 0 );
	fgSizer1->Add( offset_scroll_real, 0, wxEXPAND, 0 );


	fgSizer1->AddGrowableCol(1,1);
//	fgSizer1->AddGrowableRow(1,1);
#endif

	this->SetSizer( fgSizer1 );
	this->Layout();

#ifdef __WXMSW__
///At windows, OnResize cannot update required fields immeditialy, this hack fixes this behaviour.
	hex_ctrl->ChangeSize();
	text_ctrl->ChangeSize();
	offset_ctrl->ChangeSize();
#endif

	//Formating Hex and byteview column labels
	//This needed bellow hex_ctrl->ChangeSize() because it's updates the IsDenied function.
	wxString address,byteview,temp_address;
	for( int j = 0 ; j < ByteShownPerLine ; j++ ) {
		temp_address << wxString::Format( wxT("%02X"), j%0x100 );
		//if(j >= ByteShownPerLine/(GetCharToHexSize()/2))
		//	continue;
		byteview << wxString::Format( wxT("%01X"), (j*(GetCharToHexSize()/2))%0x10 );
		}

	//Adjusting custom hex formatting bar - Converting 00010203 -> 00 01 02 03 for "xx " format.
	for( int x = 0, i=0 ; x < hex_x && static_cast<unsigned>(i) < temp_address.Len() ; x++ )
		if(hex_ctrl->IsDenied(x))
			address << wxT(" ");
		else
			address << temp_address[i++];

	m_static_address->SetLabel(address);
	m_static_byteview->SetLabel( byteview );

#ifdef _DEBUG_SIZE_
	std::cout<< "HexEditorCtrl After ReSize=(" << x << ',' << y << ")\n"
	         << "Offset Scrll: \t(" << offset_scroll->GetSize().GetX() << ',' << offset_scroll->GetSize().GetY()<<")\n"
	         << "Offset Ctrl: \t(" << offset_ctrl->GetSize().GetX() << ',' << offset_ctrl->GetSize().GetY()<<")\n"
	         << "Hex Ctrl: \t(" << hex_ctrl->GetSize().GetX() << ',' << hex_ctrl->GetSize().GetY()<<")\n"
	         << "Text Ctrl: \t(" << text_ctrl->GetSize().GetX() << ',' << text_ctrl->GetSize().GetY()<<")\n";
#endif
	}

inline uint8_t HexEditorCtrl::GetCharToHexSize( void ) {
	if(text_ctrl->FontEnc == wxFONTENCODING_UTF16LE ||
	      text_ctrl->FontEnc == wxFONTENCODING_UTF16BE )
		return 4;
	if(text_ctrl->FontEnc == wxFONTENCODING_UTF32LE ||
	      text_ctrl->FontEnc == wxFONTENCODING_UTF32BE )
		return 8;
	return 2;
	}
//------EVENTS---------//
void HexEditorCtrl::OnMouseLeft(wxMouseEvent& event) {
	select->SetState( false );
	if( event.GetEventObject() == hex_ctrl ) {
		hex_ctrl->SetFocus();
		focus=HEX_CTRL;
		SetLocalHexInsertionPoint( hex_ctrl->PixelCoordToInternalPosition( event.GetPosition() ) );
		}
	else if( event.GetEventObject() == text_ctrl ) {
		text_ctrl->SetFocus();
		focus=TEXT_CTRL;
		SetLocalHexInsertionPoint( GetCharToHexSize() * text_ctrl->PixelCoordToInternalPosition( event.GetPosition() ) );
		}
	else if( event.GetEventObject() == offset_ctrl ) {
		event.Skip(); //to lower level for copy offset to clipboard
		}

	ClearPaint(); //redraw cursor shadow after movement.
	}

void HexEditorCtrl::OnMouseMove( wxMouseEvent& event ) {
	if(event.m_leftDown) {									//if left button is pressed
		int new_hex_location=0;								// initialize new_hex_location variable
		if( event.GetEventObject() == hex_ctrl ) 		// if this event from hex_ctrl area
			new_hex_location = hex_ctrl->PixelCoordToInternalPosition( event.GetPosition() ); //than take it's location on hex chars
		else if ( event.GetEventObject() == text_ctrl ) { //if we got this event from text area
			new_hex_location = GetCharToHexSize()*(text_ctrl->PixelCoordToInternalPosition( event.GetPosition() )); //Than we needed to multiply with 2 for take it's hex location.
			}

		int old_hex_location = GetLocalHexInsertionPoint();	//requesting old hex location
		if( new_hex_location != old_hex_location ) {				//if hex selection addresses are different, start selection routine

			if( !select->GetState() )	//if this is new selection start
				if( Selector() == false )	//and  select without focus
					return;						//don't make anything.
			SetLocalHexInsertionPoint( new_hex_location );	//Moving cursor to new location.
			Selector();							//Making actual selection.
			PaintSelection();
			}
		}
	else {
		if( event.GetEventObject() == hex_ctrl ||
		      event.GetEventObject() == text_ctrl ||
		      event.GetEventObject() == offset_ctrl ) {
			TagElement* tg = static_cast<wxHexCtrl*>(event.GetEventObject())->GetTagByPix( event.GetPosition() );
			if( (tg == NULL && TAGMutex==true) || 	 //If there is no Tag at under and tag mutex available
			      (tg != NULL && !tg->visible) )		// or Changed to new tag
				TagHideAll();
			}
		event.Skip(); //enable tags but problems with paint?
		}
	}

void HexEditorCtrl::OnMouseSelectionEnd( wxMouseEvent& event ) {
	event.Skip();
	}

void HexEditorCtrl::OnMouseRight( wxMouseEvent& event ) {
	//Not used code. HexEditor::OnMouseRight( wxMouseEvent& event ) used instead.
	if(event.GetEventObject() == hex_ctrl)
		LastRightClickAt = hex_ctrl->PixelCoordToInternalPosition( event.GetPosition() )/2;
	else if(event.GetEventObject() == text_ctrl)
		LastRightClickAt = text_ctrl->PixelCoordToInternalPosition( event.GetPosition() );
	else if( event.GetEventObject() == offset_ctrl) {
		//m_static_offset->SetLabel( offset_ctrl->hex_offset==true ? _("Offset: DEC") : _("Offset: HEX"));
		//event.Skip(true);
		offset_ctrl->OnMouseRight( event );
		int x,y;
		DoGetSize(&x,&y);
		wxSizeEvent mevent(wxSize(x,y));
		OnResize(mevent);
		return;//to avoid ShowContextMenu
		}
#ifdef _DEBUG_
	else
		std::cout << "Right click captured without ctrl!\n";
#endif
	ShowContextMenu( event );
	}

void HexEditorCtrl::OnFocus( wxFocusEvent& event) {
#ifdef _DEBUG_
	std::cout << "HexEditorCtrl::OnFocus( wxFocusEvent& event ) \n" ;
#endif
	if( event.GetWindow() == hex_ctrl ||
	      event.GetWindow() == text_ctrl  )
		LastFocused=event.GetWindow();
	event.Skip();//let wxHexCtrl::Focus set the cursor
	}

void HexEditorCtrl::OnKillFocus( wxFocusEvent& event) {
#ifdef _DEBUG_
	std::cout << "HexEditorCtrl::OnKillFocus( wxFocusEvent& event ) \n" ;
#endif
	TagHideAll();
	event.Skip();
	}

void HexEditorCtrl::TagCreator( bool QuickTag ) {
	if( select->GetState() ) {
		TagElement *TE = new TagElement;
		TE->start=select->StartOffset;
		TE->end=select->EndOffset;

		srand ( time(NULL) );
		//static keeps color values for next tag here!
		static wxColour last_tag_color = rand();
		static wxColour last_font_color = wxSystemSettings::GetColour( wxSYS_COLOUR_WINDOWTEXT );

		TE->NoteClrData.SetColour( QuickTag ? rand() : last_tag_color );
		TE->FontClrData.SetColour( last_font_color );

		int a=wxID_SAVE;
		if( !QuickTag ) {
			TagDialog x( *TE, this );
			a=x.ShowModal(); //This one blocks
			//a=x.Show();
			}
		if( a == wxID_SAVE ) {
			last_tag_color = TE->NoteClrData.GetColour();
			last_font_color = TE->FontClrData.GetColour();
			MainTagArray.Add( TE );

			//Sorting TAGs array because of quick painting code look sorted tags and just paint required ones.
			MainTagArray.Sort( TagElementSort );

			PreparePaintTAGs();
			ClearPaint();
			text_ctrl->RePaint();
			hex_ctrl ->RePaint();
			}
		//delete TE; NO! Don't delete this tags!
		}
	//event.Skip( true );
	wxUpdateUIEvent eventx( TAG_CHANGE_EVENT );
	GetEventHandler()->ProcessEvent( eventx );
	}

void HexEditorCtrl::OnTagQuick( wxCommandEvent& event ) {
	TagCreator( true );
	}

void HexEditorCtrl::OnTagAddSelection( wxCommandEvent& event ) {
	TagCreator( false );
	}

void HexEditorCtrl::OnTagEdit( wxCommandEvent& event ) {
	TagElement *TAG;
	uint64_t pos = LastRightClickAt;
#ifdef _DEBUG_TAG_
	std::cout << " Tag Edit on " << pos << std::endl;
#endif
	for( unsigned i = 0 ; i < MainTagArray.Count() ; i++ ) {
		TAG = MainTagArray.Item(i);
		if( TAG->isCover(pos) ) {
			TagHideAll();	//Hide first, or BUG by double hide...
			TagElement TAGtemp = *TAG;
			TagDialog *x=new TagDialog( TAGtemp, this );
			switch( x->ShowModal() ) { //blocker
				//switch( x->Show() ){ //Non-blocker but allways return 1=True...
				case wxID_SAVE:
					*TAG = TAGtemp;
					PreparePaintTAGs();
					ClearPaint();
					text_ctrl->RePaint();
					hex_ctrl ->RePaint();
					break;
				case wxID_DELETE:
					delete TAG;
					MainTagArray.Remove(TAG);
					PreparePaintTAGs();
					ClearPaint();
					text_ctrl->RePaint();
					hex_ctrl ->RePaint();
					break;
				default:
					break;
				}
			}
		}
	wxUpdateUIEvent eventx( TAG_CHANGE_EVENT );
	GetEventHandler()->ProcessEvent( eventx );
	}

void HexEditorCtrl::TagHideAll( void ) {
	hex_ctrl->OnTagHideAll();
	text_ctrl->OnTagHideAll();
	TAGMutex = false;
	}

bool HexEditorCtrl::LoadTAGS( wxFileName flnm, int64_t use_offset ) {
	wxXmlDocument doc;
	if( flnm.IsFileReadable() )
		if( doc.Load( flnm.GetFullPath(), wxT("UTF-8")) )
			if (doc.GetRoot()->GetName() == wxT("wxHexEditor-ng_XML_TAG")) {
				wxXmlNode *child = doc.GetRoot()->GetChildren();

				child = child->GetChildren();	//<filename> -> <TAG>
				while (child) {
					if (child->GetName() == wxT("TAG")) {
						wxString propvalue = child->GetAttribute(wxT("id"), wxEmptyString);
#ifdef _DEBUG_TAG_
						std::cout << "TAG ID:" << propvalue.ToAscii() << " readed.\n";
#endif
						TagElement *tmp = new TagElement();
						long long unsigned xxl=0;
						for( wxXmlNode *element = child->GetChildren() ; element != NULL ; element = element->GetNext() ) {
							if (element->GetName() == wxT("start_offset")) {
#ifdef __WXMSW__	//I don't knwo why but ToULongLong dowen't work on windows by mingw.
								xxl = atoll( element->GetNodeContent().ToAscii() );
#else
								element->GetNodeContent().ToULongLong( &xxl, 10 );
#endif
								tmp->start = xxl + use_offset;
								}
							else if (element->GetName() == wxT("end_offset")) {
#ifdef __WXMSW__
								xxl = atoll( element->GetNodeContent().ToAscii() );
#else
								element->GetNodeContent().ToULongLong( &xxl, 10 );
#endif
								tmp->end = xxl + use_offset;
								}
							else if (element->GetName() == wxT("tag_text"))
								tmp->tag = element->GetNodeContent();
							else if (element->GetName() == wxT("font_colour"))
								tmp->FontClrData.SetColour( wxColour(element->GetNodeContent()) );
							else if (element->GetName() == wxT("note_colour"))
								tmp->NoteClrData.SetColour( wxColour(element->GetNodeContent()) );
							}
#ifdef _DEBUG_TAG_
						tmp->print();
#endif
						MainTagArray.Add(tmp);
						}
					child = child->GetNext();
					}
				MainTagArray.Sort(TagElementSort);
				PreparePaintTAGs();
				ClearPaint();
				text_ctrl->RePaint();
				hex_ctrl ->RePaint();
				return true;
				}
	return false;
	}

//Moves tags for deletion & insertions
void HexEditorCtrl::MoveTAGs( uint64_t location, int64_t size ) {
	for( unsigned i = 0 ; i < MainTagArray.Count() ; i++ ) {
		TagElement *TAG = MainTagArray.Item(i);

		//Deletion, (-size) double negation indicates deletion range.
		if( size < 0 && TAG->start >= location && TAG->start < location+(-size) ) {
			//Deletion of code if start inside deletion selection.
			//i-- due MainTagArray.Count() shrinks
			MainTagArray.RemoveAt(i--);
			continue;
			}

		//Insert operation
		if( TAG->start >= location ) {
			TAG->start += size;
			TAG->end += size;
			}
		}

	for( unsigned i = 0 ; i < HighlightArray.Count() ; i++ ) {
		TagElement *TAG = HighlightArray.Item(i);

		//Deletion, (-size) double negation indicates deletion range.
		if( size < 0 && TAG->start >= location && TAG->start <= location+(-size) ) {
			//Deletion of code if start inside deletion selection.
			//i-- due HighlightArray.Count() shrinks
			HighlightArray.RemoveAt(i--);
			continue;
			}

		//Insert operation
		if( TAG->start >= location ) {
			TAG->start += size;
			TAG->end += size;
			}
		}

	wxUpdateUIEvent eventx( TAG_CHANGE_EVENT );
	GetEventHandler()->ProcessEvent( eventx );

	eventx.SetId( SEARCH_CHANGE_EVENT );
	GetEventHandler()->ProcessEvent( eventx );
	}

bool HexEditorCtrl::SaveTAGS( wxFileName flnm ) {
	if( MainTagArray.Count() ==  0) {
//		if( wxFileName::FileName( flnm.GetFullPath() ).FileExists() )
//			wxRemoveFile( flnm.GetFullPath() );
		if( wxFileName::FileName( flnm.GetFullPath() << wxT(".tags") ).FileExists() )
			wxRemoveFile( flnm.GetFullPath() << wxT(".tags") );
		return false;
		}
	else {
		wxXmlDocument doc;

		wxXmlNode *node_Root = new wxXmlNode( NULL, wxXML_ELEMENT_NODE, wxT("wxHexEditor-ng_XML_TAG"), wxEmptyString, NULL, NULL);

#if wxCHECK_VERSION(3, 0, 0)
		wxXmlAttribute *prop_filename = new wxXmlAttribute( wxT("path"), flnm.GetFullPath(), NULL);
#else
		wxXmlProperty *prop_filename = new wxXmlProperty( wxT("path"), flnm.GetFullPath(), NULL);
#endif
		wxXmlNode *node_File = new wxXmlNode( node_Root, wxXML_ELEMENT_NODE, wxT("filename"), wxEmptyString, prop_filename, NULL);

		MainTagArray.Sort(TagElementSort);
		for(signed i = MainTagArray.Count()-1 ; i>=0 ; i-- ) {
			//Used reverse order for make XML offsets increasing.
			TagElement *TAG = MainTagArray.Item(i);
#if wxCHECK_VERSION(3, 0, 0)
			wxXmlAttribute *ID = new wxXmlAttribute( wxT("id"), wxString::Format(wxT("%d"),i), NULL );
#else
			wxXmlProperty *ID = new wxXmlProperty( wxT("id"), wxString::Format(wxT("%d"),i), NULL );
#endif

			wxXmlNode *node_Tag = new wxXmlNode( node_File, wxXML_ELEMENT_NODE, wxT("TAG"), wxEmptyString, ID, NULL);

			wxXmlNode *element_NoteColour		= new wxXmlNode( node_Tag, wxXML_ELEMENT_NODE, wxT("note_colour"), wxEmptyString, NULL, NULL);
			new wxXmlNode( element_NoteColour, wxXML_TEXT_NODE, wxT("note_colour"), TAG->NoteClrData.GetColour().GetAsString(wxC2S_HTML_SYNTAX), NULL, NULL);

			wxXmlNode *element_FontColour		= new wxXmlNode( node_Tag, wxXML_ELEMENT_NODE, wxT("font_colour"), wxEmptyString, NULL, element_NoteColour);
			new wxXmlNode( element_FontColour, wxXML_TEXT_NODE, wxT("font_colour"), TAG->FontClrData.GetColour().GetAsString(wxC2S_HTML_SYNTAX), NULL, NULL);

			wxXmlNode *element_TagText		= new wxXmlNode( node_Tag, wxXML_ELEMENT_NODE, wxT("tag_text"), wxEmptyString, NULL, element_FontColour);
			new wxXmlNode( element_TagText, wxXML_TEXT_NODE, wxT("tag_text"), TAG->tag, NULL, NULL);

			wxXmlNode *element_End 			= new wxXmlNode( node_Tag, wxXML_ELEMENT_NODE, wxT("end_offset"), wxEmptyString, NULL, element_TagText);
			new wxXmlNode( element_End, wxXML_TEXT_NODE, wxT("end_offset"), wxString::Format("%" wxLongLongFmtSpec "u",TAG->end ), NULL, NULL);

			wxXmlNode *element_Start		= new wxXmlNode( node_Tag, wxXML_ELEMENT_NODE, wxT("start_offset"), wxEmptyString, NULL, element_End);
			new wxXmlNode( element_Start, wxXML_TEXT_NODE, wxT("start_offset"), wxString::Format("%" wxLongLongFmtSpec "u", TAG->start ), NULL, NULL);
			}
		doc.SetFileEncoding( wxT("UTF-8") );
		doc.SetRoot( node_Root );
		wxString path = flnm.GetFullPath();
		if( !path.Lower().EndsWith(wxT(".tags")) )
			path += wxT(".tags");

		return doc.Save(path);
		}
	}

void HexEditorCtrl::SearchResultsToTAGs( void ) {
	std::cout << "wxHexEditorngCtrl::SearchResultsToTAGs()" << std::endl;
	for( unsigned i = 0 ; i < HighlightArray.Count() ; i++ ) {
		std::cout << "i:" << i << std::endl;
		TagElement *TAG = HighlightArray.Item(i);
		TagElement *tmp = new TagElement();
		*tmp=*TAG;
		bool found=false;
		for(unsigned i=0; i < MainTagArray.Count(); i++)
			if(*MainTagArray.Item(i)==*tmp)
				found=true;
		if(!found )
			MainTagArray.Add( tmp );
		}
	MainTagArray.Sort(TagElementSort);
	WX_CLEAR_ARRAY( HighlightArray );

	wxUpdateUIEvent eventx( TAG_CHANGE_EVENT );
	GetEventHandler()->ProcessEvent( eventx );

	eventx.SetId( SEARCH_CHANGE_EVENT );
	GetEventHandler()->ProcessEvent( eventx );
	}

void HexEditorCtrl::UpdateUI(wxUpdateUIEvent& event) {
	std::cout << "wxHexEditorngCtrl::OnUpdateUI()" << std::endl;
	//m_static_offset->SetLabel( offset_ctrl->hex_offset==false ? _("Offset: DEC") : _("Offset: HEX"));
	}

//------ADAPTERS----------//
//returns position of Hex Cursor
int HexEditorCtrl::GetLocalHexInsertionPoint() {
	return (hex_ctrl->IsShown() ? hex_ctrl->GetInsertionPoint() : text_ctrl->GetInsertionPoint()*GetCharToHexSize() );
	}
//returns position of Text Cursor
int HexEditorCtrl::GetLocalInsertionPoint() {
	return (FindFocus() == text_ctrl ? text_ctrl->GetInsertionPoint()*(GetCharToHexSize()/2) : hex_ctrl->GetInsertionPoint()/2 );
	}

void HexEditorCtrl::SetLocalHexInsertionPoint( int hex_location ) {	//Sets position of Hex Cursor
#ifdef _DEBUG_CARET_
	std::cout<< "Caret at Hex:" << std::dec << hex_location << std::endl;
#endif // _DEBUG_CARET_
	text_ctrl->SetInsertionPoint( hex_location/GetCharToHexSize() );
	hex_ctrl->SetInsertionPoint( hex_location );
	}
int64_t HexEditorCtrl::CursorOffset( void ) {
	return GetLocalInsertionPoint() + page_offset;
	}

uint64_t HexEditorCtrl::ProcessRAM_GetFootPrint() {
	uint64_t fp = 0;
	for( unsigned i = 0; i+1 < ProcessRAMMap.Count() ; i+=2) {
		fp += ProcessRAMMap.Item(i+1);
		fp -= ProcessRAMMap.Item(i);
		}
	return fp;
	}

uint64_t HexEditorCtrl::ProcessRAM_GetVirtualOffset( uint64_t offset ) {
	uint64_t fp=0;
	for( unsigned i = 0; i+1 < ProcessRAMMap.Count() ; i+=2) {
		ProcessRAMMap.Item(i);
		if( i == 0 && ProcessRAMMap.Item(i) > offset )
			return 0;
		//If map end smaller than offset, just add memory map size to "fp"
		if( offset > ProcessRAMMap.Item(i+1) ) {
			fp += ProcessRAMMap.Item(i+1);
			fp -= ProcessRAMMap.Item(i);
			}
		//if map end bigger than offset, and map start smaller than offset,
		else if( offset >= ProcessRAMMap.Item(i) ) {
			ProcessRAMMap.Item(i);
			fp += offset - ProcessRAMMap.Item(i);
			return fp;
			}
		//if map start bigger than offset, means offset is non mapped region.
		else //( offset < ProcessRAMMap.Item(i) )
			return fp;
		}
	return 0;
	}

uint64_t HexEditorCtrl::ProcessRAM_FindNextMap( uint64_t offset, bool backward ) {
	if(ProcessRAMMap.Count()) {
		if(backward) {
			for( int i=ProcessRAMMap.Count()-2; i >= 0 ; i-=2 )
				if( ProcessRAMMap.Item(i+1) < offset )
					return ProcessRAMMap.Item(i+1);
			}
		else {
			for( unsigned i=0; i < ProcessRAMMap.Count() ; i+=2 )
				if( ProcessRAMMap.Item(i) > offset )
					return ProcessRAMMap.Item(i);
			}
		}
	return 0;
	}

bool HexEditorCtrl::ProcessRAM_FindMap( uint64_t offset, uint64_t& start, uint64_t& end, bool backward ) {
	start=0;
	end=0;
	if(ProcessRAMMap.Count())
		for( unsigned i=0; i < ProcessRAMMap.Count() ; i+=2 )
			if(( ProcessRAMMap.Item(i) <=  offset ) &&
			      ( ProcessRAMMap.Item(i+1) >= offset )) {
				start=ProcessRAMMap.Item(i);
				end=ProcessRAMMap.Item(i+1);
				if( backward && i >= 2 ) {
					start=ProcessRAMMap.Item(i-2);
					end=ProcessRAMMap.Item(i-2+1);
					}
				else
					return false;
				return true;
				}
	return false;
	}

wxHugeScrollBar::wxHugeScrollBar( wxScrollBar* m_scrollbar_ ) {
	m_range = m_thumb = 0;
	m_scrollbar = m_scrollbar_;

	m_scrollbar->Connect( wxEVT_SCROLL_TOP, wxScrollEventHandler( wxHugeScrollBar::OnOffsetScroll ), NULL, this );
	m_scrollbar->Connect( wxEVT_SCROLL_BOTTOM, wxScrollEventHandler( wxHugeScrollBar::OnOffsetScroll ), NULL, this );
	m_scrollbar->Connect( wxEVT_SCROLL_LINEUP, wxScrollEventHandler( wxHugeScrollBar::OnOffsetScroll ), NULL, this );
	m_scrollbar->Connect( wxEVT_SCROLL_LINEDOWN, wxScrollEventHandler( wxHugeScrollBar::OnOffsetScroll ), NULL, this );
	m_scrollbar->Connect( wxEVT_SCROLL_PAGEUP, wxScrollEventHandler( wxHugeScrollBar::OnOffsetScroll ), NULL, this );
	m_scrollbar->Connect( wxEVT_SCROLL_PAGEDOWN, wxScrollEventHandler( wxHugeScrollBar::OnOffsetScroll ), NULL, this );
	m_scrollbar->Connect( wxEVT_SCROLL_THUMBTRACK, wxScrollEventHandler( wxHugeScrollBar::OnOffsetScroll ), NULL, this );
	m_scrollbar->Connect( wxEVT_SCROLL_THUMBRELEASE, wxScrollEventHandler( wxHugeScrollBar::OnOffsetScroll ), NULL, this );
	m_scrollbar->Connect( wxEVT_SCROLL_CHANGED, wxScrollEventHandler( wxHugeScrollBar::OnOffsetScroll ), NULL, this );
	}

wxHugeScrollBar::~wxHugeScrollBar() {
	m_scrollbar->Disconnect( wxEVT_SCROLL_TOP, wxScrollEventHandler( wxHugeScrollBar::OnOffsetScroll ), NULL, this );
	m_scrollbar->Disconnect( wxEVT_SCROLL_BOTTOM, wxScrollEventHandler( wxHugeScrollBar::OnOffsetScroll ), NULL, this );
	m_scrollbar->Disconnect( wxEVT_SCROLL_LINEUP, wxScrollEventHandler( wxHugeScrollBar::OnOffsetScroll ), NULL, this );
	m_scrollbar->Disconnect( wxEVT_SCROLL_LINEDOWN, wxScrollEventHandler( wxHugeScrollBar::OnOffsetScroll ), NULL, this );
	m_scrollbar->Disconnect( wxEVT_SCROLL_PAGEUP, wxScrollEventHandler( wxHugeScrollBar::OnOffsetScroll ), NULL, this );
	m_scrollbar->Disconnect( wxEVT_SCROLL_PAGEDOWN, wxScrollEventHandler( wxHugeScrollBar::OnOffsetScroll ), NULL, this );
	m_scrollbar->Disconnect( wxEVT_SCROLL_THUMBTRACK, wxScrollEventHandler( wxHugeScrollBar::OnOffsetScroll ), NULL, this );
	m_scrollbar->Disconnect( wxEVT_SCROLL_THUMBRELEASE, wxScrollEventHandler( wxHugeScrollBar::OnOffsetScroll ), NULL, this );
	m_scrollbar->Disconnect( wxEVT_SCROLL_CHANGED, wxScrollEventHandler( wxHugeScrollBar::OnOffsetScroll ), NULL, this );
	}

void wxHugeScrollBar::SetThumbPosition(int64_t setpos) {
#ifdef _DEBUG_SCROLL_
	std::cout << "SetThumbPosition()" << setpos << std::endl;
#endif
	m_thumb = setpos;
	if( m_range <= 2147483647) {
		m_scrollbar->SetThumbPosition( setpos );
		}
	else {
#ifdef _DEBUG_SCROLL_
		std::cout << "m_Range: " << m_range << std::endl;
		std::cout << "SetThumbPositionx(): " << static_cast<int>(setpos*(2147483648.0/m_range)) << std::endl;
#endif
		m_scrollbar->SetThumbPosition(  static_cast<int>(setpos*(2147483648.0/m_range)) );
		}
	}

void wxHugeScrollBar::SetScrollbar( int64_t Current_Position,int page_x, int64_t new_range, int pagesize, bool repaint ) {
	m_range = new_range;
	if(new_range <= 2147483647) { //if representable with 32 bit
		m_scrollbar->SetScrollbar( Current_Position, page_x, new_range, pagesize, repaint );
		}
	else {
#ifdef _DEBUG_SCROLL_
		std::cout << "new_range " << new_range << std::endl;
		std::cout << "Current_Position :" << (Current_Position*(2147483647/new_range)) << std::endl;
#endif
		m_scrollbar->SetScrollbar( (Current_Position*(2147483647/new_range)), page_x, 2147483647, pagesize, repaint );
		}
	SetThumbPosition( Current_Position );
	}

void wxHugeScrollBar::OnOffsetScroll( wxScrollEvent& event ) {
	if( m_range <= 2147483647) {
		m_thumb = event.GetPosition();
		}
	else {	//64bit mode
		int64_t here =event.GetPosition();
		if(here == 2147483646)	//if maximum set
			m_thumb = m_range-1;	//than give maximum m_thumb which is -1 from range
		else
			m_thumb = static_cast<int64_t>(here*(m_range/2147483647.0));
		}
	wxYieldIfNeeded();

#ifdef _DEBUG_SCROLL_
	if(event.GetEventType() == wxEVT_SCROLL_CHANGED)
		std::cout << "wxEVT_SCROLL_CHANGED"  << std::endl;
	if(event.GetEventType() == wxEVT_SCROLL_THUMBTRACK)
		std::cout << "wxEVT_SCROLL_THUMBTRACK"  << std::endl;
	if(event.GetEventType() == wxEVT_SCROLL_THUMBRELEASE)
		std::cout << "wxEVT_SCROLL_THUMBRELEASE"  << std::endl;
	if( event.GetEventType() == wxEVT_SCROLL_LINEDOWN )
		std::cout << "wxEVT_SCROLL_LINEDOWN"  << std::endl;
	if( event.GetEventType() == wxEVT_SCROLL_LINEUP )
		std::cout << "wxEVT_SCROLL_LINEUP" << std::endl;
	if( event.GetEventType() == wxEVT_SCROLL_PAGEUP )
		std::cout << "wxEVT_SCROLL_PAGEUP" << std::endl;
	if( event.GetEventType() == wxEVT_SCROLL_PAGEDOWN )
		std::cout << "wxEVT_SCROLL_PAGEDOWN" << std::endl;
	if( event.GetEventType() == wxEVT_SCROLLWIN_LINEUP )
		std::cout << "wxEVT_SCROLLWIN_LINEUP" << std::endl;
	if( event.GetEventType() == wxEVT_SCROLLWIN_LINEDOWN )
		std::cout << "wxEVT_SCROLLWIN_LINEDOWN" << std::endl;
#endif
	event.Skip();
	}

