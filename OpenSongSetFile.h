#pragma once
#include "../xml/XMLTagHandler.h"
#include <wx/string.h>
#include <vector>
#include "Project.h"

class OpenSongSetFile :
	public XMLTagHandler
{
protected:
	class NotesTag :
		public XMLTagHandler
	{
	protected:
		wxString text;
	public:
		virtual void HandleXMLContent(const wxString & content);
		virtual bool HandleXMLTag(const wxChar *tag, const wxChar **attrs)
		{
			return true;
		}
		virtual XMLTagHandler *HandleXMLChild(const wxChar *tag)
		{
			return NULL;
		}
		wxString getText()
		{
			return text;
		}
	};

	class TitleTag :
		public XMLTagHandler
	{
	protected:
		wxString text;
	public:
		virtual void HandleXMLContent(const wxString & content);
		virtual bool HandleXMLTag(const wxChar *tag, const wxChar **attrs)
		{
			return true;
		}
		virtual XMLTagHandler *HandleXMLChild(const wxChar *tag)
		{
			return NULL;
		}
		wxString getText()
		{
			return text;
		}
	};

	class SlideGroupTag :
		public XMLTagHandler
	{
	protected:
		wxString name;
		wxString slideType;
		wxArrayString projectAttrNames;
		wxArrayString projectAttrValues;
	public:
		virtual XMLTagHandler *HandleXMLChild(const wxChar *tag);
		virtual bool HandleXMLTag(const wxChar *tag, const wxChar **attrs);
		NotesTag notesTag;
		NotesTag titleTag;
		wxString getName()
		{
			return name;
		}
		wxString getType()
		{
			return slideType;
		}
	};

	class SlideGroupsTag :
		public XMLTagHandler
	{
	public:
		virtual XMLTagHandler *HandleXMLChild(const wxChar *tag);
		virtual bool HandleXMLTag(const wxChar *tag, const wxChar **attrs)
		{
			return true;
		}
		std::vector<SlideGroupTag> children;
	} slideGroupsTag;

	wxArrayString projectAttrNames;
	wxArrayString projectAttrValues;
	virtual bool HandleXMLTag(const wxChar *tag, const wxChar **attrs);
	virtual XMLTagHandler *HandleXMLChild(const wxChar *tag);

public:
	bool parse(wxString& fname);
	wxString getReadings();
	OpenSongSetFile(void);
	~OpenSongSetFile(void);
};
