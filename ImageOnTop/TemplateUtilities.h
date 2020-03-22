#ifndef _TEMPLATEUTILITIES_H
#define _TEMPLATEUTILITIES_H


/**
 *  No-op deleter, useful when used with boost::shared_ptr.
 */
template<typename T>
class DummyDeleter
{
public:
	void operator ()(T *&) throw() {;}
};


#endif // _TEMPLATEUTILITIES_H
