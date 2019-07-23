#include "geometry.hpp"

void Position::ToXYZ(Position &p, float xyz[3])
{
    float w = p.w;
    w = (1 - (w + 1) / 4);
    xyz[0] = p.x * w;
    xyz[1] = p.y * w;
    xyz[2] = p.z * w;
}

void Vertex::AddEdge(Edge *e, bool start)
{
    connections[nConn++] = {.edge = e,
                            .start = start};
}

Edge *Edge::GetNextEdge(int next, bool &forward)
{
    Vertex *v = vertex[forward];
    int i;
    for (i = 0; i < v->nConn; i++, next--)
    {
        if (this == v->connections[i].edge)
            i++;
        if (!next)
            break;
    }

    forward = v->connections[i].start;
    return v->connections[i].edge;
}

Edge *Edge::Turn(Position::Axis axis, bool &forward)
{
    Vertex *v = vertex[forward];
    for (int i = 0; i < v->nConn; i++)
    {
        if (v->connections[i].edge->Pos().axis[axis] == 0)
        {
            forward = v->connections[i].start;
            return v->connections[i].edge;
        }
    }
    return this;
}

Edge::Edge(CRGB *leds_, int size, Position pos) : leds(leds_, size),
                                                  position(pos)
{
}

void Edge::SetVerticies(Vertex *start, Vertex *end)
{
    vertex[0] = start;
    vertex[1] = end;
    vertex[0]->AddEdge(this, true);
    vertex[1]->AddEdge(this, false);
}

Vertex *Tesseract::VertexAt(Position pos)
{
    int index = 0;
    index |= ((pos.x + 1) / 2) << 3;
    index |= ((pos.y + 1) / 2) << 2;
    index |= ((pos.z + 1) / 2) << 1;
    index |= ((pos.w + 1) / 2) << 0;
    return &verticies[index];
}

void Tesseract::Print()
{
#ifdef SIMULATOR
    for (int i = 0; i < 16; i++)
    {
        // std::cout << verticies[i];
    }

    std::ofstream pos;
    pos.open("positions.csv");
    for (int i = 0; i < 32; i++)
    {
        int edgeLength = edges[i].Size() + 2;
        for (int j = 1; j < edgeLength - 1; j++)
        {
            float xyzw[4], frac;
            frac = j / ((float)(edgeLength - 1));
            for (int k = 0; k < 4; k++)
            {
                float start = edges[i].vertex[0]->Pos().axis[k];
                float end = edges[i].vertex[1]->Pos().axis[k];
                xyzw[k] = start + (end - start) * frac;
            }
            xyzw[3] = 1 - (xyzw[3] + 1) / 4;
            pos << xyzw[0] * xyzw[3] << ',' << xyzw[1] * xyzw[3] << ',' << xyzw[2] * xyzw[3] << "\n";
        }
    }
    pos.close();
#endif
}

Tesseract::Tesseract(Position::Axis a[], CRGB *ledStrip, int size) : leds(ledStrip, size)
{
    int i = 0;
    for (int x = -1; x < 2; x += 2)
    {
        for (int y = -1; y < 2; y += 2)
        {
            for (int z = -1; z < 2; z += 2)
            {
                for (int w = -1; w < 2; w += 2)
                {
                    verticies[i] = Vertex(Position(x, y, z, w));
                    verticies[i].index = i;
                    i++;
                }
            }
        }
    }

    bool outer = true;

    Position vertexPos(-1, -1, -1, -1);
    Position edgePos;
    Vertex *vStart = VertexAt(vertexPos);
    Vertex *vEnd;
    int ledPos = 0, ledSize;
    for (i = 0; i < 32; i++)
    {
        if (a[i] == Position::W)
        {
            ledSize = diagSize;
            outer = !outer;
        }
        else if (outer)
        {
            ledSize = outerSize;
        }
        else
        {
            ledSize = innerSize;
        }

        edgePos = vertexPos;
        edgePos.axis[a[i]] = 0;
        vertexPos.axis[a[i]] *= -1;

        vEnd = VertexAt(vertexPos);
        edges[i] = Edge(&ledStrip[ledPos], ledSize, edgePos);
        edges[i].SetVerticies(vStart, vEnd);

        ledPos += ledSize;
        vStart = vEnd;
    }
#ifdef SIMULATOR
    Print();
    colorFile.open("colors.csv");
#endif
}

void Tesseract::Clear()
{
    for (int i = 0; i < 32; i++)
    {
        edges[i].Leds()->Reset();
    }
}

void Tesseract::Draw()
{
#ifdef SIMULATOR
    for (int i = 0; i < 32; i++)
    {
        for (int j = 0; j < edges[i].Size(); j++)
        {
            CRGB color = edges[i].Leds()->Leds()[j];
            int value = color.r | (color.g << 8) | (color.b << 16);
            colorFile << value;
            if (i == 31 && j == edges[i].Size() - 1)
                colorFile << "\n";
            else
                colorFile << ", ";
        }
    }
    colorFile.flush();
#endif
}

template <int E, int V>
void Shape<E, V>::GetDistances(Vertex *vStart, unsigned int distances[V])
{
    bool visited[V];
    for (int i = 0; i < V; i++)
    {
        distances[i] = -1;
        visited[i] = false;
    }
    distances[vStart->index] = 0;
    for (int i = 0; i < V; i++)
    {
        int minIndex = 0;
        unsigned int min = -1;
        for (int j = 0; j < V; j++)
        {
            if (!visited[j] && distances[j] < min)
            {
                min = distances[j];
                minIndex = j;
            }
        }
        visited[minIndex] = true;

        Vertex *v = &verticies[minIndex];
        for (int i = 0; i < v->nConn; i++)
        {
            Edge *e = v->connections[i].edge;
            Vertex *u = e->vertex[v->connections[i].start];
            unsigned int dist = distances[minIndex] + e->Size();
            if (dist < distances[u->index])
                distances[u->index] = dist;
        }
    }
}

template class Shape<32, 16>;