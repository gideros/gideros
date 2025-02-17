#ifndef PR_H_INCLUDED
#define PR_H_INCLUDED

#include "sprite.h"
#include "texturebase.h"
#include "prpath.h"

class Path2D : public Sprite
{
private:
	static bool initialized;
    static VertexBuffer<unsigned short> *quadIndices;
	unsigned int path;
private:
    virtual void doDraw(const CurrentTransform&, float sx, float sy, float ex, float ey);
	virtual void extraBounds(float* minx, float* miny, float* maxx, float* maxy) const;
    int getMixedColor(int c1, int c2, float a1, float a2, float a, float &ao);

	TextureBase* texturebase_;
	Matrix4 textureMatrix_;
    float minx_, miny_, maxx_, maxy_;
    float filla_,fillr_,fillg_,fillb_;
    float linea_,liner_,lineg_,lineb_;
    float thickness_;
    float feather_;
    float margin_;
    float flatness_;
    bool convex_;
    int c1_, c2_, c3_, c4_;
    float a1_, a2_, a3_, a4_;
    VertexBuffer<unsigned char> colors_;
public:
    struct PathPoint {
    	float x;
    	float y;
    	float angle;
    	float offset;
    };
	Path2D(Application* application);
    virtual Sprite *clone() { Path2D *clone=new Path2D(application_); clone->cloneFrom(this); return clone; }
    void cloneFrom(Path2D *);
    virtual ~Path2D();
    void setTexture(TextureBase *texturebase, const Matrix4* matrix = NULL);
	void setPath(int num_commands, const unsigned char *commands, int num_coords, const float *coords);
	void setPath(const PrPath *path);
	void setConvex(bool convex);
	void setFillColor(unsigned int color, float alpha);
	void setLineColor(unsigned int color, float alpha);
	void setLineThickness(float thickness, float feather, float margin = -1, float flatness = -1);
	void getPathPoints(float offset, float advance,int max, float flatness, int maxsub,std::vector<PathPoint> &points);
    float getPathOffset(float px, float py, float accuracy, float &nextSegment);
    float getSegmentSize(unsigned int start,unsigned int end, float &length, float &nextSegment);
	static int buildPath(PrPath *);
	static void removePath(int);
	void drawPath(int path,Matrix4 xform,float fill[4],float line[4],TextureData *texture,bool convex,const Matrix4 *textureMatrix=NULL,VertexBuffer<unsigned char> *cb=NULL);
	void strokePath(int path,Matrix4 xform,float line[4]);
	void fillPath(int path,Matrix4 xform,float fill[4],TextureData *texture,bool convex,const Matrix4 *textureMatrix=NULL,VertexBuffer<unsigned char> *cb=NULL);
	void impressPath(int path,Matrix4 xform,ShaderEngine::DepthStencil stencil);
	void fillBounds(VertexBuffer<float> *vb,float *fill,TextureData *texture,ShaderEngine::DepthStencil stencil,const Matrix4 *textureMatrix=NULL,VertexBuffer<unsigned char> *cb=NULL);
	static void getPathBounds(int path,bool fill,bool stroke,float *minx,float *miny,float *maxx,float *maxy);
    void setGradient(int c1, float a1, int c2, float a2, int c3, float a3, int c4, float a4);
    bool hasGradient()
    {
        return !colors_.empty();
    }
    void getGradient(int &c1, float &a1, int &c2, float &a2, int &c3, float &a3, int &c4, float &a4)
    {
        c1 = c1_, a1 = a1_, c2 = c2_, a2 = a2_, c3 = c3_, a3 = a3_, c4 = c4_, a4 = a4_;
    }
    void setGradientWithAngle(int co1, float a1, int co2, float a2, float angle);
    void clearGradient()
    {
        colors_.clear();
    }
};


#define PATHCMD_CLOSE_PATH                                   0x00
#define PATHCMD_MOVE_TO                                      0x02
#define PATHCMD_RELATIVE_MOVE_TO                             0x03
#define PATHCMD_LINE_TO                                      0x04
#define PATHCMD_RELATIVE_LINE_TO                             0x05
#define PATHCMD_HORIZONTAL_LINE_TO                           0x06
#define PATHCMD_RELATIVE_HORIZONTAL_LINE_TO                  0x07
#define PATHCMD_VERTICAL_LINE_TO                             0x08
#define PATHCMD_RELATIVE_VERTICAL_LINE_TO                    0x09
#define PATHCMD_QUADRATIC_CURVE_TO                           0x0A
#define PATHCMD_RELATIVE_QUADRATIC_CURVE_TO                  0x0B
#define PATHCMD_CUBIC_CURVE_TO                               0x0C
#define PATHCMD_RELATIVE_CUBIC_CURVE_TO                      0x0D
#define PATHCMD_SMOOTH_QUADRATIC_CURVE_TO                    0x0E
#define PATHCMD_RELATIVE_SMOOTH_QUADRATIC_CURVE_TO           0x0F
#define PATHCMD_SMOOTH_CUBIC_CURVE_TO                        0x10
#define PATHCMD_RELATIVE_SMOOTH_CUBIC_CURVE_TO               0x11
#define PATHCMD_SMALL_CCW_ARC_TO                             0x12
#define PATHCMD_RELATIVE_SMALL_CCW_ARC_TO                    0x13
#define PATHCMD_SMALL_CW_ARC_TO                              0x14
#define PATHCMD_RELATIVE_SMALL_CW_ARC_TO                     0x15
#define PATHCMD_LARGE_CCW_ARC_TO                             0x16
#define PATHCMD_RELATIVE_LARGE_CCW_ARC_TO                    0x17
#define PATHCMD_LARGE_CW_ARC_TO                              0x18
#define PATHCMD_RELATIVE_LARGE_CW_ARC_TO                     0x19
#define PATHCMD_RESTART_PATH                                 0xF0
#define PATHCMD_DUP_FIRST_CUBIC_CURVE_TO                     0xF2
#define PATHCMD_DUP_LAST_CUBIC_CURVE_TO                      0xF4
#define PATHCMD_RECT                                         0xF6
#define PATHCMD_CIRCULAR_CCW_ARC_TO                          0xF8
#define PATHCMD_CIRCULAR_CW_ARC_TO                           0xFA
#define PATHCMD_CIRCULAR_TANGENT_ARC_TO                      0xFC
#define PATHCMD_ARC_TO                                       0xFE
#define PATHCMD_RELATIVE_ARC_TO                              0xFF

#define PATHJOIN_NONE			0
#define PATHJOIN_ROUND  		1
#define PATHJOIN_BEVEL  		2
#define PATHJOIN_MITER_REVERT	3
#define PATHJOIN_MITER_TRUNCATE	4

#define PATHEND_FLAT			0
#define PATHEND_ROUND			1
#define PATHEND_TRIANGLE		2
#define PATHEND_SQUARE			3

#endif
