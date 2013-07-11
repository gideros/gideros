#ifndef AE_DOCUMENT_H
#define AE_DOCUMENT_H

#include <refptr.h>

#include <vector>
#include <transform.h>
#include <map>

class Sprite;
class TextureBase;
class BitmapData;
class Bitmap;
class Stage;

enum TweenType
{
	eTweenNone,
	eTweenMotion,
	eTweenMotionWithEnd,
};

/*struct Parameter
{
	int type;
	float value;
}; */

struct SpriteInstance : public Referenced
{
	SpriteInstance(SpriteInstance* instance, bool clonetr = false);
	SpriteInstance(TextureBase* texture);
	~SpriteInstance();

	Sprite* sprite;							// sprite->bitmap->textureregion->texture
	Bitmap* bitmap;
	BitmapData* textureregion;
	TextureBase* texture;
//	std::vector<Parameter> parameters;
	float pivotx, pivoty;

	Transform transform;
};

inline bool operator == (const SpriteInstance& s1, const SpriteInstance& s2)
{
	return s1.textureregion == s2.textureregion;
}

struct Keyframe : public Referenced
{
	Keyframe() {}
	Keyframe(int start, int end, TweenType tweentype = eTweenNone) : start(start), end(end), tweentype(tweentype) {}
	
	virtual ~Keyframe();

	int start, end;
	TweenType tweentype;

	void addSprite(SpriteInstance* instance);
	void removeSprite(SpriteInstance* instance);

	std::vector<SpriteInstance*> sprites;
	std::map<SpriteInstance*, SpriteInstance*> links;
//	std::vector<std::pair<SpriteInstance*, SpriteInstance*> > links;
};

struct Layer : public Referenced
{
	Layer(const std::string& name);
	~Layer();

	std::string name;
	bool visible;
	bool locked;
	std::vector<Keyframe*> keyframes;
};

class Document
{
public:
	Document();
	~Document();

	const std::vector<Keyframe*>& keyframes(int layer);

	Keyframe* getKeyframe(int layer, int frame) const;
	int getKeyframeIndex(int layer, int frame) const;

	void insertKeyframe(int layer, int index, Keyframe* keyframe);
	void eraseKeyframe(int layer, int index);

	void validate();

	void load();

	void setCurrentFrame(int frame);
	int getCurrentFrame() const;
	Keyframe* getCurrentKeyframe(int layer) const;

	TextureBase* getRandomTexture() const;

	void addSpriteInstance(int layer, int frame, SpriteInstance* instance);
	void removeSpriteInstance(int layer, int frame, SpriteInstance* instance);
	void setSpriteInstances(int layer, int frame, const std::vector<SpriteInstance*>& instances);

	void createLinks();

	void changeSelection(SpriteInstance* instance, bool multiple);
	void clearSelection();
	void setSelection(const std::set<SpriteInstance*>& selection);
	const std::set<SpriteInstance*>& getSelection() const;

	void copySelected(const std::set<SpriteInstance*>& selection);
	std::set<SpriteInstance*> pasteClipboard(int layer, int frame, const std::vector<SpriteInstance*>& clipboard);
	void deleteSelected(int frame, const std::set<SpriteInstance*>& selection);

	const std::vector<SpriteInstance*>& getClipboard() const;

	int getCurrentLayerIndex() const
	{
		return currentlayer_;
	}

	Layer* getCurrentLayer() const
	{
		return layers_[currentlayer_];
	}

	const std::vector<Layer*>& layers() const
	{
		return layers_;
	}

	void moveLayers(const std::set<int>& layers, int newpos);
	void changeLayerSelection(Layer* layer, bool multiple);
	bool isSelected(Layer* layer) const;
	bool isSelected(SpriteInstance* instance) const;

private:

private:
//	std::vector<Keyframe*> keyframes_;
	std::vector<Layer*> layers_;
	int currentframe_;
	int currentlayer_;

private:
	std::vector<TextureBase*> textures_;
	std::set<SpriteInstance*> selection_;
	std::set<Layer*> layerselection_;

private:
	std::vector<SpriteInstance*> clipboard_;
};

#endif
