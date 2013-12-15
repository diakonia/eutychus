#pragma once
#include <wx\image.h>

class EutychusResImage :
	public wxImage
{

public:
	enum resImageId {
		logo
	};
	EutychusResImage(resImageId id);
	~EutychusResImage(void);
};
