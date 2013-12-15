#include "EutychusTemplate.h"
#include "../xml/XMLFileReader.h"
#include <vector>

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

bool EutychusTemplate::writeOut(wxString &fname, wxArrayString &subNames, wxArrayString &subValues)
{
	XMLFileWriter xfw;
	xfw.Open(fname,wxT("w"));
	GetActiveProject()->WriteXMLHeader(xfw);
	xfw.StartTag(wxT("project"));

	for(wxArrayString::iterator valIter = projectAttrValues.begin(), nameIter = projectAttrNames.begin();
		nameIter != projectAttrNames.end(); valIter++, nameIter++)
	{
		xfw.WriteAttr(*nameIter,doSubs(*valIter,subNames,subValues));
	}

	if(tags.children.size() > 0)
	{
		xfw.StartTag(wxT("tags"));
		for(std::vector<MetaDataTag>::iterator mdtIter = tags.children.begin();
			mdtIter != tags.children.end(); mdtIter++)
		{
			xfw.StartTag(wxT("tag"));
			xfw.WriteAttr(wxT("name"),mdtIter->name);
			xfw.WriteAttr(wxT("value"),doSubs(mdtIter->value,subNames,subValues));
			xfw.EndTag(wxT("tag"));
		}
		xfw.EndTag(wxT("tags"));
	}
	xfw.Close();
	return true;
}

void EutychusTemplate::updateProject(AudacityProject * proj, wxArrayString &subNames, wxArrayString &subValues)
{
	XMLTagHandler * tagsHandler = proj->HandleXMLChild(wxT("tags"));
	XMLTagHandler * tagHandler = tagsHandler->HandleXMLChild(wxT("tag"));
	
	wxArrayString attrs;
	for(wxArrayString::iterator valIter = projectAttrValues.begin(), nameIter = projectAttrNames.begin();
		nameIter != projectAttrNames.end(); valIter++, nameIter++)
	{
		attrs.Add(*nameIter);
		attrs.Add(doSubs(*valIter,subNames,subValues));
	}

	const wxChar** _attrs = new const wxChar*[attrs.size() + 1];
	int i = 0;
	for(wxArrayString::iterator iter = attrs.begin();iter != attrs.end();iter++)
		_attrs[i++] = *iter;
	_attrs[i++] = NULL;

	proj->HandleXMLTag(wxT("project"),_attrs);
	delete _attrs;

	if(tags.children.size() > 0)
	{
		for(std::vector<MetaDataTag>::iterator mdtIter = tags.children.begin();
			mdtIter != tags.children.end(); mdtIter++)
		{
			const wxChar *attrs[5];
			attrs[0] = wxT("name");
			attrs[1] = mdtIter->name;
			attrs[2] = wxT("value");
			wxString attrTemp = doSubs(mdtIter->value,subNames,subValues);
			attrs[3] = attrTemp;
			attrs[4] = NULL;
			tagHandler->HandleXMLTag(wxT("tag"),attrs);
		}
	}
}

void EutychusTemplate::updateProjectMetaDataTag(AudacityProject * proj, wxString& tagName, wxArrayString &subNames, wxArrayString &subValues)
{
	XMLTagHandler * tagsHandler = proj->HandleXMLChild(wxT("tags"));
	XMLTagHandler * tagHandler = tagsHandler->HandleXMLChild(wxT("tag"));

	std::vector<MetaDataTag>::iterator mdtIter;
	for(mdtIter = tags.children.begin();
		mdtIter != tags.children.end() && mdtIter->name != tagName; mdtIter++)
		;;
	if(mdtIter != tags.children.end())
	{
		const wxChar *attrs[5];
		attrs[0] = wxT("name");
		attrs[1] = mdtIter->name;
		attrs[2] = wxT("value");
		wxString attrTemp = doSubs(mdtIter->value,subNames,subValues);
		attrs[3] = attrTemp;
		attrs[4] = NULL;
		tagHandler->HandleXMLTag(wxT("tag"),attrs);
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

