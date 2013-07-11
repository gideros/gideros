#include "timelinewidget.h"
#include <QPainter>
#include <QMenu>
#include <QMouseEvent>
#include <QDebug>
#include <QUndoCommand>
#include <QUndoStack>
#include "controller.h"
#include <QApplication>


LayerWidget::LayerWidget(QWidget* parent) : QWidget(parent)
{
	framewidth_ = 10;
	frameheight_= 20;
	framey_ = 24;

	topcolor_ = QColor(0xFFFFFF);
	maincolor_ = QColor(0xECE9D8);
	bottomcolor_ = QColor(0xACA899);
	selectedcolor_ = QColor(0x316AC5);
	backgroundcolor_ = QColor(0xF6F4EC);
	textcolor_ = QColor(QRgb(0x000000));
	selectedtextcolor_ = QColor(QRgb(0xFFFFFF));

	dragedlayerpos_ = -1;
}

void LayerWidget::paintEvent(QPaintEvent* event)
{
	QPainter painter(this);

	painter.setPen(QPen(Qt::NoPen));
	painter.setBrush(backgroundcolor_);
	painter.drawRect(0, 0, width(), height());

	QFont font("tahoma", 7);
	painter.setFont(font);

	const std::vector<Layer*>& layers = document_->layers();
	
	for (std::size_t i = 0; i < layers.size(); ++i)
	{
		Layer* layer = layers[i];

		bool selected = document_->isSelected(layer);

		QTransform transform;
		transform.translate(0, framey_ + frameheight_ * i);
		painter.setTransform(transform);

		painter.setPen(QPen(Qt::NoPen));
		if (selected)
			painter.setBrush(selectedcolor_);
		else
			painter.setBrush(maincolor_);
		painter.drawRect(0, 0, width(), frameheight_);

		painter.setPen(topcolor_);
		painter.drawLine(0, 0, width(), 0);
		painter.setPen(bottomcolor_);
		painter.drawLine(0, frameheight_-1, width(), frameheight_-1);

		if (selected)
			painter.setPen(selectedtextcolor_);
		else
			painter.setPen(textcolor_);
		painter.drawText(20, 14, layer->name.c_str());
	}

	if (newlayerpos_ >= 0)
	{
		QTransform transform;
		transform.translate(0, framey_ + frameheight_ * newlayerpos_);
		painter.setTransform(transform);

		painter.setPen(QPen(Qt::NoPen));
		painter.setBrush(Qt::Dense4Pattern);
		painter.drawRect(0, -2, width(), 4);
	}
}

void LayerWidget::mousePressEvent(QMouseEvent* event)
{
	int x = event->x();
	int y = event->y();

	int layer = (y - framey_) / frameheight_;

	if (y < framey_ || layer < 0 || layer >= document_->layers().size())
		return;
	
	//bool s = QApplication::keyboardModifiers() & Qt::ControlModifier;
	bool s = false;
	controller_->changeLayerSelection(document_->layers()[layer], s);

	dragedlayerpos_ = layer;
	newlayerpos_ = -1;
	dragstarty_ = y;

	update();
}
void LayerWidget::mouseMoveEvent(QMouseEvent* event)
{
	int x = event->x();
	int y = event->y();

	int yy = (y - (framey_ - frameheight_/2));
	int layer = (yy < 0) ? -(-yy/frameheight_) : (yy/frameheight_);
	layer = std::min(std::max(layer, 0), (int)document_->layers().size());

	if (dragedlayerpos_ >= 0)
	{
		if (newlayerpos_ < 0)
		{
			int dy = abs(y - dragstarty_);
			if (dy >= 5)
				newlayerpos_ = layer;
		}
		else
			newlayerpos_ = layer;

		update();
	}
}

void LayerWidget::mouseReleaseEvent(QMouseEvent* event)
{
	if (dragedlayerpos_ >= 0 && newlayerpos_ >= 0)
	{

	}

	dragedlayerpos_ = -1;
	newlayerpos_ = -1;
	update();
}


void LayerWidget::setController(Controller* controller)
{
	controller_ = controller;
	document_ = controller_->document();

//	setFixedSize(100 * framewidth_, frameheight_ * document_->getLayerCount() + framey_ + 1);
}



TimelineWidget::TimelineWidget(QWidget *parent) :
	QWidget(parent)
{
	frameborder_ = QColor(0xECE9D8);
	framefill_ = QColor(0xFFFFFF);
	framefill5_ = QColor(0xF6F4EC);
	frameregionborder_ = QColor(QRgb(0x000000));
	frameregionfill_ = QColor(0xFFFFFF);
	frameregioninside_ = QColor(QRgb(0x000000));
	tickcolor_ = QColor(0xACA899);
	textcolor_ = QColor(QRgb(0x000000));
	currentframeborder_ = QColor(0xCC0000);
	currentframefill_ = QColor(0xFF9999);
	backgroundcolor_ = QColor(0xF6F4EC);

	framewidth_ = 10;
	frameheight_= 20;
	framey_ = 24;

	removeTweenAction_ = new QAction(tr("Remove Tween"), this);
	connect(removeTweenAction_, SIGNAL(triggered()), this, SLOT(removeTween()));

	createTweenAction_ = new QAction(tr("Create Tween"), this);
	connect(createTweenAction_, SIGNAL(triggered()), this, SLOT(createTween()));

	createTweenWithEndAction_ = new QAction(tr("Create Tween with End Frame"), this);
	connect(createTweenWithEndAction_, SIGNAL(triggered()), this, SLOT(createTweenWithEnd()));

	createKeyframeAction_ = new QAction(tr("Create Keyframe"), this);
	connect(createKeyframeAction_, SIGNAL(triggered()), this, SLOT(createKeyframe()));

	deleteKeyframeAction_ = new QAction(tr("Delete Keyframe"), this);
	connect(deleteKeyframeAction_, SIGNAL(triggered()), this, SLOT(deleteKeyframe()));

	insertFrameAction_ = new QAction(tr("Insert Frame"), this);
	connect(insertFrameAction_, SIGNAL(triggered()), this, SLOT(insertFrame()));

	deleteFrameAction_ = new QAction(tr("Delete Frame"), this);
	connect(deleteFrameAction_, SIGNAL(triggered()), this, SLOT(deleteFrame()));

	setContextMenuPolicy(Qt::CustomContextMenu);
	connect(this, SIGNAL(customContextMenuRequested  (const QPoint&)),
			this, SLOT  (onCustomContextMenuRequested(const QPoint&)));

	selectionbegin_ = 0;
	selectionend_ = 0;
	selectionlayer_ = 0;

	actiontype_ = eActionNone;
}

void TimelineWidget::paintEvent(QPaintEvent* event)
{
//	validate();

	QFont font("tahoma", 7);
	QPainter painter(this);
	painter.setFont(font);

	for (std::size_t i = 1; i < 100; ++i)
	{
		int x = (i - 1) * framewidth_;

		painter.setPen(tickcolor_);
		painter.drawLine(x, 0, x, 3);
		painter.drawLine(x, framey_, x, framey_-3);
	}

	{
		int frame = document_->getCurrentFrame();
		int x = (frame - 1) * framewidth_;
		int y = 0;

		painter.setPen(currentframeborder_);
		painter.setBrush(currentframefill_);

		painter.drawRect(x, y, framewidth_, framey_-1);
	}

	painter.setPen(textcolor_);
	for (std::size_t i = 1; i < 100; ++i)
	{
		int x = (i - 1) * framewidth_;

		if (i == 1 || (i % 5) == 0)
			painter.drawText(x, 14, QString::number(i));
	}

	for (int layer = 0; layer < document_->layers().size(); ++layer)
	{
		const std::vector<Keyframe*>& frames_ = document_->keyframes(layer);

		QTransform transform;
		transform.translate(0, framey_ + frameheight_ * layer);

		painter.setTransform(transform);
		
		for (std::size_t i = 1; i <= 100; ++i)
		{
			painter.setPen(frameborder_);

			if ((i - 1) % 5 == 0)
				painter.setBrush(framefill5_);
			else
				painter.setBrush(framefill_);

			int x = (i - 1) * framewidth_;
			int y = 0;

			painter.drawRect(x, y, framewidth_, frameheight_);
		}

		for (std::size_t i = 0; i < frames_.size(); ++i)
		{
			int s = frames_[i]->start;
			int e = frames_[i]->end;

			int x = (s - 1) * framewidth_;
			int y = 0;
			int width = (e - s + 1) * framewidth_;
			int height = frameheight_;

			painter.setPen(frameregionborder_);
			painter.setBrush(frameregionfill_);

			painter.drawRect(x, y, width, height);

			painter.setPen(frameregioninside_);
			painter.setBrush(frameregioninside_);

			int sx = x + 5;
			int sy = y + 10;

			int ex = x + width - 8;
			int ey = y + 10;

			if (s == e)
			{
				painter.drawRect(sx-2, sy-3, 4, 6);
			}
			else
			{
				switch (frames_[i]->tweentype)
				{
				case eTweenNone:
					painter.drawRect(sx-2, sy-3, 4, 6);
				break;
				case eTweenMotion:
				{
					painter.drawRect(sx-2, sy-3, 4, 6);
					painter.drawLine(sx, sy, ex, ey);
					QPolygon polygon(3);
					polygon.setPoint(0, ex, ey - 3);
					polygon.setPoint(1, ex + 6, ey);
					polygon.setPoint(2, ex, ey + 3);
					painter.drawConvexPolygon(polygon);
				}
				break;
				case eTweenMotionWithEnd:
					painter.drawRect(sx-2, sy-3, 4, 6);
					painter.drawLine(sx, sy, ex, ey);
					QPolygon polygon(3);
					polygon.setPoint(0, ex - 3, ey - 3);
					polygon.setPoint(1, ex + 3, ey);
					polygon.setPoint(2, ex - 3, ey + 3);
					painter.drawConvexPolygon(polygon);
					painter.drawRect(ex + 2, ey-3, 4, 6);
				break;
				}
			}
		}

		for (std::size_t i = 0; i < frames_.size(); ++i)
		{
			if (frames_[i]->links.size() == 0)
			{
				// no link
			}
			else if (frames_[i]->links.size() == frames_[i]->sprites.size())
			{
				// full link
			}
			else
			{
				// partial link
			}
		}
	}

	if (isSelected())
	{
		QTransform transform;
		transform.translate(0, framey_ + frameheight_ * selectionlayer_);
		painter.setTransform(transform);

		for (int i = selectionbegin_; i <= selectionend_; ++i)
		{
			painter.setPen(QPen(Qt::NoPen));
			painter.setBrush(QBrush(QColor(0x316AC5)));

			int x = (i - 1) * framewidth_;
			int y = 0;

			painter.drawRect(x+1, y+2, framewidth_-1, frameheight_-3);
		}
	}
}


void TimelineWidget::onCustomContextMenuRequested(const QPoint& pos)
{
	int x = pos.x();
	int y = pos.y();

	int layer = (y - framey_) / frameheight_;

	if (y < framey_ || layer < 0 || layer >= document_->layers().size())
		return;

	QMenu menu(this);

	int framenum = (x / framewidth_) + 1;

	selectionbegin_ = framenum;
	selectionend_ = framenum;
	selectionlayer_ = layer;
	repaint();

	Keyframe* frame = document_->getKeyframe(layer, framenum);

	if (frame != NULL)
	{
		menu.addAction(removeTweenAction_);
		menu.addAction(createTweenAction_);
//		menu.addAction(createTweenWithEndAction_);
		menu.addSeparator();
	}


	menu.addAction(createKeyframeAction_);
	menu.addAction(deleteKeyframeAction_);

	menu.addSeparator();

	menu.addAction(insertFrameAction_);
	menu.addAction(deleteFrameAction_);

	menu.exec(QCursor::pos());
}

void TimelineWidget::removeTween()
{
	if (isSelected() == false)
		return;

	controller_->changeTweenType(selectionlayer_, selectionbegin_, eTweenNone);
}

void TimelineWidget::createTween()
{
	if (isSelected() == false)
		return;

	controller_->changeTweenType(selectionlayer_, selectionbegin_, eTweenMotion);
}
void TimelineWidget::createTweenWithEnd()
{
	if (isSelected() == false)
		return;

	controller_->changeTweenType(selectionlayer_, selectionbegin_, eTweenMotionWithEnd);
}

void TimelineWidget::mousePressEvent(QMouseEvent* event)
{
	int x = event->x();
	int y = event->y();

	int framenum = (x / framewidth_) + 1;

	controller_->setCurrentFrame(framenum);

	if (y < framey_)
	{
	}
	else
	{
		int layer = (y - framey_) / frameheight_;

		if (layer >=0 || layer < document_->layers().size())
		{
			selectionbegin_ = framenum;
			selectionend_ = framenum;
			selectionlayer_ = layer;

			actiontype_ = eActionSelect;
			selectstart_ = framenum;
		}
	}
	update();
}
void TimelineWidget::mouseMoveEvent(QMouseEvent* event)
{
	int x = event->x();
	int y = event->y() - framey_;

	if (actiontype_ == eActionSelect)
	{
		int framenum = (x / framewidth_) + 1;

		if (framenum >= 1)
		{
			controller_->setCurrentFrame(framenum);

			selectionbegin_ = selectstart_;
			selectionend_ = framenum;

			if (selectionbegin_ > selectionend_)
				std::swap(selectionbegin_, selectionend_);
			update();
		}
	}
}
void TimelineWidget::mouseReleaseEvent(QMouseEvent* event)
{
	if (actiontype_ == eActionSelect)
	{
		actiontype_ = eActionNone;
	}
}

bool TimelineWidget::isSelected() const
{
	return selectionbegin_ >= 1 && selectionend_ >= 1;
}

void TimelineWidget::createKeyframe()
{
	if (isSelected() == false)
		return;

	controller_->createKeyframe(selectionlayer_, selectionbegin_);
}

void TimelineWidget::deleteKeyframe()
{
	if (isSelected() == false)
		return;

	if (document_->keyframes(selectionlayer_).size() <= 1)
		return;

	if (document_->getKeyframe(selectionlayer_, selectionbegin_) == NULL)
		return;

	controller_->deleteKeyframe(selectionlayer_, selectionbegin_);
}

void TimelineWidget::insertFrame()
{
	return;

/*	if (isSelected() == false)
		return;

	Frame* frame = getFrame(selectionbegin_);

	if (frame == NULL)
		return;

	int delta = selectionend_ - selectionbegin_ + 1;

	int index = getFrameIndex(selectionbegin_);

	for (int i = index; i < frames_.size(); ++i)
	{
		frames_[i]->end += delta;

		if (i != index)
			frames_[i]->start += delta;
	}

	update(); */
}

void TimelineWidget::deleteFrame()
{

}

void TimelineWidget::setController(Controller* controller)
{
	controller_ = controller;
	document_ = controller_->document();

	setFixedSize(100 * framewidth_, frameheight_ * document_->layers().size() + framey_ + 1);
}
