/**********************************************************************

Audacity: A Digital Audio Editor

EutychusTrackPanel.h

James Crook

**********************************************************************/

#ifndef __AUDACITY_S_R_TRACK_PANEL__
#define __AUDACITY_S_R_TRACK_PANEL__

#include "TrackPanel.h"
#include "EutychusPanel.h"

class EutychusTrackPanel : public TrackPanel
{
protected:
	void createEutychusPanel(void);
	EutychusPanel* eutychusPanel;
	wxSizer* panelParentSizer;
public:
	EutychusTrackPanel( 
		wxWindow * parent, wxWindowID id,
		const wxPoint & pos,
		const wxSize & size,
		TrackList * tracks,
		ViewInfo * viewInfo,
		TrackPanelListener * listener,
		AdornedRulerPanel * ruler);

	void attachEutychusPanel(EutychusPanel *pnl);
	void detachEutychusPanel();

	EutychusPanel* GetEutychusPanel()
	{
		return eutychusPanel;
	}

	// Upgrades an existing TrackPanel to a EutychusTrackPanel
	static void Upgrade( TrackPanel ** ppTrackPanel );

	virtual void OnPaint(wxPaintEvent & event);
	virtual void SetFocusedTrack(Track *t);
	static void showPanel();
	virtual void OnCaptureKey(wxCommandEvent & event);
};

// Factory function.
TrackPanel * EutychusTrackPanelFactory(wxWindow * parent,
								 wxWindowID id,
								 const wxPoint & pos,
								 const wxSize & size,
								 TrackList * tracks,
								 ViewInfo * viewInfo,
								 TrackPanelListener * listener,
								 AdornedRulerPanel * ruler);

#endif
