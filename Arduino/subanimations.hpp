#ifndef SUBANIMATION_HPP
#define SUBANIMATION_HPP

#include "geometry.hpp"
#include "ledStrip.hpp"

template <class T>
class SubAnimation
{
public:
	virtual void Draw(T *t, float prog) = 0;
};

class SubAniStripFade : public SubAnimation<LedStrip>
{
public:
	enum Style
	{
		InsideOut,
		OutsideIn,
		Forward,
		Backwards
	};
	enum Style styleIn;
	enum Style styleOut;
	float fadeInStart;
	float fadeOutStart;
	float fadeDuration;

	virtual void Draw(LedStrip *t, float prog) override;
};

class SubAniVertexFill : public SubAnimation<Vertex>
{
public:
	virtual void Draw(Vertex *v, float vpSize) override;
};

template <class T>
class SubAniPlaneSlice : public SubAnimation<T>
{
private:
	float DotProduct(Position p);
public:
	float planeNormal[3];
	float planeThickness;
	void GetDistance(Edge *e, float &d1, float &d2);
	virtual void Draw(T *t, float planeOffset) override;
};


#endif