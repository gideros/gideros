#ifndef CANVASWIDGET_H
#define CANVASWIDGET_H

#include <QGLWidget>

#include <transform.h>

class QUndoStack;
class Application;
class Controller;
class Stage;
class Document;
class SpriteInstance;

class CanvasWidget : public QGLWidget
{
    Q_OBJECT
public:
	CanvasWidget(QWidget *parent = 0);
	~CanvasWidget();

	void setController(Controller* controller);

signals:

public slots:

private slots:
	void addRandomSprite();


protected:
	virtual void initializeGL();
	virtual void paintGL();
	virtual void resizeGL(int width, int height);

	virtual void mouseMoveEvent(QMouseEvent* event);
	virtual void mousePressEvent(QMouseEvent* event);
	virtual void mouseReleaseEvent(QMouseEvent* event);

	virtual void contextMenuEvent(QContextMenuEvent* event);

private:
	Controller* controller_;
	Document* document_;
	Application* application_;
	Stage* stage_;

private:
	//bool dragging_;
	int startx_, starty_;
	float pivotx_, pivoty_;
	float refx_, refy_;
	float sx_, sy_;
	float rotation_;
	float tx_, ty_;
	Transform starttransform_;

private:
	QAction* addRandomSpriteAction_;

	enum Mode
	{
		eModeNone,
		eModeScale,
		eModeScaleX,
		eModeScaleY,
		eModeRotate,
		eModeTranslate,
	};
	Mode mode_;
};

#endif // CANVASWIDGET_H
