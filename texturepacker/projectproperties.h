#ifndef PROJECTPROPERTIES_H
#define PROJECTPROPERTIES_H

struct ProjectProperties
{
	ProjectProperties()
	{
		clear();
	}

	void clear()
	{
		padding = 1;
        extrude = 0;
		forceSquare = false;
		removeAlphaBorder = false;
		alphaThreshold = 0;
		showUnusedAreas = false;
	}

    int padding;
    int extrude;
    bool forceSquare;
	bool removeAlphaBorder;
	double alphaThreshold;
	bool showUnusedAreas;
};

#endif // PROJECTPROPERTIES_H
