#include <gmesh.h>
#include <ogl.h>
#include <color.h>

GMesh::GMesh(Application *application,bool is3d) : Sprite(application)
{
    texture_ = NULL;
    sx_ = 1;
    sy_ = 1;
    r_ = 1;
    g_ = 1;
    b_ = 1;
    a_ = 1;
    boundsDirty_ = false;
    minx_ = miny_ = 1e30;
    maxx_ = maxy_ = -1e30;
    meshtype_=GL_TRIANGLES;
    mesh3d_=is3d;
}

GMesh::~GMesh()
{
    if (texture_)
        texture_->unref();
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

    boundsDirty_ = true;
}

void GMesh::setIndex(int i, unsigned short index)
{
    if (i >= indices_.size())
        indices_.resize(i + 1);

    indices_[i] = index;

    boundsDirty_ = true;
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

    textureCoordinates_[i * 2] = u * sx_;
    textureCoordinates_[i * 2 + 1] = v * sy_;
}

void GMesh::setVertexArray(const float *vertices, size_t size)
{
    vertices_.assign(vertices, vertices + size);

    boundsDirty_ = true;
}

void GMesh::setIndexArray(const unsigned short *indices, size_t size)
{
    indices_.assign(indices, indices + size);

    boundsDirty_ = true;
}

void GMesh::setColorArray(const unsigned int *colors, const float *alphas, size_t size)
{
    originalColors_.resize(size);
    colors_.resize(size * 4);
    for (size_t i = 0; i < size; ++i)
        setColor(i, colors[i], alphas[i]);
}

void GMesh::setTextureCoordinateArray(const float *textureCoordinates, size_t size)
{
    originalTextureCoordinates_.assign(textureCoordinates, textureCoordinates + size);

    textureCoordinates_.resize(size);
    for (size_t i = 0; i < size; i += 2)
    {
        textureCoordinates_[i] = originalTextureCoordinates_[i] * sx_;
        textureCoordinates_[i + 1] = originalTextureCoordinates_[i + 1] * sy_;
    }
}

void GMesh::resizeVertexArray(size_t size)
{
    vertices_.resize(size * (mesh3d_?3:2));

    boundsDirty_ = true;
}

void GMesh::resizeIndexArray(size_t size)
{
    indices_.resize(size);

    boundsDirty_ = true;
}

void GMesh::resizeColorArray(size_t size)
{
    colors_.resize(size);
}

void GMesh::resizeTextureCoordinateArray(size_t size)
{
    originalTextureCoordinates_.resize(size * 2);
    textureCoordinates_.resize(size * 2);
}

void GMesh::clearVertexArray()
{
    vertices_.clear();

    boundsDirty_ = true;
}

void GMesh::clearIndexArray()
{
    indices_.clear();

    boundsDirty_ = true;
}

void GMesh::clearColorArray()
{
    colors_.clear();
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

void GMesh::getIndex(int i, unsigned short *index) const
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
}

void GMesh::setTexture(TextureBase *texture)
{
    if (texture)
        texture->ref();
    if (texture_)
        texture_->unref();
    texture_ = texture;

    float psx = sx_;
    float psy = sy_;

    if (texture_)
    {
        sx_ = texture_->uvscalex / texture_->data->exwidth;
        sy_ = texture_->uvscaley / texture_->data->exheight;
    }
    else
    {
        sx_ = 1;
        sy_ = 1;
    }

    if (psx != sx_ || psy != sy_)
    {
        for (size_t i = 0; i < textureCoordinates_.size(); i += 2)
        {
            textureCoordinates_[i] = originalTextureCoordinates_[i] * sx_;
            textureCoordinates_[i + 1] = originalTextureCoordinates_[i + 1] * sy_;
        }
    }
}

void GMesh::clearTexture()
{
    setTexture(NULL);
}

void GMesh::doDraw(const CurrentTransform &, float sx, float sy, float ex, float ey)
{
    if (texture_ && !textureCoordinates_.empty())
    {
        oglEnable(GL_TEXTURE_2D);
        oglBindTexture(GL_TEXTURE_2D, texture_->data->id());
    }
    else
        oglDisable(GL_TEXTURE_2D);

    glVertexPointer(mesh3d_?3:2, GL_FLOAT, 0, &vertices_[0]);
    oglEnableClientState(GL_VERTEX_ARRAY);

    if (!colors_.empty())
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
        }

        glColorPointer(4, GL_UNSIGNED_BYTE, 0, &colors_[0]);
        oglEnableClientState(GL_COLOR_ARRAY);
    }

    if (texture_ && !textureCoordinates_.empty())
    {
        glTexCoordPointer(2, GL_FLOAT, 0, &textureCoordinates_[0]);
        oglEnableClientState(GL_TEXTURE_COORD_ARRAY);
    }

    oglDrawElements(meshtype_, indices_.size(), GL_UNSIGNED_SHORT, &indices_[0]);

    oglDisableClientState(GL_VERTEX_ARRAY);

    if (!colors_.empty())
    {
        oglDisableClientState(GL_COLOR_ARRAY);
        glMultColor(1, 1, 1, 1);
    }

    if (texture_ && !textureCoordinates_.empty())
        oglDisableClientState(GL_TEXTURE_COORD_ARRAY);
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

