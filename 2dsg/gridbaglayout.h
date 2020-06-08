#ifndef GRIDBAGLAYOUT_H_
#define GRIDBAGLAYOUT_H_

#include <stddef.h>
#include <vector>
struct GridInsets {
	float top;
	float left;
	float bottom;
	float right;
    GridInsets() : top(0),left(0),bottom(0),right(0) { }
    GridInsets(float t,float l, float b, float r) : top(t),left(l),bottom(b),right(r) { }
};

struct GridBagConstraints {
    size_t gridx;
    size_t gridy;
    size_t gridwidth;
    size_t gridheight;
    double weightx;
    double weighty;
    enum _Anchor {
      CENTER=0, NORTH, NORTHEAST,EAST,SOUTHEAST,SOUTH,SOUTHWEST,WEST,NORTHWEST
    } anchor;
    enum _FillMode {
    	NONE=0,BOTH,HORIZONTAL,VERTICAL
    } fill;
    GridInsets insets;
    float ipadx;
    float ipady;
    size_t tempX;
    size_t tempY;
    size_t tempWidth;
    size_t tempHeight;
    float minWidth;
    float minHeight;
    float aminWidth,aminHeight;
    float prefWidth,prefHeight;
    float anchorX,anchorY;


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
        fill = NONE;

        insets = GridInsets(0, 0, 0, 0);
        ipadx = 0;
        ipady = 0;

        prefWidth=prefHeight=minWidth=minHeight=aminWidth=aminHeight=-1;
        tempHeight=tempWidth=tempX=tempY=0;
    }

    GridBagConstraints(size_t gridx, size_t gridy,
                              size_t gridwidth, size_t gridheight,
                              double weightx, double weighty,
							  _Anchor anchor, _FillMode fill,
                              GridInsets insets, float ipadx, float ipady,
                              float anchorX,float anchorY) {
        this->gridx = gridx;
        this->gridy = gridy;
        this->gridwidth = gridwidth;
        this->gridheight = gridheight;
        this->fill = fill;
        this->ipadx = ipadx;
        this->ipady = ipady;
        this->insets = insets;
        this->anchor  = anchor;
        this->anchorX  = anchorX;
        this->anchorY  = anchorY;
        this->weightx = weightx;
        this->weighty = weighty;

        prefWidth=prefHeight=minWidth=minHeight=aminWidth=aminHeight=-1;
        tempHeight=tempWidth=tempX=tempY=0;
    }

    bool isVerticallyResizable() {
        return (fill == BOTH || fill == VERTICAL);
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
    GridBagLayoutInfo(size_t width, size_t height) : startx(0), starty(0),reqWidth(0),reqHeight(0), minWidth(), minHeight(),weightX(),weightY() {
        this->width = width;
        this->height = height;
    }
    GridBagLayoutInfo() : width(0), height(0), startx(0), starty(0),reqWidth(0),reqHeight(0), minWidth(), minHeight(),weightX(),weightY()
    {
    }
};

class Sprite;
class GridBagLayout {
    GridBagConstraints defaultConstraints;
    GridBagLayoutInfo layoutInfo;
    Sprite *componentAdjusting;
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
    void AdjustForGravity(GridBagConstraints *constraints, Rectangle &r);
public:
    std::vector<float> columnWidths;
    std::vector<float> rowHeights;
    std::vector<double> columnWeights;
    std::vector<double> rowWeights;
    GridInsets pInsets;
    bool equalizeCells;
    bool dirty;
    bool resizeContainer;
    GridBagLayoutInfo getLayoutInfo(Sprite *parent, int sizeflag);
    void getMinSize(Sprite *parent, GridBagLayoutInfo info, float &w,float &h, GridInsets &insets);
    void ArrangeGrid(Sprite *parent,float pw,float ph);
    GridBagLayoutInfo *getCurrentLayoutInfo() { return &layoutInfo; }
};

#endif /* GRIDBAGLAYOUT_H_ */
