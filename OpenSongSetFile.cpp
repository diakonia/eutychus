#include "OpenSongSetFile.h"
#include "../xml/XMLFileReader.h"
#include <vector>

OpenSongSetFile::OpenSongSetFile(void)
{
}

OpenSongSetFile::~OpenSongSetFile(void)
{
}

bool OpenSongSetFile::parse(wxString &fname)
{
	XMLFileReader xfr;
	return xfr.Parse(this,fname);
}

bool OpenSongSetFile::HandleXMLTag(const wxChar *tag, const wxChar **attrs)
{
	if(wxString(tag) == wxT("set"))
	{
		while(*attrs != NULL)
		{
			projectAttrNames.Add(wxString(*attrs++));
			if(*attrs != NULL)
				projectAttrValues.Add(wxString(*attrs++));
			else
				projectAttrValues.Add(wxString(wxT("")));
		}
		return true;
	}
	else
		return false;
}

XMLTagHandler *OpenSongSetFile::HandleXMLChild(const wxChar *tag)
{
	if(wxString(tag) == wxT("slide_groups"))
	{
		return &slideGroupsTag;
	}
	return NULL;
}

XMLTagHandler *OpenSongSetFile::SlideGroupsTag::HandleXMLChild(const wxChar *tag)
{
	if(wxString(tag) == wxT("slide_group"))
	{
		OpenSongSetFile::SlideGroupTag newChild;
		children.push_back(newChild);
		return &children.back();
	}
	return NULL;
}

XMLTagHandler *OpenSongSetFile::SlideGroupTag::HandleXMLChild(const wxChar *tag)
{
	if(wxString(tag) == wxT("notes"))
	{
		return &notesTag;
	}
	if(wxString(tag) == wxT("title"))
	{
		return &titleTag;
	}
	return NULL;
}

bool OpenSongSetFile::SlideGroupTag::HandleXMLTag(const wxChar *tag, const wxChar **attrs)
{
	if(wxString(tag) == wxT("slide_group"))
	{
		while(*attrs != NULL)
		{
			if(wxString(*attrs) == wxT("name"))
			{
				attrs++;
				name = wxString(*attrs++);
			}
			else if(wxString(*attrs) == wxT("type"))
			{
				attrs++;
				slideType = wxString(*attrs++);
			}
			else
			{
				projectAttrNames.Add(wxString(*attrs++));
				if(*attrs != NULL)
					projectAttrValues.Add(wxString(*attrs++));
				else
					projectAttrValues.Add(wxString(wxT("")));
			}
		}
		return true;
	}
	else
		return false;
}

void OpenSongSetFile::NotesTag::HandleXMLContent(const wxString & content)
{
	text = content;
}

wxString OpenSongSetFile::getReadings()
{
	wxString retVal = wxT("");
	for(std::vector<SlideGroupTag>::iterator iter = slideGroupsTag.children.begin();
		iter != slideGroupsTag.children.end();
		iter++)
	{
		wxString reading;

		if(iter->getType() == wxT("custom"))
		{
			if(iter->getName() == wxT("Gospel Reading") || iter->getName() == wxT("Reading"))
			{
				reading = iter->notesTag.getText();
				int i;
				if((i = reading.Find(wxT(" : "))) >= 0)
					reading = reading.Mid(i+3);
			}
		}
		else if(iter->getType() == wxT("scripture"))
		{
			reading = iter->titleTag.getText();
		}

		if(reading != wxT(""))
			if(retVal == wxT(""))
				retVal = wxT("Readings:\n") + reading;
			else
				retVal = retVal + wxT(";\n") + reading;
	}
	return retVal;
}