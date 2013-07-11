#ifndef POINT_H
#define POINT_H

template <typename T>
struct Point2
{
	Point2() : x(T()), y(T()) {}
	Point2(T x, T y) : x(x), y(y) {}

	T x;
	T y;
};

template <typename T>
inline bool operator == (const Point2<T>& p0, const Point2<T>& p1)
{
	return p0.x == p1.x && p0.y == p1.y;
}

template <typename T>
inline bool operator != (const Point2<T>& p0, const Point2<T>& p1)
{
	return p0.x != p1.x || p0.y != p1.y;
}

typedef Point2<int> Point2i;
typedef Point2<float> Point2f;




#endif