
#pragma once

#include <boost/make_shared.hpp>
#include <boost/scoped_ptr.hpp>
#include <boost/function.hpp>
#include <boost/type_traits/function_traits.hpp>
#include <boost/preprocessor.hpp>

namespace boost {

/*
 * cfunction 用于将 boost::function 拆分为一个 C 函数指针和一个 void * user_data 指针.
 *
 * 只要接受 一个 C 函数指针 + void * user_data 的地方, 就可以利用 cfunction 封装 boost::function,
 * 而 boost::function 又可以继续封装 boost::bind, boost::bind 的强大就无需多说了吧.
 *
 * 例子在是这样的, 相信这个例子一看大家全明白了, 呵呵
 *
static void * my_thread_func2(int a, int b, int c, cfunction<void *(*) (void *),  void*()> func2)
{
	// 故意睡一下，保证 那个线程的 func2 对象析构，这个拷贝的 func2 也能维持好 C 函数的数据，避免下面的代码崩溃！
	sleep(10);
	std::cout << "这样就不用管理 func2 啦！自动释放不是？" << std::endl;
	std::cout <<  a <<  b <<  c <<  std::endl;
	return NULL;
}

static void * my_thread_func(int a, int b,  int c)
{
	cfunction<void *(*) (void *),  void*()> func2;

	// 通过将自己绑定进去，func2 这个对象就不必保持生命期， 回调执行完毕会自动清空数据！
	func2 = boost::bind(&my_thread_func2, 3, 2, 3, func2);

	pthread_t new_thread;

	pthread_create(&new_thread, NULL, func2.c_func_ptr(), func2.c_void_ptr());

	// 让线程进入不可 join 状态，无需等待线程退出
	pthread_detach(new_thread);

	std::cout <<  a <<  b <<  c <<  std::endl;
	return NULL;
	// 这里 func2 对象析构了，但是估计那个线程函数还没有执行完毕，如果不 bind 进去，到这里析构后，那个线程就会崩溃。
	// 但是，因为将 func2 对象绑定进去了，所以那个线程可以继续安然无恙的运行。
}

int main(int, char*[])
{
	cfunction<void *(*) (void *),  void*()> func;

	func = boost::bind(&my_thread_func, 1, 2, 3);

	pthread_t new_thread;

	pthread_create(&new_thread, NULL, func.c_func_ptr(), func.c_void_ptr());
	// 等待 线程退出，这样这个 func 对象要保证回调期间的生命期
	pthread_join(new_thread, NULL);

	// 等待另一个unjoinable 的线程退出
	sleep(100);
    return 0;
}

 *
 */
template<typename CFuncType, typename ClosureSignature>
class cfunction
{
public:
	typedef typename boost::function_traits<ClosureSignature>::result_type return_type;
	typedef boost::function<ClosureSignature> closure_type;
	private:
		boost::shared_ptr<closure_type> m_wrapped_func;
	public:
		cfunction()
			: m_wrapped_func(make_shared<closure_type>())
		{
		}

		cfunction(const closure_type &bindedfuntor)
			: m_wrapped_func(make_shared<closure_type>())
		{
			*m_wrapped_func = bindedfuntor;
		}

		cfunction& operator = (const closure_type& bindedfuntor)
		{
			*m_wrapped_func = closure_type(bindedfuntor);
			return *this;
		}

		void * c_void_ptr()
		{
			return m_wrapped_func.get();
		}

		CFuncType c_func_ptr()
		{
			return (CFuncType)wrapperd_callback;
		}

	private: // 这里是一套重载, 被 c_func_ptr() 依据 C 接口的类型挑一个出来并实例化.
		static return_type wrapperd_callback(void* user_data)
		{
			closure_type *  wrapped_func = reinterpret_cast<closure_type*>(user_data);

			return (*wrapped_func)();
		}

#define ARG(z, n, _) Arg ## n arg ## n

#define TEXT(z, n, text) text ## n

#define TTP(z, n, _) \
		template< \
			BOOST_PP_ENUM_ ## z(BOOST_PP_INC(n), TEXT, typename Arg) \
		> \
		static return_type wrapperd_callback(\
				BOOST_PP_ENUM_ ## z(BOOST_PP_INC(n), ARG, nil), void* user_data) \
		{\
			closure_type * wrapped_func = reinterpret_cast<closure_type*>(user_data); \
			return (*wrapped_func)(BOOST_PP_ENUM_ ## z(BOOST_PP_INC(n), TEXT, arg) ); \
		}

		BOOST_PP_REPEAT_FROM_TO(0, 9, TTP, nil)
};


} // namespace boost
