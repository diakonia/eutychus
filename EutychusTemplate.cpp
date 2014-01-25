#include "EutychusTemplate.h"
#include "../xml/XMLFileReader.h"
#include <vector>
#include "../Tags.h"


EutychusTemplate::EutychusTemplate(void)
{
}

EutychusTemplate::~EutychusTemplate(void)
{
}

bool EutychusTemplate::parse(wxString &fname)
{
	XMLFileReader xfr;
	return xfr.Parse(this,fname);
}

static wxString doSubs(wxString& input, wxArrayString& subNames, wxArrayString& subValues)
{
	wxString retVal = input;
	for(wxArrayString::iterator subNameIter = subNames.begin(), subValueIter = subValues.begin();
		subNameIter != subNames.end(); subNameIter++, subValueIter++)
	{
		retVal.Replace(*subNameIter,*subValueIter,true);
	}
	return retVal;
}


void EutychusTemplate::updateProject(AudacityProject * proj, wxArrayString &subNames, wxArrayString &subValues)
{
	if(tags.children.size() > 0)
	{
		for(std::vector<MetaDataTag>::iterator mdtIter = tags.children.begin();
			mdtIter != tags.children.end(); mdtIter++)
		{
			proj->GetTags()->SetTag(mdtIter->name,doSubs(mdtIter->value,subNames,subValues));
		}
	}
}

void EutychusTemplate::updateProjectMetaDataTag(AudacityProject * proj, wxString& tagName, wxArrayString &subNames, wxArrayString &subValues)
{

	std::vector<MetaDataTag>::iterator mdtIter;
	for(mdtIter = tags.children.begin();
		mdtIter != tags.children.end() && mdtIter->name != tagName; mdtIter++)
		;;
	if(mdtIter != tags.children.end())
	{
		proj->GetTags()->SetTag(mdtIter->name,doSubs(mdtIter->value,subNames,subValues));
	}

}

bool EutychusTemplate::HandleXMLTag(const wxChar *tag, const wxChar **attrs)
{
	if(wxString(tag) == wxT("project"))
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

XMLTagHandler *EutychusTemplate::HandleXMLChild(const wxChar *tag)
{
	if(wxString(tag) == wxT("tags"))
	{
		return &tags;
	}
	return NULL;
}

XMLTagHandler *EutychusTemplate::MetaDataTags::HandleXMLChild(const wxChar *tag)
{
	if(wxString(tag) == wxT("tag"))
	{
		EutychusTemplate::MetaDataTag newChild;
		children.push_back(newChild);
		return &children.back();
	}
	return NULL;
}

bool EutychusTemplate::MetaDataTag::HandleXMLTag(const wxChar *tag, const wxChar **attrs)
{
	if(wxString(tag) == wxT("tag"))
	{
		while(*attrs != NULL)
		{
			wxString a(*attrs);
			if(a == wxT("name"))
			{
				*attrs++;
				name = wxString(*attrs++);
			}else if(a == wxT("value"))
			{
				*attrs++;
				value = wxString(*attrs++);
			}
			else
			{
				return false;
			}
		}
		return true;
	}
	else
		return false;
}

