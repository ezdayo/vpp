/**
 *
 * @file      vpp/util/templates.hpp
 *
 * @brief     This is a collection of helpers aimed at easing template meta-
 *            programming 
 *
 *            This file is part of the VPP framework (see link).
 *
 * @author    Olivier Stoltz-Douchet <ezdayo@gmail.com>
 *
 * @copyright (c) 2019-2020 Olivier Stoltz-Douchet
 * @license   http://opensource.org/licenses/MIT MIT
 * @link      https://github.com/ezdayo/vpp
 *
 **/

#pragma once

#include <type_traits>

namespace Util {

template <typename T> 
    struct remove_reference_wrapper {
        using type = T; };

template <typename T> 
    struct remove_reference_wrapper<std::reference_wrapper<T>> { 
        using type = T; };

template <typename T>
    using remove_reference_wrapper_t = 
        typename remove_reference_wrapper<T>::type;

template <typename T>
    using remove_reference_t = 
        typename std::remove_reference<T>::type;

template <typename T>
    using containee_t =
        typename remove_reference_t<T>::value_type;
        
template <typename T>
    using containee_object_t =
        remove_reference_wrapper_t<containee_t<T>>;
        
template <typename T>
    using iterator_t =
        typename std::remove_reference<T>::type::iterator;
       
template <typename T>
    struct storable_wrapper {
        using type = T; };

template <typename T>
    struct storable_wrapper<T&> {
        using type = typename std::reference_wrapper<T>::type; };

template <typename T>
    using storable_wrapper_t =
        typename storable_wrapper<T>::type;

}  // namespace Util
