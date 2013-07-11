#include <matrix.h>
#include <sprite.h>

#include "controller.h"
#include "document.h"
#include "canvaswidget.h"
#include "timelinewidget.h"
#include <QUndoCommand>
#include <QUndoStack>


static void ref(const std::vector<SpriteInstance*>& instances)
{
	for (std::size_t i = 0; i < instances.size(); ++i)
		instances[i]->ref();
}

static void unref(const std::vector<SpriteInstance*>& instances)
{
	for (std::size_t i = 0; i < instances.size(); ++i)
		instances[i]->unref();
}

static void ref(const std::set<SpriteInstance*>& instances)
{
	std::set<SpriteInstance*>::const_iterator iter, e = instances.end();
	for (iter = instances.begin(); iter != instances.end(); ++iter)
		(*iter)->ref();
}

static void unref(const std::set<SpriteInstance*>& instances)
{
	std::set<SpriteInstance*>::const_iterator iter, e = instances.end();
	for (iter = instances.begin(); iter != instances.end(); ++iter)
		(*iter)->unref();
}

class ChangeTweenTypeCommand : public QUndoCommand
{
public:
	ChangeTweenTypeCommand(Keyframe* keyframe, TweenType tweentype, Controller* controller) :
		keyframe_(keyframe),
		tweentype_(tweentype),
		controller_(controller)
	{
		setText("Change Tween Type");

		keyframe_->ref();
		oldtweentype_ = keyframe_->tweentype;
	}

	~ChangeTweenTypeCommand()
	{
		keyframe_->unref();
	}

	virtual void redo()
	{
		keyframe_->tweentype = tweentype_;
		controller_->update();
	}

	virtual void undo()
	{
		keyframe_->tweentype = oldtweentype_;
		controller_->update();
	}

private:
	Keyframe* keyframe_;
	TweenType tweentype_;
	TweenType oldtweentype_;
	Controller* controller_;
};


class CreateKeyframeCommand : public QUndoCommand
{
public:
	CreateKeyframeCommand(int layer, int selection, Controller* controller) :
		controller_(controller),
		document_(controller->document()),
		layer_(layer)
	{
		setText("Create Keyframe");

		keyframe_ = document_->getKeyframe(layer_, selection);

		if (keyframe_ == NULL)
		{
			// create new frame
			int s = document_->keyframes(layer_).back()->end + 1;
			int e = selection;

			newkeyframe_ = new Keyframe(s, e, eTweenNone);
		}
		else
		{
			// split frame
			newkeyframe_ = new Keyframe(keyframe_->start, selection - 1);
			insertindex_ = document_->getKeyframeIndex(layer_, selection);

			switch (keyframe_->tweentype)
			{
				case eTweenNone:
					newkeyframe_->tweentype = eTweenNone;
					break;
				case eTweenMotion:
					newkeyframe_->tweentype = eTweenMotion;
					break;
				case eTweenMotionWithEnd:
					newkeyframe_->tweentype = eTweenMotion;
					break;
			}
		}
	}

	~CreateKeyframeCommand()
	{
		newkeyframe_->unref();
	}

	virtual void undo()
	{
		if (keyframe_ == NULL)
		{
			document_->eraseKeyframe(layer_, document_->keyframes(layer_).size() - 1);
		}
		else
		{
			keyframe_->start = newkeyframe_->start;
			document_->eraseKeyframe(layer_, insertindex_);
		}

		controller_->update();
	}

	virtual void redo()
	{
		if (keyframe_ == NULL)
		{
			document_->insertKeyframe(layer_, document_->keyframes(layer_).size(), newkeyframe_);
		}
		else
		{
			keyframe_->start = newkeyframe_->end + 1;
			document_->insertKeyframe(layer_, insertindex_, newkeyframe_);
		}

		controller_->update();
	}

private:
	Controller* controller_;
	Document* document_;
	Keyframe* keyframe_;
	Keyframe* newkeyframe_;
	int insertindex_;
	int layer_;
};

class DeleteKeyframeCommand : public QUndoCommand
{
public:
	DeleteKeyframeCommand(int layer, int selection, Controller* controller) :
		controller_(controller),
		document_(controller->document()),
		layer_(layer)
	{
		setText("Delete Keyframe");

		keyframe_ = document_->getKeyframe(layer_, selection);
		index_ = document_->getKeyframeIndex(layer_, selection);

		keyframe_->ref();
	}

	~DeleteKeyframeCommand()
	{
		keyframe_->unref();
	}

	void redo()
	{
		if (index_ == 0)
			document_->keyframes(layer_)[index_ + 1]->start = keyframe_->start;
		else
			document_->keyframes(layer_)[index_ - 1]->end = keyframe_->end;

		document_->eraseKeyframe(layer_, index_);

		controller_->update();
	}

	void undo()
	{
		document_->insertKeyframe(layer_, index_, keyframe_);

		if (index_ == 0)
			document_->keyframes(layer_)[index_ + 1]->start = keyframe_->end + 1;
		else
			document_->keyframes(layer_)[index_ - 1]->end = keyframe_->start - 1;

		controller_->update();
	}

private:
	Controller* controller_;
	Document* document_;

	int index_;

	Keyframe* keyframe_;

	int layer_;
};

class TransformSpriteCommand : public QUndoCommand
{
public:
	TransformSpriteCommand(SpriteInstance* instance, const Transform& starttransform, const Transform& endtransform, Controller* controller) :
		controller_(controller),
		document_(controller->document()),
		instance_(instance),
		starttransform_(starttransform),
		endtransform_(endtransform)
	{
		setText("Transform Sprite");
//		printf("TransformSprite() %x\n", instance_);
		instance_->ref();
	}

	~TransformSpriteCommand()
	{
//		printf("~TransformSprite() %x\n", instance_);
		instance_->unref();
	}

	virtual void redo()
	{
//		printf("TransformSprite::redo() %x\n", instance_);
		instance_->transform = endtransform_;
		instance_->sprite->setTransform(instance_->transform);
		controller_->update();
	}
	virtual void undo()
	{
//		printf("TransformSprite::undo() %x\n", instance_);
		instance_->transform = starttransform_;
		instance_->sprite->setTransform(instance_->transform);
		controller_->update();
	}

private:
	Controller* controller_;
	Document* document_;

	SpriteInstance* instance_;
	Transform starttransform_;
	Transform endtransform_;
};

class AddSpriteCommand : public QUndoCommand
{
public:
	AddSpriteCommand(int layer, SpriteInstance* instance, int frame, Controller* controller) :
		controller_(controller),
		document_(controller->document()),
		instance_(instance),
		frame_(frame),
		layer_(layer)
	{
		setText("Add Sprite");
//		printf("AddSprite() %x\n", instance_);
		instance_->ref();
	}

	~AddSpriteCommand()
	{
//		printf("~AddSprite() %x\n", instance_);
		instance_->unref();
	}

	virtual void redo()
	{
//		printf("AddSprite::redo() %x\n", instance_);
		document_->addSpriteInstance(layer_, frame_, instance_);
		controller_->update();
	}
	virtual void undo()
	{
//		printf("AddSprite::undo() %x\n", instance_);
		document_->removeSpriteInstance(layer_, frame_, instance_);
		controller_->update();
	}

private:
	Controller* controller_;
	Document* document_;

	SpriteInstance* instance_;
	int frame_;

	int layer_;
};

class SelectNoneCommand : public QUndoCommand
{
public:
	SelectNoneCommand(Controller* controller) :
		controller_(controller),
		document_(controller->document())
	{
		setText("Select None");
		oldselection_ = document_->getSelection();
		frame_ = document_->getCurrentFrame();

		ref(oldselection_);
	}

	~SelectNoneCommand()
	{
		unref(oldselection_);
	}

	virtual void redo()
	{
		document_->setCurrentFrame(frame_);
		document_->clearSelection();
		controller_->update();
	}
	virtual void undo()
	{
		document_->setCurrentFrame(frame_);
		document_->setSelection(oldselection_);
		controller_->update();
	}

private:
	Controller* controller_;
	Document* document_;
	std::set<SpriteInstance*> oldselection_;
	int frame_;
};

class ChangeSelectionCommand : public QUndoCommand
{
public:
	ChangeSelectionCommand(SpriteInstance* instance, bool multiple, Controller* controller) :
		controller_(controller),
		document_(controller->document()),
		instance_(instance),
		multiple_(multiple)
	{
		setText("Change Selection");
		oldselection_ = document_->getSelection();
		frame_ = document_->getCurrentFrame();

		ref(oldselection_);
	}

	~ChangeSelectionCommand()
	{
		unref(oldselection_);
	}

	virtual void redo()
	{
		document_->setCurrentFrame(frame_);
		document_->changeSelection(instance_, multiple_);
		controller_->update();
	}
	virtual void undo()
	{
		document_->setCurrentFrame(frame_);
		document_->setSelection(oldselection_);
		controller_->update();
	}

private:
	Controller* controller_;
	Document* document_;
	std::set<SpriteInstance*> oldselection_;
	SpriteInstance* instance_;
	bool multiple_;
	int frame_;
};

class GotoFrameCommand : public QUndoCommand
{
public:
	GotoFrameCommand(int frame, Controller* controller) :
		controller_(controller),
		document_(controller->document()),
		newframe_(frame)
	{
		setText("Go to Frame");
		oldframe_ = controller_->getCurrentFrame();
	}
	virtual void redo()
	{
		document_->setCurrentFrame(newframe_);
		controller_->update();
	}
	virtual void undo()
	{
		document_->setCurrentFrame(oldframe_);
		controller_->update();
	}

private:
	Controller* controller_;
	Document* document_;

	int newframe_;
	int oldframe_;
};


class DeleteSelectedCommand : public QUndoCommand
{
public:
	DeleteSelectedCommand(Controller* controller) : 
		controller_(controller),
		document_(controller->document())
	{
		setText("Delete");
		//frame_ = document_->getCurrentFrame();
		//oldinstances_ = document_->getCurrentKeyframe()->sprites;
		//oldselection_ = document_->getSelection();

		//ref(oldinstances_);
		//ref(oldselection_);
	}

	~DeleteSelectedCommand()
	{
		//unref(oldinstances_);
		//unref(oldselection_);
	}

	virtual void redo()
	{
		//document_->setCurrentFrame(frame_);
		//document_->deleteSelected(frame_, oldselection_);
		//controller_->update();
	}

	virtual void undo()
	{
		//document_->setCurrentFrame(frame_);
		//document_->setSpriteInstances(frame_, oldinstances_);
		//document_->setSelection(oldselection_);
		//controller_->update();
	}

private:
	Controller* controller_;
	Document* document_;
	std::set<SpriteInstance*> oldselection_;
	std::vector<SpriteInstance*> oldinstances_;
	int frame_;
};

class PasteClipboardCommand : public QUndoCommand
{
public:
	PasteClipboardCommand(int layer, Controller* controller) : 
		controller_(controller),
		document_(controller->document()),
		layer_(layer)
	{
		setText("Paste");
		frame_ = document_->getCurrentFrame();
		oldinstances_ = document_->getCurrentKeyframe(layer_)->sprites;
		oldselection_ = document_->getSelection();
		clipboard_ = document_->getClipboard();

		newselection_ = document_->pasteClipboard(layer_, frame_, clipboard_);
		
		newinstances_ = document_->getCurrentKeyframe(layer_)->sprites;

		ref(oldinstances_);
		ref(oldselection_);
		ref(clipboard_);
		ref(newinstances_);
		ref(newselection_);
	}

	~PasteClipboardCommand()
	{
		unref(oldinstances_);
		unref(oldselection_);
		unref(clipboard_);
		unref(newinstances_);
		unref(newselection_);
	}

	virtual void redo()
	{
		document_->setCurrentFrame(frame_);
		document_->setSpriteInstances(layer_, frame_, newinstances_);
		document_->setSelection(newselection_);
		controller_->update();
	}

	virtual void undo()
	{
		document_->setCurrentFrame(frame_);
		document_->setSpriteInstances(layer_, frame_, oldinstances_);
		document_->setSelection(oldselection_);
		controller_->update();
	}

private:
	Controller* controller_;
	Document* document_;
	int frame_;
	std::set<SpriteInstance*> oldselection_;
	std::vector<SpriteInstance*> oldinstances_;
	std::vector<SpriteInstance*> newinstances_;
	std::vector<SpriteInstance*> clipboard_;
	std::set<SpriteInstance*> newselection_;
	int layer_;
};

Controller::Controller(	QUndoStack* undoStack, 
						TimelineWidget* timeline, 
						CanvasWidget* canvas, 
						LayerWidget* layer,
						Document* document, QObject* parent) :
	QObject(parent),
	undoStack_(undoStack),
	timeline_(timeline),
	canvas_(canvas),
	document_(document),
	layer_(layer)
{
}

void Controller::changeTweenType(Keyframe* keyframe, TweenType tweentype)
{
	if (keyframe->tweentype != tweentype)
		undoStack_->push(new ChangeTweenTypeCommand(keyframe, tweentype, this));
}

void Controller::update()
{
	document_->validate();
	timeline_->update();
	canvas_->update();
	layer_->update();
}

void Controller::changeTweenType(int layer, int frame, TweenType tweentype)
{
	Keyframe* keyframe = document_->getKeyframe(layer, frame);

	if (keyframe != NULL)
		changeTweenType(keyframe, tweentype);
}

void Controller::createKeyframe(int layer, int frame)
{
	Keyframe* keyframe = document_->getKeyframe(layer, frame);

	if (keyframe && keyframe->start == keyframe->end)
		return;

	undoStack_->push(new CreateKeyframeCommand(layer, frame, this));
}

void Controller::deleteKeyframe(int layer, int frame)
{
	Keyframe* keyframe = document_->getKeyframe(layer, frame);

	if (keyframe == NULL)
		return;

	undoStack_->push(new DeleteKeyframeCommand(layer, frame, this));
}

void Controller::setCurrentFrame(int frame)
{
	if (frame == document_->getCurrentFrame())
		return;

	if (document_->getSelection().empty())
	{
		document_->setCurrentFrame(frame);
		update();
	}
	else
	{
		undoStack_->beginMacro("Clear Frame Selection");
		undoStack_->push(new SelectNoneCommand(this));
		undoStack_->push(new GotoFrameCommand(frame, this));
		undoStack_->endMacro();
	}
}

int Controller::getCurrentFrame() const
{
	return document_->getCurrentFrame();
}

void Controller::load()
{
	document_->load();
	update();
}

void Controller::transformSprite(SpriteInstance* instance, const Transform& start, const Transform& end)
{
	undoStack_->push(new TransformSpriteCommand(instance, start, end, this));
}

void Controller::addRandomSprite(int x, int y)
{
	if (document_->getCurrentFrame() == NULL)
		return;

	int frame = document_->getCurrentFrame();

	TextureBase* texture = document_->getRandomTexture();

	SpriteInstance* instance = new SpriteInstance(texture);
	instance->sprite->setXY(x, y);
	instance->transform = instance->sprite->transform();
	
	undoStack_->push(new AddSpriteCommand(0, instance, frame, this));

	instance->unref();
}

void Controller::changeSelection(SpriteInstance* instance, bool multiple)
{
	if (multiple == false && document_->getSelection().find(instance) != document_->getSelection().end())
		return;

	undoStack_->push(new ChangeSelectionCommand(instance, multiple, this));
}

void Controller::selectNone()
{
	if (document_->getSelection().empty())
		return;

	undoStack_->push(new SelectNoneCommand(this));
}


void Controller::deleteSelected()
{
	if (document_->getSelection().empty())
		return;

	undoStack_->push(new DeleteSelectedCommand(this));
}

void Controller::copySelected()
{
	if (document_->getSelection().empty())
		return;

	document_->copySelected(document_->getSelection());
}

void Controller::pasteClipboard()
{
	int layer = document_->getCurrentLayerIndex();

	if (document_->getCurrentKeyframe(layer) == NULL)
		return;

	if (document_->getClipboard().empty())
		return;

	undoStack_->push(new PasteClipboardCommand(layer, this));
}


void Controller::moveLayers(const std::set<int>& layers, int newpos)
{
	document_->moveLayers(layers, newpos);
	update();
}

void Controller::changeLayerSelection(Layer* layer, bool multiple)
{
	document_->changeLayerSelection(layer, multiple);
	update();
}
