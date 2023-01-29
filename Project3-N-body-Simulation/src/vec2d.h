#ifndef N_BODY_VEC2D_H
#define N_BODY_VEC2D_H

#include <cmath>
#include <ostream>

class vec2d {
  public:
    
    double x, y;
    
    vec2d() : x(0.0), y(0.0) { }
    vec2d(double x) : x(x), y(x) { }
    vec2d(double x, double y) : x(x), y(y) { }
    vec2d(const vec2d &v) : x(v.x), y(v.y) { }

    inline vec2d operator+(const vec2d &v) const {
      return vec2d(v.x + x, v.y + y);
    }

    inline vec2d operator-(const vec2d &v) const {
      return vec2d(x - v.x, y - v.y);
    }

    inline vec2d operator*(double a) const {
      return vec2d(x * a, y * a);
    }

    inline vec2d operator/(double d) const {
      return vec2d(x / d, y / d);
    }

    inline vec2d &operator+=(const vec2d &rhs) {
      x += rhs.x;
      y += rhs.y;
      return *this;
    }

    inline vec2d &operator-=(const vec2d &rhs) {
      x -= rhs.x;
      y -= rhs.y;
      return *this;
    }

    inline vec2d &operator*=(double a) {
      x *= a;
      y *= a;
      return *this;
    }

    inline vec2d &operator/=(double d) {
      x /= d;
      y /= d;
      return *this;
    }

    inline vec2d operator-(void) const {
      return vec2d(-x, -y);
    }

    inline double norm2() {
      return x*x + y*y;
    }

    inline double norm() {
      return sqrt(x*x + y*y);
    }

    inline vec2d unit() {
      double len = this->norm();
      if(len < 1e-9) return *this;
      return *this / len;
    }

    inline vec2d &operator=(double rhs) {
      x = y = rhs;
      return *this;
    }
    inline vec2d &operator=(const vec2d &rhs) {
      if(this != &rhs) {
        x = rhs.x;
        y = rhs.y;
      } 
      return *this;
    }
};

inline vec2d operator*(double a, const vec2d &v) {
  return v * a;
}

inline double dot(const vec2d &a, const vec2d &b) {
  return a.x * b.x + a.y * b.y;
}

inline double cross(const vec2d &a, const vec2d &b) {
  return a.x * b.y - a.y * b.x;
}

inline std::ostream &operator<<(std::ostream &os, const vec2d &v) {
  os << "(" << v.x << ", " << v.y << ")";
  return os;
}

#endif