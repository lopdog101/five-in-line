#ifndef gomoku_primitivesH
#define gomoku_primitivesH

#include <math.h>
#include <limits>
#include <vector>

#ifdef max
#undef max
#endif

namespace Gomoku
{
//Точка
template<class T>
struct pointT
{
	T x;
	T y;
	//Не чем не инициализируется
	pointT(){}
	pointT(T _x,T _y){x=_x;y=_y;}
	inline bool operator==(const pointT& rhs) const{return x==rhs.x&&y==rhs.y;}
	inline bool operator!=(const pointT& rhs) const{return x!=rhs.x||y!=rhs.y;}

	inline void operator+=(const pointT& p)
	{
		x+=p.x;
		y+=p.y;
	}

	inline void operator-=(const pointT& p)
	{
		x-=p.x;
		y-=p.y;
	}

	inline pointT operator-(const pointT& p) const
	{
		return point(x-p.x,y-p.y);
	}

	inline pointT operator+(const pointT& p) const
	{
		return point(x+p.x,y+p.y);
	}
};

//Прямоугольник
template<class T>
struct rectT
{
	T x1;
	T y1;
	T x2;
	T y2;

	//не тривиальный конструктор в point не позволяет сделать union
	inline const pointT<T>& p1() const{return *reinterpret_cast<const pointT<T>*>(&x1);}
	inline pointT<T>& p1(){return *reinterpret_cast< pointT<T>*>(&x1);}
	inline const pointT<T>& p2() const{return *reinterpret_cast<const pointT<T>*>(&x2);}
	inline pointT<T>& p2(){return *reinterpret_cast< pointT<T>* >(&x2);}

	rectT(){}
	rectT(T _x1,T _y1,T _x2,T _y2)
	{
		x1=_x1;y1=_y1;x2=_x2;y2=_y2;
	}

	rectT(const pointT<T>& _p1,const pointT<T>& _p2)
	{
		p1()=_p1;
		p2()=_p2;
	}

	inline void normalize()
	{
		T tmp;
		if(x1>x2){tmp=x2;x2=x1;x1=tmp;}
		if(y1>y2){tmp=y2;y2=y1;y1=tmp;}
	}

	inline bool operator==(const rectT& rhs) const{return x1==rhs.x1&&y1==rhs.y1&&x2==rhs.x2&&y2==rhs.y2;}
	inline bool operator!=(const rectT& rhs) const{return x1!=rhs.x1||y1!=rhs.y1||x2!=rhs.x2||y2!=rhs.y2;}
	inline bool is_valid() const{return x1<=x2&&y1<=y2;}

	inline T distance(const rectT& r) const{T dx=x2-x1;T dy=y2-y1;return sqrt(dx*dx+dy*dy);}
	inline T area() const{return (x2-x1)*(y2-y1);}

	//Intersect two rects: construct common rect
	//result rect is invalid if source rects are not intersect
    inline void operator*=(rectT const& r)
	{ 
        if(x2>r.x2)x2=r.x2; 
        if(y2>r.y2)y2=r.y2;
        if(x1<r.x1)x1=r.x1;
        if(y1<r.y1)y1=r.y1;
    }

	//Intersect two rects: construct common rect
	//result rect is invalid if source rects are not intersect
    inline rectT operator*(const rectT& r2) const
	{ 
        rectT r=*this;
        if(r.x2>r2.x2) r.x2 = r2.x2; 
        if(r.y2>r2.y2) r.y2 = r2.y2;
        if(r.x1<r2.x1) r.x1 = r2.x1;
        if(r.y1<r2.y1) r.y1 = r2.y1;
        return r;
    }

	//Intersect two rects: construct common rect
    inline void operator+=(rectT const& r)
	{ 
        if(x2<r.x2)x2=r.x2; 
        if(y2<r.y2)y2=r.y2;
        if(x1>r.x1)x1=r.x1;
        if(y1>r.y1)y1=r.y1;
    }

	inline void operator+=(const pointT<T>& r)
	{
		//Специально не делал else
		//чтобы инверсная бесконечность превращалась в точку
		if(x2<r.x)x2=r.x;
		if(x1>r.x)x1=r.x;
		if(y2<r.y)y2=r.y;
		if(y1>r.y)y1=r.y;
	}

	//Intersect two rects: construct common rect
    inline rectT operator+(const rectT& r2) const
	{ 
        rectT r=*this;
        if(r.x2<r2.x2) r.x2 = r2.x2; 
        if(r.y2<r2.y2) r.y2 = r2.y2;
        if(r.x1>r2.x1) r.x1 = r2.x1;
        if(r.y1>r2.y1) r.y1 = r2.y1;
        return r;
    }

	//Check intersect two rects
    inline bool operator&(rectT const& r) const{return x1<=r.x2&&r.x1<=x2&&y1<=r.y2&&r.y1<=y2;}
	inline bool operator&(const pointT<T>& r) const{return x1<=r.x&&x2>=r.x&&y1<=r.y&&y2>=r.y;}

	/**
     * Check if rect is part of other rectanle
     * @return <code>true</code> if this rect is part of rect <code>r</code>
     */
    inline bool operator<=(rectT const& r) const{return x1>=r.x1&&x2<=r.x2&&y1>=r.y1&&y2<=r.y2;}

	/**
     * Check if other rectanle is part of rect
     * @return <code>true</code> if this rect contains rect <code>r</code>
     */
    inline bool operator>=(rectT const& r) const{return r.x1>=x1&&r.x2<=x2&&r.y1>=y1&&r.y2<=y2;}

    /**
     * Check if rect is strict subpart of other rectanle
     * @return <code>true</code> if this rect is part of rect <code>r</code> and not the same
     */
    inline bool operator<(rectT const& r) const{return *this<=r&&*this!=r;}
    /**
     * Check if rect is strict subpart of other rectanle
     * @return <code>true</code> if this rect contains rect <code>r</code> and not the same
     */
    inline bool operator>(rectT const& r) const{return *this >= r && *this != r;}

	inline void set_infinity()
	{
		x2=y2=std::numeric_limits<T>::max();
		x1=y1=-x2;
	}

	inline void set_inverse_infinity()
	{
		x1=y1=std::numeric_limits<T>::max();
		x2=y2=-x1;
	}

	inline T width() const{return x2-x1;}
	inline T height() const{return y2-y1;}
	inline T middle_x() const{return (x2+x1)/2;}
	inline T middle_y() const{return (y2+y1)/2;}
};

//Прямоугольник на всю область рациональных чисел
template<class T>
struct rect_infinityT : public rectT<T>
{
	rect_infinityT(){this->set_infinity();}
};

//Инвертированый прямоугольник на всю область рациональных чисел
template<class T>
struct rect_inverse_infinityT : public rectT<T>
{
	rect_inverse_infinityT(){this->set_inverse_infinity();}
};

// int точка
typedef pointT<int> point;
// int прямоугольник
typedef rectT<int> rect;
// int R
typedef rect_infinityT<int> rect_infinity;
// int inverse R
typedef rect_inverse_infinityT<int> rect_inverse_infinity;


template<class Iter>
inline rect get_bbox(Iter from,Iter to)
{
	rect ret=rect_inverse_infinity();
	for(;from!=to;++from)
		ret+=*from;
	return ret;
}

}//namespace Gomoku


#endif
