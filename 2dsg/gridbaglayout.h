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
    unsigned int gridx;
    unsigned int gridy;
    //Span in grid
    unsigned int gridwidth;
    unsigned int gridheight;
    //Relativeplacement
    bool gridRelative;
    //If min size is too high, place the sprite in next(+1) or previous(-1) column or row depedning on gridx/gridy relative placement. 0 to disable
    signed char overflowMode;
    //Hide priority: if non 0 and min size is too high, hide prioritary Sprites
    unsigned char hidePriority;
    bool tempHide;
    //Anchor direction
    enum _Anchor {
      CENTER=0, NORTH, NORTHEAST,EAST,SOUTHEAST,SOUTH,SOUTHWEST,WEST,NORTHWEST
    } anchor;
    //Relative weight
    float weightx;
    float weighty;
    //Fill factors
    float fillX,fillY;
    //insets in cell
    GridInsets insets;
    float ipadx;
    float ipady;
    //Temporary placement
    unsigned int tempX;
    unsigned int tempY;
    unsigned int tempWidth;
    unsigned int tempHeight;
    float minWidth;
    float minHeight;
    //Minimum and prefered sizes
    float aminWidth,aminHeight;
    float prefWidth,prefHeight;
    //Anchor placement
    float anchorX,anchorY;
    //Extra size, after layout
    float extraW,extraH;
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
    //Is in group, set by gideros
    bool inGroup;
    //Auto clip
    bool autoClip;

    int64_t resolvedMap;
    std::map<int,std::string> resolved;

    GridBagConstraints() {
        gridx = 0;
        gridy = 0;
        gridwidth = 1;
        gridheight = 1;

        gridRelative=false;
        overflowMode=0;
        hidePriority=0;

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
        tempHide=false;
        offsetX=offsetY=originX=originY=extraW=extraH=0;

        optimizeSize=false;
        group=false;
        inGroup=false;
        autoClip=false;

        resolvedMap=0;
    }
};

#define GRIDBAGLAYOUT_ARRAY_PREALLOCATED    2
struct GridBagLayoutInfo {
    unsigned int width, height;          /* number of  cells: horizontal and vertical */
    float startx, starty;         /* starting point for layout */
    float reqWidth, reqHeight;
    float *minWidth; /* largest minWidth in each column */
    float minWidthPre[GRIDBAGLAYOUT_ARRAY_PREALLOCATED];
    float *minHeight; /* largest minHeight in each row */
    float minHeightPre[GRIDBAGLAYOUT_ARRAY_PREALLOCATED];
    float *weightX; /* largest weight in each column */
    float weightXPre[GRIDBAGLAYOUT_ARRAY_PREALLOCATED];
    float *weightY;/* largest weight in each row */
    float weightYPre[GRIDBAGLAYOUT_ARRAY_PREALLOCATED];
    float cellSpacingX,cellSpacingY;
    bool valid;
    GridBagLayoutInfo(unsigned int width, unsigned int height) : startx(0), starty(0),reqWidth(0),reqHeight(0),cellSpacingX(0),cellSpacingY(0),valid(false) {
        this->width=0;
        this->height=0;
        setSize(width,height);
    }
    void setSize(unsigned int width, unsigned int height) {
        if (this->width>GRIDBAGLAYOUT_ARRAY_PREALLOCATED)
        {
            delete[] minWidth;
            delete[] weightX;
        }
        if (this->height>GRIDBAGLAYOUT_ARRAY_PREALLOCATED)
        {
            delete[] minHeight;
            delete[] weightY;
        }
        this->width = width;
        this->height = height;
        if (width>GRIDBAGLAYOUT_ARRAY_PREALLOCATED) {
            minWidth=new float[width];
            weightX=new float[width];
        }
        else {
            minWidth=minWidthPre;
            weightX=weightXPre;
        }
        if (height>GRIDBAGLAYOUT_ARRAY_PREALLOCATED) {
            minHeight=new float[height];
            weightY=new float[height];
        }
        else {
            minHeight=minHeightPre;
            weightY=weightYPre;
        }
        for (unsigned int i=0;i<width;i++)
            minWidth[i]=weightX[i]=0;
        for (unsigned i=0;i<height;i++)
            minHeight[i]=weightY[i]=0;
    }
    GridBagLayoutInfo() : width(0), height(0), startx(0), starty(0),reqWidth(0),reqHeight(0), cellSpacingX(0),cellSpacingY(0), valid(false)
    {
        setSize(0,0);
    }
    ~GridBagLayoutInfo()
    {
        if (this->width>GRIDBAGLAYOUT_ARRAY_PREALLOCATED)
        {
            delete[] minWidth;
            delete[] weightX;
        }
        if (this->height>GRIDBAGLAYOUT_ARRAY_PREALLOCATED)
        {
            delete[] minHeight;
            delete[] weightY;
        }
    }
    GridBagLayoutInfo& operator=(const GridBagLayoutInfo& o)
    {
        if (this->width>GRIDBAGLAYOUT_ARRAY_PREALLOCATED)
        {
            delete[] minWidth;
            delete[] weightX;
        }
        if (this->height>GRIDBAGLAYOUT_ARRAY_PREALLOCATED)
        {
            delete[] minHeight;
            delete[] weightY;
        }
        width=o.width;
        height=o.height;
        startx=o.startx;
        starty=o.starty;
        reqWidth=o.reqWidth;
        reqHeight=o.reqHeight;
        cellSpacingX=o.cellSpacingX;
        cellSpacingY=o.cellSpacingY;
        valid=o.valid;
        if (width>GRIDBAGLAYOUT_ARRAY_PREALLOCATED) {
            minWidth=new float[width];
            memcpy(minWidth,o.minWidth,sizeof(*minWidth)*width);
            weightX=new float[width];
            memcpy(weightX,o.weightX,sizeof(*weightX)*width);
        }
        else
        {
            minWidth=minWidthPre;
            weightX=weightXPre;
            for (int i=0;i<GRIDBAGLAYOUT_ARRAY_PREALLOCATED;i++)
            {
                minWidthPre[i]=o.minWidthPre[i];
                weightXPre[i]=o.weightXPre[i];
            }
        }
        if (height>GRIDBAGLAYOUT_ARRAY_PREALLOCATED) {
            minHeight=new float[height];
            memcpy(minHeight,o.minHeight,sizeof(*minHeight)*height);
            weightY=new float[height];
            memcpy(weightY,o.weightY,sizeof(*weightY)*height);
        }
        else
        {
            minHeight=minHeightPre;
            weightY=weightYPre;
            for (int i=0;i<GRIDBAGLAYOUT_ARRAY_PREALLOCATED;i++)
            {
                minHeightPre[i]=o.minHeightPre[i];
                weightYPre[i]=o.weightYPre[i];
            }
        }
        return *this;
    }
    GridBagLayoutInfo(const GridBagLayoutInfo &o) {
        *this=o;
    }
};

class Sprite;
class GridBagLayout {
    GridBagLayoutInfo layoutInfo;
protected:
    struct Rectangle {
    	float x,y,width,height;
    	Rectangle(): x(0),y(0),width(0),height(0) { }
    };
    /*void getLayoutOrigin (float &x,float &y);
    void getLayoutDimension(float *wdim,float *hdim,int &wsize, int &hsize);
    void getLayoutWeights(double *wdim,double *hdim,int &wsize, int &hsize);*/
    void preInitMaximumArraySizes(std::vector<Sprite *> &candidates,size_t &a0,size_t &a1);
    void AdjustForGravity(Sprite *comp,GridBagConstraints *constraints, Rectangle &r);
public:
    int64_t resolvedMap;
    std::map<int,std::string> resolved;
    std::map<int,std::map<int,std::string>> resolvedArray;
    GridBagLayoutInfo layoutInfoCache[2];
    std::vector<float> columnWidths;
    std::vector<float> rowHeights;
    std::vector<float> columnWeights;
    std::vector<float> rowWeights;
    GridInsets pInsets;
    bool optimizing;
    bool equalizeCells;
    bool dirty;
    bool placing;
    bool resizeContainer;
    bool worldAlign;
    float cellSpacingX,cellSpacingY;
    float gridAnchorX,gridAnchorY;
    float offsetX,offsetY;
    float originX,originY;
    float zOffset;
    GridBagLayout() :
            optimizing(false),equalizeCells(false),dirty(false),placing(false),resizeContainer(false),worldAlign(false),
    		cellSpacingX(0),cellSpacingY(0), gridAnchorX(0.5), gridAnchorY(0.5),
            offsetX(0),offsetY(0),originX(0),originY(0),
			zOffset(0)
    {
        resolvedMap=0;
     }
    GridBagLayoutInfo *getLayoutInfo(Sprite *parent, int sizeflag, float pwidth, float pheight, GridBagLayoutInfo *nocache);
    void getMinSize(Sprite *parent, GridBagLayoutInfo *info, float &w,float &h, GridInsets &insets);
    void ArrangeGrid(Sprite *parent,float pw,float ph);
    GridBagLayoutInfo *getCurrentLayoutInfo() { return &layoutInfo; }
};

#define STRKEY_LAYOUT_columnWidths      1
#define STRKEY_LAYOUT_rowHeights        2
#define STRKEY_LAYOUT_columnWeights     3
#define STRKEY_LAYOUT_rowWeights        4
#define STRKEY_LAYOUT_insetTop          5
#define STRKEY_LAYOUT_insetLeft         6
#define STRKEY_LAYOUT_insetBottom       7
#define STRKEY_LAYOUT_insetRight        8
#define STRKEY_LAYOUT_equalizeCells     9
#define STRKEY_LAYOUT_resizeContainer   10
#define STRKEY_LAYOUT_worldAlign        11
#define STRKEY_LAYOUT_cellSpacingX      12
#define STRKEY_LAYOUT_cellSpacingY      13
#define STRKEY_LAYOUT_gridAnchorX       14
#define STRKEY_LAYOUT_gridAnchorY       15
#define STRKEY_LAYOUT_zOffset           16

#define STRKEY_LAYOUT_reqWidth          20
#define STRKEY_LAYOUT_reqHeight         21
#define STRKEY_LAYOUT_startx            22
#define STRKEY_LAYOUT_starty            23
#define STRKEY_LAYOUT_weightX           24
#define STRKEY_LAYOUT_weightY           25
#define STRKEY_LAYOUT_width             26
#define STRKEY_LAYOUT_height            27

#define STRKEY_LAYOUT_gridx             30
#define STRKEY_LAYOUT_gridy             31
#define STRKEY_LAYOUT_gridwidth         32
#define STRKEY_LAYOUT_gridheight        33
#define STRKEY_LAYOUT_weightx           34
#define STRKEY_LAYOUT_weighty           35
#define STRKEY_LAYOUT_anchor            36
#define STRKEY_LAYOUT_fillx             37
#define STRKEY_LAYOUT_filly             38
#define STRKEY_LAYOUT_aspectRatio       39
#define STRKEY_LAYOUT_anchorx           40
#define STRKEY_LAYOUT_anchory           41
#define STRKEY_LAYOUT_offsetx           42
#define STRKEY_LAYOUT_offsety           43
#define STRKEY_LAYOUT_originx           44
#define STRKEY_LAYOUT_originy           45
#define STRKEY_LAYOUT_ipadx             46
#define STRKEY_LAYOUT_ipady             47
#define STRKEY_LAYOUT_minWidth          48
#define STRKEY_LAYOUT_minHeight         49
#define STRKEY_LAYOUT_prefWidth         50
#define STRKEY_LAYOUT_prefHeight        51
#define STRKEY_LAYOUT_shrink            52
#define STRKEY_LAYOUT_group             53
#define STRKEY_LAYOUT_autoclip          54
#define STRKEY_LAYOUT_gridRelative      55
#define STRKEY_LAYOUT_overflowMode      56
#define STRKEY_LAYOUT_hidePriority      57
#define STRKEY_LAYOUT_extraw            58
#define STRKEY_LAYOUT_extrah            59

#endif /* GRIDBAGLAYOUT_H_ */
