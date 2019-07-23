#include "animation.hpp"
#include "helpers.hpp"

void AnimationBase::Update(unsigned long time, bool pause)
{
    if (!pause)
    {
        prog += (int)((time - lastTime) * speed);
        prog = prog % duration;
        if (prog < 0)
            prog += duration;
    }
    lastTime = time;
}

void AnimationBase::Reset()
{
    prog = 0;
    progessOffset = 0;
#ifdef SIMULATOR
    lastTime = 0;
#else
    lastTime = millis();
#endif
}

void AnimationBase::SetDuration(int d)
{
    duration = d;
    Reset();
}

int AnimationBase::Progress()
{
    int p = prog;
    if (progessOffset)
    {
        p = (p + progessOffset) % duration;
        if (p < 0)
            p += duration;
    }
    return p;
}

template <class T>
void AniParticle<T>::Draw(T *t)
{
    bool forward = true;
    int p = this->Progress();

    int offset = mapFloat(p, 0, this->duration, 0, this->duration / 25);
    int hueOffset = colorCycle ? mapFloat(p, 0, this->duration, 0, 255) : 0;
    srand(startEdge << 4);

    Edge *e = &t->edges[startEdge];
    int edgeSize;
    while (offset >= 0)
    {
        edgeSize = e->Size();
        if (offset - edgeSize + 1 < streakSize)
            e->Leds()->SetDir(forward).SetOffset(offset).SetWrap(false).SetViewport(0, edgeSize).DrawStreak(streakSize, hue + hueOffset, false, 255);
        offset -= edgeSize;

        int nextEdges = e->vertex[forward]->nConn - 1;
        e = e->GetNextEdge(rand() % nextEdges, forward);
    }
}

AniFillOutIn::AniFillOutIn()
{
    duration = 6000;
    const float totalTime = (Tesseract::outerSize + Tesseract::diagSize * 2 + Tesseract::innerSize) * 2;
    fade[0].fadeDuration = Tesseract::outerSize / totalTime;
    fade[1].fadeDuration = Tesseract::diagSize * 2 / totalTime;
    fade[2].fadeDuration = Tesseract::innerSize / totalTime;

    fade[0].fadeInStart = 0;
    fade[1].fadeInStart = fade[0].fadeInStart + fade[0].fadeDuration;
    fade[2].fadeInStart = fade[1].fadeInStart + fade[1].fadeDuration;

    fade[0].fadeOutStart = fade[2].fadeInStart + fade[2].fadeDuration;
    fade[1].fadeOutStart = fade[0].fadeOutStart + fade[0].fadeDuration;
    fade[2].fadeOutStart = fade[1].fadeOutStart + fade[1].fadeDuration;

    fade[0].styleIn = SubAniStripFade::InsideOut;
    fade[1].styleIn = SubAniStripFade::Forward;
    fade[2].styleIn = SubAniStripFade::OutsideIn;

    fade[0].styleOut = SubAniStripFade::OutsideIn;
    fade[1].styleOut = SubAniStripFade::Backwards;
    fade[2].styleOut = SubAniStripFade::InsideOut;
}

void AniFillOutIn::Draw(Tesseract *t)
{
    Edge *e;
    float p = mapFloat(Progress(), 0, duration, 0, 3);
    char hueOffset = mapFloat(Progress(), 0, duration, 0, 255);
    p = p - int(p);
    for (int i = 0; i < 32; i++)
    {
        e = &t->edges[i];
        if (e->Pos().w == -1)
        {
            fade[0].Draw(e->Leds(), p);
        }
        else if (e->Pos().w == 0)
        {
            if (e->vertex[0]->Pos().w == 1)
                e->Leds()->SetDir(false);
            fade[1].Draw(e->Leds(), p);
        }
        else if (e->Pos().w == 1)
        {
            fade[2].Draw(e->Leds(), p);
        }
        e->Leds()->DrawGradient(hueOffset, hueOffset + 255, e->Size());
    }
}

void AniVertexFill::Draw(Tesseract *t)
{
    Vertex *v = &t->verticies[startVertex];
    int maxLeds = (v->Pos().w == -1) ? maxLedsOuter : maxLedsInner;
    float offset = mapFloat(Progress(), 0, duration, 0, maxLeds);
    if (sink)
        offset = maxLeds - offset;

    unsigned int distances[Tesseract::nVerticies];
    t->GetDistances(v, distances);

    for (int i = 0; i < Tesseract::nVerticies; i++)
    {
        Vertex *v = &t->verticies[i];
        float vOffset = offset - distances[i];
        fill.Draw(v, vOffset);
        for (int j = 0; j < v->nConn; j++)
        {
            v->connections[j].edge->Leds()->DrawColor(this->color);
        }
    }
}

template <class T>
void AniPlaneRotate<T>::Draw(T *t)
{
    float rad = mapFloat(this->Progress(), 0, this->duration, 0, 2 * PI);
    slice.planeThickness = planeThickness;
    slice.planeNormal[0] = planeNormal[0];
    slice.planeNormal[1] = planeNormal[1];
    slice.planeNormal[2] = planeNormal[2];
    rotateZ(rad, slice.planeNormal);
    slice.Draw(t, planeOffset);
    for (int i = 0; i < T::nEdges; i++)
    {
        t->edges[i].Leds()->DrawColor(this->color);
    }
}

template <class T>
void AniRainbowFill<T>::Draw(T *t)
{
    const float sqrt3 = 1.73205080757f;
    const float thickness = 0.4;
    float offset = mapFloat(this->Progress(), 0, this->duration, 0, 4 * sqrt3 - thickness);
    slice.planeThickness = thickness;
    slice.planeNormal[0] = 1;
    slice.planeNormal[1] = 1;
    slice.planeNormal[2] = 1;
    slice.Draw(t, -2 * sqrt3 + offset);
    for (int i = 0; i < T::nEdges; i++)
    {
        float d1, d2;
        Edge *e = &t->edges[i];
        slice.GetDistance(e, d1, d2);
        unsigned char hue1 = mapFloat(d1, -2 * sqrt3, 2 * sqrt3, hue, hue + 100);
        unsigned char hue2 = mapFloat(d2, -2 * sqrt3, 2 * sqrt3, hue, hue + 100);
        if (d1 > d2)
            e->Leds()->SetOffset(0).DrawGradient(hue2, hue1, e->Size());
        else
            e->Leds()->SetOffset(0).DrawGradient(hue1, hue2, e->Size());
    }
}

void AniTheater::Draw(Tesseract *t)
{
    const int skip = 3;
    int idx = (this->Progress() / 50) % skip;
    t->Leds()->SetDir(true).SetOffset(0).SetWrap(false).SetViewport(0, t->Leds()->Size());
    for (int i = idx; i < t->Leds()->Size(); i += skip)
    {
        t->Leds()->SetLED(i, this->color);
    }
}

void SolidCube(Tesseract *t, int w, CRGB color)
{
    for (int i = 0; i < t->nEdges; i++)
    {
        Edge *e = &t->edges[i];
        if (e->Pos().w == w)
        {
            e->Leds()->SetDir(true).SetOffset(0).SetWrap(false).SetViewport(0, e->Size()).DrawColor(color);
        }
    }
}

void Parallel(Tesseract *t, Position::Axis axis, CRGB color)
{
    for (int i = 0; i < t->nEdges; i++)
    {
        Edge *e = &t->edges[i];
        if (e->Pos().axis[axis] == 0)
        {
            e->Leds()->SetDir(true).SetOffset(0).SetWrap(false).SetViewport(0, e->Size()).DrawColor(color);
        }
    }
}

void AniSolidCube::Draw(Tesseract *t)
{
    int progress = this->Progress();
    int hue = mapFloat(progress, 0, duration, 0, 255);
    CHSV color = CHSV(hue, 255, 100);
    progress = progress % 3000;
    if (progress < 500)
        SolidCube(t, -1, color);
    else if (progress >= 500 && progress < 1000)
        Parallel(t, Position::Axis::X, color);
    else if (progress >= 1000 && progress < 1500)
        Parallel(t, Position::Axis::Y, color);
    else if (progress >= 1500 && progress < 2000)
        Parallel(t, Position::Axis::Z, color);
    else if (progress >= 2000 && progress < 2500)
        Parallel(t, Position::Axis::W, color);
    else if (progress >= 2500 && progress < 3000)
        SolidCube(t, 1, color);
}

void AniTargeting::Draw(Tesseract *t)
{
    Edge *e;
    float p = mapFloat(Progress(), 0, duration, 0, 1);
    for (int i = 0; i < 32; i++)
    {
        e = &t->edges[i];
        if (e->Pos().w == -1)
        {
            e->Leds()->SetDir(true).SetOffset(0).SetWrap(false).SetViewport(11, 6);
        }
        else if (e->Pos().w == 0)
        {
            if (e->vertex[0]->Pos().w == 1)
                e->Leds()->SetDir(false);
            fade.Draw(e->Leds(), p);
            e->Leds()->DrawGradient(HUE_YELLOW, HUE_BLUE, e->Size());
        }
        else if (e->Pos().w == 1)
        {
            e->Leds()->SetDir(true).SetOffset(0).SetWrap(false).SetViewport(0, e->Size());
            e->Leds()->SetLED(3, this->color);
        }
    }
}

template <class T>
void AniBuildup<T>::Draw(T *t)
{
    const int onDuration = 50;
    Edge *e;
    int offset = mapFloat(this->Progress(), 0, this->duration, 0, this->duration / onDuration);
    fadeToBlackBy(t->Leds()->Leds(), t->Leds()->Size(), 10);
    srand(10);
    int edge = 0;
    for (int i = 0; i < offset; i++)
        edge = rand() % 32;

    e = &t->edges[edge];
    e->Leds()->SetDir(true).SetOffset(0).SetWrap(false).SetViewport(0, e->Size()).DrawColor(CHSV(offset * 144, 255, 100));
    for (int i = 0; i < offset; i++)
        edge = rand() % 32;
    e = &t->edges[edge];
    e->Leds()->SetDir(true).SetOffset(0).SetWrap(false).SetViewport(0, e->Size()).DrawColor(CHSV(offset * 144 + 128, 255, 100));
}

template <class T>
void AniRainbow<T>::Draw(T *t)
{
    for (int i = 0; i < t->nEdges; i++)
    {
        Edge *e = &t->edges[i];
        e->Leds()->SetDir(true).SetOffset(0).SetWrap(false).SetViewport(0, e->Size());
        if (!mode)
        {
            int hueOffset = mapFloat(this->Progress(), 0, this->duration, 0, 255);
            e->Leds()->DrawColor(CHSV(hueOffset, 255, 100));
        }
        else
        {
            e->Leds()->DrawRandom(3);
        }
    }
}

void AniClimb::Draw(Tesseract *t)
{
    const int spacing = 4;
    const int gradSize = 4;
    int offset = mapFloat(this->Progress(), 0, this->duration, 0, 42);
    for (int i = 0; i < t->nEdges; i++)
    {
        Edge *e = &t->edges[i];
        int edgeWidth = e->Size();
        Position top = Position(1, 1, 1, -1);

        float d1 = getDistance(top, e->vertex[0]->Pos());
        float d2 = getDistance(top, e->vertex[1]->Pos());
        if (d2 < d1)
            e->Leds()->SetDir(true);
        else
            e->Leds()->SetDir(false);

        e->Leds()->SetWrap(true).SetViewport(0, e->Size());
        while (edgeWidth > 0)
        {
            e->Leds()->SetOffset(offset + e->Size() - edgeWidth).DrawGradient(HUE_YELLOW, HUE_BLUE, gradSize);
            edgeWidth -= gradSize + spacing;
        }
    }
}

void AniWipe::Draw(Tesseract *t)
{
    float offset = mapFloat(this->Progress(), 0, this->duration, 0, t->Leds()->Size());
    if (invert)
        t->Leds()->SetViewport(offset, t->Leds()->Size() - offset);
    else
        t->Leds()->SetViewport(0, offset);
    t->Leds()->SetDir(true).SetOffset(0).SetWrap(false).DrawColor(this->color);
}

void AniSquare::Draw(Tesseract *t)
{
    Edge *e;
    bool forward;
    Vertex *v = t->VertexAt(startPos);
    for (int i = 0; i < v->nConn; i++)
    {
        if (v->connections[i].edge->Pos().axis[turnAxis[0]] == 0)
        {
            e = v->connections[i].edge;
            forward = v->connections[i].start;
        }
    }

    float offset = mapFloat(this->Progress(), 0, this->duration, 0, maxOffset) + streakSize;
    int edgeSize;
    int turnNum = 1;
    while (offset >= 0)
    {
        edgeSize = e->Size();
        if (offset - edgeSize + 1 < streakSize)
            e->Leds()->SetDir(forward).SetOffset(offset).SetWrap(false).SetViewport(0, edgeSize).DrawStreak(streakSize, hue, false, 200);
        offset -= edgeSize;

        e = e->Turn(turnAxis[turnNum % 2], forward);
        turnNum++;
    }
}

template class AniBuildup<Tesseract>;
template class AniRainbow<Tesseract>;
template class AniRainbowFill<Tesseract>;
template class AniParticle<Tesseract>;
template class AniPlaneRotate<Tesseract>;
