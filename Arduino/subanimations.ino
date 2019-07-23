#include "subanimations.hpp"
#include "helpers.hpp"

void SubAniVertexFill::Draw(Vertex *v, float vpSize)
{
    for (int i = 0; i < v->nConn; i++)
    {
        Edge *e = v->connections[i].edge;
        float vp = constrain(vpSize, 0, e->Size());
        e->Leds()->SetDir(v->connections[i].start).SetViewport(0, vp);
    }
}

void SubAniStripFade::Draw(LedStrip *l, float prog)
{
    float ledSize = l->Size();
    float vpStart;
    float vpSize;
    enum Style style;

    if (prog < fadeOutStart)
    {
        style = styleIn;
        vpSize = mapFloat(prog, fadeInStart, fadeInStart + fadeDuration, 0, ledSize);
    }
    else
    {
        style = styleOut;
        vpSize = mapFloat(prog, fadeOutStart, fadeOutStart + fadeDuration, ledSize, 0);
    }
    vpSize = constrain(vpSize, 0, ledSize);

    if (style == InsideOut)
        vpStart = (ledSize - vpSize) / 2;
    else if (style == OutsideIn)
        vpStart = ledSize - vpSize / 2;
    else if (style == Forward)
        vpStart = 0;
    else if (style == Backwards)
        vpStart = ledSize - vpSize;

    l->SetViewport(vpStart, vpSize);
}

template <class T>
float SubAniPlaneSlice<T>::DotProduct(Position p)
{
    float xyz[3];
    Position::ToXYZ(p, xyz);
    return xyz[0] * planeNormal[0] + xyz[1] * planeNormal[1] + xyz[2] * planeNormal[2];
}

template <class T>
void SubAniPlaneSlice<T>::Draw(T *t, float planeOffset)
{
    float l1, r1, l2, r2;

    l2 = planeOffset;
    r2 = planeOffset + planeThickness;

    for (int i = 0; i < T::nEdges; i++)
    {
        Edge *e = &t->edges[i];
        int size = e->Size();
        l1 = DotProduct(e->vertex[0]->Pos());
        r1 = DotProduct(e->vertex[1]->Pos());

        bool forward = (l1 < r1);
        if (!forward)
            swap(l1, r1);

        float vpStart = size * (l2 - l1) / (r1 - l1 + 0.01);
        float vpSize = size * (r2 - l2) / (r1 - l1 + 0.01);

        if (vpStart < 0)
        {
            vpSize += vpStart;
            vpStart = 0;
        }

        if (vpSize > size - vpStart)
            vpSize = size - vpStart;

        if (vpSize > 0)
            e->Leds()->SetDir(forward).SetViewport(vpStart, vpSize);
        else
            e->Leds()->SetViewport(0, 0);
    }
}

template <class T>
void SubAniPlaneSlice<T>::GetDistance(Edge *e, float &d1, float &d2)
{
    d1 = DotProduct(e->vertex[0]->Pos());
    d2 = DotProduct(e->vertex[1]->Pos());
}
template class SubAniPlaneSlice<Tesseract>;
