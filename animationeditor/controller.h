#ifndef AE_CONTROLLER_H
#define AE_CONTROLLER_H

#include <QObject>

#include "document.h"

class TimelineWidget;
class CanvasWidget;
class QUndoStack;
class Matrix;
class LayerWidget;

class Controller : public QObject
{
//    Q_OBJECT

public:
	explicit Controller(QUndoStack* undoStack, 
						TimelineWidget* timeline, 
						CanvasWidget* canvas, 
						LayerWidget* layer,
						Document* document, QObject* parent = 0);

	void changeTweenType(Keyframe* keyframe, TweenType tweentype);
	void changeTweenType(int layer, int frame, TweenType tweentype);
	void createKeyframe(int layer, int frame);
	void deleteKeyframe(int layer, int frame);

	void setCurrentFrame(int frame);
	int getCurrentFrame() const;

	void transformSprite(SpriteInstance* instance, const Transform& start, const Transform& end);

	void addRandomSprite(int x, int y);		// TODO: to be removed

	void changeSelection(SpriteInstance* instance, bool multiple);
	void selectNone();

	void load();

	Document* document() const
	{
		return document_;
	}

	void deleteSelected();
	void copySelected();
	void pasteClipboard();

	void moveLayers(const std::set<int>& layers, int newpos);
	void changeLayerSelection(Layer* layer, bool multiple);

signals:

public slots:
	void update();

private:
	QUndoStack* undoStack_;
	TimelineWidget* timeline_;
	CanvasWidget* canvas_;
	LayerWidget* layer_;
	Document* document_;
};

#endif // CONTROLLER_H
