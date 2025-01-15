#include <gmesh.h>
#include <ogl.h>
#include <color.h>

GMesh::GMesh(Application *application,bool is3d) : Sprite(application)
{
	for (int t=0;t<MESH_MAX_TEXTURES;t++)
	{
		texture_[t] = NULL;
		sx_[t] = 1;
		sy_[t] = 1;
	}
    for (int k=0;k<MESH_MAX_ARRAYS;k++)
	{
        genericArray[k].ptr=NULL;
        genericArray[k].cache=NULL;
        genericArray[k].modified=true;
        genericArray[k].count=0;
        genericArray[k].offset=0;
        genericArray[k].stride=0;
        genericArray[k].master=-1;
    }
    r_ = 1;
    g_ = 1;
    b_ = 1;
    a_ = 1;
    boundsDirty_ = false;
    minx_ = miny_ = 1e30;
    maxx_ = maxy_ = -1e30;
    meshtype_=ShaderProgram::Triangles;
    mesh3d_=is3d;
    instanceCount_=0;
    cullMode_=ShaderEngine::CULL_NONE;
}

void GMesh::cloneFrom(GMesh *s) {
    Sprite::cloneFrom(s);
    r_ = s->r_, g_ = s->g_, b_ = s->b_, a_ = s->a_;
    minx_ = s->minx_;
    miny_ = s->miny_;
    maxx_ = s->maxx_;
    maxy_ = s->maxy_;
    meshtype_=s->meshtype_;
    instanceCount_=s->instanceCount_;
    cullMode_=s->cullMode_;

    for (int t=0;t<MESH_MAX_TEXTURES;t++) {
        if ((texture_[t]=s->texture_[t])!=NULL)
                texture_[t]->ref();
        sx_[t]=s->sx_[t];
        sy_[t]=s->sy_[t];
    }

    for (int k=0;k<MESH_MAX_ARRAYS;k++)
    {
        genericArray[k].type=s->genericArray[k].type;
        genericArray[k].mult=s->genericArray[k].mult;
        genericArray[k].count=s->genericArray[k].count;
        genericArray[k].offset=s->genericArray[k].offset;
        genericArray[k].stride=s->genericArray[k].stride;
        genericArray[k].master=s->genericArray[k].master;
        genericArray[k].bufsize=s->genericArray[k].bufsize;
        genericArray[k].cache=NULL;
        if (genericArray[k].bufsize&&(s->genericArray[k].ptr))
        {
            genericArray[k].ptr=malloc(genericArray[k].bufsize);
            memcpy(genericArray[k].ptr,s->genericArray[k].ptr,genericArray[k].bufsize);
            genericArray[k].modified=true;
        }
    }

    originalTextureCoordinates_.assign(s->originalTextureCoordinates_.cbegin(),s->originalTextureCoordinates_.cend());
    textureCoordinates_.assign(s->textureCoordinates_.cbegin(),s->textureCoordinates_.cend());
    textureCoordinates_.Update();
    vertices_.assign(s->vertices_.cbegin(),s->vertices_.cend());
    vertices_.Update();
    indices_.assign(s->indices_.cbegin(),s->indices_.cend());
    indices_.Update();
    originalColors_.assign(s->originalColors_.cbegin(),s->originalColors_.cend());
    colors_.assign(s->colors_.cbegin(),s->colors_.cend());
    colors_.Update();
}

GMesh::~GMesh()
{
	for (int t=0;t<MESH_MAX_TEXTURES;t++)
		if (texture_[t])
			texture_[t]->unref();

   for (int k=0;k<MESH_MAX_ARRAYS;k++)
        if (genericArray[k].ptr)
    	{
            if (genericArray[k].cache)
                delete genericArray[k].cache;
            free(genericArray[k].ptr);
    	}
}

bool GMesh::is3d()
{
	return mesh3d_;
}

void GMesh::setVertex(int i, float x, float y,float z)
{
	int order=mesh3d_?3:2;
    if (i * order + order - 1 >= vertices_.size())
        vertices_.resize(i * order + order);

    vertices_[i * order] = x;
    vertices_[i * order + 1] = y;
    if (mesh3d_)
        vertices_[i * order + 2] = z;

    vertices_.Update();
    boundsDirty_ = true;
	invalidate(INV_GRAPHICS|INV_BOUNDS);
}

void GMesh::setIndex(int i, unsigned int index)
{
    if (i >= indices_.size())
        indices_.resize(i + 1);

    indices_[i] = index;
    indices_.Update();

    boundsDirty_ = true;
	invalidate(INV_GRAPHICS|INV_BOUNDS);
}

void GMesh::setColor(int i, unsigned int color, float alpha)
{
    if (i >= originalColors_.size())
    {
        originalColors_.resize(i + 1);
        colors_.resize(i * 4 + 3 + 1);
    }

    originalColors_[i].color = color;
    originalColors_[i].alpha = alpha;

    alpha = std::min(std::max(alpha, 0.f), 1.f);

    unsigned int r = ((color >> 16) & 0xff) * r_ * a_ * alpha;
    unsigned int g = ((color >> 8) & 0xff) * g_ * a_ * alpha;
    unsigned int b = (color & 0xff) * b_ * a_ * alpha;
    unsigned int a = 255 * a_ * alpha;

    colors_[i * 4] = r;
    colors_[i * 4 + 1] = g;
    colors_[i * 4 + 2] = b;
    colors_[i * 4 + 3] = a;
    colors_.Update();
	invalidate(INV_GRAPHICS);
}

void GMesh::setTextureCoordinate(int i, float u, float v)
{
    if (i * 2 + 1 >= originalTextureCoordinates_.size())
    {
        originalTextureCoordinates_.resize(i * 2 + 1 + 1);
        textureCoordinates_.resize(i * 2 + 1 + 1);
    }

    originalTextureCoordinates_[i * 2] = u;
    originalTextureCoordinates_[i * 2 + 1] = v;

    textureCoordinates_[i * 2] = u * sx_[0];
    textureCoordinates_[i * 2 + 1] = v * sy_[0];
    textureCoordinates_.Update();
	invalidate(INV_GRAPHICS);
}

void GMesh::setGenericArray(int index,const void *pointer, ShaderProgram::DataType type, int mult, int count,int offset,int stride,int masterIndex)
{
    if ((index<0)||(index>=MESH_MAX_ARRAYS)) return;
    if (masterIndex>=MESH_MAX_ARRAYS) return;
    if (masterIndex<0)
    {
        int ps=4;
        switch (type)
        {
        case ShaderProgram::DBYTE:
        case ShaderProgram::DUBYTE:
            ps=1;
            break;
        case ShaderProgram::DSHORT:
        case ShaderProgram::DUSHORT:
            ps=2;
            break;
        case ShaderProgram::DINT:
        case ShaderProgram::DFLOAT:
            ps=4;
            break;
        }
        size_t esz=ps*mult*count;
        if ((!pointer)&&esz) return;
        if ((!genericArray[index].ptr)||(esz!=genericArray[index].bufsize))
        {
            if (genericArray[index].ptr)
                free(genericArray[index].ptr);
            if (esz)
                genericArray[index].ptr=malloc(esz);
            else
                genericArray[index].ptr=NULL;
        }
        genericArray[index].bufsize=esz;
        if (esz)
            memcpy(genericArray[index].ptr,pointer,esz);
        genericArray[index].modified=true;
    }
    else {
        if (genericArray[index].ptr)
            free(genericArray[index].ptr);
        genericArray[index].ptr=NULL;
        genericArray[index].bufsize=0;
    }
    genericArray[index].mult=mult;
    genericArray[index].type=type;
    genericArray[index].count=count;
    genericArray[index].offset=offset;
    genericArray[index].stride=stride;
    genericArray[index].master=masterIndex;
    invalidate(INV_GRAPHICS);
}

void GMesh::setVertexArray(const float *vertices, size_t size)
{
    vertices_.assign(vertices, vertices + size);
    vertices_.Update();

    boundsDirty_ = true;
	invalidate(INV_GRAPHICS|INV_BOUNDS);
}

void GMesh::setIndexArray(const unsigned int *indices, size_t size)
{
    indices_.assign(indices, indices + size);
    indices_.Update();

    boundsDirty_ = true;
	invalidate(INV_GRAPHICS|INV_BOUNDS);
}

void GMesh::setColorArray(const unsigned int *colors, const float *alphas, size_t size)
{
    originalColors_.resize(size);
    colors_.resize(size * 4);
    for (size_t i = 0; i < size; ++i)
        setColor(i, colors[i], alphas[i]);
    colors_.Update();
	invalidate(INV_GRAPHICS);
}

void GMesh::setTextureCoordinateArray(const float *textureCoordinates, size_t size)
{
    originalTextureCoordinates_.assign(textureCoordinates, textureCoordinates + size);

    textureCoordinates_.resize(size);
    for (size_t i = 0; i < size; i += 2)
    {
        textureCoordinates_[i] = originalTextureCoordinates_[i] * sx_[0];
        textureCoordinates_[i + 1] = originalTextureCoordinates_[i + 1] * sy_[0];
    }
    textureCoordinates_.Update();
	invalidate(INV_GRAPHICS);
}

void GMesh::resizeVertexArray(size_t size)
{
    vertices_.resize(size * (mesh3d_?3:2));
    vertices_.Update();

    boundsDirty_ = true;
	invalidate(INV_GRAPHICS|INV_BOUNDS);
}

void GMesh::resizeIndexArray(size_t size)
{
    indices_.resize(size);
    indices_.Update();

    boundsDirty_ = true;
	invalidate(INV_GRAPHICS|INV_BOUNDS);
}

void GMesh::resizeColorArray(size_t size)
{
	originalColors_.resize(size);
	colors_.resize(size * 4);
    colors_.Update();
	invalidate(INV_GRAPHICS);
}

void GMesh::resizeTextureCoordinateArray(size_t size)
{
    originalTextureCoordinates_.resize(size * 2);
    textureCoordinates_.resize(size * 2);
    textureCoordinates_.Update();
	invalidate(INV_GRAPHICS);
}

void GMesh::clearVertexArray()
{
    vertices_.clear();
    vertices_.Update();

    boundsDirty_ = true;
	invalidate(INV_GRAPHICS|INV_BOUNDS);
}

void GMesh::clearIndexArray()
{
    indices_.clear();
    indices_.Update();

    boundsDirty_ = true;
	invalidate(INV_GRAPHICS|INV_BOUNDS);
}

void GMesh::clearColorArray()
{
	originalColors_.clear();
    colors_.clear();
    colors_.Update();
	invalidate(INV_GRAPHICS);
}

size_t GMesh::getVertexArraySize() const
{
    return vertices_.size() / (mesh3d_?3:2);
}

size_t GMesh::getIndexArraySize() const
{
    return indices_.size();
}

size_t GMesh::getColorArraySize() const
{
    return originalColors_.size();
}

size_t GMesh::getTextureCoordinateArraySize() const
{
    return originalTextureCoordinates_.size() / 2;
}

void GMesh::getVertex(int i, float *x, float *y, float *z) const
{
	int order=mesh3d_?3:2;
    *x = vertices_[i * order];
    *y = vertices_[i * order + 1];
    if (mesh3d_)
    	*z = vertices_[i * order + 2];
}

void GMesh::getIndex(int i, unsigned int *index) const
{
    *index = indices_[i];
}

void GMesh::getColor(int i, unsigned int *color, float *alpha) const
{
    *color = originalColors_[i].color;
    *alpha = originalColors_[i].alpha;
}

void GMesh::getTextureCoordinate(int i, float *u, float *v) const
{
    *u = originalTextureCoordinates_[i * 2];
    *v = originalTextureCoordinates_[i * 2 + 1];
}

void GMesh::clearTextureCoordinateArray()
{
    originalTextureCoordinates_.clear();
    textureCoordinates_.clear();
	textureCoordinates_.Update();
	invalidate(INV_GRAPHICS);
}

void GMesh::setPrimitiveType(ShaderProgram::ShapeType type)
{
	meshtype_=type;
	invalidate(INV_GRAPHICS);
}

void GMesh::setInstanceCount(size_t instances)
{
	instanceCount_=instances;
	invalidate(INV_GRAPHICS);
}


void GMesh::setTexture(TextureBase *texture,int slot)
{
    if (texture)
        texture->ref();
    if (texture_[slot])
        texture_[slot]->unref();
    texture_[slot] = texture;

    float psx = sx_[slot];
    float psy = sy_[slot];

    if (texture_[slot])
    {
        sx_[slot] = texture_[slot]->uvscalex / texture_[slot]->data->exwidth;
        sy_[slot] = texture_[slot]->uvscaley / texture_[slot]->data->exheight;
    }
    else
    {
        sx_[slot] = 1;
        sy_[slot] = 1;
    }

    if ((slot==0)&& (psx != sx_[slot] || psy != sy_[slot]))
    {
        for (size_t i = 0; i < textureCoordinates_.size(); i += 2)
        {
            textureCoordinates_[i] = originalTextureCoordinates_[i] * sx_[0];
            textureCoordinates_[i + 1] = originalTextureCoordinates_[i + 1] * sy_[0];
        }
        textureCoordinates_.Update();
    }
	invalidate(INV_GRAPHICS);
}

void GMesh::clearTexture(int slot)
{
    setTexture(NULL,slot);
}

void GMesh::setCullMode(ShaderEngine::CullMode cullMode)
{
	cullMode_=cullMode;
	invalidate(INV_GRAPHICS);
}

void GMesh::doDraw(const CurrentTransform &, float sx, float sy, float ex, float ey)
{
    G_UNUSED(sx); G_UNUSED(sy); G_UNUSED(ex); G_UNUSED(ey);
    if (mesh3d_)
	{
		 ShaderEngine::DepthStencil stencil=ShaderEngine::Engine->pushDepthStencil();
		 stencil.dTest=true;
		 stencil.cullMode=cullMode_;
		 ShaderEngine::Engine->setDepthStencil(stencil);
	}
	if (vertices_.size() == 0) return;

    ShaderEngine::StandardProgram stdp=colors_.empty()?ShaderEngine::STDP_BASIC:ShaderEngine::STDP_COLOR;
	if (texture_[0] && !textureCoordinates_.empty())
		stdp=colors_.empty()?ShaderEngine::STDP_TEXTURE:ShaderEngine::STDP_TEXTURECOLOR;
	ShaderProgram *p=getShader(stdp,mesh3d_?ShaderEngine::STDPV_3D:0);
	if (texture_[0] && !textureCoordinates_.empty())
        p->bindTexture(0,texture_[0]->data->id());
	for (int t=1;t<MESH_MAX_TEXTURES;t++)
		if (texture_[t])
			p->bindTexture(t,texture_[t]->data->id());
    
    if (!genericArray[ShaderProgram::DataVertex].count)
    {
        p->setData(ShaderProgram::DataVertex,ShaderProgram::DFLOAT,mesh3d_?3:2, &vertices_[0],vertices_.size()/(mesh3d_?3:2),vertices_.modified,&vertices_.bufferCache);
        vertices_.modified=false;
    }

    if ((!colors_.empty())&&(!genericArray[ShaderProgram::DataColor].count))
    {
        float r, g, b, a;
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
        }

        p->setData(ShaderProgram::DataColor,ShaderProgram::DUBYTE,4,&colors_[0],colors_.size()/4,colors_.modified,&colors_.bufferCache);
        colors_.modified=false;
    }

    if (texture_[0] && !textureCoordinates_.empty())
    {
    	int sc=p->getSystemConstant(ShaderProgram::SysConst_TextureInfo);
    	if (sc>=0)
    	{
        	float textureInfo[4]={0,0,0,0};
       		textureInfo[0]=(float)texture_[0]->data->width / (float)texture_[0]->data->exwidth;
        	textureInfo[1]=(float)texture_[0]->data->height / (float)texture_[0]->data->exheight;
        	textureInfo[2]=1.0/texture_[0]->data->exwidth;
        	textureInfo[3]=1.0/texture_[0]->data->exheight;
    		p->setConstant(sc,ShaderProgram::CFLOAT4,1,textureInfo);
    	}
        if (!genericArray[ShaderProgram::DataTexture].count)
        {
            p->setData(ShaderProgram::DataTexture,ShaderProgram::DFLOAT,2, &textureCoordinates_[0],textureCoordinates_.size()/2,textureCoordinates_.modified,&textureCoordinates_.bufferCache);
            textureCoordinates_.modified=false;
        }
    }

    for (int k=0;k<MESH_MAX_ARRAYS;k++) {
        int mi=genericArray[k].master;
        if (mi<0) mi=k;
        if (genericArray[mi].ptr)
    	{
            p->setData(k,genericArray[k].type,genericArray[k].mult, genericArray[mi].ptr,genericArray[k].count,genericArray[mi].modified,&genericArray[mi].cache,genericArray[k].stride,genericArray[k].offset);
            genericArray[mi].modified=false;
    	}
    }

    p->drawElements(meshtype_, indices_.size(), ShaderProgram::DINT, &indices_[0],indices_.modified,&indices_.bufferCache,0,0,instanceCount_);
    indices_.modified=false;
}

void GMesh::childrenDrawn()
{
    if (mesh3d_)
   	 ShaderEngine::Engine->popDepthStencil();
}

void GMesh::extraBounds(float *minx, float *miny, float *maxx, float *maxy) const
{
    if (boundsDirty_)
    {
        minx_ = miny_ = 1e30;
        maxx_ = maxy_ = -1e30;
        int order=mesh3d_?3:2;

        for (size_t i = 0; i < indices_.size(); i += 3)
        {
            for (int j = 0; j < 3; ++j)
            {
                int index = indices_[i + j];
                float x = vertices_[index * order];
                float y = vertices_[index * order + 1];

                minx_ = std::min(minx_, x);
                miny_ = std::min(miny_, y);
                maxx_ = std::max(maxx_, x);
                maxy_ = std::max(maxy_, y);
            }
        }
    }

    if (minx)
        *minx = minx_;
    if (miny)
        *miny = miny_;
    if (maxx)
        *maxx = maxx_;
    if (maxy)
        *maxy = maxy_;
}

