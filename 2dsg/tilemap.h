#ifndef TILEMAP_H
#define TILEMAP_H

#include "sprite.h"
#include "texturebase.h"

class Application;

class TileMap : public Sprite
{
public:
	static const int EMPTY_TILE = -2147483647 - 1;

    static const int FLIP_HORIZONTAL = 4;
    static const int FLIP_VERTICAL   = 2;
    static const int FLIP_DIAGONAL   = 1;

	TileMap(Application* application,
			int width, int height,
			TextureBase* texture, 
			int tilewidth, int tileheight,
			int spacingx, int spacingy,
			int marginx, int marginy,
			int displaywidth, int displayheight);

	virtual ~TileMap();
	
	
    void set(int x, int y, int tx, int ty, int flip, GStatus* status = NULL);
    void get(int x, int y, int* tx, int* ty, int *flip, GStatus* status = NULL) const;

	// TODO:
//	void rotate(int dx, int dy);
	void shift(int dx, int dy);

	static bool isEmpty(int x, int y)
	{
		return (x == EMPTY_TILE) && (y == EMPTY_TILE);
	}

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

private:
    virtual void doDraw(const CurrentTransform&, float sx, float sy, float ex, float ey);
    virtual void extraBounds(float* minx, float* miny, float* maxx, float* maxy) const;

private:

    struct TileId
    {
        TileId() : x(EMPTY_TILE), y(EMPTY_TILE), flip(0) {}
        TileId(int x, int y) : x(x), y(y), flip(0) {}
        TileId(int x, int y, int flip) : x(x), y(y), flip(flip) {}

        int x;
        int y;
        int flip;
    };

    std::vector<TileId> tileids_;
	std::vector<float> vertices;
	std::vector<float> texcoords;
};


#endif
