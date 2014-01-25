#pragma once
#include "../xml/XMLTagHandler.h"
#include <wx/string.h>
#include <vector>
#include "Project.h"

class EutychusTemplate :
	public XMLTagHandler
{
protected:
	class MetaDataTag :
		public XMLTagHandler
	{
	public:
		wxString name;
		wxString value;
		virtual bool HandleXMLTag(const wxChar *tag, const wxChar **attrs);
		virtual XMLTagHandler *HandleXMLChild(const wxChar *tag)
		{
			return NULL;
		}
	};

	class MetaDataTags :
		public XMLTagHandler
	{
	public:
		virtual XMLTagHandler *HandleXMLChild(const wxChar *tag);
		virtual bool HandleXMLTag(const wxChar *tag, const wxChar **attrs)
		{
			return true;
		}
		std::vector<MetaDataTag> children;
	} tags;

	wxArrayString projectAttrNames;
	wxArrayString projectAttrValues;
	virtual bool HandleXMLTag(const wxChar *tag, const wxChar **attrs);
	virtual XMLTagHandler *HandleXMLChild(const wxChar *tag);

public:
	bool parse(wxString& fname);
	void updateProject( AudacityProject * proj, wxArrayString &subNames, wxArrayString &subValues);
	void updateProjectMetaDataTag( AudacityProject * proj, wxString& tagName, wxArrayString &subNames, wxArrayString &subValues);
	EutychusTemplate(void);
	~EutychusTemplate(void);
};
