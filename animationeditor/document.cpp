#include <platform.h>
#include <sprite.h>
#include <texturebase.h>
#include <bitmapdata.h>
#include <bitmap.h>
#include <texture.h>
#include <algorithm>

#include "document.h"
#include <QUndoCommand>

SpriteInstance::SpriteInstance(SpriteInstance* instance, bool clonetr) : texture(texture)
{
	if (clonetr)
		textureregion = instance->textureregion->clone();
	else
		textureregion = instance->textureregion;

	bitmap = new Bitmap(textureregion);
	sprite = new Sprite;
	sprite->addChild(bitmap);

	sprite->setMatrix(instance->sprite->matrix());

	transform = sprite->transform();

	pivotx = instance->pivotx;
	pivoty = instance->pivoty;

	bitmap->unref();

	if (clonetr)
		textureregion->unref();
}

SpriteInstance::SpriteInstance(TextureBase* texture) : texture(texture)
{
	textureregion = new TextureRegion(texture);
	bitmap = new Bitmap(textureregion);
	sprite = new Sprite;
	sprite->addChild(bitmap);

	transform = sprite->transform();

	pivotx = bitmap->width() / 2;
	pivoty = bitmap->height() / 2;

	textureregion->unref();
	bitmap->unref();
}

SpriteInstance::~SpriteInstance()
{
	sprite->unref();
}

Keyframe::~Keyframe()
{
	for (std::size_t i = 0; i < sprites.size(); ++i)
		sprites[i]->unref();
}

void Keyframe::addSprite(SpriteInstance* instance)
{
	sprites.push_back(instance);
	instance->ref();
}

void Keyframe::removeSprite(SpriteInstance* instance)
{
	std::vector<SpriteInstance*>::iterator iter = std::find(sprites.begin(), sprites.end(), instance);

	if (iter != sprites.end())
	{
		sprites.erase(iter);
		instance->unref();
	}
}


Layer::Layer(const std::string& name) : name(name)
{
	visible = true;
	locked = false;
	keyframes.push_back(new Keyframe(1, 1, eTweenNone));
}

Layer::~Layer()
{
	for (std::size_t i = 0; i < keyframes.size(); ++i)
		keyframes[i]->unref();
}

Document::Document()
{
	layers_.push_back(new Layer("Layer 1"));
	layers_.push_back(new Layer("Layer 2"));
	layers_.push_back(new Layer("Layer 3"));


	layerselection_.insert(layers_[0]);
	layers_[0]->ref();

	currentframe_ = 1;
	currentlayer_ = 0;
}

Document::~Document()
{
	clearSelection();

	for (std::size_t i = 0; i < layers_.size(); ++i)
		layers_[i]->unref();

	for (std::size_t i = 0; i < textures_.size(); ++i)
		textures_[i]->unref();

	for (std::size_t i = 0; i < clipboard_.size(); ++i)
		clipboard_[i]->unref();

	for (std::set<Layer*>::iterator iter = layerselection_.begin(); iter != layerselection_.end(); ++iter)
		(*iter)->unref();
}

Keyframe* Document::getKeyframe(int layer, int frame) const
{
	std::vector<Keyframe*>& keyframes_ = layers_[layer]->keyframes;

	int i = getKeyframeIndex(layer, frame);
	if (i == -1)
		return NULL;

	return keyframes_[i];
}

int Document::getKeyframeIndex(int layer, int frame) const
{
	std::vector<Keyframe*>& keyframes_ = layers_[layer]->keyframes;

	for (std::size_t i = 0; i < keyframes_.size(); ++i)
	{
		int s = keyframes_[i]->start;
		int e = keyframes_[i]->end;

		if (s <= frame && frame <= e)
			return i;
	}

	return -1;
}

const std::vector<Keyframe*>& Document::keyframes(int layer)
{
	std::vector<Keyframe*>& keyframes_ = layers_[layer]->keyframes;

	return keyframes_;
}

void Document::validate()
{
/*	int s = keyframes_.size();

	Q_ASSERT(s > 0);

	Q_ASSERT(keyframes_[0]->start == 1);

	for (int i = 1; i < s; ++i)
		Q_ASSERT(keyframes_[i]->start == keyframes_[i - 1]->end + 1);

	for (int i = 0; i < s - 1; ++i)
		Q_ASSERT(keyframes_[i]->end == keyframes_[i + 1]->start - 1); */
}

void Document::load()
{
	Texture* texture1 = new Texture("test1.png");
	Texture* texture2 = new Texture("test2.png");
	Texture* texture3 = new Texture("test3.png");

	textures_.push_back(texture1);
	textures_.push_back(texture2);
	textures_.push_back(texture3);
}

void Document::setCurrentFrame(int frame)
{
	currentframe_ = frame;
}
int Document::getCurrentFrame() const
{
	return currentframe_;
}

TextureBase* Document::getRandomTexture() const
{
	int i = rand() % textures_.size();
	return textures_[i];
}

void Document::addSpriteInstance(int layer, int frame, SpriteInstance* instance)
{
	Keyframe* keyframe = getKeyframe(layer, frame);

	if (keyframe == NULL)
		return;

	keyframe->addSprite(instance);

	createLinks();
}

void Document::removeSpriteInstance(int layer, int frame, SpriteInstance* instance)
{
	Keyframe* keyframe = getKeyframe(layer, frame);

	if (keyframe == NULL)
		return;
	
	keyframe->removeSprite(instance);

	createLinks();
}

Keyframe* Document::getCurrentKeyframe(int layer) const
{
	return getKeyframe(layer, getCurrentFrame());
}

void Document::createLinks()
{
	for (std::size_t layer = 0; layer < layers_.size(); ++layer)
	{
		std::vector<Keyframe*>& keyframes_ = layers_[layer]->keyframes;

		for (std::size_t i = 0; i < keyframes_.size(); ++i)
			keyframes_[i]->links.clear();

		for (std::size_t i = 1; i < keyframes_.size(); ++i)
		{
			Keyframe* k0 = keyframes_[i - 1];
			Keyframe* k1 = keyframes_[i];

			for (std::size_t j = 0; j < k0->sprites.size(); ++j)
			{
				for (std::size_t k = 0; k < k1->sprites.size(); ++k)
				{
					SpriteInstance* s1 = k0->sprites[j];
					SpriteInstance* s2 = k1->sprites[k];

					if (*s1 == *s2)
					{
	//					k0->links.push_back(std::make_pair(s1, s2));
						k0->links[s1] = s2;
						break;
					}
				}
			}
		}
	}
}

void Document::insertKeyframe(int layer, int index, Keyframe* keyframe)
{
	std::vector<Keyframe*>& keyframes_ = layers_[layer]->keyframes;

	keyframes_.insert(keyframes_.begin() + index, keyframe);
	keyframe->ref();
	createLinks();
}
void Document::eraseKeyframe(int layer, int index)
{
	std::vector<Keyframe*>& keyframes_ = layers_[layer]->keyframes;

	keyframes_[index]->unref();
	keyframes_.erase(keyframes_.begin() + index);
	createLinks();
}

void Document::changeSelection(SpriteInstance* instance, bool multiple)
{
	if (multiple == false)
	{
		clearSelection();
		selection_.insert(instance);
		instance->ref();
	}
	else
	{
		if (selection_.find(instance) == selection_.end())
		{
			selection_.insert(instance);
			instance->ref();
		}
		else
		{
			selection_.erase(instance);
			instance->unref();
		}
	}
}

void Document::clearSelection()
{
	std::set<SpriteInstance*>::iterator iter, e = selection_.end();
	for (iter = selection_.begin(); iter != e; ++iter)
		(*iter)->unref();
	selection_.clear();
}

void Document::setSelection(const std::set<SpriteInstance*>& selection)
{
	clearSelection();

	selection_ = selection;

	std::set<SpriteInstance*>::iterator iter, e = selection_.end();
	for (iter = selection_.begin(); iter != e; ++iter)
		(*iter)->ref();
}

const std::set<SpriteInstance*>& Document::getSelection() const
{
	return selection_;
}


void Document::copySelected(const std::set<SpriteInstance*>& selection)
{
	if (selection.empty())
		return;

	for (std::size_t i = 0; i < clipboard_.size(); ++i)
		clipboard_[i]->unref();
	clipboard_.clear();

	std::set<SpriteInstance*>::const_iterator iter, e = selection.end();
	for (iter = selection.begin(); iter != selection.end(); ++iter)
	{
		SpriteInstance* instance = *iter;
		SpriteInstance* newinstance = new SpriteInstance(instance);
		clipboard_.push_back(newinstance);
	}
}

std::set<SpriteInstance*> Document::pasteClipboard(int layer, int frame, const std::vector<SpriteInstance*>& clipboard)
{
	std::set<SpriteInstance*> newinstances;
	
	// ayni frame'de ayni SpriteInstance olmasina izin verme (for all instances instance->textureregion should be 
	// different from each other)

	Keyframe* keyframe = getKeyframe(layer, frame);

	if (keyframe == NULL)
		return newinstances;

	if (clipboard.empty())
		return newinstances;
	
//	printf("Document::pasteClipboard()");
	for (std::size_t i = 0; i < clipboard.size(); ++i)
	{
		SpriteInstance* instance = clipboard[i];

		bool found = false;
		for (std::size_t j = 0; j < keyframe->sprites.size(); ++j)
		{
			if (*instance == *keyframe->sprites[i])
			{
				found = true;
				break;
			}
		}

		SpriteInstance* newinstance = new SpriteInstance(instance, found);
//		printf("%x-%x, ", clipboard[i], instance);
		addSpriteInstance(layer, frame, newinstance);
		newinstances.insert(newinstance);
		newinstance->unref();
	}
//	printf("Document::pasteClipboard()\n");

	createLinks();

	return newinstances;
}

void Document::deleteSelected(int frame, const std::set<SpriteInstance*>& selection)
{
	if (selection.empty())
		return;

	int layer = 0;

	Keyframe* keyframe = getKeyframe(layer, frame);

	if (keyframe == NULL)
		return;

	for (int i = (int)keyframe->sprites.size() - 1; i >= 0; --i)
	{
		SpriteInstance* instance = keyframe->sprites[i];

		if (selection.find(instance) != selection.end())
		{
			instance->unref();
			keyframe->sprites.erase(keyframe->sprites.begin() + i);
		}
	}

	clearSelection();
}

bool Document::isSelected(SpriteInstance* instance) const
{
	return selection_.find(instance) != selection_.end();
}

void Document::setSpriteInstances(int layer, int frame, const std::vector<SpriteInstance*>& instances)
{
	Keyframe* keyframe = getKeyframe(layer, frame);

	if (keyframe == NULL)
		return;

	for (std::size_t i = 0; i < instances.size(); ++i)
		instances[i]->ref();

	for (std::size_t i = 0; i < keyframe->sprites.size(); ++i)
		keyframe->sprites[i]->unref();

	keyframe->sprites = instances;

	createLinks();
}

const std::vector<SpriteInstance*>& Document::getClipboard() const
{
	return clipboard_;
}

void Document::moveLayers(const std::set<int>& layers, int newpos)
{
	int smaller = 0;
	for (std::set<int>::const_iterator iter = layers.begin(); iter != layers.end(); ++iter)
		if (*iter < newpos)
			smaller++;
	newpos -= smaller;

	std::vector<Layer*> v;
	for (std::set<int>::const_iterator iter = layers.begin(); iter != layers.end(); ++iter)
		v.push_back(layers_[*iter]);

	for (std::set<int>::const_reverse_iterator iter = layers.rbegin(); iter != layers.rend(); ++iter)
		layers_.erase(layers_.begin() + *iter);

	for (std::size_t i = 0; i < v.size(); ++i)
		layers_.insert(layers_.begin() + newpos, v[i]);
}

void Document::changeLayerSelection(Layer* layer, bool multiple)
{
	if (multiple == false)
	{
		for (std::set<Layer*>::iterator iter = layerselection_.begin(); iter != layerselection_.end(); ++iter)
			(*iter)->unref();
		layerselection_.clear();
		layerselection_.insert(layer);
		layer->ref();
	}
	else
	{
		if (layerselection_.find(layer) == layerselection_.end())
		{
			layerselection_.insert(layer);
			layer->ref();
		}
		else
		{
			layerselection_.erase(layer);
			layer->unref();
		}
	}

	// don't allow empty selection
	if (layerselection_.empty())
	{
		layerselection_.insert(layer);
		layer->ref();
	}
}

bool Document::isSelected(Layer* layer) const
{
	return layerselection_.find(layer) != layerselection_.end();
}
