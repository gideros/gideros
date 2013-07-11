#include <application.h>
#include "canvaswidget.h"
#include "document.h"
#include "controller.h"
#include <QMouseEvent>
#include <QMenu>
#include <algorithm>
#include <QApplication>

static Matrix R(float angle)
{
	float r = angle * M_PI / 180;
	return Matrix(std::cos(-r), -std::sin(-r), std::sin(-r), std::cos(r));
}

static Matrix T(float tx, float ty)
{
	return Matrix(1, 0, 0, 1, tx, ty);
}

static Matrix S(float sx, float sy)
{
	return Matrix(sx, 0, 0, sy);
}

static Matrix generateScaleTransform(float x1, float y1, float x2, float y2, float x3, float y3, float x4, float y4, float rotation, float* osx = NULL, float* osy = NULL)
{
	// x1,y1 -> x3,y3
	// x2,y2 -> x4,y4

/*	printf("%g %g -> %g %g\n", x1, y1, x3, y3);
	printf("%g %g -> %g %g\n", x2, y2, x4, y4); */
	{
		Matrix m = R(-rotation);
		m.transformPoint(x3, y3, &x3, &y3);
		m.transformPoint(x4, y4, &x4, &y4);
	}

	float sx = (x4 - x3) / (x2 - x1);
	float sy = (y4 - y3) / (y2 - y1);

	if (osx)
		sx = *osx;
	if (osy)
		sy = *osy;

	return R(rotation)* T(x3, y3) * S(sx, sy) * T(-x1, -y1);

/*	float x, y;
	m.transformPoint(x1, y1, &x, &y);
	printf("%g %g\n", x, y);
	m.transformPoint(x2, y2, &x, &y);
	printf("%g %g\n", x, y); */
}

static Matrix generateRotationTransform(float x1, float y1, float x2, float y2, 
										float sx, float sy, float rotation)
{
	Matrix m = R(rotation) * S(sx, sy);

	m.transformPoint(x1, y1, &x1, &y1);

	return T(x2 - x1, y2 - y1) * m;
}



CanvasWidget::CanvasWidget(QWidget *parent) :
	QGLWidget(parent)
{
	application_ = new Application;
	mode_ = eModeNone;

	addRandomSpriteAction_ = new QAction(tr("Add Random Sprite"), this);
	connect(addRandomSpriteAction_, SIGNAL(triggered()), this, SLOT(addRandomSprite()));
}

CanvasWidget::~CanvasWidget()
{
	application_->releaseView();
	delete application_;
}

void CanvasWidget::paintGL()
{
	glClearColor(1, 1, 1, 1);
	glClear(GL_COLOR_BUFFER_BIT);

	Stage* stage = application_->stage();

	while (stage->childCount())
		stage->removeChildAt(0);

	for (int layer = 0; layer < document_->layers().size(); ++layer)
	{
		Keyframe* keyframe = document_->getKeyframe(layer, document_->getCurrentFrame());
		
		if (keyframe)
		{
			for (std::size_t i = 0; i < keyframe->sprites.size(); ++i)
				stage->addChild(keyframe->sprites[i]->sprite);

			if (mode_ == eModeNone)
			{
				// interpolate 
				for (std::size_t i = 0; i < keyframe->sprites.size(); ++i)
				{
					SpriteInstance* instance = keyframe->sprites[i];

					if (keyframe->links.find(instance) != keyframe->links.end())
					{
						SpriteInstance* next = keyframe->links[instance];

						float sx = instance->transform.x();
						float ex = next->transform.x();

						float sy = instance->transform.y();
						float ey = next->transform.y();

						float sr = instance->transform.rotation();
						float er = next->transform.rotation();

						float ssx = instance->transform.scaleX();
						float esx = next->transform.scaleX();

						float ssy = instance->transform.scaleY();
						float esy = next->transform.scaleY();


						int s = keyframe->start;
						int e = keyframe->end;
						int c = document_->getCurrentFrame();

						float t = (float)(c - s) / (float)(e - s + 1);

						float ix = sx * (1 - t) + ex * t;
						float iy = sy * (1 - t) + ey * t;
						float ir = sr * (1 - t) + er * t;
						float isx = ssx * (1 - t) + esx * t;
						float isy = ssy * (1 - t) + esy * t;

						instance->sprite->setXY(ix, iy);
						instance->sprite->setRotation(ir);
						instance->sprite->setScaleXY(isx, isy);
					}
				}
			}
		}
	}

	application_->renderScene();

	std::set<SpriteInstance*> selection = document_->getSelection();

	std::set<SpriteInstance*>::iterator iter, e = selection.end();
	for (iter = selection.begin(); iter != e; ++iter)
	{
		SpriteInstance* instance = *iter;
		Sprite* sprite = instance->sprite;

		float minx, miny, maxx, maxy;
		sprite->objectBounds(&minx, &miny, &maxx, &maxy);

		float x[4], y[4];
		sprite->localToGlobal(minx, miny, &x[0], &y[0]);
		sprite->localToGlobal(maxx, miny, &x[1], &y[1]);
		sprite->localToGlobal(maxx, maxy, &x[2], &y[2]);
		sprite->localToGlobal(minx, maxy, &x[3], &y[3]);

		float px, py;
		sprite->localToGlobal(instance->pivotx, instance->pivoty, &px, &py);

		glColor4f(0, 0, 0, 1);
		glDisable(GL_TEXTURE_2D);
		glBegin(GL_LINE_LOOP);
		glVertex2f(x[0], y[0]);
		glVertex2f(x[1], y[1]);
		glVertex2f(x[2], y[2]);
		glVertex2f(x[3], y[3]);
		glEnd();

		glPointSize(7);
		glBegin(GL_POINTS);
		glVertex2f(x[2], y[2]);
		glVertex2f((x[1] + x[2])/2, (y[1] + y[2])/2);
		glVertex2f((x[2] + x[3])/2, (y[2] + y[3])/2);
		glVertex2f(px - (x[2] - x[1]), py - (y[2] - y[1]));
		glEnd();

		glPointSize(5);
		glBegin(GL_POINTS);
		glVertex2f(px, py);
		glEnd();

		glPointSize(1);

		glBegin(GL_LINES);
		glVertex2f(px, py);
		glVertex2f(px - (x[2] - x[1]), py - (y[2] - y[1]));
		glEnd();

		glColor4f(1, 1, 1, 1);
	}
}

void CanvasWidget::initializeGL()
{
	application_->initView();

	stage_ = application_->stage();
	//stage_->setXY(50, 150);
	//stage_->setScaleXY(1.5, 1.5);
	//stage_->setRotation(30);
}

void CanvasWidget::resizeGL(int width, int height)
{
	application_->setResolution(width, height);
}

void CanvasWidget::setController(Controller* controller)
{
	controller_ = controller;
	document_ = controller_->document();
}

void CanvasWidget::mousePressEvent(QMouseEvent* event)
{
	int layer = 0;
	Keyframe* keyframe = document_->getKeyframe(layer, document_->getCurrentFrame());

	if (keyframe == NULL)
		return;

	// first look for handles
	if (document_->getSelection().size() == 1)
	{
		SpriteInstance* instance = *document_->getSelection().begin();

		Sprite* sprite = instance->sprite;

		float minx, miny, maxx, maxy;
		sprite->objectBounds(&minx, &miny, &maxx, &maxy);

		float x[4], y[4];
		sprite->localToGlobal(minx, miny, &x[0], &y[0]);
		sprite->localToGlobal(maxx, miny, &x[1], &y[1]);
		sprite->localToGlobal(maxx, maxy, &x[2], &y[2]);
		sprite->localToGlobal(minx, maxy, &x[3], &y[3]);

		float px, py;
		sprite->localToGlobal(instance->pivotx, instance->pivoty, &px, &py);

		stage_->matrix().inverseTransformPoint(px, py, &pivotx_, &pivoty_);

		starttransform_ = sprite->transform();
		startx_ = event->x();
		starty_ = event->y();
		sx_ = sprite->scaleX();
		sy_ = sprite->scaleY();
		rotation_ = sprite->rotation();
		tx_ = sprite->x();
		ty_ = sprite->y();

		// scale
		if (mode_ == eModeNone)
		{
			float dx = event->x() - x[2];
			float dy = event->y() - y[2];

			if (sqrt(dx * dx + dy * dy) <= 7)
			{
				mode_ = eModeScale;
				refx_ = maxx;
				refy_ = maxy;
			}
		}
		// scale x
		if (mode_ == eModeNone)
		{
			float dx = event->x() - (x[1] + x[2])/2;
			float dy = event->y() - (y[1] + y[2])/2;

			if (sqrt(dx * dx + dy * dy) <= 7)
			{
				mode_ = eModeScaleX;
				refx_ = maxx;
				refy_ = (miny + maxy) / 2;
			}
		}
		// scale y
		if (mode_ == eModeNone)
		{
			float dx = event->x() - (x[2] + x[3])/2;
			float dy = event->y() - (y[2] + y[3])/2;

			if (sqrt(dx * dx + dy * dy) <= 7)
			{
				mode_ = eModeScaleY;
				refx_ = (minx + maxx) / 2;
				refy_ = maxy;
			}
		}
		// rotation
		if (mode_ == eModeNone)
		{
			float dx = event->x() - (px - (x[2] - x[1]));
			float dy = event->y() - (py - (y[2] - y[1]));

			if (sqrt(dx * dx + dy * dy) <= 7)
			{
				mode_ = eModeRotate;
			}
		}
		// translation
		if (mode_ == eModeNone)
		{
			if (sprite->hitTestPoint(event->x(), event->y()))
			{
				mode_ = eModeTranslate;
				stage_->matrix().inverseTransformPoint(event->x(), event->y(), &refx_, &refy_);
			}
		}
	}
	else if (document_->getSelection().size() > 1)
	{
		// multiple selection, allow translate


	}


	if (mode_ == eModeNone)
	{
		SpriteInstance* instance = NULL;

		for (int i = (int)keyframe->sprites.size() - 1; i >= 0; --i)
		{
			Sprite* sprite = keyframe->sprites[i]->sprite;
				
			if (sprite->hitTestPoint(event->x(), event->y()))
			{
				instance = keyframe->sprites[i];
				break;
			}
		}

		if (instance)
		{
			bool s = QApplication::keyboardModifiers() & Qt::ShiftModifier;

			if (s)
			{
				controller_->changeSelection(instance, true);
			}
			else
			{
				if (document_->getSelection().find(instance) == document_->getSelection().end())
					controller_->changeSelection(instance, false);
			}
		}
		else
		{
			if (document_->getSelection().empty() == false)
				controller_->selectNone();
		}
	}

	update();
}
void CanvasWidget::mouseMoveEvent(QMouseEvent* event)
{
	switch (mode_)
	{
		case eModeScale:
		{
			bool s = QApplication::keyboardModifiers() & Qt::ShiftModifier;

			SpriteInstance* instance = *document_->getSelection().begin();
			float ex, ey;
			stage_->matrix().inverseTransformPoint(event->x(), event->y(), &ex, &ey);
			Matrix m = generateScaleTransform(instance->pivotx, instance->pivoty, refx_, refy_, pivotx_, pivoty_, ex, ey, rotation_);
			instance->sprite->setMatrix(m);
			update();
			break;
		}

		case eModeScaleX:
		{
			SpriteInstance* instance = *document_->getSelection().begin();
			float ex, ey;
			stage_->matrix().inverseTransformPoint(event->x(), event->y(), &ex, &ey);
			Matrix m = generateScaleTransform(instance->pivotx, instance->pivoty, refx_, refy_, pivotx_, pivoty_, ex, ey, rotation_, NULL, &sy_);
			instance->sprite->setMatrix(m);
			update();
			break;
		}
		case eModeScaleY:
		{
			SpriteInstance* instance = *document_->getSelection().begin();
			float ex, ey;
			stage_->matrix().inverseTransformPoint(event->x(), event->y(), &ex, &ey);
			Matrix m = generateScaleTransform(instance->pivotx, instance->pivoty, refx_, refy_, pivotx_, pivoty_, ex, ey, rotation_, &sx_, NULL);
			instance->sprite->setMatrix(m);
			update();
			break;
		}
		case eModeRotate:
		{
			SpriteInstance* instance = *document_->getSelection().begin();
		
			float ex, ey;
			stage_->matrix().inverseTransformPoint(event->x(), event->y(), &ex, &ey);

			float dx = ex - pivotx_;
			float dy = ey - pivoty_;
			float angle = atan2(dy, dx) * 180 / M_PI + 90;

			Matrix m = generateRotationTransform(instance->pivotx, instance->pivoty, pivotx_, pivoty_, sx_, sy_, angle); 
			instance->sprite->setMatrix(m);
			update();
			break;
		}
		case eModeTranslate:
		{
			SpriteInstance* instance = *document_->getSelection().begin();
			float ex, ey;
			stage_->matrix().inverseTransformPoint(event->x(), event->y(), &ex, &ey);
			instance->sprite->setXY(ex - refx_ + tx_, ey - refy_ + ty_);
			update();
			break;
		}

	}


//	if (dragging_)
	{
/*		Sprite* sprite = selection_->sprite;
		int dx = event->x() - startx_;
		int dy = event->y() - starty_;
		sprite->setX(sprite->x() + dx);
		sprite->setY(sprite->y() + dy);
		startx_ = event->x();
		starty_ = event->y();
		update(); */
	}
}
void CanvasWidget::mouseReleaseEvent(QMouseEvent* event)
{
	if (mode_ != eModeNone)
	{
		mode_ = eModeNone;
		SpriteInstance* instance = *document_->getSelection().begin();
//		if (starttransform_.matrix() != instance->sprite->matrix())
			controller_->transformSprite(instance, starttransform_, instance->sprite->transform());
	}
}

void CanvasWidget::contextMenuEvent(QContextMenuEvent* event)
{
	QMenu menu(this);

	menu.addAction(addRandomSpriteAction_);

	menu.exec(QCursor::pos());
}

void CanvasWidget::addRandomSprite()
{
	controller_->addRandomSprite(rand() % 100, rand() % 100);
}
