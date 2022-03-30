#include "tilemap.h"
#include "ogl.h"
#include "application.h"

TileMap::TileMap(Application* application,
				 int width, int height,
				 TextureBase* texture, 
				 int tilewidth, int tileheight,
				 int spacingx, int spacingy,
				 int marginx, int marginy,
                 int displaywidth, int displayheight) : Sprite(application)
{
	texture_ = texture;
	texture_->ref();

    tileids_.resize(width * height, TileId());
		
	width_ = width;
	height_ = height;
	tilewidth_ = tilewidth;
	tileheight_ = tileheight;
	spacingx_ = spacingx;
	spacingy_ = spacingy;
	marginx_ = marginx;
	marginy_ = marginy;
	displaywidth_ = displaywidth;
	displayheight_ = displayheight;
	repeatx_= false;
	repeaty_ = false;
}

void TileMap::cloneFrom(TileMap *s)
{
    Sprite::cloneFrom(s);

    repeatx_=s->repeatx_;
    repeaty_=s->repeaty_;
    tileids_=s->tileids_;
    vertices=s->vertices;
    texcoords=s->texcoords;
    colors=s->colors;
}

void TileMap::setTexture(TextureBase* texture,
		int tilewidth, int tileheight,
		int spacingx, int spacingy,
		int marginx, int marginy)
{
	TextureBase *oldtex=texture_;
	texture_ = texture;
	texture_->ref();
	oldtex->unref();

	tilewidth_ = tilewidth;
	tileheight_ = tileheight;
	spacingx_ = spacingx;
	spacingy_ = spacingy;
	marginx_ = marginx;
	marginy_ = marginy;
	invalidate(INV_GRAPHICS);
}

TileMap::~TileMap()
{
	texture_->unref();
}

void TileMap::set(int x, int y, uint16_t tx, uint16_t ty, int flip, uint32_t tint, GStatus *status/* = NULL*/)
{
	if (x < 0 || y < 0 || x >= width_ || y >= height_)
	{
		if (status)
			*status = GStatus(2006); // Error #2006: The supplied index is out of bounds.

		return;
	}
	
	//	int id = (tx < 0 || ty < 0) ? -1 : (ty << 8 | tx);

	int index = x + y * width_;

    tileids_[index].x = tx;
    tileids_[index].y = ty;
    tileids_[index].flip = flip;
    tileids_[index].tint = tint;
	invalidate(INV_GRAPHICS);
}

void TileMap::get(int x, int y, uint16_t* tx, uint16_t* ty, int *flip, uint32_t *tint, GStatus *status/* = NULL*/) const
{
	if (x < 0 || y < 0 || x >= width_ || y >= height_)
	{
		if (status)
			*status = GStatus(2006); // Error #2006: The supplied index is out of bounds.

		return;
	}

	int index = x + y * width_;

	if (tx)
        *tx = tileids_[index].x;
	if (ty)
        *ty = tileids_[index].y;
    if (flip)
        *flip = tileids_[index].flip;
    if (tint)
        *tint = tileids_[index].tint;
}

void TileMap::shift(int dx, int dy)
{
	invalidate(INV_GRAPHICS);
	while (dx < 0)
	{
		shiftleft();
		dx++;
	}
	while (dx > 0)
	{
		shiftright();
		dx--;
	}
	while (dy < 0)
	{
		shiftup();
		dy++;
	}
	while (dy > 0)
	{
		shiftdown();
		dy--;
	}
}

void TileMap::shiftleft()
{
	for (int y = 0; y < height_; y++)
	{
		for (int x = 1; x < width_; x++)
		{
			int index = x + y * width_;
			// transfer [x][y] -> [x-1][y]

			tileids_[index - 1] = tileids_[index];
		}

		{
			int index = (width_ - 1) + y * width_;

            tileids_[index] = TileId();
		}
	}
}

void TileMap::shiftright()
{
	for (int y = 0; y < height_; y++)
	{
		for (int x = width_ - 2; x >= 0; x--)
		{
			int index = x + y * width_;
			// transfer [x][y] -> [x+1][y]

			tileids_[index + 1] = tileids_[index];
		}

		{
			int index = 0 + y * width_;

            tileids_[index] = TileId();
		}
	}
}

void TileMap::shiftup()
{
	for (int x = 0; x < width_; x++)
	{
		for (int y = 1; y < height_; y++)
		{
			int index = x + y * width_;
			// transfer [x][y] -> [x][y-1]

			tileids_[index - width_] = tileids_[index];
		}

		{
			int index = x + (height_ - 1) * width_;

            tileids_[index] = TileId();
		}
	}
}

void TileMap::shiftdown()
{
	for (int x = 0; x < width_; x++)
	{
		for (int y = height_ - 2; y >= 0; y--)
		{
			int index = x + y * width_;
			// transfer [x][y] -> [x][y+1]

			tileids_[index + width_] = tileids_[index];
		}

		{
			int index = x + 0 * width_;

            tileids_[index] = TileId();
		}
	}
}

void TileMap::extraBounds(float* minx, float* miny, float* maxx, float* maxy) const
{
    if (minx)
        *minx = 0;
    if (miny)
        *miny = 0;
    if (maxx)
        *maxx = width_ * displaywidth_;
    if (maxy)
        *maxy = height_ * displayheight_;
}

void TileMap::doDraw(const CurrentTransform& transform, float hsx, float hsy, float hex, float hey)
{
	int sx, sy, ex, ey;
	{
        // inverse transformed hardware start/end x/y
        float x1, y1, x2, y2, x3, y3, x4, y4;
        Matrix inverse = transform.inverse();
        inverse.transformPoint(hsx, hsy, &x1, &y1);
        inverse.transformPoint(hex, hsy, &x2, &y2);
        inverse.transformPoint(hsx, hey, &x3, &y3);
        inverse.transformPoint(hex, hey, &x4, &y4);
        float thsx = std::min(std::min(x1, x2), std::min(x3, x4));
        float thsy = std::min(std::min(y1, y2), std::min(y3, y4));
        float thex = std::max(std::max(x1, x2), std::max(x3, x4));
        float they = std::max(std::max(y1, y2), std::max(y3, y4));

        // cell size width, cell size height
        float csw = std::max(std::max(tilewidth_, tileheight_), displaywidth_);
        float csh = std::max(std::max(tilewidth_, tileheight_), displayheight_);

        sx = floor((thsx - csw) / displaywidth_) + 1;
        sy = floor(thsy / displayheight_);
        ex = floor(thex / displaywidth_) + 1;
        ey = floor((they + csh) / displayheight_);

        // extra 1 block around
        sx--;
        sy--;
        ex++;
        ey++;

        if (!repeatx_)
        {
            sx = std::max(sx, 0);
            ex = std::min(ex, width_);
        }
        if (!repeaty_)
        {
            sy = std::max(sy, 0);
            ey = std::min(ey, height_);
        }
	}

	int tileCount = 0;
	uint32_t hasTint=0xFFFFFFFF;
	for (int y = sy; y < ey; ++y)
		for (int x = sx; x < ex; ++x)
		{
			int rx=repeatx_?((x%width_)+width_)%width_:x;
			int ry=repeaty_?((y%height_)+height_)%height_:y;
			int index = rx + ry * width_;

			if (!(tileids_[index].flip&FLIP_EMPTY))
			{
				tileCount++;
				hasTint&=tileids_[index].tint;
			}
		}

	if (tileCount == 0)
		return;

	vertices.resize(tileCount * 12);
	texcoords.resize(tileCount * 12);
	if (hasTint!=0xFFFFFFFF)
		colors.resize(tileCount*24);

	int pos = 0;
	float textureMargin=((texture_->data->parameters.filter==eNearest)||
			(spacingx_!=0)||(spacingy_!=0))?0.1f:0.5f;

	for (int y = sy; y < ey; ++y)
		for (int x = sx; x < ex; ++x)
		{
			int rx=repeatx_?((x%width_)+width_)%width_:x;
			int ry=repeaty_?((y%height_)+height_)%height_:y;
			int index = rx + ry * width_;

            int tx = tileids_[index].x;
            int ty = tileids_[index].y;
            int flip = tileids_[index].flip;

			if (!(tileids_[index].flip&FLIP_EMPTY))
			{
                bool flip_horizontal = (flip & FLIP_HORIZONTAL);
                bool flip_vertical = (flip & FLIP_VERTICAL);
                bool flip_diagonal = (flip & FLIP_DIAGONAL);

                float x0, y0, x1, y1;

                if (!flip_diagonal)
                {
                    x0 = x * displaywidth_;
                    y0 = y * displayheight_ - (tileheight_ - displayheight_);
                    x1 = x0 + tilewidth_;
                    y1 = y0 + tileheight_;
                }
                else
                {
                    x0 = x * displaywidth_;
                    y0 = y * displayheight_ - (tilewidth_ - displayheight_);
                    x1 = x0 + tileheight_;
                    y1 = y0 + tilewidth_;
                }

				vertices[pos + 0]  = x0; vertices[pos + 1]  = y0;
				vertices[pos + 2]  = x1; vertices[pos + 3]  = y0;
				vertices[pos + 4]  = x0; vertices[pos + 5]  = y1;
				vertices[pos + 6]  = x1; vertices[pos + 7]  = y0;
				vertices[pos + 8]  = x1; vertices[pos + 9]  = y1;
				vertices[pos + 10] = x0; vertices[pos + 11] = y1;

				int u0 = marginx_ + tx * (tilewidth_ + spacingx_);
				int v0 = marginy_ + ty * (tileheight_ + spacingy_);
				int u1 = marginx_ + tx * (tilewidth_ + spacingx_) + tilewidth_;
				int v1 = marginy_ + ty * (tileheight_ + spacingy_) + tileheight_;

                float fu0 = (textureMargin+u0*texture_->data->scale) / (float)texture_->data->exwidth;
                float fv0 = (textureMargin+v0*texture_->data->scale) / (float)texture_->data->exheight;
                float fu1 = (-textureMargin+u1*texture_->data->scale) / (float)texture_->data->exwidth;
                float fv1 = (-textureMargin+v1*texture_->data->scale) / (float)texture_->data->exheight;

               /* fu0 *= texture_->uvscalex;
                fv0 *= texture_->uvscaley;
                fu1 *= texture_->uvscalex;
                fv1 *= texture_->uvscaley;*/

                if (flip_horizontal)
                    std::swap(fu0, fu1);

                if (flip_vertical)
                    std::swap(fv0, fv1);

                if (flip_diagonal && (flip_vertical != flip_horizontal))
                {
                    std::swap(fu0, fu1);
                    std::swap(fv0, fv1);
                }

                if (!flip_diagonal)
                {
                    texcoords[pos + 0]  = fu0; texcoords[pos + 1]  = fv0;
                    texcoords[pos + 2]  = fu1; texcoords[pos + 3]  = fv0;
                    texcoords[pos + 4]  = fu0; texcoords[pos + 5]  = fv1;
                    texcoords[pos + 6]  = fu1; texcoords[pos + 7]  = fv0;
                    texcoords[pos + 8]  = fu1; texcoords[pos + 9]  = fv1;
                    texcoords[pos + 10] = fu0; texcoords[pos + 11] = fv1;
                }
                else
                {
                    texcoords[pos + 0]  = fu0; texcoords[pos + 1]  = fv0;
                    texcoords[pos + 2]  = fu0; texcoords[pos + 3]  = fv1;
                    texcoords[pos + 4]  = fu1; texcoords[pos + 5]  = fv0;
                    texcoords[pos + 6]  = fu0; texcoords[pos + 7]  = fv1;
                    texcoords[pos + 8]  = fu1; texcoords[pos + 9]  = fv1;
                    texcoords[pos + 10] = fu1; texcoords[pos + 11] = fv0;
                }

            	if (hasTint!=0xFFFFFFFF)
            	{
            		int cpos=pos*2;
            		unsigned int color = tileids_[index].tint&0xFFFFFF;
                    float alpha = (1.0/255)*(tileids_[index].tint>>24);

                    alpha = std::min(std::max(alpha, 0.f), 1.f);

                    unsigned int r = ((color >> 16) & 0xff) * alpha;
                    unsigned int g = ((color >> 8) & 0xff) * alpha;
                    unsigned int b = (color & 0xff) * alpha;
                    unsigned int a = tileids_[index].tint>>24;

            		for (int k=0;k<6;k++)
            		{
            			colors[cpos++]=r; colors[cpos++]=g; colors[cpos++]=b; colors[cpos++]=a;
            		}
            	}


				pos += 12;
			}
		}

	ShaderEngine::Engine->bindTexture(0,texture_->data->id());
	ShaderProgram *shd=getShader(((hasTint!=0xFFFFFFFF)?ShaderEngine::STDP_TEXTURECOLOR:ShaderEngine::STDP_TEXTURE));
    shd->setData(ShaderProgram::DataVertex,ShaderProgram::DFLOAT,2,&vertices[0],vertices.size()/2,true,NULL);
    shd->setData(ShaderProgram::DataTexture,ShaderProgram::DFLOAT,2,&texcoords[0],texcoords.size()/2,true,NULL);
	if (hasTint!=0xFFFFFFFF)
        shd->setData(ShaderProgram::DataColor,ShaderProgram::DUBYTE,4,&colors[0],colors.size()/4,true,NULL);
    shd->drawArrays(ShaderProgram::Triangles,0,tileCount * 6);
}



