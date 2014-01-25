#include "EutychusPanel.h"
#include "Project.h"
#include "ShuttleGui.h"
#include "../Prefs.h"
#include "EutychusTemplate.h"
#include <string>
#include <ctime>
#include <wx/wfstream.h>
#include <wx/sstream.h>
#include <wx/txtstrm.h>
#include <wx/datetime.h>
#include <wx/debug.h>
#include "Registrar.h"
#include "../AudioIO.h"
#include "toolbars/ControlToolBar.h"
#include "../Tags.h"
#include "EutychusTrackPanel.h"
#include "EutychusResImage.h"
#include "../export/export.h"

IMPLEMENT_CLASS( EutychusPanel, wxPanel );

enum {
	newButtonID = 10000,
	saveButtonID,
	testButtonID,
	serviceStartDatePickerID,
	amPmChoiceID,
	preacherChoiceID,
	eventTypeChoiceID,
	eventDetailBoxID,
	readingsBoxID,
	markSectionStartButtonID,
	sectionStartBoxID,
	sectionComboID,
	markSectionEndButtonID,
	sectionEndBoxID,
};

BEGIN_EVENT_TABLE(EutychusPanel, wxPanel)
EVT_BUTTON(newButtonID, EutychusPanel::onNewButtonClick)
EVT_BUTTON(saveButtonID, EutychusPanel::onSaveButtonClick)
EVT_BUTTON(testButtonID, EutychusPanel::onTestButtonClick)
EVT_BUTTON(markSectionStartButtonID, EutychusPanel::onMarkSectionStartButtonClick)
EVT_BUTTON(markSectionEndButtonID, EutychusPanel::onMarkSectionEndButtonClick)
EVT_COMBOBOX(sectionComboID,EutychusPanel::onSectionComboClick)
EVT_CHOICE(amPmChoiceID,EutychusPanel::onAmPmChoiceClick)
EVT_DATE_CHANGED(serviceStartDatePickerID,EutychusPanel::onServiceStartDateChange)
EVT_CLOSE(EutychusPanel::OnClose)
END_EVENT_TABLE()

bool EutychusPanel::createPanelInhibited = false;

EutychusPanel::EutychusPanel(AudacityProject* project) :
wxPanel(project),
serviceSections()
{
	manualTime = false;
	this->project = project;
	openSongSetFile = NULL;
	preachers.Empty();
	int loop=0;
	wxString preacher, eventType, serviceSectionDetail;
	while((preacher = gPrefs->Read(wxString::Format(wxT("/EutychusServiceRecorder/Preacher%02d"),loop),wxT("")))!= wxT(""))
	{
		preachers.Add(preacher);
		loop++;
	}
	loop = 0;
	while((eventType = gPrefs->Read(wxString::Format(wxT("/EutychusServiceRecorder/EventType%02d"),loop),wxT("")))!= wxT(""))
	{
		eventTypes.Add(eventType);
		loop++;
	}
	loop = 0;
	while((serviceSectionDetail = gPrefs->Read(wxString::Format(wxT("/EutychusServiceRecorder/ServiceSections%02d"),loop),wxT("")))!= wxT(""))
	{
		wxString serviceSectionName = serviceSectionDetail.BeforeLast(wxChar(':'));
		boolean upload = (serviceSectionDetail.AfterLast(wxChar(':')) == wxT("1"));
		if(loop > 0)
			sections.Add(serviceSectionName);
		serviceSections.push_back(ServiceSection(serviceSectionName, upload));
		loop++;
	}
	ShuttleGui S(this, eIsCreating);
	wxArrayString empty;
	static const wxString colon = wxT(":");
	static const wxChar* _amPM[] = {wxT("AM"),wxT("PM")};
	static const wxArrayString amPM(2, _amPM);
	S.StartStatic(wxEmptyString);
	{
		S.StartHorizontalLay(wxALIGN_LEFT );
		S.Id(wxID_ANY).AddIcon(new wxBitmap(EutychusResImage(EutychusResImage::logo)));
		wxStaticText* txt = new wxStaticText(this,wxID_ANY,_("Eutychus Service Recorder"));
		wxFont font = txt->GetFont();
		font.SetPointSize(12);
		font.SetWeight(wxFONTWEIGHT_BOLD);
		txt->SetFont(font);
		S.Id(wxID_ANY).AddWindow(txt,wxALIGN_LEFT|wxALL);
		S.EndHorizontalLay();
	}
	S.EndStatic();
	S.StartStatic(_("Service"));
	{
		S.StartMultiColumn(2);
		{
			S.Id(wxID_ANY).AddPrompt(_("Date") + colon);

			S.StartHorizontalLay(wxALIGN_LEFT | wxSHRINK);
			{
				serviceStartDatePicker = new wxDatePickerCtrl(this,serviceStartDatePickerID);
				S.Id(serviceStartDatePickerID).AddWindow(serviceStartDatePicker);
				amPmChoice = S.Id(amPmChoiceID).AddChoice(wxEmptyString,wxEmptyString,&amPM);
			}
			S.EndHorizontalLay();
			preacherChoice = S.Id(preacherChoiceID).AddCombo(_("&Preacher") + colon,
				preachers[0],
				&preachers);
			eventTypeChoice = S.Id(eventTypeChoiceID).AddCombo(_("Event &Type") + colon,
				eventTypes[0],
				&eventTypes);
			eventDetailBox = S.Id(eventDetailBoxID).AddTextBox(_("Event &Detail") +  colon,
				wxT(""),50);
			S.Id(wxID_ANY).AddPrompt(_("&Readings etc.") + colon);
			readingsBox = S.Id(readingsBoxID).AddTextWindow(wxT(""));
			S.Id(-1).AddFixedText(wxString(wxT(" ")));
			S.StartMultiColumn(2);
			{
				newButton = S.Id(newButtonID).AddButton(_("&New"));
				saveButton = S.Id(saveButtonID).AddButton(_("&Save && Upload"));
				saveButton->Disable();
				//				S.Id(testID).AddButton(_("&Test"));
			}
			S.EndMultiColumn();
			sectionChoice = S.Id(sectionComboID).AddCombo(_("Section") + wxString(wxT(":")),sections[0],&sections);
			S.Id(-1).AddPrompt(_("Times") + colon);
			S.StartMultiColumn(2);
			{
				sectionStartBox =  S.Id(sectionStartBoxID).AddTextBox(wxT(""),wxT(""),19);
				sectionStartBox->Enable(false);
				sectionEndBox = S.Id(sectionEndBoxID).AddTextBox(wxT(""),wxT(""),19);
				sectionEndBox->Enable(false);
				S.Id(markSectionStartButtonID).AddButton(_("^="));
				S.Id(markSectionEndButtonID).AddButton(_("=^"));
			}
			S.EndMultiColumn();
		}
		S.EndMultiColumn();
	}
	S.EndStatic();

	Fit();
	project->GetControlToolBar()->Disable();
	if(wxDateTime::GetTmNow()->tm_hour>=12)
	{
		eventTypeChoice->SetSelection(1);
		amPmChoice->SetSelection(1);
	}
	else
	{
		eventTypeChoice->SetSelection(0);
		amPmChoice->SetSelection(0);
	}
	saveDirectory = gPrefs->Read(wxT("/EutychusServiceRecorder/SaveDirectory"),wxT("C:\\Users\\Public\\Documents\\ServiceRecordings\\"));
	archiveDirectory = gPrefs->Read(wxT("/EutychusServiceRecorder/ArchiveDirectory"),wxT("C:\\Users\\Public\\Documents\\ServiceRecordings\\ArchivalCopies"));
	uploadDirectory = gPrefs->Read(wxT("/EutychusServiceRecorder/UploadDirectory"),wxT("C:\\Users\\Public\\Documents\\ServiceRecordings\\ToWebSite"));
	openSongSetDirectory = gPrefs->Read(wxT("/EutychusServiceRecorder/OpenSongSetDirectory"),wxT("C:\\Users\\Public\\Documents\\OpenSong\\Sets\\"));
	aupTemplate = gPrefs->Read(wxT("/EutychusServiceRecorder/AupTemplate"),wxT("C:\\Users\\Public\\Documents\\ServiceRecordings\\template.aup"));
	uploadCommand = gPrefs->Read(wxT("/EutychusServiceRecorder/UploadCommand"),wxT("C:\\audio-uploader\\audio-uploader.py \"%FILE%\" \"%SECTION%\" http://stpolycarpchurch.org.uk/teaching/xmlrpc.php \"%USER%\" \"%PASSWORD%\""));

	if(autoInit)
	{
		wxCommandEvent e;
		onNewButtonClick(e);
	}
	autoInit = false;
}

void EutychusPanel::loadOpenSongSetFile()
{
	if(wxFileExists(openSongSetPathName))
	{
		openSongSetFile = new OpenSongSetFile();
		openSongSetFile->parse(openSongSetPathName);
	}
	else
	{
		if(openSongSetFile != NULL)
			delete openSongSetFile;
		openSongSetFile = NULL;
	}
}

const wxString TITLE_SEP = wxT(" - "); 

wxString EutychusPanel::tryRawProjName(int tryInstanceNo)
{
	wxDateTime dateTimeStart(timeStart);
	wxString retVal;

	if(manualTime)
	{
		dateTimeStart = serviceStartDatePicker->GetValue();
		if(amPmChoice->GetSelection() == 1)
			dateTimeStart.SetHour(12);
	}

	retVal = dateTimeStart.Format(wxT("%Y-%m-%d-%p"));
	if(tryInstanceNo > 1)
		retVal += wxT("-") + wxString::Format(wxT("%d"),tryInstanceNo);
	if(eventTypeChoice->GetValue() != wxT(""))
	{
		wxString temp = eventTypeChoice->GetValue();
		temp.Replace(wxT(" "),wxT(""),true);
		retVal += wxT("-") + temp;
	}
	if(eventDetailBox->GetValue() != wxT(""))
	{
		wxString temp = eventDetailBox->GetValue();
		retVal += wxT("-");
		for(wxString::iterator chiter = temp.begin();chiter != temp.end();chiter++)
			if(wxIsalnum(*chiter))
				retVal += *chiter;
	}
	return retVal;
}

wxString EutychusPanel::tryProjName(int tryInstanceNo)
{
	return tryRawProjName(tryInstanceNo) + wxT("-all-aud");
}

wxString EutychusPanel::tryPathName(int tryInstanceNo)
{
	return (saveDirectory + tryProjName(tryInstanceNo)) + wxT(".aup");
}

wxString EutychusPanel::tryDataDirName(int tryInstanceNo)
{
	return (saveDirectory + tryProjName(tryInstanceNo)) + wxT("_data");
}

void EutychusPanel::updateAlbumSubs()
{
	wxDateTime dateTimeStart(timeStart);

	if(manualTime)
	{
		dateTimeStart = serviceStartDatePicker->GetValue();
		if(amPmChoice->GetSelection() == 1)
			dateTimeStart.SetHour(12);
	}
	else
	{
		serviceStartDatePicker->SetValue(dateTimeStart);
		amPmChoice->SetSelection(-1);
		if(dateTimeStart.GetHour() >= 12)
			amPmChoice->SetSelection(1);
		else
			amPmChoice->SetSelection(0);
	}
	rawProjName = tryRawProjName(instanceNo);
	subNames.Empty(); subVals.Empty();
	subNames.Add(wxT("%YEAR%")); subVals.Add(dateTimeStart.Format(wxT("%Y")));
	subNames.Add(wxT("%AUDACITYVERSION%")); subVals.Add(AUDACITY_VERSION_STRING);
	wxString churchName = gPrefs->Read(wxT("/EutychusServiceRecorder/ChurchName"),wxT("**Church name not set!!**"));
	wxString temp  = dateTimeStart.Format(wxT("%#d%%s %B %Y"));
	wxString suffix;
	int dom = dateTimeStart.GetDay();
	if( dom == 1 || dom == 21 || dom == 31)
	{
		suffix = wxT("st");
	}
	else if(dom == 2 || dom == 22)
	{
		suffix = wxT("nd");
	}
	else if(dom == 3 || dom == 23)
	{
		suffix = wxT("rd");
	}
	else
	{
		suffix = wxT("th");
	}
	wxString title = wxString::Format(temp,suffix);
	if(eventTypeChoice->GetValue() != wxT(""))
	{
		title += TITLE_SEP;
		title += eventTypeChoice->GetValue();
	}
	if(eventDetailBox->GetValue() != wxT(""))
	{
		title += TITLE_SEP;
		title += eventDetailBox->GetValue();
	}
	projName = tryProjName(instanceNo);
	subNames.Add(wxT("%PROJNAME%")); subVals.Add(projName);

	wxString artist = churchName;
	if(preacherChoice->GetValue() != wxT(""))
	{
		artist += TITLE_SEP;
		artist += preacherChoice->GetValue();
	}
	subNames.Add(wxT("%ALBUM%")); subVals.Add(title);
	subNames.Add(wxT("%TITLE%")); subVals.Add(title);
	subNames.Add(wxT("%ARTIST%")); subVals.Add(artist);
	aupPathName = tryPathName(instanceNo);
	archivePathName = archiveDirectory + rawProjName + wxT("-all-flac24.flac");
	wxString prevSetPathName = openSongSetPathName;
	openSongSetPathName = openSongSetDirectory + dateTimeStart.Format(wxT("%Y-%m-%d-"));

	if(eventTypeChoice->GetValue() == wxT("Morning Service"))
		openSongSetPathName += wxT("Morning");
	else if(eventTypeChoice->GetValue() == wxT("Evening Service"))
		openSongSetPathName += wxT("Evening");
	else
	{
		openSongSetPathName += eventTypeChoice->GetValue();
	}

	if(prevSetPathName != openSongSetPathName)
	{
		loadOpenSongSetFile();
		if(openSongSetFile != NULL)
			readingsBox->SetValue(openSongSetFile->getReadings());
	}
	subNames.Add(wxT("%COMMENTS%")); subVals.Add(readingsBox->GetValue());
}

void EutychusPanel::updateTitleSubs(const wxString& sectionName)
{
	int index = subNames.Index(wxT("%ALBUM%"));
	wxString title = subVals[index];
	index = subNames.Index(wxT("%TITLE%"));
	if(sectionName != wxT(""))
		title += TITLE_SEP + sectionName;
	subVals[index] = title;
}

void EutychusPanel::findFreeFile()
{
	instanceNo = 1;
	wxString tryDataDir = tryDataDirName(instanceNo);
	wxString tryPath = tryPathName(instanceNo);
	while(wxFileExists(tryPath) || wxDirExists(tryPath) ||
		wxFileExists(tryDataDir) || wxDirExists(tryDataDir))
	{
		instanceNo++;
		tryDataDir = tryDataDirName(instanceNo);
		tryPath = tryPathName(instanceNo);
	}
}

bool EutychusPanel::autoInit = false;

void EutychusPanel::onNewButtonClick(wxCommandEvent &e)
{
	if(gAudioIO->IsBusy() || gAudioIO->IsPaused())
	{
		wxBell();
		return;
	}
	if(aupPathName != wxEmptyString)
	{
		autoInit = true;
		project->OnNew();
		return;
	}
	newButton->Disable();
	time(&timeStart);
   project = GetActiveProject();

	findFreeFile();
	updateAlbumSubs();

   project->SaveAs(aupPathName,false,true);

   EutychusTemplate srt;
	srt.parse(aupTemplate);
	srt.updateProject(project,subNames,subVals);

   project->Save();

	sectionChoice->SetSelection(0);
	project->GetControlToolBar()->Enable();
	saveButton->Enable();
}

LabelTrack* EutychusPanel::getLabelTrack()
{
	TrackList* tracks = project->GetTrackPanel()->GetTracks();
	TrackListIterator iter(tracks);

	Track* t = iter.First();

	while (t) {
		if (t->GetKind() == Track::Label)
			return (LabelTrack *) t;

		t = iter.Next();
	}
	return NULL;
}

void EutychusPanel::AddLabel(double left, double right, wxString labelText)
{

	LabelTrack *lt = getLabelTrack();

	// If none found, start a new label track and use it
	if (!lt) {
		lt = new LabelTrack(project->GetDirManager());
		project->GetTracks()->Add(lt);
	}
	int loop = 0;
	int count = lt->GetNumLabels();

	while(loop<count && (lt->GetLabel(loop)->title != labelText))
		loop++;

	if(loop < count)
	{
		lt->MayAdjustLabel(loop,-1,true,left);
		lt->MayAdjustLabel(loop,1,true,right);
	}
	else
	{
		int index = lt->AddLabel(left, right,labelText);
	}
}

void EutychusPanel::loadFromMetaData()
{
	wxString fn = wxFileName(project->GetFileName()).GetName();
	wxString tail = wxEmptyString;
	instanceNo = 1;
	bool foundDate=false;

	int index = -1;
	int count = 0;
	while( count < 4 && (index = fn.find(wxChar('-'),++index)) > 0)
	{
		count++;
	}
	if(index>0)
	{
		tail = fn.Mid(index+1);
		fn.Truncate(index);
	}

	wxDateTime dt;
	if(dt.ParseFormat(fn + wxT("-12"),wxT("%Y-%m-%d-%p-%I")))
	{
		/* PM does not seem to get used by no hour is s
		wxString amString, pmString;
		dt.GetAmPmStrings(&amString, &pmString);
		if(fn.AfterLast(wxChar('-')).CompareTo(pmString,wxString::ignoreCase)==0)
			dt.SetHour(12); */
		this->timeStart = dt.GetTicks();
		index = tail.find(wxChar('-'));
		if(index > -1)
		{
			long i;
			tail.Truncate(index);
			if(tail.IsNumber())
			{
				tail.ToLong(&i);
				instanceNo = i;
			}
		}
		foundDate = true;
	}
	else if(dt.ParseFormat(fn,wxT("%Y-%m-%d")))
	{
		this->timeStart = dt.GetTicks();
		foundDate = true;
	}
	else if(dt.ParseDate(fn))
	{
		wxMessageBox(wxT("Eutychus identified the date of this file as:") +
			dt.Format() + wxT(". Please verfiy and correct as needed."),wxT("Eutychus Service Recorder"));
		this->timeStart = dt.GetTicks();
		foundDate = true;
	}
	else
	{
		wxMessageBox(wxT("Unable to determine date of file - Please set manually!"),
			wxT("Eutychus Service Recorder"));
		time(&timeStart);
		foundDate = false;
	}
	if(foundDate)
	{
		manualTime = true;
		wxDateTime tempDT(timeStart);
		this->serviceStartDatePicker->SetValue(tempDT);
		amPmChoice->SetSelection(-1);
		if(tempDT.GetHour() >= 12)
		{
			this->amPmChoice->SetSelection(1);
		}
		else
		{
			this->amPmChoice->SetSelection(0);
		}
	}

	Tags* tags = project->GetTags();
	wxString artist =  tags->GetTag(wxT("ARTIST"));
	if(artist != wxEmptyString)
	{
		int i = artist.Find(TITLE_SEP);
		if(i > -1)
			preacherChoice->SetValue(artist.Mid(i+TITLE_SEP.length()));
	}
	wxString album = tags->GetTag(wxT("ALBUM"));
	if(album != wxEmptyString)
	{
		int i = album.Find(TITLE_SEP);
		wxString temp = album.Mid(i+TITLE_SEP.length());
		i = temp.Find(TITLE_SEP);
		if(i > 0)
		{
			eventTypeChoice->SetValue(temp.Left(i));
			eventDetailBox->SetValue(temp.Mid(i+TITLE_SEP.length()));
		}
		else
			eventTypeChoice->SetValue(temp);
	}
	wxString comments = tags->GetTag(wxT("COMMENTS"));
	if(comments != wxEmptyString)
	{
		readingsBox->SetValue(comments);
	}
}

void EutychusPanel::loadSectionsFromLabels()
{
	LabelTrack *lt = getLabelTrack();

	if (!lt)
	{
		return;
	}

	int loop = 0;
	int count = lt->GetNumLabels();
	while(loop<count)
	{
		const LabelStruct* label = lt->GetLabel(loop);
		for(std::vector<ServiceSection>::iterator iter = serviceSections.begin();
			iter != serviceSections.end();
			iter++)
		{
			if(iter->getName() == label->title)
			{
				iter->setStart(label->t);
				if(label->t != label->t1)
					iter->setEnd(label->t1);
			}
		}
		loop++;
	}
}

void transformFile(wxString& inPath, wxString outPath, wxString& find, wxString& replace)
{
	wxFileInputStream inStream(inPath);
	wxTextInputStream textInStream(inStream,wxT("\n"));
	wxFileOutputStream outStream(outPath);
	wxTextOutputStream textOutStream(outStream);
	while(!inStream.Eof())
	{
		wxString line = textInStream.ReadLine();
		line.Replace(find,replace,true);
		textOutStream.WriteString(line);
		textOutStream.WriteString(wxT("\n"));
	}
	outStream.Close();
}



void EutychusPanel::onSaveButtonClick(wxCommandEvent &e)
{
	if(gAudioIO->IsBusy())
	{
		wxBell();
		return;
	}
	AudacityApp* app = (AudacityApp*)(wxApp::GetInstance());
	EutychusTemplate srt;
	srt.parse(aupTemplate);
	wxString aupPathNameLast = aupPathName;
	wxString projNameLast = projName;
	wxString dirNameLast = saveDirectory + projNameLast + wxT("_data");

	updateAlbumSubs();

	bool anySet = false;
	wxString firstUploadable = wxEmptyString;

	for(std::vector<ServiceSection>::iterator iter = serviceSections.begin();
		iter != serviceSections.end();
		iter++)
	{
		if(iter->isUpload())
		{
			if(firstUploadable == wxEmptyString)
				firstUploadable = iter->getName();
			if(iter->isEndSet() && iter->isStartSet())
			{
				anySet = true;
				break;
			}
		}
	}
	
	if(!anySet && wxMessageBox(
		wxT("You have not marked the start and end of any uploadable sections, e.g. the section called \"") + firstUploadable + wxT("\"; This means nothing will be uploaded. Do you wish to continue?"),wxT("Upload failure"),
		wxCENTRE | wxYES_NO) != wxYES)
		return;

	if(aupPathNameLast != aupPathName)
	{
		findFreeFile();
		updateAlbumSubs();
		srt.updateProject(project,subNames,subVals);
		if(project->SaveAs(aupPathName,false,true))
		{
			if(wxFileExists(aupPathNameLast))
			{
				wxRemoveFile(aupPathNameLast);
				wxRmDir(dirNameLast);
			}
		}
	}
	else
	{
		srt.updateProject(project,subNames,subVals);
		project->Save();
	}
	updateTitleSubs(wxEmptyString);
	srt.updateProjectMetaDataTag(project,wxString(wxT("TITLE")),subNames,subVals);

	gPrefs->Write(wxT("FileFormats/FLACLevel"),wxT("8"));
	gPrefs->Write(wxT("FileFormats/FLACBitDepth"),wxT("24"));
	if(wxFileExists(archivePathName))
		wxRemoveFile(archivePathName);
	if(!executeExport(EXPORT_ARCHIVAL))
		return;

	for(std::vector<ServiceSection>::iterator iter = serviceSections.begin();
		iter != serviceSections.end();
		iter++)
	{
		if(iter->isUpload() && iter->isEndSet() && iter->isStartSet())
		{
			wxString pathName = getExtractPathName(iter->getName());
			if(wxFileExists(pathName))
				wxRemoveFile(pathName);
			if(iter == serviceSections.begin())
			{
				if(!executeExport(EXPORT_ALL,&(*iter)))
					return;
			}
			else
			{
				updateTitleSubs(iter->getName());
				srt.updateProjectMetaDataTag(project,wxString(wxT("TITLE")),subNames,subVals);
				project->SelectNone();
				project->SelectAllIfNone();
				project->SetSel0(iter->getStart());
				project->SetSel1(iter->getEnd());
				project->RedrawProject();
				project->ProcessPendingEvents();
				wxYieldIfNeeded();
				if(!executeExport(EXPORT_EXTRACT,&(*iter)))
					return;
			}
		}
	}

	app->ProcessPendingEvents();
	app->Yield();
	for(std::vector<ServiceSection>::iterator iter = serviceSections.begin();
		iter != serviceSections.end();
		iter++)
	{
		if(iter->isUpload() && iter->isEndSet() && iter->isStartSet())
		{
			wxString cmdLine=uploadCommand;
			cmdLine.Replace(wxT("%FILE%"),getExtractPathName(iter->getName()) ,true);
			bool keepTrying = true;
			long exitCode = wxExecute(cmdLine,wxEXEC_SYNC);
			while(exitCode != 0 && wxMessageBox(iter->getName() + wxT(": Upload failed! Retry?"),wxT("Upload failure"),wxCENTRE | wxYES_NO) == wxYES)
			{
				exitCode = wxExecute(cmdLine,wxEXEC_SYNC);
			}
		}
	}
	newButton->Enable();
}

bool EutychusPanel::executeExport(EutychusPanel::ExportAction exportAction, EutychusPanel::ServiceSection* extractSection)
{
	Exporter exporter;
	double t0, t1;
	wxString fileType =
		(exportAction==EutychusPanel::EXPORT_ARCHIVAL) ?
		wxT("FLAC") : wxT("MP3");
	wxString fileName =
		(exportAction == EXPORT_ARCHIVAL) ? 
		archivePathName : getExtractPathName(extractSection->getName());

	if(exportAction == EutychusPanel::EXPORT_ALL || exportAction == EutychusPanel::EXPORT_ARCHIVAL)
	{
		t0 = 0.0;
		t1 = project->GetTracks()->GetEndTime();
	}
	else
	{
		t0 = extractSection->getStart();
		t1 = extractSection->getEnd();
	}
	return exporter.Process(project,2,fileType,fileName,false,t0,t1);

}

void EutychusPanel::onTestButtonClick(wxCommandEvent &e)
{
}

static wxString TimeElapsed2Text(double te)
{
	int tei = (int)floor(te);
	double secs = fmod(te,60.0);
	return wxString::Format(wxT("%02dh %02dm %04.1fs"),
		tei / 3600, (tei / 60) % 60, secs);
}

void EutychusPanel::setSectionStart(double newStart)
{
	using namespace std;

	sectionStartBox->SetValue(TimeElapsed2Text(newStart));
	ServiceSection& sect = serviceSections[sectionChoice->GetSelection()+1];
	sect.setStart(newStart);
	AddLabel(newStart,sect.getEnd(),sect.getName());
	vector<ServiceSection>::iterator iter = serviceSections.begin();
	int loop = 0;
	while(iter != serviceSections.end())
	{
		if(iter->isStartSet() && ! iter->isEndSet() && iter->getStart() < newStart)
		{
			ServiceSection& sect = serviceSections[loop];
			iter->setEnd(newStart);
			AddLabel(sect.getStart(),sect.getEnd(),sect.getName());
		}
		iter++; loop++;
	}
	project->GetTrackPanel()->Refresh();
}

void EutychusPanel::setSectionEnd(double newEnd)
{
	ServiceSection& sect = serviceSections[sectionChoice->GetSelection()+1];
	sectionEndBox->SetValue(TimeElapsed2Text(newEnd));
	sect.setEnd(newEnd);
	if(sect.isStartSet())
	{
		AddLabel(sect.getStart(),sect.getEnd(),sect.getName());
		project->GetTrackPanel()->Refresh();
	}
}

void EutychusPanel::onMarkSectionStartButtonClick(wxCommandEvent &e)
{
	double st;
	if(gAudioIO->IsPaused() || gAudioIO->IsBusy() )
		st  = gAudioIO->GetStreamTime();
	else
		st = project->GetSel0();
	setSectionStart( st == BAD_STREAM_TIME ? 0.0 : st );
}

void EutychusPanel::onMarkSectionEndButtonClick(wxCommandEvent &e)
{
	double et;
	if(gAudioIO->IsPaused() || gAudioIO->IsBusy() )
		et  = gAudioIO->GetStreamTime();
	else
		et = project->GetSel1();
	setSectionEnd( et == BAD_STREAM_TIME ? 0.0 : et );
}

void EutychusPanel::onSectionComboClick(wxCommandEvent& e)
{
	ServiceSection& sect = serviceSections[sectionChoice->GetSelection()+1];
	if(sect.isStartSet())
		sectionStartBox->SetValue(TimeElapsed2Text(sect.getStart()));
	else
		sectionStartBox->SetValue(wxT(""));
	if(sect.isEndSet())
		sectionEndBox->SetValue(TimeElapsed2Text(sect.getEnd()));
	else
		sectionEndBox->SetValue(wxT(""));
}

wxString EutychusPanel::getExtractPathName(wxString& sectionName)
{
	wxString s = sectionName;
	s.Replace(wxT(" "),wxT(""),true);
	return uploadDirectory + rawProjName + wxT("-") + s + wxT("-mp3.mp3");
}

EutychusPanel::~EutychusPanel(void)
{
}

void EutychusPanel::OnClose(wxCloseEvent&)
{
	Destroy();
}

void EutychusPanel::checkForNewFile()
{
	if(aupPathName != project->GetFileName())
	{
		loadFromMetaData();
		loadSectionsFromLabels();
		project->GetControlToolBar()->Enable();
		saveButton->Enable(true);
		sectionChoice->SetSelection(0);
		onSectionComboClick(wxCommandEvent());
		aupPathName = project->GetFileName();
	}
}
void EutychusPanel::onServiceStartDateChange(wxDateEvent& event)
{
	manualTime = true;
	wxDateTime temp = serviceStartDatePicker->GetValue();
	if(amPmChoice->GetSelection() == 1)
		temp.SetHour(12);
	else
		temp.SetHour(0);
	timeStart = temp.GetTicks();
}

void EutychusPanel::onAmPmChoiceClick(wxCommandEvent& e)
{
	manualTime = true;
	wxDateTime temp = serviceStartDatePicker->GetValue();
	if(amPmChoice->GetSelection() == 1)
		temp.SetHour(12);
	else
		temp.SetHour(0);
	timeStart = temp.GetTicks();
}

