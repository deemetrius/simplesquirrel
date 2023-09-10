#pragma once
#ifndef SSQ_BINDING_HEADER_H
#define SSQ_BINDING_HEADER_H

#include "helpers.h"
#include "allocators.hpp"
#include <functional>
#include <cstring>

namespace ssq {
#ifndef DOXYGEN_SHOULD_SKIP_THIS
    namespace detail {
        // function_traits and make_function credits by @tinlyx https://stackoverflow.com/a/21665705

        // For generic types that are functors, delegate to its 'operator()'
        template <typename T>
        struct function_traits
            : public function_traits<decltype(&T::operator())>
        {};

        // for pointers to member function
        template <typename ClassType, typename ReturnType, typename... Args>
        struct function_traits<ReturnType(ClassType::*)(Args...) const> {
            //enum { arity = sizeof...(Args) };
            typedef std::function<ReturnType (Args...)> f_type;
        };

        // for pointers to member function
        template <typename ClassType, typename ReturnType, typename... Args>
        struct function_traits<ReturnType(ClassType::*)(Args...) > {
            typedef std::function<ReturnType (Args...)> f_type;
        };

        // for function pointers
        template <typename ReturnType, typename... Args>
        struct function_traits<ReturnType (*)(Args...)>  {
            typedef std::function<ReturnType (Args...)> f_type;
        };

        template <typename L> 
        inline typename function_traits<L>::f_type make_function(L l){
            return static_cast<typename function_traits<L>::f_type>(l);
        }

        template <typename T> struct Param {static const SQChar type = _SC('.');};

        template <> struct Param<char> {static const SQChar type = _SC('i');};
        template <> struct Param<signed char> {static const SQChar type = _SC('i');};
        template <> struct Param<short> {static const SQChar type = _SC('i');};
        template <> struct Param<int> {static const SQChar type = _SC('i');};
        template <> struct Param<long> {static const SQChar type = _SC('i');};
        template <> struct Param<unsigned char> {static const SQChar type = _SC('i');};
        template <> struct Param<unsigned short> {static const SQChar type = _SC('i');};
        template <> struct Param<unsigned int> {static const SQChar type = _SC('i');};
        template <> struct Param<unsigned long> {static const SQChar type = _SC('i');};
#ifdef _SQ64
        template <> struct Param<long long> {static const SQChar type = _SC('i');};
        template <> struct Param<unsigned long long> {static const SQChar type = _SC('i');};
#endif
        template <> struct Param<float> {static const SQChar type = _SC('f');};
        template <> struct Param<double> {static const SQChar type = _SC('f');};
#ifdef SQUNICODE
        template <> struct Param<std::wstring> {static const SQChar type = _SC('s');};
#else
        template <> struct Param<std::string> {static const SQChar type = _SC('s');};
#endif
        template <> struct Param<Class> {static const SQChar type = _SC('y');};
        template <> struct Param<Function> {static const SQChar type = _SC('c');};
        template <> struct Param<Table> {static const SQChar type = _SC('t');};
        template <> struct Param<Array> {static const SQChar type = _SC('a');};
        template <> struct Param<Instance> {static const SQChar type = _SC('x');};
        template <> struct Param<std::nullptr_t> {static const SQChar type = _SC('o');};

        template <typename A>
        static void paramPackerType(SQChar* ptr) {
            *ptr = Param<typename std::remove_const<typename std::remove_reference<A>::type>::type>::type;
        }

        template <typename ...B>
        static void paramPacker(SQChar* ptr) {
            int _[] = { 0, (paramPackerType<B>(ptr++), 0)... };
            (void)_;
            *ptr = _SC('\0');
        }

        template<typename Ret, typename... Args>
        static void bindUserData(HSQUIRRELVM vm, const std::function<Ret(Args...)>& func) {
            auto funcStruct = reinterpret_cast<detail::FuncPtr<Ret(Args...)>*>(sq_newuserdata(vm, sizeof(detail::FuncPtr<Ret(Args...)>)));
            funcStruct->ptr = new std::function<Ret(Args...)>(func);
            sq_setreleasehook(vm, -1, &detail::funcReleaseHook<Ret, Args...>);
        }

        template<typename T, typename... Args>
        static Object addClass(HSQUIRRELVM vm, const SQChar* name, const std::function<T*(Args...)>& allocator, bool release = true) {
            static const auto hashCode = typeid(T*).hash_code();
            static const std::size_t nparams = sizeof...(Args);

            Object clsObj(vm);
            
            sq_pushstring(vm, name, scstrlen(name));
            sq_newclass(vm, false);

            HSQOBJECT obj;
            sq_getstackobj(vm, -1, &obj);
            addClassObj(vm, hashCode, obj);

            sq_getstackobj(vm, -1, &clsObj.getRaw());
            sq_addref(vm, &clsObj.getRaw());

            sq_settypetag(vm, -1, reinterpret_cast<SQUserPointer>(hashCode));

            sq_pushstring(vm, _SC("constructor"), -1);
            bindUserData<T*>(vm, allocator);
            static SQChar params[33];
            paramPacker<T*, Args...>(params);

            if (release) {
                sq_newclosure(vm, &detail::classAllocator<T, Args...>, 1);
            } else {
                sq_newclosure(vm, &detail::classAllocatorNoRelease<T, Args...>, 1);
            }

            sq_setparamscheck(vm, nparams + 1, params);
            sq_newslot(vm, -3, false); // Add the constructor method

            sq_newslot(vm, -3, SQFalse); // Add the class

            return clsObj;
        }

        template<typename T>
        static Object addAbstractClass(HSQUIRRELVM vm, const SQChar* name) {
            static const auto hashCode = typeid(T*).hash_code();
            Object clsObj(vm);

            sq_pushstring(vm, name, strlen(name));
            sq_newclass(vm, false);

            HSQOBJECT obj;
            sq_getstackobj(vm, -1, &obj);
            addClassObj(vm, hashCode, obj);

            sq_getstackobj(vm, -1, &clsObj.getRaw());
            sq_addref(vm, &clsObj.getRaw());

            sq_settypetag(vm, -1, reinterpret_cast<SQUserPointer>(hashCode));
            sq_newslot(vm, -3, SQFalse); // Add the class

            return clsObj;
        }

        template<class Ret, class... Args, size_t... Is>
        static Ret callGlobal(HSQUIRRELVM vm, FuncPtr<Ret(Args...)>* funcPtr, index_list<Is...>) {
            return funcPtr->ptr->operator()(detail::pop<typename std::remove_reference<Args>::type>(vm, Is + 1)...);
        }

        template<int offet, typename R, typename... Args>
        struct func {
            static SQInteger global(HSQUIRRELVM vm) {
                try {
                    static const std::size_t nparams = sizeof...(Args);

                    FuncPtr<R(Args...)>* funcPtr;
                    sq_getuserdata(vm, -1, reinterpret_cast<void**>(&funcPtr), nullptr);

                    push(vm, std::forward<R>(callGlobal(vm, funcPtr, index_range<offet, sizeof...(Args) + offet>())));
                    return 1;
                } catch (std::exception& e) {
                    return sq_throwerror(vm, FromUtf8(e.what()).c_str());
                }
            }
        };

        template<int offet, typename... Args>
        struct func<offet, void, Args...> {
            static SQInteger global(HSQUIRRELVM vm) {
                try {
                    FuncPtr<void(Args...)>* funcPtr;
                    sq_getuserdata(vm, -1, reinterpret_cast<void**>(&funcPtr), nullptr);

                    callGlobal(vm, funcPtr, index_range<offet, sizeof...(Args) + offet>());
                    return 0;
                } catch (std::exception& e) {
                    return sq_throwerror(vm, ToSqString(e.what()).c_str());
                }
            }
        };

        template<typename R, typename... Args>
        static void addFunc(HSQUIRRELVM vm, const SQChar* name, const std::function<R(Args...)>& func) {
            static const std::size_t nparams = sizeof...(Args);

            sq_pushstring(vm, name, scstrlen(name));

            bindUserData(vm, func);
            static SQChar params[33];
            paramPacker<void, Args...>(params);

            sq_newclosure(vm, &detail::func<1, R, Args...>::global, 1);
            sq_setparamscheck(vm, nparams + 1, params);
            if(SQ_FAILED(sq_newslot(vm, -3, SQFalse))) {
                throw TypeException("Failed to bind function");
            }
        }

        template<typename R, typename... Args>
        static void addMemberFunc(HSQUIRRELVM vm, const SQChar* name, const std::function<R(Args...)>& func, bool isStatic) {
            static const std::size_t nparams = sizeof...(Args);

            sq_pushstring(vm, name, scstrlen(name));

            bindUserData(vm, func);
            static SQChar params[33];
            paramPacker<Args...>(params);

            sq_newclosure(vm, &detail::func<0, R, Args...>::global, 1);
            sq_setparamscheck(vm, nparams, params);
            if(SQ_FAILED(sq_newslot(vm, -3, isStatic))) {
                throw TypeException("Failed to bind member function");
            }
        }
    }
#endif
}

#endif