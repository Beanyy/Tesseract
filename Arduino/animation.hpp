#ifndef ANIMATION_HPP
#define ANIMATION_HPP

#include "ledStrip.hpp"
#include "geometry.hpp"
#include "subanimations.hpp"

#ifdef SIMULATOR
#include <ctime>
unsigned long millis();
#endif

class AnimationBase
{
private:
	unsigned long lastTime;
	int prog;

public:
	int duration; //Try to make this a number with many factors.
	int progessOffset;
	float speed;
	CRGB color;

	AnimationBase() : duration(0),
					  speed(1.0f)
	{
		Reset();
		color = CRGB(0, 128, 0);
	}

	virtual void Update(unsigned long time, bool pause = false);
	void Reset();
	void SetDuration(int d);
	int Progress();
};

template <class T>
class Animation : public AnimationBase
{
public:
	virtual void Draw(T *t) = 0;
};

template <class T>
class AniParticle : public Animation<T>
{
public:
	int startEdge;
	int streakSize;
	unsigned char hue;
	bool colorCycle;
	AniParticle()
	{
		this->duration = 80000;
	}
	virtual void Draw(T *t) override;
};

class AniFillOutIn : public Animation<Tesseract>
{
private:
	SubAniStripFade fade[3];

public:
	AniFillOutIn();
	virtual void Draw(Tesseract *t) override;
};

class AniTheater : public Animation<Tesseract>
{
public:
	AniTheater()
	{
		duration = 1050;
	}
	virtual void Draw(Tesseract *t) override;
};

class AniVertexFill : public Animation<Tesseract>
{
private:
	static const int nVerticies = sizeof(Tesseract::verticies) / sizeof(Vertex);
	static const int maxLedsOuter = 37;
	static const int maxLedsInner = 31;
	SubAniVertexFill fill;

public:
	AniVertexFill()
	{
		duration = 2000;
	}
	int startVertex;
	bool sink;
	virtual void Draw(Tesseract *t) override;
};

template <class T>
class AniPlaneRotate : public Animation<T>
{
public:
	AniPlaneRotate()
	{
		this->duration = 2000;
	}
	SubAniPlaneSlice<T> slice;
	float planeThickness;
	float planeOffset;
	float planeNormal[3];
	virtual void Draw(T *t) override;
};

template <class T>
class AniRainbowFill : public Animation<T>
{
public:
	AniRainbowFill()
	{
		this->duration = 2000;
	}
	SubAniPlaneSlice<T> slice;
	unsigned char hue;
	virtual void Draw(T *t) override;
};

class AniSolidCube : public Animation<Tesseract>
{
public:
	AniSolidCube()
	{
		this->duration = 12000;
	}
	virtual void Draw(Tesseract *t) override;
};

class AniTargeting : public Animation<Tesseract>
{
private:
	SubAniStripFade fade;

public:
	AniTargeting()
	{
		this->duration = 500;
		fade.fadeInStart = 0;
		fade.fadeDuration = 0.5;
		fade.fadeOutStart = fade.fadeDuration;
		fade.styleIn = SubAniStripFade::Forward;
		fade.styleOut = SubAniStripFade::Backwards;
	}
	virtual void Draw(Tesseract *t) override;
};

class AniClimb : public Animation<Tesseract>
{
public:
	AniClimb()
	{
		this->duration = 3000;
	}
	virtual void Draw(Tesseract *t) override;
};

class AniWipe : public Animation<Tesseract>
{
public:
	AniWipe()
	{
		this->duration = 5000;
	}
	char invert;
	virtual void Draw(Tesseract *t) override;
};

class AniSquare : public Animation<Tesseract>
{
public:
	AniSquare()
	{
		this->duration = 4000;
	}
	unsigned char hue;
    int streakSize;
	int maxOffset;
	Position startPos;
	Position::Axis turnAxis[2];
	virtual void Draw(Tesseract *t) override;
};

template <class T>
class AniBuildup : public Animation<T>
{
public:
	AniBuildup()
	{
		this->duration = 10000;
	}
	virtual void Draw(T *t) override;
};

template <class T>
class AniRainbow : public Animation<T>
{
public:
	AniRainbow()
	{
		this->duration = 1000;
	}
	int mode;
	virtual void Draw(T *t) override;
};

void SolidCube(Tesseract *t, int w, CRGB color);
void Parallel(Tesseract *t, Position::Axis axis, CRGB color);
#endif
