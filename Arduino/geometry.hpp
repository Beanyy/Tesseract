#ifndef GEOMETRY_HPP
#define GEOMETRY_HPP
#include "LedStrip.hpp"

#ifdef SIMULATOR
#include <crgb.hpp>
#include <iostream>
#include <iomanip>
#include <fstream>
#endif

class Position
{
public:
  enum Axis
  {
    X = 0,
    Y,
    Z,
    W
  };
  Position(int8_t x_ = 0, int8_t y_ = 0, int8_t z_ = 0, int8_t w_ = 0) : x(x_),
                                                                         y(y_),
                                                                         z(z_),
                                                                         w(w_){};
  union {
    struct
    {
      int8_t x, y, z, w;
    };
    int8_t axis[4];
  };
  static void ToXYZ(Position &p, float xyz[3]);
#ifdef SIMULATOR
  friend std::ostream &operator<<(std::ostream &out, const Position &p)
  {
    out << "x: " << std::setw(2) << (int)p.x;
    out << " y: " << std::setw(2) << (int)p.y;
    out << " z: " << std::setw(2) << (int)p.z;
    out << " w: " << std::setw(2) << (int)p.w;
    return out;
  }
#endif
};

class Vertex;

class Edge
{
private:
  LedStrip leds;
  Position position;

public:
  Vertex *vertex[2];

  Edge(){};
  Edge(CRGB *leds_, int size, Position pos);

  Edge *GetNextEdge(int next, bool &forward);
  Edge *Turn(Position::Axis axis, bool &forward);

  void SetVerticies(Vertex *start, Vertex *end);
  Position Pos() { return position; }
  LedStrip *Leds() { return &leds; }
  int Size() { return leds.Size(); }
};

class Vertex
{
private:
  Position position;

public:
  Vertex(){};
  Vertex(Position pos) : position(pos),
                         nConn(0){};
  void AddEdge(Edge *e, bool start);
  Position Pos() { return position; }
  struct Connection
  {
    Edge *edge;
    bool start;
  };
  Connection connections[4];
  int nConn;
  uint32_t index;

#ifdef SIMULATOR
  friend std::ostream &operator<<(std::ostream &out, const Vertex &v)
  {
    out << "Connection Size: " << v.nConn << "\n";
    out << "Vertex: " << v.position << "\n";
    for (int j = 0; j < 4; j++)
    {
      out << "\tEdge: " << v.connections[j].edge->Pos() << " " << v.connections[j].start << "\n";
    }
    return out;
  }
#endif
};

template <int E, int V>
class Shape
{
public:
  static const int nEdges = E;
  static const int nVerticies = V;
  Edge edges[E];
  Vertex verticies[V];
  void GetDistances(Vertex *vStart, unsigned int distances[V]);
};

class Tesseract : public Shape<32, 16>
{
private:
  void Print();
  LedStrip leds;
#ifdef SIMULATOR
  std::ofstream colorFile;
#endif
public:
  static const uint8_t outerSize = 14;
  static const uint8_t diagSize = 6;
  static const uint8_t innerSize = 7;

  Tesseract(Position::Axis a[], CRGB *ledStrip, int size);
  Vertex *VertexAt(Position pos);
  LedStrip *Leds() { return &leds; }
  void Draw();
  void Clear();
};
#endif
