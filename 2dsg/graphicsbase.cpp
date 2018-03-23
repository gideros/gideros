#include "graphicsbase.h"
#include "ogl.h"
#include "color.h"
#include <algorithm>

void GraphicsBase::clear()
{
	mode = ShaderProgram::Triangles;
	r_ = g_ = b_ = a_ = 1;
	isWhite_ = true;
	data = NULL;
	indices.clear();
	vertices.clear();
	texcoords.clear();
	colors.clear();
}

void GraphicsBase::draw(ShaderProgram *shp, VertexBuffer<unsigned short> *commonIndices)
{
	if (!commonIndices)
		commonIndices = &indices;
	if (commonIndices->empty())
		return;

	if (isWhite_ == false)
	{
		glPushColor();
		glMultColor(r_, g_, b_, a_);
	}

	if (data)
	{
        ShaderEngine::Engine->bindTexture(0,data->id());
        if (!shp) {
        	if (colors.empty())
        		shp=(data->parameters.format==eA8)?ShaderProgram::stdTextureAlpha:ShaderProgram::stdTexture;
        	else
        		shp=(data->parameters.format==eA8)?ShaderProgram::stdTextureAlphaColor:ShaderProgram::stdTextureColor;
        }
        shp->setData(ShaderProgram::DataTexture,ShaderProgram::DFLOAT,2,&texcoords[0],texcoords.size(),texcoords.modified,&texcoords.bufferCache);

    	int sc=shp->getSystemConstant(ShaderProgram::SysConst_TextureInfo);
    	if (sc>=0)
    	{
        	float textureInfo[4]={0,0,0,0};
       		textureInfo[0]=(float)data->width / (float)data->exwidth;
        	textureInfo[1]=(float)data->height / (float)data->exheight;
        	textureInfo[2]=1.0/data->exwidth;
        	textureInfo[3]=1.0/data->exheight;
    		shp->setConstant(sc,ShaderProgram::CFLOAT4,1,textureInfo);
    	}
		texcoords.modified = false;
	}
	else
	{
        if (!shp) shp=colors.empty()?ShaderProgram::stdBasic:ShaderProgram::stdColor;
	}

    if (!colors.empty())
    {
     /*   float r, g, b, a;
        glGetColor(&r, &g, &b, &a);
        if (r != r_ || g != g_ || b != b_ || a != a_)
        {
            r_ = r;
            g_ = g;
            b_ = b;
            a_ = a;

            for (size_t i = 0; i < originalColors_.size(); ++i)
            {
                unsigned int color = originalColors_[i].color;
                float alpha = originalColors_[i].alpha;

                alpha = std::min(std::max(alpha, 0.f), 1.f);

                unsigned int r = ((color >> 16) & 0xff) * r_ * a_ * alpha;
                unsigned int g = ((color >> 8) & 0xff) * g_ * a_ * alpha;
                unsigned int b = (color & 0xff) * b_ * a_ * alpha;
                unsigned int a = 255 * a_ * alpha;

                colors_[i * 4] = r;
                colors_[i * 4 + 1] = g;
                colors_[i * 4 + 2] = b;
                colors_[i * 4 + 3] = a;
            }
            colors_.Update();
        }*/

        shp->setData(ShaderProgram::DataColor,ShaderProgram::DUBYTE,4,&colors[0],colors.size()/4,colors.modified,&colors.bufferCache);
        colors.modified=false;
    }

    shp->setData(ShaderProgram::DataVertex,ShaderProgram::DFLOAT,2,&vertices[0],vertices.size(),vertices.modified,&vertices.bufferCache);
	shp->drawElements(mode, commonIndices->size(), ShaderProgram::DUSHORT, &((*commonIndices)[0]), commonIndices->modified, &commonIndices->bufferCache);
	vertices.modified = false;
	commonIndices->modified = false;

	if (isWhite_ == false)
	{
		glPopColor();
	}
}

void GraphicsBase::getBounds(float* pminx, float* pminy, float* pmaxx, float* pmaxy) const
{
	float minx = 1e30;
	float miny = 1e30;
	float maxx = -1e30;
	float maxy = -1e30;

	for (size_t i = 0; i < vertices.size(); ++i)
	{
		float x = vertices[i].x;
		float y = vertices[i].y;

		minx = std::min(minx, x);
		miny = std::min(miny, y);
		maxx = std::max(maxx, x);
		maxy = std::max(maxy, y);
	}

	if (pminx)
		*pminx = minx;
	if (pminy)
		*pminy = miny;
	if (pmaxx)
		*pmaxx = maxx;
	if (pmaxy)
		*pmaxy = maxy;
}
