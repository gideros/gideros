#include "shape.h"
#include <stdlib.h>

//#include "clipper.hpp"
//using namespace clipper;

//#include "triangulate.h"
//using namespace triangulate;

#include "internal_glu.h"

#ifndef M_PI
#define M_PI 3.141592654
#endif

#ifndef __EMSCRIPTEN__
#ifndef __APPLE__
//emscripten and Xcode have it already
static bool isnan(float x)
{
    return x != x;
}
#endif
#endif

class Tesselator
{
public:
	void tesselate(const std::vector<std::vector<Point2f> >& pgs, bool evenodd = true)
	{
		GLUtesselator* tess = internal_gluNewTess();
		internal_gluTessCallback(tess, GLU_TESS_BEGIN_DATA, (void (*)())&BeginCallback_s);
		internal_gluTessCallback(tess, GLU_TESS_VERTEX_DATA, (void (*)())&VertexCallback_s);
		internal_gluTessCallback(tess, GLU_TESS_END_DATA, (void (*)())&EndCallback_s);
		internal_gluTessCallback(tess, GLU_TESS_COMBINE_DATA, (void (*)())&CombineCallback_s);
		internal_gluTessCallback(tess, GLU_TESS_ERROR_DATA, (void (*)())&ErrorCallback_s);
		internal_gluTessCallback(tess, GLU_TESS_EDGE_FLAG_DATA, (void (*)())&EdgeCallback_s);

		if (evenodd)
			internal_gluTessProperty(tess, GLU_TESS_WINDING_RULE, GLU_TESS_WINDING_ODD); else
			internal_gluTessProperty(tess, GLU_TESS_WINDING_RULE, GLU_TESS_WINDING_NONZERO);

		internal_gluTessProperty(tess, GLU_TESS_BOUNDARY_ONLY, 0); //GL_FALSE
		internal_gluTessBeginPolygon(tess, this);
		for (std::size_t i = 0; i < pgs.size(); ++i)
		{
			internal_gluTessBeginContour(tess);
			for (std::size_t j = 0; j < pgs[i].size(); ++j)
			{
				GLdouble *vert = new GLdouble[3];
				vert[0] = (GLdouble)pgs[i][j].x;
				vert[1] = (GLdouble)pgs[i][j].y;
				vert[2] = 0;
				AddToCleanup(vert);
				internal_gluTessVertex(tess, vert, vert);
			}
			internal_gluTessEndContour(tess);
		}
		internal_gluTessEndPolygon(tess);
		DoCleanup();
		internal_gluDeleteTess(tess);
	}

	std::vector<float> triangles;

private:
	static void BeginCallback_s(GLenum type, void* polygon_data)   
	{   
		Tesselator* that = static_cast<Tesselator*>(polygon_data);
		that->BeginCallback(type);
	} 

	static void EndCallback_s(void* polygon_data)   
	{   
		Tesselator* that = static_cast<Tesselator*>(polygon_data);
		that->EndCallback();
	}

	static void VertexCallback_s(GLvoid *vertex, void* polygon_data)   
	{   
		Tesselator* that = static_cast<Tesselator*>(polygon_data);
		that->VertexCallback(vertex);
	} 

	static void CombineCallback_s(GLdouble coords[3], GLdouble *data[4], GLfloat weight[4], GLdouble **dataOut, void* polygon_data)   
	{   
		Tesselator* that = static_cast<Tesselator*>(polygon_data);
		that->CombineCallback(coords, data, weight, dataOut);
	}   

	static void EdgeCallback_s(GLboolean flag, void* polygon_data)
	{
		// empty
	}

	static void ErrorCallback_s(GLenum errorCode, void* polygon_data)
	{   
		// empty
	}   

	void BeginCallback(GLenum type)   
	{   
	} 

	void EndCallback()   
	{   
	}

	void VertexCallback(GLvoid *vertex)   
	{   
		double* v = (double *)vertex;
		triangles.push_back(v[0]);
		triangles.push_back(v[1]);
//		triangles.push_back(v[2]);
	} 

	void CombineCallback(GLdouble coords[3], GLdouble *data[4], GLfloat weight[4], GLdouble **dataOut)
	{   
		GLdouble *vert = new GLdouble[3];
		vert[0] = coords[0];   
		vert[1] = coords[1];   
		vert[2] = coords[2];  
		*dataOut = vert;
		AddToCleanup(vert);
	}

	std::vector<GLdouble *> cleanupStore;
	void AddToCleanup(GLdouble *ptr)   
	{   
		cleanupStore.push_back(ptr);
	}   

	void DoCleanup()   
	{   
		for (std::size_t i = 0; i < cleanupStore.size(); ++i)
			delete [] cleanupStore[i];
		cleanupStore.clear();
	}   
};

static void createGraphicsBase(const std::vector<std::vector<Point2f> >& polygons, GraphicsBase& result, bool evenodd, bool texcoords = false, float stx = 0, float sty = 0, const Matrix4* matrix = NULL)
{
	Tesselator tes;
	tes.tesselate(polygons, evenodd);

	int start = result.vertices.size();
	int ntriangles = tes.triangles.size() / 2;

	for (std::size_t i = 0; i < ntriangles; ++i)
	{
		float x = tes.triangles[i * 2 + 0];
		float y = tes.triangles[i * 2 + 1];

		result.vertices.push_back(Point2f(x, y));
		if (texcoords)
		{
			float tx, ty;
			matrix->transformPoint(x, y, &tx, &ty);
			result.texcoords.push_back(Point2f(tx * stx, ty * sty));
		}

		result.indices.push_back(start + i);
	}
	result.vertices.Update();
	result.indices.Update();
	if (texcoords)
		result.texcoords.Update();
}

static void lineToPolygon(float x0, float y0, float x1, float y1, float thickness, std::vector<Point2f>& polygon)
{
	x0 += 0.5f;
	y0 += 0.5f;
	x1 += 0.5f;
	y1 += 0.5f;

	const int dangle = 45;

	int npoints = 0;
	for (int i = 90; i <= 270; i += dangle)
		npoints += 2;

	polygon.resize(npoints);

	float ux = x1 - x0;
	float uy = y1 - y0;

	float l = std::sqrt(ux * ux + uy * uy);

	if (l == 0)
	{
		ux = 1;
		uy = 0;
	}
	else
	{
		ux /= l;
		uy /= l;
	}

	float vx = -uy;
	float vy =  ux;

	int j = 0;

	for (int i = 90; i <= 270; i += dangle)
	{
		float angle = i * M_PI / 180;

		float x = cos(angle) * thickness * 0.5f;
		float y = sin(angle) * thickness * 0.5f;

		polygon[j].x = x * ux + y * vx + x0;
		polygon[j++].y = x * uy + y * vy + y0;
	}

	for (int i = 270; i <= 90 + 360; i += dangle)
	{
		float angle = i * M_PI / 180;

		float x = cos(angle) * thickness * 0.5f + l;
		float y = sin(angle) * thickness * 0.5f;

		polygon[j].x = x * ux + y * vx + x0;
		polygon[j++].y = x * uy + y * vy + y0;
	}
}

static void createSolidLineStrip(float r, float g, float b, float a,
								   float thickness,
								   const std::vector<Point2f>& points,
									GraphicsBase& result)
{
	if (points.size() < 2)
		return;

	std::vector<std::vector<Point2f> > polygon(points.size() - 1);

	for (std::size_t i = 1; i < points.size(); ++i)
	{
		const Point2f& p0 = points[i - 1];
		const Point2f& p1 = points[i];

		lineToPolygon(p0.x, p0.y, p1.x, p1.y, thickness, polygon[i - 1]);
	}

	createGraphicsBase(polygon, result, false);
	result.setColor(r, g, b, a);
} 


static void createSolidPolygon(	float r, float g, float b, float a,
								const std::vector<std::vector<Point2f> > & contours,
								bool evenodd,
								GraphicsBase& result)
{
	createGraphicsBase(contours, result, evenodd);
	result.setColor(r, g, b, a);
}
static void createTexturePolygon(	TextureBase* texture, const Matrix4& matrix,
									const std::vector<std::vector<Point2f> > & contours,
									bool evenodd,
									GraphicsBase& result)
{
    createGraphicsBase(contours, result, evenodd, true, texture->uvscalex / texture->data->exwidth, texture->uvscaley / texture->data->exheight, &matrix);
	result.data = texture->data;
}

Shape::Shape(Application *application) : Sprite(application)
{
	clear();
}

void Shape::cloneFrom(Shape *s) {
    Sprite::cloneFrom(s);

    matrix_=s->matrix_;
    fillr_=s->fillr_, fillg_=s->fillg_, fillb_=s->fillb_, filla_=s->filla_;
    fillType_=s->fillType_;
    liner_=s->liner_, lineg_=s->lineg_, lineb_=s->lineb_, linea_=s->linea_;
    thickness_=s->thickness_;
    min_=s->min_, max_=s->max_;
    Point2f min_, max_;
    windingRule_=s->windingRule_;
    paths_=s->paths_;
    textures_=s->textures_;
    for (std::size_t i = 0; i < textures_.size(); ++i)
        textures_[i]->ref();
    texture_=s->texture_;
    graphicsBases_=s->graphicsBases_;
    for (GraphicsBaseList::iterator iter = graphicsBases_.begin(), e = graphicsBases_.end(); iter != e; ++iter)
        iter->cloned();
}

Shape::~Shape()
{
	clear();
}

void Shape::clearFillStyle()
{
	fillType_ = eNone;
}

void Shape::setSolidFillStyle(unsigned int color, float alpha)
{
	fillType_ = eSolid;

	int r = (color >> 16) & 0xff;
	int g = (color >> 8) & 0xff;
	int b = color & 0xff;

	fillr_ = r / 255.f;
	fillg_ = g / 255.f;
	fillb_ = b / 255.f;
	filla_ = alpha;
}

void Shape::setTextureFillStyle(TextureBase* texture, const Matrix4* matrix)
{
	fillType_ = eTexture;

	textures_.push_back(texture);
	texture->ref();

	texture_ = texture;

	if (matrix)
		matrix_ = matrix->inverse();
	invalidate(INV_GRAPHICS);
}

void Shape::clear()
{
	graphicsBases_.clear();

	for (std::size_t i = 0; i < textures_.size(); ++i)
		textures_[i]->unref();
	textures_.clear();

	fillType_ = eNone;
	fillr_ = 0;
	fillg_ = 0;
	fillb_ = 0;
	filla_ = 0;
	texture_ = NULL;

	liner_ = 0;
	lineg_ = 0;
	lineb_ = 0;
	linea_ = 0;
	thickness_ = 0;

	paths_.clear();

	matrix_.set();

	min_ = Point2f( 1e30f,  1e30f);
	max_ = Point2f(-1e30f, -1e30f);
	invalidate(INV_GRAPHICS|INV_BOUNDS);
}

void Shape::setLineStyle(float thickness, unsigned int color, float alpha)
{
	int r = (color >> 16) & 0xff;
	int g = (color >> 8) & 0xff;
	int b = color & 0xff;

	liner_ = r / 255.f;
	lineg_ = g / 255.f;
	lineb_ = b / 255.f;
	linea_ = alpha;

	thickness_ = thickness;
	if (thickness_ < 0.f)
		thickness_ = 0.f;
}

void Shape::beginPath(WindingRule windingRule)
{
	windingRule_ = windingRule;
	paths_.clear();
}

void Shape::moveTo(float x, float y)
{
    if (isnan(x) || isnan(y))
        return;

	paths_.push_back(std::vector<Point2f>());

	paths_.back().push_back(Point2f(x, y));
}

void Shape::lineTo(float x, float y)
{
    if (isnan(x) || isnan(y))
        return;

	ensureSubpath(x, y);

	paths_.back().push_back(Point2f(x, y));
}

void Shape::closePath()
{
	if (paths_.empty())
		return;

	if (paths_.back().empty())
		return;

	std::vector<Point2f>& subpath = paths_.back();

	subpath.push_back(subpath.front());	// close subpath

	moveTo(subpath.back().x, subpath.back().y);
}

void Shape::ensureSubpath(float x, float y)
{
	if (paths_.empty() || paths_.back().empty())
		moveTo(x, y);
}

void Shape::endPath()
{
	for (int i = (int)paths_.size() - 1; i >= 0; --i)
			if (paths_[i].size() <= 1)
				paths_.erase(paths_.begin() + i);

	for (std::size_t i = 0; i < paths_.size(); ++i)
			for (std::size_t j = 0; j < paths_[i].size(); ++j)
				extendBounds(paths_[i][j].x, paths_[i][j].y, thickness_);

	if (!paths_.empty())
	{
		// draw fill
		switch (fillType_)
		{
		case eNone:
			break;
		case eSolid:
			graphicsBases_.push_back(GraphicsBase());
			createSolidPolygon(fillr_, fillg_, fillb_, filla_, paths_, windingRule_ == eEvenOdd, graphicsBases_.back());
			if (graphicsBases_.back().indices.empty() || graphicsBases_.back().vertices.empty())
				graphicsBases_.pop_back();;
			break;
		case eTexture:
			graphicsBases_.push_back(GraphicsBase());
			createTexturePolygon(texture_, matrix_, paths_, windingRule_ == eEvenOdd, graphicsBases_.back());
			if (graphicsBases_.back().indices.empty() || graphicsBases_.back().vertices.empty())
				graphicsBases_.pop_back();;
			break;
		}

		// draw stroke
		if (thickness_ > 0)
		{
			for (std::size_t i = 0; i < paths_.size(); ++i)
			{
				graphicsBases_.push_back(GraphicsBase());
				createSolidLineStrip(liner_, lineg_, lineb_, linea_, thickness_, paths_[i], graphicsBases_.back());
				if (graphicsBases_.back().indices.empty() || graphicsBases_.back().vertices.empty())
					graphicsBases_.pop_back();;
			}
		}
		invalidate(INV_GRAPHICS|INV_BOUNDS);
	}

	paths_.clear();
}

void Shape::extendBounds(float x, float y, float thickness)
{
	if (thickness < 0)
		thickness = 0.f;

	min_.x = std::min(x - thickness * 0.5f, min_.x);
	min_.y = std::min(y - thickness * 0.5f, min_.y);
	max_.x = std::max(x + thickness * 0.5f, max_.x);
	max_.y = std::max(y + thickness * 0.5f, max_.y);
}

void Shape::extraBounds(float* minx, float* miny, float* maxx, float* maxy) const
{
	if (minx)
		*minx = min_.x;
	if (miny)
		*miny = min_.y;
	if (maxx)
		*maxx = max_.x;
	if (maxy)
		*maxy = max_.y;
}


void Shape::doDraw(const CurrentTransform&, float sx, float sy, float ex, float ey)
{
	for (GraphicsBaseList::iterator iter = graphicsBases_.begin(), e = graphicsBases_.end(); iter != e; ++iter)
		iter->draw(getShader(iter->getShaderType()));
}
