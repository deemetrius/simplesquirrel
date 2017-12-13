#pragma once
#ifndef SQUIRREL_BIND_ARRAY_HEADER_H
#define SQUIRREL_BIND_ARRAY_HEADER_H

#include "object.hpp"
#include "args.hpp"
#include <squirrel.h>
#include <vector>

namespace SquirrelBind {
    /**
    * @brief Squirrel intance of array object
    */
    class SQBIND_API SqArray: public SqObject {
    public:
        /**
        * @brief Constructs empty array
        */
        SqArray(HSQUIRRELVM vm, size_t len = 0);
        /**
        * @brief Constructs array out of std::vector
        */
        template<typename T>
        SqArray(HSQUIRRELVM vm, const std::vector<T>& vector):SqObject(vm) {
            sq_newarray(vm, 0);
            sq_getstackobj(vm, -1, &obj);
            sq_addref(vm, &obj);

            for(const auto& val : vector) {
                detail::push(vm, val);
                if(SQ_FAILED(sq_arrayappend(vm, -2))) {
                    sq_pop(vm, 2);
                    throw SqTypeException("Failed to push value to back of the array");
                }
            }

            sq_pop(vm, 1); // Pop array
        }
        /**
        * @brief Converts SqObject to SqArray
        * @throws SqTypeException if the SqObject is not type of an array
        */
        explicit SqArray(const SqObject& object);
        /**
        * @brief Copy constructor
        */
        SqArray(const SqArray& other);
        /**
        * @brief Move constructor
        */
        SqArray(SqArray&& other) NOEXCEPT;
        /**
        * @brief Returns the size of the array
        */
        size_t size();
        /**
        * @brief Pushes an element to the back of the array
        */
        template<typename T>
        void push(const T& value) {
            sq_pushobject(vm, obj);
            detail::push(vm, value);
            if(SQ_FAILED(sq_arrayappend(vm, -2))) {
                sq_pop(vm, 2);
                throw SqTypeException("Failed to push value to back of the array");
            }
            sq_pop(vm, 1);
        }
        /**
        * @brief Pops an element from the back of the array and returns it
        */
        template<typename T>
        T popAndGet() {
            sq_pushobject(vm, obj);
            auto s = sq_getsize(vm, -1);
            if(s == 0) {
                sq_pop(vm, 1);
                throw SqTypeException("Out of bounds");
            }

            try {
                if(SQ_FAILED(sq_arraypop(vm, -1, true))) {
                    sq_pop(vm, 1);
                    throw SqTypeException("Failed to pop value from back of the array");
                }
                T ret(detail::pop<T>(vm, -1));
                sq_pop(vm, 1);
                return std::move(ret);
            } catch (...) {
                sq_pop(vm, 1);
                std::rethrow_exception(std::current_exception());
            }
        }
        /**
        * @brief Pops an element from the back of the array
        */
        void pop();
        /**
        * @brief Returns an element from the specific index
        * @throws SqTypeException if the index is out of bounds or element cannot be returned
        */
        template<typename T>
        T get(size_t index) {
            sq_pushobject(vm, obj);
            auto s = static_cast<size_t>(sq_getsize(vm, -1));
            if(index >= s) {
                sq_pop(vm, 1);
                throw SqTypeException("Out of bounds");
            }
            detail::push(vm, index);
            if(SQ_FAILED(sq_get(vm, -2))) {
                sq_pop(vm, 1);
                throw SqTypeException("Failed to get value from the array");
            }
            try {
                T ret(detail::pop<T>(vm, -1));
                sq_pop(vm, 2);
                return std::move(ret);
            } catch (...) {
                sq_pop(vm, 2);
                std::rethrow_exception(std::current_exception());
            }
        }
        /**
         * Returns the element at the start of the array
         * @throws SqTypeException if the array is empty or element cannot be returned
         */
        template<typename T>
        T begin() {
            return get<T>(0);
        }
        /**
         * Returns the element at the end of the array
         * @throws SqTypeException if the array is empty or element cannot be returned
         */
        template<typename T>
        T back() {
            auto s = size();
            if (s == 0) throw SqTypeException("Out of bounds");
            return get<T>(s - 1);
        }
        /**
        * @brief Sets an element at the specific index
        * @throws SqTypeException if the index is out of bounds or element cannot be set
        */
        template<typename T>
        void set(size_t index, const T& value) {
            sq_pushobject(vm, obj);
            auto s = static_cast<size_t>(sq_getsize(vm, -1));
            if(index >= s) {
                sq_pop(vm, 1);
                throw SqTypeException("Out of bounds");
            }
            detail::push(vm, index);
            detail::push(vm, value);
            if(SQ_FAILED(sq_set(vm, -3))) {
                sq_pop(vm, 1);
                throw SqTypeException("Failed to set value in the array");
            }
            sq_pop(vm, 1);
        }
        /**
         * @brief Converts this array to std::vector of objects
         */
        std::vector<SqObject> convertRaw();
        /**
         * @brief Converts this array to std::vector of specific type T
         */
        template<typename T>
        std::vector<T> convert() {
            sq_pushobject(vm, obj);
            auto s = static_cast<size_t>(sq_getsize(vm, -1));
            std::vector<T> ret;
            ret.reserve(s);
            while(s--) {
                sq_arraypop(vm, -1, true);
                ret.push_back(detail::pop<T>(vm, -1));
            }
            sq_pop(vm, 1);
            return ret;
        }
        /**
        * @brief Copy assingment operator
        */ 
        SqArray& operator = (const SqArray& other);
        /**
        * @brief Move assingment operator
        */
        SqArray& operator = (SqArray&& other) NOEXCEPT;
    };
}

#endif