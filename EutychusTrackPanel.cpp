/**********************************************************************

Audacity: A Digital Audio Editor

Registrar.cpp

James Crook

Audacity is free software.
This file is licensed under the wxWidgets license, see License.txt

********************************************************************//**

\class EutychusTrackPanel
\brief EutychusTrackPanel is the start of the new TrackPanel.

*//********************************************************************/

#include <wx/wx.h>
#include <wx/object.h>
#include <wx/control.h>
#include "ShuttleGui.h"
#include "widgets/LinkingHtmlWindow.h"
#include "SkewedRuler.h"
#include "Registrar.h"
#include "EutychusTrackPanel.h"
#include "../Project.h"

TrackPanel * EutychusTrackPanelFactory(wxWindow * parent,
								 wxWindowID id,
								 const wxPoint & pos,
								 const wxSize & size,
								 TrackList * tracks,
								 ViewInfo * viewInfo,
								 TrackPanelListener * listener,
								 AdornedRulerPanel * ruler)
{
	return new EutychusTrackPanel(
		parent,
		id,
		pos,
		size,
		tracks,
		viewInfo,
		listener,
		ruler);
}

int EutychusTrackPanelDispatch( Registrar & R, t_RegistrarDispatchType Type )
{
	switch( Type )
	{
	case RegResource:
		R.pShowFn = EutychusTrackPanel::showPanel;
		break;
	case RegArtist:
		break;
	case RegDataType:
		break;
	case RegCommand:
		break;
	case RegMenuItem:
		break;
	default:
		break;
	}
	return 1;
}

void EutychusTrackPanel::createEutychusPanel(void)
{
	if(! EutychusPanel::isCreatePanelInhibited())
	{
		EutychusPanel* pnl = new EutychusPanel(GetProject() /*,0,0,128,-1 */);
		attachEutychusPanel(pnl);
	}
}

void EutychusTrackPanel::attachEutychusPanel(EutychusPanel* pnl)
{
	wxWindow* ppnl = GetParent();
	wxWindow* rpnl = GetProject()->GetRulerPanel();
	wxSizer* szr =  GetProject()->GetSizer();
	szr->Detach(ppnl);
	szr->Detach(rpnl);

	wxBoxSizer* szrInner = new wxBoxSizer(wxHORIZONTAL);
	wxBoxSizer* szrInner2 = new wxBoxSizer(wxVERTICAL);
	szrInner2->Add(rpnl,0,wxEXPAND | wxALIGN_LEFT | wxALIGN_TOP);
	szrInner2->Add(ppnl,1,wxEXPAND | wxALIGN_LEFT);
	szrInner->Add(pnl,0,wxALIGN_LEFT | wxALIGN_TOP | wxEXPAND);
	szrInner->Add(szrInner2,1,wxALIGN_TOP | wxEXPAND);
	szr->Insert( 1, szrInner, 1, wxEXPAND | wxALIGN_LEFT );
	szr->Layout();
	eutychusPanel = pnl;
	panelParentSizer = szrInner;


}

void EutychusTrackPanel::detachEutychusPanel()
{
	panelParentSizer->Detach(eutychusPanel);
}

EutychusTrackPanel::EutychusTrackPanel(
						   wxWindow * parent, wxWindowID id, const wxPoint & pos, const wxSize & size,
						   TrackList * tracks, ViewInfo * viewInfo, TrackPanelListener * listener,
						   AdornedRulerPanel * ruler) : 
TrackPanel(
		   parent, id, pos, size,
		   tracks, viewInfo, listener, ruler)
{
	eutychusPanel = NULL;
}


// Here is a sample function that shows that EutychusTrackPanel is being invoked.
void EutychusTrackPanel::OnPaint(wxPaintEvent & event)
{
	// Hmm... Log debug will only show if you open the log window.
	// wxLogDebug( wxT("Paint EutychusTrackPanel requested") );
	TrackPanel::OnPaint( event );
}

void EutychusTrackPanel::SetFocusedTrack( Track *t )
{
	static int reenterGuard = 0;
	if(!reenterGuard)
	{
		reenterGuard++;
		if(GetProject()->GetFileName() != wxT("") && eutychusPanel != NULL)
		{
			eutychusPanel->checkForNewFile();
		}
		reenterGuard--;
	}
	TrackPanel::SetFocusedTrack(t);

	AudacityProject* p = GetProject();
	if (p && p->HasKeyboardCapture()) {
		wxCommandEvent e(EVT_RELEASE_KEYBOARD);
		e.SetEventObject(this);
		GetParent()->GetEventHandler()->ProcessEvent(e);
	}

	wxCommandEvent e(EVT_CAPTURE_KEYBOARD);
	e.SetEventObject(this);
	GetParent()->GetEventHandler()->ProcessEvent(e);
}

void EutychusTrackPanel::showPanel()
{
	EutychusTrackPanel* srtpnl = (EutychusTrackPanel*)(GetActiveProject()->GetTrackPanel());
	srtpnl->createEutychusPanel();
	AudacityProject *p = GetActiveProject();

}

void EutychusTrackPanel::OnCaptureKey(wxCommandEvent & event)
{
	wxWindow* fw = wxWindow::FindFocus();
	if(fw == NULL)
	{
		TrackPanel::OnCaptureKey(event);
	}
	if(fw->IsKindOf(CLASSINFO(wxTextCtrl)) || 
		fw->IsKindOf(CLASSINFO(wxComboBox)) ||
		fw->IsKindOf(CLASSINFO(wxChoice)) ||
		fw->IsKindOf(CLASSINFO(wxListBox)))
	{
		event.Skip(false);
	}
	else
		TrackPanel::OnCaptureKey(event);
}
