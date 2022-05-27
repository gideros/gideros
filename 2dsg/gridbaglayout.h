#ifndef GRIDBAGLAYOUT_H_
#define GRIDBAGLAYOUT_H_

#include <stddef.h>
#include <vector>
#include <map>
#include <string>
struct GridInsets {
	float top;
	float left;
	float bottom;
	float right;
    GridInsets() : top(0),left(0),bottom(0),right(0) { }
    GridInsets(float t,float l, float b, float r) : top(t),left(l),bottom(b),right(r) { }
};

struct GridBagConstraints {
    //Placement in grid
    size_t gridx;
    size_t gridy;
    size_t gridwidth;
    size_t gridheight;
    //Relative weight
    double weightx;
    double weighty;
    //Anchor direction
    enum _Anchor {
      CENTER=0, NORTH, NORTHEAST,EAST,SOUTHEAST,SOUTH,SOUTHWEST,WEST,NORTHWEST
    } anchor;
    //Fill factors
    float fillX,fillY;
    //insets in cell
    GridInsets insets;
    float ipadx;
    float ipady;
    //Temporary placement
    size_t tempX;
    size_t tempY;
    size_t tempWidth;
    size_t tempHeight;
    float minWidth;
    float minHeight;
    //Minimum and prefered sizes
    float aminWidth,aminHeight;
    float prefWidth,prefHeight;
    //Anchor placement
    float anchorX,anchorY;
    //Absolute offset
    float offsetX,offsetY;
    //Source anchor point/relative offset
    float originX,originY;
    //Target aspect ratio
    float aspectRatio;
    //Pack
    bool optimizeSize;
    //Object group
    bool group;

    std::map<std::string,std::string> resolved;

    GridBagConstraints() {
        gridx = 0;
        gridy = 0;
        gridwidth = 1;
        gridheight = 1;

        weightx = 0;
        weighty = 0;
        anchor = CENTER;
        anchorX=0.5;
        anchorY=0.5;
        fillX = 0;
        fillY = 0;
        aspectRatio = 0;

        insets = GridInsets(0, 0, 0, 0);
        ipadx = 0;
        ipady = 0;

        prefWidth=prefHeight=minWidth=minHeight=aminWidth=aminHeight=-1;
        tempHeight=tempWidth=tempX=tempY=0;
        offsetX=offsetY=originX=originY=0;

        optimizeSize=false;
        group=false;
    }

    GridBagConstraints(size_t gridx, size_t gridy,
                              size_t gridwidth, size_t gridheight,
                              double weightx, double weighty,
                              _Anchor anchor, float fillX, float fillY,
                              GridInsets insets, float ipadx, float ipady,
                              float anchorX,float anchorY,
							  float offsetX,float offsetY,
                              float originX,float originY,
                              float aspectRatio) {
        this->gridx = gridx;
        this->gridy = gridy;
        this->gridwidth = gridwidth;
        this->gridheight = gridheight;
        this->fillX = fillX;
        this->fillY = fillY;
        this->ipadx = ipadx;
        this->ipady = ipady;
        this->insets = insets;
        this->anchor  = anchor;
        this->anchorX  = anchorX;
        this->anchorY  = anchorY;
        this->weightx = weightx;
        this->weighty = weighty;
        this->offsetX = offsetX;
        this->offsetY = offsetY;
        this->originX = originX;
        this->originY = originY;
        this->aspectRatio = aspectRatio;

        prefWidth=prefHeight=minWidth=minHeight=aminWidth=aminHeight=-1;
        tempHeight=tempWidth=tempX=tempY=0;
        optimizeSize=false;
    }
};

struct GridBagLayoutInfo {
    size_t width, height;          /* number of  cells: horizontal and vertical */
    float startx, starty;         /* starting point for layout */
    float reqWidth, reqHeight;
    std::vector<float> minWidth;             /* largest minWidth in each column */
    std::vector<float> minHeight;            /* largest minHeight in each row */
    std::vector<double> weightX;           /* largest weight in each column */
    std::vector<double> weightY;           /* largest weight in each row */
    bool valid;
    GridBagLayoutInfo(size_t width, size_t height) : startx(0), starty(0),reqWidth(0),reqHeight(0), minWidth(), minHeight(),weightX(),weightY(),valid(false) {
        this->width = width;
        this->height = height;
    }
    GridBagLayoutInfo() : width(0), height(0), startx(0), starty(0),reqWidth(0),reqHeight(0), minWidth(), minHeight(),weightX(),weightY(), valid(false)
    {
    }
};

class Sprite;
class GridBagLayout {
    GridBagConstraints defaultConstraints;
    GridBagLayoutInfo layoutInfo;
protected:
    struct Rectangle {
    	float x,y,width,height;
    	Rectangle(): x(0),y(0),width(0),height(0) { }
    };
    GridBagConstraints *lookupConstraints(Sprite *comp);
    /*void getLayoutOrigin (float &x,float &y);
    void getLayoutDimension(float *wdim,float *hdim,int &wsize, int &hsize);
    void getLayoutWeights(double *wdim,double *hdim,int &wsize, int &hsize);*/
    void preInitMaximumArraySizes(Sprite *parent,size_t &a0,size_t &a1);
    void AdjustForGravity(Sprite *comp,GridBagConstraints *constraints, Rectangle &r);
public:
    GridBagLayoutInfo layoutInfoCache[2];
    std::vector<float> columnWidths;
    std::vector<float> rowHeights;
    std::vector<double> columnWeights;
    std::vector<double> rowWeights;
    GridInsets pInsets;
    bool optimizing;
    bool equalizeCells;
    bool dirty;
    bool resizeContainer;
    float cellSpacingX,cellSpacingY;
    float gridAnchorX,gridAnchorY;
    float zOffset;
    std::map<std::string,std::string> resolved;
    std::map<std::string,std::map<int,std::string>> resolvedArray;
    GridBagLayout() :
            optimizing(false),equalizeCells(false),dirty(false),resizeContainer(false),
    		cellSpacingX(0),cellSpacingY(0), gridAnchorX(0.5), gridAnchorY(0.5),
			zOffset(0)
    {

    }
    GridBagLayoutInfo getLayoutInfo(Sprite *parent, int sizeflag);
    void getMinSize(Sprite *parent, GridBagLayoutInfo &info, float &w,float &h, GridInsets &insets);
    void ArrangeGrid(Sprite *parent,float pw,float ph);
    GridBagLayoutInfo *getCurrentLayoutInfo() { return &layoutInfo; }
};

#endif /* GRIDBAGLAYOUT_H_ */
