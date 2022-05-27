#ifndef TILEMAP_H
#define TILEMAP_H

#include "sprite.h"
#include "texturebase.h"
#include <stdint.h>
class Application;

class TileMap : public Sprite
{
public:
    static const int FLIP_HORIZONTAL = 4;
    static const int FLIP_VERTICAL   = 2;
    static const int FLIP_DIAGONAL   = 1;
    static const int FLIP_EMPTY      = 8;

	TileMap(Application* application,
			int width, int height,
			TextureBase* texture, 
			int tilewidth, int tileheight,
			int spacingx, int spacingy,
			int marginx, int marginy,
			int displaywidth, int displayheight);
    virtual Sprite *clone() { TileMap *clone=new TileMap(application_,width_,height_,texture_,tilewidth_,tileheight_,spacingx_,spacingy_,marginx_,marginy_,displaywidth_,displayheight_); clone->cloneFrom(this); return clone; }
    void cloneFrom(TileMap *);

	virtual ~TileMap();
	
	
    void set(int x, int y, uint16_t tx, uint16_t ty, int flip, uint32_t tint, GStatus* status = NULL);
    void get(int x, int y, uint16_t* tx, uint16_t* ty, int *flip, uint32_t *tint, GStatus* status = NULL) const;
    void setRepeat(bool x,bool y) { repeatx_=x; repeaty_=y; }
    void setTexture(TextureBase* texture,
			int tilewidth, int tileheight,
			int spacingx, int spacingy,
			int marginx, int marginy);

	// TODO:
//	void rotate(int dx, int dy);
	void shift(int dx, int dy);

private:
	void shiftleft();
	void shiftright();
	void shiftup();
	void shiftdown();

	int width_, height_;
	TextureBase* texture_; 
	int tilewidth_, tileheight_;
	int spacingx_, spacingy_;
	int marginx_, marginy_;
	int displaywidth_, displayheight_;
	bool repeatx_,repeaty_;

private:
    virtual void doDraw(const CurrentTransform&, float sx, float sy, float ex, float ey);
    virtual void extraBounds(float* minx, float* miny, float* maxx, float* maxy) const;

private:

    struct TileId
    {
        TileId() : x(0), y(0), flip(FLIP_EMPTY), tint(0xFFFFFFFF) {}
        TileId(int x, int y) : x(x), y(y), flip(0), tint(0xFFFFFFFF) {}
        TileId(int x, int y, int flip) : x(x), y(y), flip(flip), tint(0xFFFFFFFF) {}

        uint16_t x;
        uint16_t y;
        int flip;
        uint32_t tint;
    };

    std::vector<TileId> tileids_;
	std::vector<float> vertices;
	std::vector<float> texcoords;
	std::vector<unsigned char> colors;
};


#endif
