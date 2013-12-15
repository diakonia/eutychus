#pragma once
#include "Audacity.h"
#include <wx/defs.h>
#include <wx/wx.h>
#include <wx/dynarray.h>
#include <wx/datectrl.h>
#include <wx/dateevt.h>
#include <ctime>
#include <vector>
#include "OpenSongSetFile.h"
#include "../Project.h"
#include "../Commands/Command.h"
#include "../Commands/CommandType.h"

class EutychusPanel:
	public wxPanel,
	public wxThreadHelper
{
protected:
	AudacityProject* project;

	boolean manualTime;
	time_t timeStart;
	int instanceNo;
	wxArrayString subNames; wxArrayString subVals;
	wxArrayString preachers;
	wxArrayString eventTypes;
	wxArrayString sections;
	wxString saveDirectory;
	wxString archiveDirectory;
	wxString uploadDirectory;
	wxString openSongSetDirectory;
	wxString aupTemplate;
	wxString uploadCommand;

	wxString projName;
	wxString rawProjName;
	wxString aupPathName;
	wxString archivePathName;
//	wxString prevProjectPathName;
	wxString openSongSetPathName;
	wxString tryRawProjName(int tryInstanceNo);
	wxString tryProjName(int tryInstanceNo);
	wxString tryPathName(int tryInstanceNo);
	wxString tryDataDirName(int tryInstanceNo);
	void findFreeFile();
	void updateAlbumSubs();
	void updateTitleSubs();
	void loadOpenSongSetFile();
	void OnClose(wxCloseEvent& evt);
	virtual wxThread::ExitCode Entry();
	OpenSongSetFile* openSongSetFile;

	wxDatePickerCtrl* serviceStartDatePicker;
	wxChoice* amPmChoice;
	wxComboBox* preacherChoice;
	wxComboBox* eventTypeChoice;
	wxTextCtrl* eventDetailBox;
	wxTextCtrl* readingsBox;
	wxComboBox* sectionChoice;
	wxTextCtrl* sectionStartBox;
	wxTextCtrl* sectionEndBox;

	wxButton* newButton;
	wxButton* saveButton;


	class ServiceSection
	{
	protected:
		double start;
		double end;
		wxString name;
		boolean startIsSet;
		boolean endIsSet;
		boolean upload;
	public:
		ServiceSection(wxString& name, boolean upload)
		{
			this->name = name;
			this->upload = upload;
			this->start = 0.0;
			this->end = 0.0;
			this->startIsSet = false;
			this->endIsSet = false;
		}
		void setStart(double start)
		{
			this->start = start;
			if(!endIsSet)
				this->end = start;
			startIsSet = true;
		}
		void setEnd(double end)
		{
			this->end = end;
			endIsSet = true;
		}
		wxString& getName() { return name; }
		double getEnd() { return end;}
		double getStart() { return start; }
		boolean isEndSet() { return endIsSet; }
		boolean isStartSet() { return startIsSet; }
		boolean isUpload() { return upload; }
	};

	enum ThreadAction {
		EXPORT_ARCHIVAL,
		EXPORT_ALL,
		EXPORT_EXTRACT
	} threadAction;
	
	ServiceSection* extractSection;

	bool threadExecuteExport(ThreadAction threadAction, ServiceSection* extractSection=NULL);

	std::vector<ServiceSection> serviceSections;

	LabelTrack* getLabelTrack();
	void loadFromMetaData();
	void loadSectionsFromLabels();

	static bool createPanelInhibited;

public:
	EutychusPanel(AudacityProject* project);
	~EutychusPanel(void);

	void AddLabel(double left, double right, wxString labelText);
	void onNewButtonClick(wxCommandEvent & e);
	void onSaveButtonClick(wxCommandEvent & e);
	void onTestButtonClick(wxCommandEvent & e);
	void onMarkSectionStartButtonClick(wxCommandEvent & e);
	void onChooseSectionComboClick(wxCommandEvent & e);
	void onMarkSectionEndButtonClick(wxCommandEvent & e);
	void onServiceStartDateChange(wxDateEvent& event);

	void setSectionStart(double newStart);
	void setSectionEnd(double newEnd);

	wxString getExtractPathName(wxString& sectionName);

	void checkForNewFile();

	DECLARE_CLASS(EutychusPanel);
	DECLARE_EVENT_TABLE();

public:
	static bool isCreatePanelInhibited()
	{
		return createPanelInhibited;
	}
};