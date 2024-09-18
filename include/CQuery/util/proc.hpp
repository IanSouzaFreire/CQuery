#ifndef CQUERY_UTIL_PROC_HPP
#define CQUERY_UTIL_PROC_HPP

namespace CQuery {

template <class fun_t>
fun_t proc(fun_t (*fun))
{ return fun(); }

template <class fun_t, class F>
fun_t proc(F&& fun)
{ return fun(); }

}

#endif