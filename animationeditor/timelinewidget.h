#ifndef TIMELINEWIDGET_H
#define TIMELINEWIDGET_H

#include <QWidget>
#include <QColor>

#include "document.h"

class QUndoStack;
class Controller;
class Document;

class LayerWidget : public QWidget
{
	Q_OBJECT

public:
	LayerWidget(QWidget* parent = 0);

public:
	void setController(Controller* controller);

protected:
	virtual void paintEvent(QPaintEvent* event);
	virtual void mousePressEvent(QMouseEvent* event);
	virtual void mouseMoveEvent(QMouseEvent* event);
	virtual void mouseReleaseEvent(QMouseEvent* event);

private:
	int framewidth_;
	int frameheight_;
	int framey_;

private:
	QColor topcolor_;
	QColor maincolor_;
	QColor bottomcolor_;
	QColor selectedcolor_;
	QColor backgroundcolor_;
	QColor textcolor_;
	QColor selectedtextcolor_;

private:
	int dragedlayerpos_, newlayerpos_;
	int dragstarty_;

private:
	Controller* controller_;
	Document* document_;
};


class TimelineWidget : public QWidget
{
    Q_OBJECT
public:
	TimelineWidget(QWidget *parent = 0);

	void setController(Controller* controller);

signals:

private slots:
	void onCustomContextMenuRequested(const QPoint& pos);
	void removeTween();
	void createTween();
	void createTweenWithEnd();
	void createKeyframe();
	void deleteKeyframe();
	void insertFrame();
	void deleteFrame();

protected:
	virtual void paintEvent(QPaintEvent* event);
	virtual void mousePressEvent(QMouseEvent* event);
	virtual void mouseMoveEvent(QMouseEvent* event);
	virtual void mouseReleaseEvent(QMouseEvent* event);

private:
/*	struct Frame
	{
		Frame() {}
		Frame(int start, int end, TweenType tweentype = eTweenNone) : start(start), end(end), tweentype(tweentype) {}

		int start, end;
		TweenType tweentype;
	};

	std::vector<Frame*> frames_;

	Frame* getFrame(int frame);
	int getFrameIndex(int frame); */

	QColor frameborder_;
	QColor framefill_;
	QColor framefill5_;
	QColor frameregionborder_;
	QColor frameregionfill_;
	QColor frameregioninside_;
	QColor tickcolor_;
	QColor textcolor_;
	QColor currentframeborder_;
	QColor currentframefill_;
	QColor backgroundcolor_;

	int framewidth_;
	int frameheight_;
	int framey_;

private:
	QAction* removeTweenAction_;
	QAction* createTweenAction_;
	QAction* createTweenWithEndAction_;
	QAction* createKeyframeAction_;
	QAction* deleteKeyframeAction_;
	QAction* insertFrameAction_;
	QAction* deleteFrameAction_;

	int selectionbegin_;
	int selectionend_;
	int selectionlayer_;

private:
	enum ActionType
	{
		eActionNone,
		eActionSelect,
		eActionDrag,
	};
	ActionType actiontype_;
	int selectstart_;

	bool isSelected() const;

private:
	Controller* controller_;
	Document* document_;
};

#endif // TIMELINEWIDGET_H
