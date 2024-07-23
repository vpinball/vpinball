/*
 This file is part of the Tweeny library.

 Copyright (c) 2016-2021 Leonardo Guilherme Lucena de Freitas
 Copyright (c) 2016 Guilherme R. Costa

 Permission is hereby granted, free of charge, to any person obtaining a copy of
 this software and associated documentation files (the "Software"), to deal in
 the Software without restriction, including without limitation the rights to
 use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of
 the Software, and to permit persons to whom the Software is furnished to do so,
 subject to the following conditions:

 The above copyright notice and this permission notice shall be included in all
 copies or substantial portions of the Software.

 THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS
 FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR
 COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER
 IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
 CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
*/

/**
 * @file tweeny.h
 * This file is the main header file for Tweeny. You should not need to include anything else.
 */

/**
 * @mainpage Tweeny
 *
 * Tweeny is an inbetweening library designed for the creation of complex animations for games and other beautiful
 * interactive software. It leverages features of modern C++ to empower developers with an intuitive API for
 * declaring tweenings of any type of value, as long as they support arithmetic operations.
 *
 * This document contains Tweeny's API reference. The most interesting parts are:
 *
 * * The Fine @ref manual
 * * The tweeny::from global function, to start a new tween.
 * * The tweeny::tween class itself, that has all the interesting methods for a tween.
 * * The <a href="modules.html">modules page</a> has a list of type of easings.
 *
 * This is how the API looks like:
 *
 * @code
 *
 * #include "tweeny.h"
 *
 * using tweeny::easing;
 *
 * int main() {
 *  // steps 1% each iteration
 *  auto tween = tweeny::from(0).to(100).during(100).via(easing::linear);
 *  while (tween.progress() < 1.0f) tween.step(0.01f);
 *
 *  // a tween with multiple values
 *  auto tween2 = tweeny::from(0, 1.0f).to(1200, 7.0f).during(1000).via(easing::backInOut, easing::linear);
 *
 *  // a tween with multiple points, different easings and durations
 *  auto tween3 = tweeny::from(0, 0)
 *                  .to(100, 100).during(100).via(easing::backOut, easing::backOut)
 *                  .to(200, 200).during(500).via(easing::linear);
 *  return 0;
 * }
 *
 * @endcode
 *
 * **Examples**
 *
 * * Check <a href="http://github.com/mobius3/tweeny-demos">tweeny-demos</a> repository to see demonstration code
 *
 * **Useful links and references**
 * * <a href="http://www.timotheegroleau.com/Flash/experiments/easing_function_generator.htm">Tim Groleau's easing function generator (requires flash)</a>
 * * <a href="http://easings.net/">Easing cheat sheet (contains graphics!)</a>
 */

#ifndef TWEENY_H
#define TWEENY_H

/*
 This file is part of the Tweeny library.

 Copyright (c) 2016-2021 Leonardo Guilherme Lucena de Freitas
 Copyright (c) 2016 Guilherme R. Costa

 Permission is hereby granted, free of charge, to any person obtaining a copy of
 this software and associated documentation files (the "Software"), to deal in
 the Software without restriction, including without limitation the rights to
 use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of
 the Software, and to permit persons to whom the Software is furnished to do so,
 subject to the following conditions:

 The above copyright notice and this permission notice shall be included in all
 copies or substantial portions of the Software.

 THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS
 FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR
 COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER
 IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
 CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
*/

/**
 * @file tween.h
 * This file contains the core of tweeny: the main tween class.
 */

#ifndef TWEENY_TWEEN_H
#define TWEENY_TWEEN_H

#include <tuple>
#include <vector>
#include <functional>

/*
 This file is part of the Tweeny library.

 Copyright (c) 2016-2021 Leonardo Guilherme Lucena de Freitas
 Copyright (c) 2016 Guilherme R. Costa

 Permission is hereby granted, free of charge, to any person obtaining a copy of
 this software and associated documentation files (the "Software"), to deal in
 the Software without restriction, including without limitation the rights to
 use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of
 the Software, and to permit persons to whom the Software is furnished to do so,
 subject to the following conditions:

 The above copyright notice and this permission notice shall be included in all
 copies or substantial portions of the Software.

 THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS
 FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR
 COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER
 IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
 CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
*/

/*
 * This file provides useful typedefs and traits for a tween.
 */

#ifndef TWEENY_TWEENTRAITS_H
#define TWEENY_TWEENTRAITS_H

#include <tuple>
#include <functional>
#include <type_traits>

namespace tweeny {
    template<typename T, typename... Ts> class tween;

    namespace detail {

      template<typename... Ts> struct equal {};
      template<typename T> struct equal<T> { enum { value = true }; };
      template <typename T, typename U, typename... Ts> struct equal<T, U, Ts...> {
        enum { value = std::is_same<T, U>::value && equal<T, Ts...>::value && equal<U, Ts...>::value };
      };

        template<typename T, typename...> struct first { typedef T type; };

        template<bool equal, typename... Ts>
        struct valuetype { };

        template<typename... Ts>
        struct valuetype<false, Ts...> {
            typedef std::tuple<Ts...> type;
        };

        template<typename... Ts>
        struct valuetype<true, Ts...> {
            typedef std::array<typename first<Ts...>::type, sizeof...(Ts)> type;
        };

        template<typename... Ts>
        struct tweentraits {
            typedef std::tuple<std::function<Ts(float, Ts, Ts)>...> easingCollection;
            typedef std::function<bool(tween<Ts...> &, Ts...)> callbackType;
            typedef std::function<bool(tween<Ts...> &)> noValuesCallbackType;
            typedef std::function<bool(Ts...)> noTweenCallbackType;
            typedef typename valuetype<equal<Ts...>::value, Ts...>::type valuesType;
            typedef std::array<uint16_t, sizeof...(Ts)> durationsArrayType;
            typedef tween<Ts...> type;
        };
    }
}

#endif //TWEENY_TWEENTRAITS_H

/*
 This file is part of the Tweeny library.

 Copyright (c) 2016-2021 Leonardo Guilherme Lucena de Freitas
 Copyright (c) 2016 Guilherme R. Costa

 Permission is hereby granted, free of charge, to any person obtaining a copy of
 this software and associated documentation files (the "Software"), to deal in
 the Software without restriction, including without limitation the rights to
 use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of
 the Software, and to permit persons to whom the Software is furnished to do so,
 subject to the following conditions:

 The above copyright notice and this permission notice shall be included in all
 copies or substantial portions of the Software.

 THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS
 FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR
 COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER
 IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
 CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
*/

/*
 * This file provides the declarations for a tween point utility class. A tweenpoint holds the tween values,
 * easings and durations.
 */

#ifndef TWEENY_TWEENPOINT_H
#define TWEENY_TWEENPOINT_H

#include <tuple>
#include <array>

namespace tweeny {
    namespace detail {
        /*
         * The tweenpoint class aids in the management of a tweening point by the tween class.
         * This class is private.
         */
        template<typename... Ts>
        struct tweenpoint {
            typedef detail::tweentraits<Ts...> traits;

            typename traits::valuesType values;
            typename traits::durationsArrayType durations;
            typename traits::easingCollection easings;
            typename traits::callbackType onEnterCallbacks;
            uint32_t stacked;

            /* Constructs a tweenpoint from a set of values, filling their durations and easings */
            tweenpoint(Ts... vs);

            /* Set the duration for all the values in this point */
            template<typename D> void during(D milis);

            /* Sets the duration for each value in this point */
            template<typename... Ds> void during(Ds... vs);

            /* Sets the easing functions of each value */
            template<typename... Fs> void via(Fs... fs);

            /* Sets the same easing function for all values */
            template<typename F> void via(F f);

            /* Returns the highest value in duration array */
            uint16_t duration() const;

            /* Returns the value of that specific value */
            uint16_t duration(size_t i) const;
        };
    }
}

/*
 This file is part of the Tweeny library.

 Copyright (c) 2016-2021 Leonardo Guilherme Lucena de Freitas
 Copyright (c) 2016 Guilherme R. Costa

 Permission is hereby granted, free of charge, to any person obtaining a copy of
 this software and associated documentation files (the "Software"), to deal in
 the Software without restriction, including without limitation the rights to
 use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of
 the Software, and to permit persons to whom the Software is furnished to do so,
 subject to the following conditions:

 The above copyright notice and this permission notice shall be included in all
 copies or substantial portions of the Software.

 THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS
 FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR
 COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER
 IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
 CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
*/

/*
 * This file implements the tweenpoint class
 */

#ifndef TWEENY_TWEENPOINT_TCC
#define TWEENY_TWEENPOINT_TCC

#include <algorithm>
#include <type_traits>

/*
 This file is part of the Tweeny library.

 Copyright (c) 2016-2021 Leonardo Guilherme Lucena de Freitas
 Copyright (c) 2016 Guilherme R. Costa

 Permission is hereby granted, free of charge, to any person obtaining a copy of
 this software and associated documentation files (the "Software"), to deal in
 the Software without restriction, including without limitation the rights to
 use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of
 the Software, and to permit persons to whom the Software is furnished to do so,
 subject to the following conditions:

 The above copyright notice and this permission notice shall be included in all
 copies or substantial portions of the Software.

 THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS
 FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR
 COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER
 IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
 CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
*/

/**
 * @file easing.h
 * The purpose of this file is to list all bundled easings. All easings are based on Robert Penner's easing
 * functions: http://robertpenner.com/easing/
 */

#ifndef TWEENY_EASING_H
#define TWEENY_EASING_H

#include <cmath>
#include <type_traits>

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

/**
    * @defgroup easings Easings
    * @brief Bundled easing functions based on
    *        <a href="http://robertpenner.com/easing/">Robert Penner's Easing Functions</a>
    * @details You should plug these functions into @ref tweeny::tween::via function to specify the easing used in a tween.
    * @sa tweeny::easing
    * @{
    *//**
    *   @defgroup stepped Stepped
    *   @{
    *       @brief The value does not change. No interpolation is used.
    *   @}
    *//**
    *   @defgroup default Default
    *   @{
    *       @brief A default mode for arithmetic values it will change in constant speed, for non-arithmetic value will be constant.
    *   @}
    *//**
    *   @defgroup linear Linear
    *   @{
    *       @brief The most boring ever easing function. It has no acceleration and change values in constant speed.
    *   @}
    *//**
    *   @defgroup quadratic Quadratic
    *   @{
    *       @brief The most commonly used easing functions.
    *   @}
    *//**
    *   @defgroup cubic Cubic
    *   @{
    *       @brief A bit curvier than the quadratic easing.
    *   @}
    *//**
    *   @defgroup quartic Quartic
    *   @{
    *       @brief A steeper curve. Acceleration changes faster than Cubic.
    *   @}
    *//**
    *   @defgroup quintic Quintic
    *   @{
    *       @brief An even steeper curve. Acceleration changes really fast.
    *   @}
    *//**
    *   @defgroup sinuisodal Sinuisodal
    *   @{
    *       @brief A very gentle curve, gentlier than quadratic.
    *   @}
    *//**
    *   @defgroup exponential Exponential
    *   @{
    *       @brief A very steep curve, based on the `p(t) = 2^(10*(t-1))` equation.
    *   @}
    *//**
    *   @defgroup circular Circular
    *   @{
    *       @brief A smooth, circular slope that resembles the arc of an circle.
    *   @}
    *//**
    *   @defgroup back Back
    *   @{
    *       @brief An easing function that has a "cute" natural coming back effect.
    *   @}
    *//**
    *   @defgroup elastic Elastic
    *   @{
    *       @brief An elastic easing function. Values go a little past the maximum/minimum in an elastic effect.
    *   @}
    *//**
    *   @defgroup bounce Bounce
    *   @{
    *       @brief A bouncing easing function. Values "bounce" around the maximum/minumum.
    *   @}
    *//**
    * @}
    */

namespace tweeny {
    /**
     * @brief The easing class holds all the bundled easings.
     *
     * You should pass the easing function to the @p tweeny::tween::via method, to set the easing function that will
     * be used to interpolate values in a tween point.
     *
     * **Example**:
     *
     * @code
     * auto tween = tweeny::from(0).to(100).via(tweeny::easing::linear);
     * @endcode
     */
    class easing {
        public:
            /**
             * @brief Enumerates all easings to aid in runtime when adding easins to a tween using tween::via
             *
             * The aim of this enum is to help in situations where the easing doesn't come straight from the C++
             * code but rather from a configuration file or some sort of external paramenter.
             */
            enum class enumerated {
                def,
                linear,
                stepped,
                quadraticIn,
                quadraticOut,
                quadraticInOut,
                cubicIn,
                cubicOut,
                cubicInOut,
                quarticIn,
                quarticOut,
                quarticInOut,
                quinticIn,
                quinticOut,
                quinticInOut,
                sinusoidalIn,
                sinusoidalOut,
                sinusoidalInOut,
                exponentialIn,
                exponentialOut,
                exponentialInOut,
                circularIn,
                circularOut,
                circularInOut,
                bounceIn,
                bounceOut,
                bounceInOut,
                elasticIn,
                elasticOut,
                elasticInOut,
                backIn,
                backOut,
                backInOut
            };

            /**
             * @ingroup stepped
             * @brief Value is constant.
             */
            static constexpr struct steppedEasing {
                template<typename T>
                static T run(float position, T start, T end) {
                    return start;
                }
            } stepped = steppedEasing{};
      
            /**
             * @ingroup default
             * @brief Values change with constant speed for arithmetic type only. The non-arithmetic it will be constant.
             */
            static constexpr struct defaultEasing {
                template<class...> struct voidify { using type = void; };
                template<class... Ts> using void_t = typename voidify<Ts...>::type;

                template<class T, class = void>
                struct supports_arithmetic_operations : std::false_type {};

                template<class T>
                struct supports_arithmetic_operations<T, void_t<
                    decltype(std::declval<T>() + std::declval<T>()),
                    decltype(std::declval<T>() - std::declval<T>()),
                    decltype(std::declval<T>() * std::declval<T>()),
                    decltype(std::declval<T>() * std::declval<float>()),
                    decltype(std::declval<float>() * std::declval<T>())
                    >> : std::true_type{};

                template<typename T>
                static typename std::enable_if<std::is_integral<T>::value, T>::type run(float position, T start, T end) {
                    return static_cast<T>(roundf((end - start) * position + start));
                }

                template<typename T>
                static typename std::enable_if<supports_arithmetic_operations<T>::value && !std::is_integral<T>::value, T>::type run(float position, T start, T end) {
                    return static_cast<T>((end - start) * position + start);
                }

                template<typename T>
                static typename std::enable_if<!supports_arithmetic_operations<T>::value, T>::type run(float position, T start, T end) {
                    return start;
                }
            } def = defaultEasing{};

            /**
             * @ingroup linear
             * @brief Values change with constant speed.
             */
            static constexpr struct linearEasing {
                template<typename T>
                static typename std::enable_if<std::is_integral<T>::value, T>::type run(float position, T start, T end) {
                    return static_cast<T>(roundf((end - start) * position + start));
                }

                template<typename T>
                static typename std::enable_if<!std::is_integral<T>::value, T>::type run(float position, T start, T end) {
                    return static_cast<T>((end - start) * position + start);
                }
            } linear = linearEasing{};

             /**
              * @ingroup quadratic
              * @brief Accelerate initial values with a quadratic equation.
              */
            static constexpr struct quadraticInEasing {
                template<typename T>
                static T run(float position, T start, T end) {
                    return static_cast<T>((end - start) * position * position + start);
                }
            } quadraticIn = quadraticInEasing{};

            /**
              * @ingroup quadratic
              * @brief Deaccelerate ending values with a quadratic equation.
              */
            static constexpr struct quadraticOutEasing {
                template<typename T>
                static T run(float position, T start, T end) {
                    return static_cast<T>((-(end - start)) * position * (position - 2) + start);
                }
            } quadraticOut = quadraticOutEasing{};

            /**
              * @ingroup quadratic
              * @brief Acceelerate initial and deaccelerate ending values with a quadratic equation.
              */
            static constexpr struct quadraticInOutEasing {
                template<typename T>
                static T run(float position, T start, T end) {
                    position *= 2;
                    if (position < 1) {
                        return static_cast<T>(((end - start) / 2) * position * position + start);
                    }

                    --position;
                    return static_cast<T>((-(end - start) / 2) * (position * (position - 2) - 1) + start);
                }
            } quadraticInOut = quadraticInOutEasing{};

            /**
              * @ingroup cubic
              * @brief Aaccelerate initial values with a cubic equation.
              */
            static constexpr struct cubicInEasing {
                template<typename T>
                static T run(float position, T start, T end) {
                    return static_cast<T>((end - start) * position * position * position + start);
                }
            } cubicIn = cubicInEasing{};

            /**
              * @ingroup cubic
              * @brief Deaccelerate ending values with a cubic equation.
              */
            static constexpr struct cubicOutEasing {
                template<typename T>
                static T run(float position, T start, T end) {
                    --position;
                    return static_cast<T>((end - start) * (position * position * position + 1) + start);
                }
            } cubicOut = cubicOutEasing{};

            /**
              * @ingroup cubic
              * @brief Acceelerate initial and deaccelerate ending values with a cubic equation.
              */
            static constexpr struct cubicInOutEasing {
                template<typename T>
                static T run(float position, T start, T end) {
                    position *= 2;
                    if (position < 1) {
                        return static_cast<T>(((end - start) / 2) * position * position * position + start);
                    }
                    position -= 2;
                    return static_cast<T>(((end - start) / 2) * (position * position * position + 2) + start);
                }
            } cubicInOut = cubicInOutEasing{};

            /**
              * @ingroup quartic
              * @brief Acceelerate initial values with a quartic equation.
              */
            static constexpr struct quarticInEasing {
                template<typename T>
                static T run(float position, T start, T end) {
                    return static_cast<T>((end - start) * position * position * position * position + start);
                }
            } quarticIn = quarticInEasing{};

            /**
              * @ingroup quartic
              * @brief Deaccelerate ending values with a quartic equation.
              */
            static constexpr struct quarticOutEasing {
                template<typename T>
                static T run(float position, T start, T end) {
                    --position;
                    return static_cast<T>( -(end - start) * (position * position * position * position - 1) + start);
                }
            } quarticOut = quarticOutEasing{};

            /**
              * @ingroup quartic
              * @brief Acceelerate initial and deaccelerate ending values with a quartic equation.
              */
            static constexpr struct quarticInOutEasing {
                template<typename T>
                static T run(float position, T start, T end) {
                    position *= 2;
                    if (position < 1) {
                        return static_cast<T>(((end - start) / 2) * (position * position * position * position) +
                                              start);
                    }
                    position -= 2;
                    return static_cast<T>((-(end - start) / 2) * (position * position * position * position - 2) +
                                          start);
                }
            } quarticInOut = quarticInOutEasing{};

            /**
              * @ingroup quintic
              * @brief Acceelerate initial values with a quintic equation.
              */
            static constexpr struct quinticInEasing {
                template<typename T>
                static T run(float position, T start, T end) {
                    return static_cast<T>((end - start) * position * position * position * position * position + start);
                }
            } quinticIn = quinticInEasing{};

            /**
              * @ingroup quintic
              * @brief Deaccelerate ending values with a quintic equation.
              */
            static constexpr struct quinticOutEasing {
                template<typename T>
                static T run(float position, T start, T end) {
                    position--;
                    return static_cast<T>((end - start) * (position * position * position * position * position + 1) +
                                          start);
                }
            } quinticOut = quinticOutEasing{};

            /**
              * @ingroup quintic
              * @brief Acceelerate initial and deaccelerate ending values with a quintic equation.
              */
            static constexpr struct quinticInOutEasing {
                template<typename T>
                static T run(float position, T start, T end) {
                    position *= 2;
                    if (position < 1) {
                        return static_cast<T>(
                            ((end - start) / 2) * (position * position * position * position * position) +
                            start);
                    }
                    position -= 2;
                    return static_cast<T>(
                        ((end - start) / 2) * (position * position * position * position * position + 2) +
                        start);
                }
            } quinticInOut = quinticInOutEasing{};

            /**
              * @ingroup sinusoidal
              * @brief Acceelerate initial values with a sinusoidal equation.
              */
            static constexpr struct sinusoidalInEasing {
                template<typename T>
                static T run(float position, T start, T end) {
                    return static_cast<T>(-(end - start) * cosf(position * static_cast<float>(M_PI) / 2) + (end - start) + start);
                }
            } sinusoidalIn = sinusoidalInEasing{};

            /**
              * @ingroup sinusoidal
              * @brief Deaccelerate ending values with a sinusoidal equation.
              */
            static constexpr struct sinusoidalOutEasing {
                template<typename T>
                static T run(float position, T start, T end) {
                    return static_cast<T>((end - start) * sinf(position * static_cast<float>(M_PI) / 2) + start);
                }
            } sinusoidalOut = sinusoidalOutEasing{};

            /**
              * @ingroup sinusoidal
              * @brief Acceelerate initial and deaccelerate ending values with a sinusoidal equation.
              */
            static constexpr struct sinusoidalInOutEasing {
                template<typename T>
                static T run(float position, T start, T end) {
                    return static_cast<T>((-(end - start) / 2) * (cosf(position * static_cast<float>(M_PI)) - 1) + start);
                }
            } sinusoidalInOut = sinusoidalInOutEasing{};

            /**
              * @ingroup exponential
              * @brief Acceelerate initial values with an exponential equation.
              */
            static constexpr struct exponentialInEasing {
                template<typename T>
                static T run(float position, T start, T end) {
                    return static_cast<T>((end - start) * powf(2, 10 * (position - 1)) + start);
                }
            } exponentialIn = exponentialInEasing{};

            /**
              * @ingroup exponential
              * @brief Deaccelerate ending values with an exponential equation.
              */
            static constexpr struct exponentialOutEasing {
                template<typename T>
                static T run(float position, T start, T end) {
                    return static_cast<T>((end - start) * (-powf(2, -10 * position) + 1) + start);
                }
            } exponentialOut = exponentialOutEasing{};

            /**
              * @ingroup exponential
              * @brief Acceelerate initial and deaccelerate ending values with an exponential equation.
              */
            static constexpr struct exponentialInOutEasing {
                template<typename T>
                static T run(float position, T start, T end) {
                    position *= 2;
                    if (position < 1) {
                        return static_cast<T>(((end - start) / 2) * powf(2, 10 * (position - 1)) + start);
                    }
                    --position;
                    return static_cast<T>(((end - start) / 2) * (-powf(2, -10 * position) + 2) + start);
                }
            } exponentialInOut = exponentialInOutEasing{};

            /**
              * @ingroup circular
              * @brief Acceelerate initial values with a circular equation.
              */
            static constexpr struct circularInEasing {
                template<typename T>
                static T run(float position, T start, T end) {
                    return static_cast<T>( -(end - start) * (sqrtf(1 - position * position) - 1) + start );
                }
            } circularIn = circularInEasing{};

            /**
              * @ingroup circular
              * @brief Deaccelerate ending values with a circular equation.
              */
            static constexpr struct circularOutEasing {
                template<typename T>
                static T run(float position, T start, T end) {
                    --position;
                    return static_cast<T>((end - start) * (sqrtf(1 - position * position)) + start);
                }
            } circularOut = circularOutEasing{};

            /**
              * @ingroup circular
              * @brief Acceelerate initial and deaccelerate ending values with a circular equation.
              */
            static constexpr struct circularInOutEasing {
                template<typename T>
                static T run(float position, T start, T end) {
                    position *= 2;
                    if (position < 1) {
                        return static_cast<T>((-(end - start) / 2) * (sqrtf(1 - position * position) - 1) + start);
                    }

                    position -= 2;
                    return static_cast<T>(((end - start) / 2) * (sqrtf(1 - position * position) + 1) + start);
                }
            } circularInOut = circularInOutEasing{};

            /**
              * @ingroup bounce
              * @brief Acceelerate initial values with a "bounce" equation.
              */
            static constexpr struct bounceInEasing {
                template<typename T>
                static T run(float position, T start, T end) {
                    return (end - start) - bounceOut.run((1 - position), T(), end) + start;
                }
            } bounceIn = bounceInEasing{};

            /**
              * @ingroup bounce
              * @brief Deaccelerate ending values with a "bounce" equation.
              */
            static constexpr struct bounceOutEasing {
                template<typename T>
                static T run(float position, T start, T end) {
                    T c = end - start;
                    if (position < (1 / 2.75f)) {
                        return static_cast<T>(c * (7.5625f * position * position) + start);
                    } else if (position < (2.0f / 2.75f)) {
                        float postFix = position -= (1.5f / 2.75f);
                        return static_cast<T>(c * (7.5625f * (postFix) * position + .75f) + start);
                    } else if (position < (2.5f / 2.75f)) {
                        float postFix = position -= (2.25f / 2.75f);
                        return static_cast<T>(c * (7.5625f * (postFix) * position + .9375f) + start);
                    } else {
                        float postFix = position -= (2.625f / 2.75f);
                        return static_cast<T>(c * (7.5625f * (postFix) * position + .984375f) + start);
                    }
                }
            } bounceOut = bounceOutEasing{};

            /**
            * @ingroup bounce
            * @brief Acceelerate initial and deaccelerate ending values with a "bounce" equation.
            */
            static constexpr struct bounceInOutEasing {
                template<typename T>
                static T run(float position, T start, T end) {
                    if (position < 0.5f) return static_cast<T>(bounceIn.run(position * 2, T(), end) * .5f + start);
                    else return static_cast<T>(bounceOut.run((position * 2 - 1), T(), end) * .5f + (end - start) * .5f + start);
                }
            } bounceInOut = bounceInOutEasing{};

            /**
              * @ingroup elastic
              * @brief Acceelerate initial values with an "elastic" equation.
              */
            static constexpr struct elasticInEasing {
                template<typename T>
                static T run(float position, T start, T end) {
                    if (position <= 0.00001f) return start;
                    if (position >= 0.999f) return end;
                    float p = .3f;
                    auto a = end - start;
                    float s = p / 4;
                    float postFix =
                        a * powf(2, 10 * (position -= 1)); // this is a fix, again, with post-increment operators
                    return static_cast<T>(-(postFix * sinf((position - s) * (2 * static_cast<float>(M_PI)) / p)) + start);
                }
            } elasticIn = elasticInEasing{};

            /**
              * @ingroup elastic
              * @brief Deaccelerate ending values with an "elastic" equation.
              */
            static constexpr struct elasticOutEasing {
                template<typename T>
                static T run(float position, T start, T end) {
                    if (position <= 0.00001f) return start;
                    if (position >= 0.999f) return end;
                    float p = .3f;
                    auto a = end - start;
                    float s = p / 4;
                    return static_cast<T>(a * powf(2, -10 * position) * sinf((position - s) * (2 * static_cast<float>(M_PI)) / p) + end);
                }
            } elasticOut = elasticOutEasing{};

            /**
            * @ingroup elastic
            * @brief Acceelerate initial and deaccelerate ending values with an "elastic" equation.
            */
            static constexpr struct elasticInOutEasing {
                template<typename T>
                static T run(float position, T start, T end) {
                    if (position <= 0.00001f) return start;
                    if (position >= 0.999f) return end;
                    position *= 2;
                    float p = (.3f * 1.5f);
                    auto a = end - start;
                    float s = p / 4;
                    float postFix;

                    if (position < 1) {
                        postFix = a * powf(2, 10 * (position -= 1)); // postIncrement is evil
                        return static_cast<T>(-0.5f * (postFix * sinf((position - s) * (2 * static_cast<float>(M_PI)) / p)) + start);
                    }
                    postFix = a * powf(2, -10 * (position -= 1)); // postIncrement is evil
                    return static_cast<T>(postFix * sinf((position - s) * (2 * static_cast<float>(M_PI)) / p) * .5f + end);
                }
            } elasticInOut = elasticInOutEasing{};

            /**
              * @ingroup back
              * @brief Acceelerate initial values with a "back" equation.
              */
            static constexpr struct backInEasing {
                template<typename T>
                static T run(float position, T start, T end) {
                    float s = 1.70158f;
                    float postFix = position;
                    return static_cast<T>((end - start) * (postFix) * position * ((s + 1) * position - s) + start);
                }
            } backIn = backInEasing{};

            /**
              * @ingroup back
              * @brief Deaccelerate ending values with a "back" equation.
              */
            static constexpr struct backOutEasing {
                template<typename T>
                static T run(float position, T start, T end) {
                    float s = 1.70158f;
                    position -= 1;
                    return static_cast<T>((end - start) * ((position) * position * ((s + 1) * position + s) + 1) + start);
                }
            } backOut = backOutEasing{};

            /**
            * @ingroup back
            * @brief Acceelerate initial and deaccelerate ending values with a "back" equation.
            */
            static constexpr struct backInOutEasing {
                template<typename T>
                static T run(float position, T start, T end) {
                    float s = 1.70158f;
                    float t = position;
                    auto b = start;
                    auto c = end - start;
                    float d = 1;
                    s *= (1.525f);
                    if ((t /= d / 2) < 1) return static_cast<T>(c / 2 * (t * t * (((s) + 1) * t - s)) + b);
                    float postFix = t -= 2;
                    return static_cast<T>(c / 2 * ((postFix) * t * (((s) + 1) * t + s) + 2) + b);
                }
            } backInOut = backInOutEasing{};
    };
}
#endif //TWEENY_EASING_H

/*
 This file is part of the Tweeny library.

 Copyright (c) 2016-2021 Leonardo Guilherme Lucena de Freitas
 Copyright (c) 2016 Guilherme R. Costa

 Permission is hereby granted, free of charge, to any person obtaining a copy of
 this software and associated documentation files (the "Software"), to deal in
 the Software without restriction, including without limitation the rights to
 use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of
 the Software, and to permit persons to whom the Software is furnished to do so,
 subject to the following conditions:

 The above copyright notice and this permission notice shall be included in all
 copies or substantial portions of the Software.

 THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS
 FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR
 COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER
 IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
 CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
*/

/*
 * This file provides the easing resolution mechanism so that the library user can mix lambdas and the bundled
 * pre-defined easing functions. It shall not be used directly.
 * This file is private.
 */

#ifndef TWEENY_EASINGRESOLVE_H
#define TWEENY_EASINGRESOLVE_H

#include <tuple>

namespace tweeny {
    namespace detail {
        using std::get;

        template<int I, typename TypeTuple, typename FunctionTuple, typename... Fs>
        struct easingresolve {
            static void impl(FunctionTuple &b, Fs... fs) {
                if (sizeof...(Fs) == 0) return;
                easingresolve<I, TypeTuple, FunctionTuple, Fs...>::impl(b, fs...);
            }
        };

        template<int I, typename TypeTuple, typename FunctionTuple, typename F1, typename... Fs>
        struct easingresolve<I, TypeTuple, FunctionTuple, F1, Fs...> {
            static void impl(FunctionTuple &b, F1 f1, Fs... fs) {
                get<I>(b) = f1;
                easingresolve<I + 1, TypeTuple, FunctionTuple, Fs...>::impl(b, fs...);
            }
        };

        template<int I, typename TypeTuple, typename FunctionTuple, typename... Fs>
        struct easingresolve<I, TypeTuple, FunctionTuple, easing::steppedEasing, Fs...> {
            typedef typename std::tuple_element<I, TypeTuple>::type ArgType;

            static void impl(FunctionTuple &b, easing::steppedEasing, Fs... fs) {
                get<I>(b) = easing::stepped.run<ArgType>;
                easingresolve<I + 1, TypeTuple, FunctionTuple, Fs...>::impl(b, fs...);
            }
        };

        template<int I, typename TypeTuple, typename FunctionTuple, typename... Fs>
        struct easingresolve<I, TypeTuple, FunctionTuple, easing::linearEasing, Fs...> {
            typedef typename std::tuple_element<I, TypeTuple>::type ArgType;

            static void impl(FunctionTuple &b, easing::linearEasing, Fs... fs) {
                get<I>(b) = easing::linear.run<ArgType>;
                easingresolve<I + 1, TypeTuple, FunctionTuple, Fs...>::impl(b, fs...);
            }
        };
        template<int I, typename TypeTuple, typename FunctionTuple, typename... Fs>
        struct easingresolve<I, TypeTuple, FunctionTuple, easing::defaultEasing, Fs...> {
            typedef typename std::tuple_element<I, TypeTuple>::type ArgType;

            static void impl(FunctionTuple &b, easing::defaultEasing, Fs... fs) {
                get<I>(b) = easing::def.run<ArgType>;
                easingresolve<I + 1, TypeTuple, FunctionTuple, Fs...>::impl(b, fs...);
            }
        };

        #define DECLARE_EASING_RESOLVE(__EASING_TYPE__) \
        template <int I, typename TypeTuple, typename FunctionTuple, typename... Fs> \
        struct easingresolve<I, TypeTuple, FunctionTuple, easing::__EASING_TYPE__ ## InEasing, Fs...> { \
          typedef typename std::tuple_element<I, TypeTuple>::type ArgType; \
          static void impl(FunctionTuple & b, decltype(easing::__EASING_TYPE__ ## In), Fs... fs) { \
            get<I>(b) = easing::__EASING_TYPE__ ## In.run<ArgType>; \
            easingresolve<I+1, TypeTuple, FunctionTuple, Fs...>::impl(b, fs...); \
          } \
        }; \
        \
        template <int I, typename TypeTuple, typename FunctionTuple, typename... Fs> \
          struct easingresolve<I, TypeTuple, FunctionTuple, easing::__EASING_TYPE__ ## OutEasing, Fs...> { \
          typedef typename std::tuple_element<I, TypeTuple>::type ArgType; \
          static void impl(FunctionTuple & b, decltype(easing::__EASING_TYPE__ ## Out), Fs... fs) { \
            get<I>(b) = easing::__EASING_TYPE__ ## Out.run<ArgType>; \
            easingresolve<I+1, TypeTuple, FunctionTuple, Fs...>::impl(b, fs...); \
          } \
        }; \
        \
        template <int I, typename TypeTuple, typename FunctionTuple, typename... Fs> \
          struct easingresolve<I, TypeTuple, FunctionTuple, easing::__EASING_TYPE__ ## InOutEasing, Fs...> { \
          typedef typename std::tuple_element<I, TypeTuple>::type ArgType; \
          static void impl(FunctionTuple & b, decltype(easing::__EASING_TYPE__ ## InOut), Fs... fs) { \
            get<I>(b) = easing::__EASING_TYPE__ ## InOut.run<ArgType>; \
            easingresolve<I+1, TypeTuple, FunctionTuple, Fs...>::impl(b, fs...); \
          } \
        }

        DECLARE_EASING_RESOLVE(quadratic);
        DECLARE_EASING_RESOLVE(cubic);
        DECLARE_EASING_RESOLVE(quartic);
        DECLARE_EASING_RESOLVE(quintic);
        DECLARE_EASING_RESOLVE(sinusoidal);
        DECLARE_EASING_RESOLVE(exponential);
        DECLARE_EASING_RESOLVE(circular);
        DECLARE_EASING_RESOLVE(bounce);
        DECLARE_EASING_RESOLVE(elastic);
        DECLARE_EASING_RESOLVE(back);
    }
}

#endif //TWEENY_EASINGRESOLVE_H

/*
 This file is part of the Tweeny library.

 Copyright (c) 2016-2021 Leonardo Guilherme Lucena de Freitas
 Copyright (c) 2016 Guilherme R. Costa

 Permission is hereby granted, free of charge, to any person obtaining a copy of
 this software and associated documentation files (the "Software"), to deal in
 the Software without restriction, including without limitation the rights to
 use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of
 the Software, and to permit persons to whom the Software is furnished to do so,
 subject to the following conditions:

 The above copyright notice and this permission notice shall be included in all
 copies or substantial portions of the Software.

 THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS
 FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR
 COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER
 IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
 CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
*/

/*
 * This file declares a helper struct to create a type from a integer value, to aid in template tricks.
 * This file is private.
 */
#ifndef TWEENY_INT2TYPE_H
#define TWEENY_INT2TYPE_H

namespace tweeny {
    namespace detail {
        template<std::size_t> struct int2type { };
    }
}
#endif //TWEENY_INT2TYPE_H

namespace tweeny {
    namespace detail {
        template<typename TypeTupleT, typename EasingCollectionT, typename EasingT, size_t I> void easingfill(EasingCollectionT & f, EasingT easing, int2type<I>) {
            easingresolve<I, TypeTupleT, EasingCollectionT, EasingT>::impl(f, easing);
            easingfill<TypeTupleT, EasingCollectionT, EasingT>(f, easing, int2type<I - 1>{ });
        }

        template<typename TypeTupleT, typename EasingCollectionT, typename EasingT> void easingfill(EasingCollectionT & f, EasingT easing, int2type<0>) {
            easingresolve<0, TypeTupleT, EasingCollectionT, EasingT>::impl(f, easing);
        }

        template <class ...T>
        struct are_same;

        template <class A, class B, class ...T>
        struct are_same<A, B, T...>
        {
            static const bool value = std::is_same<A, B>::value && are_same<B, T...>::value;
        };

        template <class A>
        struct are_same<A>
        {
            static const bool value = true;
        };

        template<typename... Ts>
        inline tweenpoint<Ts...>::tweenpoint(Ts... vs) : values{vs...} {
            during(static_cast<uint16_t>(0));
            via(easing::def);
        }

        template<typename... Ts>
        template<typename D>
        inline void tweenpoint<Ts...>::during(D milis) {
            for (uint16_t & t : durations) { t = static_cast<uint16_t>(milis); }
        }

        template<typename... Ts>
        template<typename... Ds>
        inline void tweenpoint<Ts...>::during(Ds... milis) {
            static_assert(sizeof...(Ds) == sizeof...(Ts),
                          "Amount of durations should be equal to the amount of values in a point");
            std::array<int, sizeof...(Ts)> list = {{ milis... }};
            std::copy(list.begin(), list.end(), durations.begin());
        }

        template<typename... Ts>
        template<typename... Fs>
        inline void tweenpoint<Ts...>::via(Fs... fs) {
            static_assert(sizeof...(Fs) == sizeof...(Ts),
                          "Number of functions passed to via() must be equal the number of values.");
            detail::easingresolve<0, std::tuple<Ts...>, typename traits::easingCollection, Fs...>::impl(easings, fs...);
        }

        template<typename... Ts>
        template<typename F>
        inline void tweenpoint<Ts...>::via(F f) {
            easingfill<typename traits::valuesType>(easings, f, int2type<sizeof...(Ts) - 1>{ });
        }

        template<typename... Ts>
        inline uint16_t tweenpoint<Ts...>::duration() const {
            return *std::max_element(durations.begin(), durations.end());
        }

        template<typename... Ts>
        inline uint16_t tweenpoint<Ts...>::duration(size_t i) const {
            return durations.at(i);
        }
    }
}
#endif //TWEENY_TWEENPOINT_TCC

#endif //TWEENY_TWEENPOINT_H

namespace tweeny {
    /**
     * @brief The tween class is the core class of tweeny. It controls the interpolation steps, easings and durations.
     *
     * It should not be constructed manually but rather from @p tweeny::from, to facilitate template argument
     * deduction (and also to keep your code clean).
     */
    template<typename T, typename... Ts>
    class tween {
        public:
            /**
             * @brief Instantiates a tween from a starting point.
             *
             * This is a static factory helper function to be used by @p tweeny::from. You should not use this directly.
             * @p t The first value in the point
             * @p vs The remaining values
             */
            static tween<T, Ts...> from(T t, Ts... vs);

        public:
            /**
             * @brief Default constructor for a tween
             *
             * This constructor is provided to facilitate the usage of containers of tweens (e.g, std::vector). It
             * should not be used manually as the tweening created by it is invalid.
             */
            tween();

            /**
             * @brief Adds a new point in this tweening.
             *
             * This will add a new tweening point with the specified values. Next calls to @p via and @p during
             * will refer to this point.
             *
             * **Example**
             *
             * @code
             * auto t = tweeny::from(0).to(100).to(200);
             * @endcode
             *
             * @param t, vs Point values
             * @returns *this
             */
            tween<T, Ts...> & to(T t, Ts... vs);

            /**
             * @brief Specifies the easing function for the last added point.
             *
             * This will specify the easing between the last tween point added by @p to and its previous step. You can
             * use any callable object. Additionally, you can use the easing objects specified in the class @p easing.
             *
             * If it is a multi-value point, you can either specify a single easing function that will be used for
             * every value or you can specify an easing function for each value. You can mix and match callable objects,
             * lambdas and bundled easing objects.
             *
             * **Example**:
             *
             * @code
             * // use bundled linear easing
             * auto tween1 = tweeny::from(0).to(100).via(tweeny::easing::linear);
             *
             * // use custom lambda easing
             * auto tween2 = tweeny::from(0).to(100).via([](float p, int a, int b) { return (b-a) * p + a; });
             * @endcode
             *
             * @param fs The functions
             * @returns *this
             * @see tweeny::easing
             */
            template<typename... Fs> tween<T, Ts...> & via(Fs... fs);

            /**
             * @brief Specifies the easing function for the last added point, accepting an enumeration.
             *
             * This will specify the easing between the last tween point added by @p to and its previous step. You can
             * use a value from the @p tweeny::easing::enumerated enum. You can then have an enumeration of your own
             * poiting to this enumerated enums, or use it directly. You can mix-and-match enumerated easings, functions
             * and easing names.
             *
             * **Example**:
             *
             * @code
             * auto tween1 = tweeny::from(0).to(100).via(tweeny::easing::enumerated::linear);
             * auto tween2 = tweeny::from(0.0f, 100.0f).to(100.0f, 0.0f).via(tweeny::easing::linear, "backOut");
             *
             * @param fs The functions
             * @returns *this
             * @see tweeny::easing
             */
            template<typename... Fs> tween<T, Ts...> & via(easing::enumerated enumerated, Fs... fs);

            /**
             * @brief Specifies the easing function for the last added point, accepting an easing name as a `std::string` value.
             *
             * This will specify the easing between the last tween point added by @p to and its previous step.
             * You can mix-and-match enumerated easings, functions and easing names.
             *
             * **Example**:
             *
             * @code
             * auto tween = tweeny::from(0.0f, 100.0f).to(100.0f, 0.0f).via(tweeny::easing::linear, "backOut");
             *
             * @param fs The functions
             * @returns *this
             * @see tweeny::easing
             */
            template<typename... Fs> tween<T, Ts...> & via(const std::string & easing, Fs... fs);

        /**
            * @brief Specifies the easing function for the last added point, accepting an easing name as a `const char *` value.
            *
            * This will specify the easing between the last tween point added by @p to and its previous step.
            * You can mix-and-match enumerated easings, functions and easing names.
            *
            * **Example**:
            *
            * @code
            * auto tween = tweeny::from(0.0f, 100.0f).to(100.0f, 0.0f).via(tweeny::easing::linear, "backOut");
            *
            * @param fs The functions
            * @returns *this
            * @see tweeny::easing
            */
            template<typename... Fs> tween<T, Ts...> & via(const char * easing, Fs... fs);

            /**
             * @brief Specifies the easing function for a specific point.
             *
             * Points starts at index 0. The index 0 refers to the first @p to call.
             * Using this function without adding a point with @p to leads to undefined
             * behaviour.
             *
             * @param index The tween point index
             * @param fs The functions
             * @returns *this
             * @see tweeny::easing
             */
            template<typename... Fs> tween<T, Ts...> & via(int index, Fs... fs);

            /**
             * @brief Specifies the duration, typically in milliseconds, for the tweening of values in last point.
             *
             * You can either specify a single duration for all values or give every value its own duration. Value types
             * must be convertible to the uint16_t type.
             *
             * **Example**:
             *
             * @code
             * // Specify that the first point will be reached in 100 milliseconds and the first value in the second
             * // point in 100, whereas the second value will be reached in 500.
             * auto tween = tweeny::from(0, 0).to(100, 200).during(100).to(200, 300).during(100, 500);
             * @endcode
             *
             * @param ds Duration values
             * @returns *this
             */
            template<typename... Ds> tween<T, Ts...> & during(Ds... ds);

            /**
             * @brief Steps the animation by the designated delta amount.
             *
             * You should call this every frame of your application, passing in the amount of delta time that
             * you want to animate.
             *
             * **Example**:
             *
             * @code
             * // tween duration is 100ms
             * auto tween = tweeny::from(0).to(100).during(100);
             *
             * // steps for 16ms
             * tween.step(16);
             * @endcode
             *
             * @param dt Delta duration
             * @param suppressCallbacks (Optional) Suppress callbacks registered with tween::onStep()
             * @returns std::tuple<Ts...> with the current tween values.
             */
            const typename detail::tweentraits<T, Ts...>::valuesType & step(int32_t dt, bool suppressCallbacks = false);

            /**
             * @brief Steps the animation by the designated delta amount.
             *
             * You should call this every frame of your application, passing in the amount of delta time that
             * you want to animate. This overload exists to match unsigned int arguments.
             *
             * @param dt Delta duration
             * @param suppressCallbacks (Optional) Suppress callbacks registered with tween::onStep()
             * @returns std::tuple<Ts...> with the current tween values.
             */
            const typename detail::tweentraits<T, Ts...>::valuesType & step(uint32_t dt, bool suppressCallbacks = false);

            /**
             * @brief Steps the animation by the designated percentage amount.
             *
             * You can use this function to step the tweening by a specified percentage delta.

             * **Example**:
             *
             * @code
             * // tween duration is 100ms
             * auto tween = tweeny::from(0).to(100).during(100);
             *
             * // steps for 16ms
             * tween.step(0.001f);
             * @endcode
             *
             * @param dp Delta percentage, between `0.0f` and `1.0f`
             * @param suppressCallbacks (Optional) Suppress callbacks registered with tween::onStep()
             * @returns std::tuple<Ts...> with the current tween values.
             */
            const typename detail::tweentraits<T, Ts...>::valuesType & step(float dp, bool suppressCallbacks = false);

            /**
             * @brief Seeks to a specified point in time based on the currentProgress.
             *
             * This function sets the current animation time and currentProgress. Callbacks set by @p call will be triggered.
             *
             * @param p The percentage to seek to, between 0.0f and 1.0f, inclusive.
             * @param suppressCallbacks (Optional) Suppress callbacks registered with tween::onSeek()
             * @returns std::tuple<Ts...> with the current tween values.
             */
            const typename detail::tweentraits<T, Ts...>::valuesType & seek(float p, bool suppressCallbacks = false);

            /**
             * @brief Seeks to a specified point in time.
             *
             * This function sets the current animation time and currentProgress. Callbacks set by @p call will be triggered.
             *
             * @param d The duration to seek to, between 0 and the total duration.
             * @param suppressCallbacks (Optional) Suppress callbacks registered with tween::onSeek()
             * @returns std::tuple<Ts...> with the current tween values.
             * @see duration
             */
            const typename detail::tweentraits<T, Ts...>::valuesType & seek(int32_t d, bool suppressCallbacks = false);

            /**
             * @brief Seeks to a specified point in time.
             *
             * This function sets the current animation time and currentProgress. Callbacks set by @p call will be triggered.
             *
             * @param d The duration to seek to, between 0 and the total duration.
             * @param suppressCallbacks (Optional) Suppress callbacks registered with tween::onSeek()
             * @returns std::tuple<Ts...> with the current tween values.
             * @see duration
             */
            const typename detail::tweentraits<T, Ts...>::valuesType & seek(uint32_t d, bool suppressCallbacks = false);

            /**
             * @brief Adds a callback that will be called when stepping occurs, accepting both the tween and
             * its values.
             *
             * You can add as many callbacks as you want. Its arguments types must be equal to the argument types
             * of a tween instance, preceded by a variable of the tween type. Callbacks can be of any callable type. It will only be called
             * via tween::step() functions. For seek callbacks, see tween::onSeek().
             *
             * Keep in mind that the function will be *copied* into an array, so any variable captured by value
             * will also be copied with it.
             *
             * If the callback returns false, it will be called next time. If it returns true, it will be removed from
             * the callback queue.
             *
             * **Example**:
             *
             * @code
             * auto t = tweeny:from(0).to(100).during(100);
             *
             * // pass a lambda
             * t.onStep([](tweeny::tween<int> & t, int v) { printf("%d ", v); return false; });
             *
             * // pass a functor instance
             * struct ftor { void operator()(tweeny::tween<int> & t, int v) { printf("%d ", v); return false; } };
             * t.onStep(ftor());
             * @endcode
             * @sa step
             * @sa seek
             * @sa onSeek
             * @param callback A callback in with the prototype `bool callback(tween<Ts...> & t, Ts...)`
             */
            tween<T, Ts...> & onStep(typename detail::tweentraits<T, Ts...>::callbackType callback);

            /**
             * @brief Adds a callback that will be called when stepping occurs, accepting only the tween.
             *
             * You can add as many callbacks as you want. It must receive the tween as an argument.
             * Callbacks can be of any callable type. It will only be called
             * via tween::step() functions. For seek callbacks, see tween::onSeek().
             *
             * Keep in mind that the function will be *copied* into an array, so any variable captured by value
             * will also be copied with it.
             *
             * If the callback returns false, it will be called next time. If it returns true, it will be removed from
             * the callback queue.
             *
             * **Example**:
             *
             * @code
             * auto t = tweeny:from(0).to(100).during(100);
             *
             * // pass a lambda
             * t.onStep([](tweeny::tween<int> & t) { printf("%d ", t.value()); return false; });
             *
             * // pass a functor instance
             * struct ftor { void operator()(tweeny::tween<int> & t) { printf("%d ", t.values()); return false; } };
             * t.onStep(ftor());
             * @endcode
             * @sa step
             * @sa seek
             * @sa onSeek
             * @param callback A callback in the form `bool f(tween<Ts...> & t)`
             */
            tween<T, Ts...> & onStep(typename detail::tweentraits<T, Ts...>::noValuesCallbackType callback);

            /**
             * @brief Adds a callback that will be called when stepping occurs, accepting only the tween values.
             *
             * You can add as many callbacks as you want. It must receive the tween values as an argument.
             * Callbacks can be of any callable type. It will only be called
             * via tween::step() functions. For seek callbacks, see tween::onSeek().
             *
             * Keep in mind that the function will be *copied* into an array, so any variable captured by value
             * will also be copied with it.
             *
             * If the callback returns false, it will be called next time. If it returns true, it will be removed from
             * the callback queue.
             *
             * **Example**:
             *
             * @code
             * auto t = tweeny:from(0).to(100).during(100);
             *
             * // pass a lambda
             * t.onStep([](int v) { printf("%d ", v); return false; });
             *
             * // pass a functor instance
             * struct ftor { void operator()(int x) { printf("%d ", x); return false; } };
             * t.onStep(ftor());
             * @endcode
             * @sa step
             * @sa seek
             * @sa onSeek
             * @param callback A callback in the form `bool f(Ts...)`
             */
            tween<T, Ts...> & onStep(typename detail::tweentraits<T, Ts...>::noTweenCallbackType callback);

            /**
             * @brief Adds a callback for that will be called when seeking occurs
             *
             * You can add as many callbacks as you want. Its arguments types must be equal to the argument types
             * of a tween instance, preceded by a variable of the tween typve. Callbacks can be of any callable type. It will be called
             * via tween::seek() functions. For step callbacks, see tween::onStep().
             *
             * Keep in mind that the function will be *copied* into an array, so any variable captured by value
             * will also be copied with it.
             *
             * If the callback returns false, it will be called next time. If it returns true, it will be removed from
             * the callback queue.
             *
             * **Example**:
             *
             * @code
             * auto t = t:from(0).to(100).during(100);
             *
             * // pass a lambda
             * t.onSeek([](tweeny::tween<int> & t, int v) { printf("%d ", v); });
             *
             * // pass a functor instance
             * struct ftor { void operator()(tweeny::tween<int> & t, int v) { printf("%d ", v); } };
             * t.onSeek(ftor());
             * @endcode
             * @param callback A callback in with the prototype `bool callback(tween<Ts...> & t, Ts...)`
             */
            tween<T, Ts...> & onSeek(typename detail::tweentraits<T, Ts...>::callbackType callback);

            /**
             * @brief Adds a callback for that will be called when seeking occurs, accepting only the tween values.
             *
             * You can add as many callbacks as you want. It must receive the tween as an argument.
             * Callbacks can be of any callable type. It will be called
             * via tween::seek() functions. For step callbacks, see tween::onStep().
             *
             * Keep in mind that the function will be *copied* into an array, so any variable captured by value
             * will also be copied again.
             *
             * If the callback returns false, it will be called next time. If it returns true, it will be removed from
             * the callback queue.
             *
             * **Example**:
             *
             * @code
             * auto t = t:from(0).to(100).during(100);
             *
             * // pass a lambda
             * t.onSeek([](int v) { printf("%d ", v); });
             *
             * // pass a functor instance
             * struct ftor { void operator()(int v) { printf("%d ", v); return false; } };
             * t.onSeek(ftor());
             * @endcode
             * @param callback A callback in the form `bool f(Ts...)`
             */
            tween<T, Ts...> & onSeek(typename detail::tweentraits<T, Ts...>::noTweenCallbackType callback);

            /**
             * @brief Adds a callback for that will be called when seeking occurs, accepting only the tween.
             *
             * You can add as many callbacks as you want. It must receive the tween as an argument.
             * Callbacks can be of any callable type. It will be called
             * via tween::seek() functions. For step callbacks, see tween::onStep().
             *
             * Keep in mind that the function will be *copied* into an array, so any variable captured by value
             * will also be copied again.
             *
             * If the callback returns false, it will be called next time. If it returns true, it will be removed from
             * the callback queue.
             *
             * **Example**:
             *
             * @code
             * auto t = t:from(0).to(100).during(100);
             *
             * // pass a lambda
             * t.onSeek([](tweeny::tween<int> & t) { printf("%d ", t.value()); return false; });
             *
             * // pass a functor instance
             * struct ftor { void operator()(tweeny::tween<int> & t) { printf("%d ",  t.value()); return false; } };
             * t.onSeek(ftor());
             * @endcode
             * @param callback A callback in the form `bool f(tween<Ts...> & t)`
             */
            tween<T, Ts...> & onSeek(typename detail::tweentraits<T, Ts...>::noValuesCallbackType callback);

            /**
             * @brief Returns the total duration of this tween
             *
             * @returns The duration of all the tween points.
             */
            uint32_t duration() const;

            /**
             * @brief Returns the current tween values
             *
             * This returns the current tween value as returned by the
             * tween::step() function, except that it does not perform a step.
             * @returns std::tuple<Ts...> with the current tween values.
             */
            const typename detail::tweentraits<T, Ts...>::valuesType & peek() const;

            /**
             * @brief Calculates and returns the tween values at a given progress
             *
             * This returns the tween value at the requested progress, without stepping
             * or seeking.
             * @returns std::tuple<Ts...> with the current tween values.
             */
             const typename detail::tweentraits<T, Ts...>::valuesType peek(float progress) const;

            /**
             * @brief Calculates and return the tween values at a given time
             *
             * This returns the tween values at the requested time, without stepping
             * or seeking.
             * @returns std::tuple<Ts...> with the calculated tween values.
             */
            const typename detail::tweentraits<T, Ts...>::valuesType peek(uint32_t time) const;

            /**
             * @brief Returns the current currentProgress of the interpolation.
             *
             * 0 means its at the values passed in the construction, 1 means the last step.
             * @returns the current currentProgress between 0 and 1 (inclusive)
             */
            float progress() const;

            /**
             * @brief Sets the direction of this tween forward.
             *
             * Note that this only affects tween::step() function.
             * @returns *this
             * @sa backward
             */
            tween<T, Ts...> & forward();

            /**
             * @brief Sets the direction of this tween backward.
             *
             * Note that this only affects tween::step() function.
             * @returns *this
             * @sa forward
             */
            tween<T, Ts...> & backward();

            /**
             * @brief Returns the current direction of this tween
             *
             * @returns -1 If it is mobin backwards in time, 1 if it is moving forward in time
             */
            int direction() const;

            /**
             * @brief Jumps to a specific tween point
             *
             * This will seek the tween to a percentage matching the beginning of that step.
             *
             * @param point The point to seek to. 0 means the point passed in tweeny::from
             * @param suppressCallbacks (optional) set to true to suppress seek() callbacks
             * @returns current values
             * @sa seek
             */
            const typename detail::tweentraits<T, Ts...>::valuesType & jump(size_t point, bool suppressCallbacks = false);

            /**
             * @brief Returns the current tween point
             *
             * @returns Current tween point
             */
            uint16_t point() const;

        private /* member types */:
            using traits = detail::tweentraits<T, Ts...>;

        private /* member variables */:
            uint32_t total = 0; // total runtime
            uint16_t currentPoint = 0; // current point
            float currentProgress = 0; // current progress
            std::vector<detail::tweenpoint<T, Ts...>> points;
            typename traits::valuesType current;
            std::vector<typename traits::callbackType> onStepCallbacks;
            std::vector<typename traits::callbackType> onSeekCallbacks;
            int8_t currentDirection = 1;

        private:
            /* member functions */
            tween(T t, Ts... vs);
            template<size_t I> void interpolate(float prog, unsigned point, typename traits::valuesType & values, detail::int2type<I>) const;
            void interpolate(float prog, unsigned point, typename traits::valuesType & values, detail::int2type<0>) const;
            void render(float p);
            void dispatch(std::vector<typename traits::callbackType> & cbVector);
            uint16_t pointAt(float progress) const;
    };

    /**
    * @brief Class specialization when a tween has a single value
    *
    * This class is preferred automatically by your compiler when your tween has only one value. It exists mainly
    * so that you dont need to use std::get<0> to obtain a single value when using tween::step, tween::seek or any other
    * value returning function. Other than that, you should look at the
    * tweeny::tween documentation.
    *
    * Except for this little detail, this class methods and behaviours are exactly the same.
    */
    template<typename T>
    class tween<T> {
        public:
            static tween<T> from(T t);

        public:
            tween(); ///< @sa tween::tween
            tween<T> & to(T t); ///< @sa tween::to
            template<typename... Fs> tween<T> & via(Fs... fs); ///< @sa tween::via
            template<typename... Fs> tween<T> & via(int index, Fs... fs); ///< @sa tween::via
            template<typename... Fs> tween<T> & via(tweeny::easing::enumerated enumerated, Fs... fs); ///< @sa tween::via
            template<typename... Fs> tween<T> & via(const std::string & easing, Fs... fs); ///< @sa tween::via
            template<typename... Fs> tween<T> & via(const char * easing, Fs... fs); ///< @sa tween::via
            template<typename... Ds> tween<T> & during(Ds... ds); ///< @sa tween::during
            const T & step(int32_t dt, bool suppressCallbacks = false); ///< @sa tween::step(int32_t dt, bool suppressCallbacks)
            const T & step(uint32_t dt, bool suppressCallbacks = false); ///< @sa tween::step(uint32_t dt, bool suppressCallbacks)
            const T & step(float dp, bool suppressCallbacks = false); ///< @sa tween::step(float dp, bool suppressCallbacks)
            const T & seek(float p, bool suppressCallbacks = false); ///< @sa tween::seek(float p, bool suppressCallbacks)
            const T & seek(int32_t d, bool suppressCallbacks = false); ///< @sa tween::seek(int32_t d, bool suppressCallbacks)
            const T & seek(uint32_t d, bool suppressCallbacks = false); ///< @sa tween::seek(uint32_t d, bool suppressCallbacks)
            tween<T> & onStep(typename detail::tweentraits<T>::callbackType callback); ///< @sa tween::onStep
            tween<T> & onStep(typename detail::tweentraits<T>::noValuesCallbackType callback); ///< @sa tween::onStep
            tween<T> & onStep(typename detail::tweentraits<T>::noTweenCallbackType callback); ///< @sa tween::onStep
            tween<T> & onSeek(typename detail::tweentraits<T>::callbackType callback); ///< @sa tween::onSeek
            tween<T> & onSeek(typename detail::tweentraits<T>::noValuesCallbackType callback); ///< @sa tween::onSeek
            tween<T> & onSeek(typename detail::tweentraits<T>::noTweenCallbackType callback); ///< @sa tween::onSeek
            const T & peek() const; ///< @sa tween::peek
            T peek(float progress) const; ///< @sa tween::peek
            T peek(uint32_t time) const; ///< @sa tween::peek
            uint32_t duration() const; ///< @sa tween::duration
            float progress() const; ///< @sa tween::progress
            tween<T> & forward(); ///< @sa tween::forward
            tween<T> & backward(); ///< @sa tween::backward
            int direction() const; ///< @sa tween::direction
            const T & jump(size_t point, bool suppressCallbacks = false); ///< @sa tween::jump
            uint16_t point() const; ///< @sa tween::point

        private /* member types */:
            using traits = detail::tweentraits<T>;

        private /* member variables */:
            uint32_t total = 0; // total runtime
            uint16_t currentPoint = 0; // current point
            float currentProgress = 0; // current progress
            std::vector<detail::tweenpoint<T>> points;
            T current;
            std::vector<typename traits::callbackType> onStepCallbacks;
            std::vector<typename traits::callbackType> onSeekCallbacks;
            int8_t currentDirection = 1;

        private:
            /* member functions */
            tween(T t);
            void interpolate(float prog, unsigned point, T & value) const;
            void render(float p);
            void dispatch(std::vector<typename traits::callbackType> & cbVector);
            uint16_t pointAt(float progress) const;
    };
}

/*
 This file is part of the Tweeny library.

 Copyright (c) 2016-2021 Leonardo Guilherme Lucena de Freitas
 Copyright (c) 2016 Guilherme R. Costa

 Permission is hereby granted, free of charge, to any person obtaining a copy of
 this software and associated documentation files (the "Software"), to deal in
 the Software without restriction, including without limitation the rights to
 use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of
 the Software, and to permit persons to whom the Software is furnished to do so,
 subject to the following conditions:

 The above copyright notice and this permission notice shall be included in all
 copies or substantial portions of the Software.

 THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS
 FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR
 COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER
 IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
 CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
*/

/*
 * The purpose of this file is to hold implementations for the tween.h file.
 */

#ifndef TWEENY_TWEEN_TCC
#define TWEENY_TWEEN_TCC

/*
 This file is part of the Tweeny library.

 Copyright (c) 2016-2021 Leonardo Guilherme Lucena de Freitas
 Copyright (c) 2016 Guilherme R. Costa

 Permission is hereby granted, free of charge, to any person obtaining a copy of
 this software and associated documentation files (the "Software"), to deal in
 the Software without restriction, including without limitation the rights to
 use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of
 the Software, and to permit persons to whom the Software is furnished to do so,
 subject to the following conditions:

 The above copyright notice and this permission notice shall be included in all
 copies or substantial portions of the Software.

 THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS
 FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR
 COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER
 IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
 CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
*/

/* This file contains code to help call a function applying a tuple as its arguments.
 * This code is private and not documented. */

#ifndef TWEENY_DISPATCHER_H
#define TWEENY_DISPATCHER_H

#include <tuple>

namespace tweeny {
    namespace detail {
        template<int ...> struct seq { };
        template<int N, int ...S> struct gens : gens<N - 1, N - 1, S...> { };
        template<int ...S> struct gens<0, S...> {
            typedef seq<S...> type;
        };

        template<typename R, typename Func, typename TupleType, int ...S>
        R dispatch(Func && f, TupleType && args, seq<S...>) {
           return f(std::get<S>(args) ...);
        }

        template<typename R, typename Func, typename... Ts>
        R call(Func && f, const std::tuple<Ts...> & args) {
            return dispatch<R>(f, args, typename gens<sizeof...(Ts)>::type());
        }
    }
}

#endif //TWEENY_DISPATCHER_H

namespace tweeny {

    namespace detail {
        template<typename T>
        T clip(const T & n, const T & lower, const T & upper) {
            return std::max(lower, std::min(n, upper));
        }
    }

    template<typename T, typename... Ts> inline tween<T, Ts...> tween<T, Ts...>::from(T t, Ts... vs) { return tween<T, Ts...>(t, vs...); }
    template<typename T, typename... Ts> inline tween<T, Ts...>::tween() { }
    template<typename T, typename... Ts> inline tween<T, Ts...>::tween(T t, Ts... vs) {
        points.emplace_back(t, vs...);
    }

    template<typename T, typename... Ts> inline tween<T, Ts...> & tween<T, Ts...>::to(T t, Ts... vs) {
        points.emplace_back(t, vs...);
        return *this;
    }

    template<typename T, typename... Ts>
    template<typename... Fs>
    inline tween<T, Ts...> & tween<T, Ts...>::via(Fs... vs) {
        points.at(points.size() - 2).via(vs...);
        return *this;
    }

    template<typename T, typename... Ts>
    template<typename... Fs>
    inline tween<T, Ts...> & tween<T, Ts...>::via(int index, Fs... vs) {
        points.at(static_cast<size_t>(index)).via(vs...);
        return *this;
    }

    template<typename T, typename... Ts>
    template<typename... Fs>
    tween<T, Ts...> & tween<T, Ts...>::via(easing::enumerated enumerated, Fs... vs) {
        switch (enumerated) {
            case easing::enumerated::def: return via(easing::def, vs...);
            case easing::enumerated::linear: return via(easing::linear, vs...);
            case easing::enumerated::stepped: return via(easing::stepped, vs...);
            case easing::enumerated::quadraticIn: return via(easing::quadraticIn, vs...);
            case easing::enumerated::quadraticOut: return via(easing::quadraticOut, vs...);
            case easing::enumerated::quadraticInOut: return via(easing::quadraticInOut, vs...);
            case easing::enumerated::cubicIn: return via(easing::cubicIn, vs...);
            case easing::enumerated::cubicOut: return via(easing::cubicOut, vs...);
            case easing::enumerated::cubicInOut: return via(easing::cubicInOut, vs...);
            case easing::enumerated::quarticIn: return via(easing::quarticIn, vs...);
            case easing::enumerated::quarticOut: return via(easing::quarticOut, vs...);
            case easing::enumerated::quarticInOut: return via(easing::quarticInOut, vs...);
            case easing::enumerated::quinticIn: return via(easing::quinticIn, vs...);
            case easing::enumerated::quinticOut: return via(easing::quinticOut, vs...);
            case easing::enumerated::quinticInOut: return via(easing::quinticInOut, vs...);
            case easing::enumerated::sinusoidalIn: return via(easing::sinusoidalIn, vs...);
            case easing::enumerated::sinusoidalOut: return via(easing::sinusoidalOut, vs...);
            case easing::enumerated::sinusoidalInOut: return via(easing::sinusoidalInOut, vs...);
            case easing::enumerated::exponentialIn: return via(easing::exponentialIn, vs...);
            case easing::enumerated::exponentialOut: return via(easing::exponentialOut, vs...);
            case easing::enumerated::exponentialInOut: return via(easing::exponentialInOut, vs...);
            case easing::enumerated::circularIn: return via(easing::circularIn, vs...);
            case easing::enumerated::circularOut: return via(easing::circularOut, vs...);
            case easing::enumerated::circularInOut: return via(easing::circularInOut, vs...);
            case easing::enumerated::bounceIn: return via(easing::bounceIn, vs...);
            case easing::enumerated::bounceOut: return via(easing::bounceOut, vs...);
            case easing::enumerated::bounceInOut: return via(easing::bounceInOut, vs...);
            case easing::enumerated::elasticIn: return via(easing::elasticIn, vs...);
            case easing::enumerated::elasticOut: return via(easing::elasticOut, vs...);
            case easing::enumerated::elasticInOut: return via(easing::elasticInOut, vs...);
            case easing::enumerated::backIn: return via(easing::backIn, vs...);
            case easing::enumerated::backOut: return via(easing::backOut, vs...);
            case easing::enumerated::backInOut: return via(easing::backInOut, vs...);
            default: return via(easing::def, vs...);
        }
    }

    template<typename T, typename... Ts>
    template<typename... Fs>
    tween<T, Ts...> & tween<T, Ts...>::via(const std::string & easing, Fs... vs) {
        if (easing == "stepped") return via(easing::stepped, vs...);
        if (easing == "linear") return via(easing::linear, vs...);
        if (easing == "quadraticIn") return via(easing::quadraticIn, vs...);
        if (easing == "quadraticOut") return via(easing::quadraticOut, vs...);
        if (easing == "quadraticInOut") return via(easing::quadraticInOut, vs...);
        if (easing == "cubicIn") return via(easing::cubicIn, vs...);
        if (easing == "cubicOut") return via(easing::cubicOut, vs...);
        if (easing == "cubicInOut") return via(easing::cubicInOut, vs...);
        if (easing == "quarticIn") return via(easing::quarticIn, vs...);
        if (easing == "quarticOut") return via(easing::quarticOut, vs...);
        if (easing == "quarticInOut") return via(easing::quarticInOut, vs...);
        if (easing == "quinticIn") return via(easing::quinticIn, vs...);
        if (easing == "quinticOut") return via(easing::quinticOut, vs...);
        if (easing == "quinticInOut") return via(easing::quinticInOut, vs...);
        if (easing == "sinusoidalIn") return via(easing::sinusoidalIn, vs...);
        if (easing == "sinusoidalOut") return via(easing::sinusoidalOut, vs...);
        if (easing == "sinusoidalInOut") return via(easing::sinusoidalInOut, vs...);
        if (easing == "exponentialIn") return via(easing::exponentialIn, vs...);
        if (easing == "exponentialOut") return via(easing::exponentialOut, vs...);
        if (easing == "exponentialInOut") return via(easing::exponentialInOut, vs...);
        if (easing == "circularIn") return via(easing::circularIn, vs...);
        if (easing == "circularOut") return via(easing::circularOut, vs...);
        if (easing == "circularInOut") return via(easing::circularInOut, vs...);
        if (easing == "bounceIn") return via(easing::bounceIn, vs...);
        if (easing == "bounceOut") return via(easing::bounceOut, vs...);
        if (easing == "bounceInOut") return via(easing::bounceInOut, vs...);
        if (easing == "elasticIn") return via(easing::elasticIn, vs...);
        if (easing == "elasticOut") return via(easing::elasticOut, vs...);
        if (easing == "elasticInOut") return via(easing::elasticInOut, vs...);
        if (easing == "backIn") return via(easing::backIn, vs...);
        if (easing == "backOut") return via(easing::backOut, vs...);
        if (easing == "backInOut") return via(easing::backInOut, vs...);
        return via(easing::def, vs...);
    }

    template<typename T, typename... Ts>
    template<typename... Fs>
    tween <T, Ts...> & tween<T, Ts...>::via(const char * easing, Fs... vs) {
        return via(std::string(easing));
    }

    template<typename T, typename... Ts>
    template<typename... Ds>
    inline tween<T, Ts...> & tween<T, Ts...>::during(Ds... ds) {
        total = 0;
        points.at(points.size() - 2).during(ds...);
        for (detail::tweenpoint<T, Ts...> & p : points) {
            total += p.duration();
            p.stacked = total;
        }
        return *this;
    }

    template<typename T, typename... Ts>
    inline const typename detail::tweentraits<T, Ts...>::valuesType & tween<T, Ts...>::step(int32_t dt, bool suppress) {
        return step(static_cast<float>(dt)/static_cast<float>(total), suppress);
    }

    template<typename T, typename... Ts>
    inline const typename detail::tweentraits<T, Ts...>::valuesType & tween<T, Ts...>::step(uint32_t dt, bool suppress) {
        return step(static_cast<int32_t>(dt), suppress);
    }

    template<typename T, typename... Ts>
    inline const typename detail::tweentraits<T, Ts...>::valuesType & tween<T, Ts...>::step(float dp, bool suppress) {
        dp *= currentDirection;
        seek(currentProgress + dp, true);
        if (!suppress) dispatch(onStepCallbacks);
        return current;
    }

    template<typename T, typename... Ts>
    inline const typename detail::tweentraits<T, Ts...>::valuesType & tween<T, Ts...>::seek(float p, bool suppress) {
        p = detail::clip(p, 0.0f, 1.0f);
        currentProgress = p;
        render(p);
        if (!suppress) dispatch(onSeekCallbacks);
        return current;
    }

    template<typename T, typename... Ts>
    inline const typename detail::tweentraits<T, Ts...>::valuesType & tween<T, Ts...>::seek(int32_t t, bool suppress) {
        return seek(static_cast<float>(t) / static_cast<float>(total), suppress);
    }

    template<typename T, typename... Ts>
    inline uint32_t tween<T, Ts...>::duration() const {
        return total;
    }

    template<typename T, typename... Ts>
    template<size_t I>
    inline void tween<T, Ts...>::interpolate(float prog, unsigned point, typename traits::valuesType & values, detail::int2type<I>) const {
        auto & p = points.at(point);
        auto pointDuration = uint32_t(p.duration() - (p.stacked - (prog * static_cast<float>(total))));
        float pointTotal = static_cast<float>(pointDuration) / static_cast<float>(p.duration(I));
        if (pointTotal > 1.0f) pointTotal = 1.0f;
        auto easing = std::get<I>(p.easings);
        std::get<I>(values) = easing(pointTotal, std::get<I>(p.values), std::get<I>(points.at(point+1).values));
        interpolate(prog, point, values, detail::int2type<I-1>{ });
    }

    template<typename T, typename... Ts>
    inline void tween<T, Ts...>::interpolate(float prog, unsigned point, typename traits::valuesType & values, detail::int2type<0>) const {
        auto & p = points.at(point);
        auto pointDuration = uint32_t(p.duration() - (p.stacked - (prog * static_cast<float>(total))));
        float pointTotal = static_cast<float>(pointDuration) / static_cast<float>(p.duration(0));
        if (pointTotal > 1.0f) pointTotal = 1.0f;
        auto easing = std::get<0>(p.easings);
        std::get<0>(values) = easing(pointTotal, std::get<0>(p.values), std::get<0>(points.at(point+1).values));
    }

    template<typename T, typename... Ts>
    inline void tween<T, Ts...>::render(float p) {
        currentPoint = pointAt(p);
        interpolate(p, currentPoint, current, detail::int2type<sizeof...(Ts) - 1 + 1 /* +1 for the T */>{ });
    }

    template<typename T, typename... Ts>
    tween<T, Ts...> & tween<T, Ts...>::onStep(typename detail::tweentraits<T, Ts...>::callbackType callback) {
        onStepCallbacks.push_back(callback);
        return *this;
    }

    template<typename T, typename... Ts>
    tween<T, Ts...> & tween<T, Ts...>::onStep(typename detail::tweentraits<T, Ts...>::noValuesCallbackType callback) {
        onStepCallbacks.push_back([callback](tween<T, Ts...> & t, T, Ts...) { return callback(t); });
        return *this;
    }

    template<typename T, typename... Ts>
    tween<T, Ts...> & tween<T, Ts...>::onStep(typename detail::tweentraits<T, Ts...>::noTweenCallbackType callback) {
        onStepCallbacks.push_back([callback](tween<T, Ts...> &, T t, Ts... vs) { return callback(t, vs...); });
        return *this;
    }

    template<typename T, typename... Ts>
    tween<T, Ts...> & tween<T, Ts...>::onSeek(typename detail::tweentraits<T, Ts...>::callbackType callback) {
        onSeekCallbacks.push_back(callback);
        return *this;
    }

    template<typename T, typename... Ts>
    tween<T, Ts...> & tween<T, Ts...>::onSeek(typename detail::tweentraits<T, Ts...>::noValuesCallbackType callback) {
        onSeekCallbacks.push_back([callback](tween<T, Ts...> & t, T, Ts...) { return callback(t); });
        return *this;
    }

    template<typename T, typename... Ts>
    tween<T, Ts...> & tween<T, Ts...>::onSeek(typename detail::tweentraits<T, Ts...>::noTweenCallbackType callback) {
        onSeekCallbacks.push_back([callback](tween<T, Ts...> &, T t, Ts... vs) { return callback(t, vs...); });
        return *this;
    }

    template<typename T, typename... Ts>
    void tween<T, Ts...>::dispatch(std::vector<typename traits::callbackType> & cbVector) {
        std::vector<size_t> dismissed;
        for (size_t i = 0; i < cbVector.size(); ++i) {
            auto && cb = cbVector[i];
            bool dismiss = detail::call<bool>(cb, std::tuple_cat(std::make_tuple(std::ref(*this)), current));
            if (dismiss) dismissed.push_back(i);
        }

        if (dismissed.size() > 0) {
            for (size_t i = 0; i < dismissed.size(); ++i) {
                size_t index = dismissed[i];
                cbVector[index] = cbVector.at(cbVector.size() - 1 - i);
            }
            cbVector.resize(cbVector.size() - dismissed.size());
        }
    }

    template<typename T, typename... Ts>
    const typename detail::tweentraits<T, Ts...>::valuesType & tween<T, Ts...>::peek() const {
      return current;
    }

    template<typename T, typename... Ts>
    const typename detail::tweentraits<T, Ts...>::valuesType tween<T, Ts...>::peek(float progress) const {
        typename detail::tweentraits<T, Ts...>::valuesType values;
        interpolate(progress, pointAt(progress), values, detail::int2type<sizeof...(Ts) - 1 + 1 /* +1 for the T */>{ });
        return values;
    }

    template<typename T, typename... Ts>
    const typename detail::tweentraits<T, Ts...>::valuesType tween<T, Ts...>::peek(uint32_t time) const {
        typename detail::tweentraits<T, Ts...>::valuesType values;
        float progress = static_cast<float>(time) / static_cast<float>(total);
        interpolate(progress, pointAt(progress), values, detail::int2type<sizeof...(Ts) - 1 + 1 /* +1 for the T */>{ });
        return values;
    }

  template<typename T, typename... Ts>
    float tween<T, Ts...>::progress() const {
        return currentProgress;
    }

    template<typename T, typename... Ts>
    tween<T, Ts...> & tween<T, Ts...>::forward() {
        currentDirection = 1;
        return *this;
    }

    template<typename T, typename... Ts>
    tween<T, Ts...> & tween<T, Ts...>::backward() {
        currentDirection = -1;
        return *this;
    }

    template<typename T, typename... Ts>
    int tween<T, Ts...>::direction() const {
        return currentDirection;
    }

    template<typename T, typename... Ts>
    inline const typename detail::tweentraits<T, Ts...>::valuesType & tween<T, Ts...>::jump(std::size_t p, bool suppress) {
        p = detail::clip(p, static_cast<size_t>(0), points.size() -1);
        return seek(static_cast<int32_t>(points.at(p).stacked), suppress);
    }

    template<typename T, typename... Ts> inline uint16_t tween<T, Ts...>::point() const {
        return currentPoint;
    }

    template<typename T, typename... Ts> inline uint16_t tween<T, Ts...>::pointAt(float progress) const {
        uint32_t t = static_cast<uint32_t>(progress * total);
        uint16_t point = 0;
        while (t > points.at(point).stacked) point++;
        if (point > 0 && t <= points.at(point - 1u).stacked) point--;
        return point;
    }
}

#endif //TWEENY_TWEEN_TCC

/*
 This file is part of the Tweeny library.

 Copyright (c) 2016-2021 Leonardo Guilherme Lucena de Freitas
 Copyright (c) 2016 Guilherme R. Costa

 Permission is hereby granted, free of charge, to any person obtaining a copy of
 this software and associated documentation files (the "Software"), to deal in
 the Software without restriction, including without limitation the rights to
 use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of
 the Software, and to permit persons to whom the Software is furnished to do so,
 subject to the following conditions:

 The above copyright notice and this permission notice shall be included in all
 copies or substantial portions of the Software.

 THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS
 FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR
 COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER
 IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
 CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
*/

/*
 * The purpose of this file is to hold implementations for the tween.h file, s
 * pecializing on the single value case.
 */
#ifndef TWEENY_TWEENONE_TCC
#define TWEENY_TWEENONE_TCC

namespace tweeny {
    template<typename T> inline tween<T> tween<T>::from(T t) { return tween<T>(t); }
    template<typename T> inline tween<T>::tween() { }
    template<typename T> inline tween<T>::tween(T t) {
        points.emplace_back(t);
    }

    template<typename T> inline tween<T> & tween<T>::to(T t) {
        points.emplace_back(t);
        return *this;
    }

    template<typename T>
    template<typename... Fs>
    inline tween<T> & tween<T>::via(Fs... vs) {
        points.at(points.size() - 2).via(vs...);
        return *this;
    }

    template<typename T>
    template<typename... Fs>
    inline tween<T> & tween<T>::via(int index, Fs... vs) {
        points.at(static_cast<size_t>(index)).via(vs...);
        return *this;
    }

    template<typename T>
    template<typename... Fs>
    tween <T> & tween<T>::via(easing::enumerated enumerated, Fs... vs) {
        switch (enumerated) {
            case easing::enumerated::def: return via(easing::def, vs...);
            case easing::enumerated::linear: return via(easing::linear, vs...);
            case easing::enumerated::stepped: return via(easing::stepped, vs...);
            case easing::enumerated::quadraticIn: return via(easing::quadraticIn, vs...);
            case easing::enumerated::quadraticOut: return via(easing::quadraticOut, vs...);
            case easing::enumerated::quadraticInOut: return via(easing::quadraticInOut, vs...);
            case easing::enumerated::cubicIn: return via(easing::cubicIn, vs...);
            case easing::enumerated::cubicOut: return via(easing::cubicOut, vs...);
            case easing::enumerated::cubicInOut: return via(easing::cubicInOut, vs...);
            case easing::enumerated::quarticIn: return via(easing::quarticIn, vs...);
            case easing::enumerated::quarticOut: return via(easing::quarticOut, vs...);
            case easing::enumerated::quarticInOut: return via(easing::quarticInOut, vs...);
            case easing::enumerated::quinticIn: return via(easing::quinticIn, vs...);
            case easing::enumerated::quinticOut: return via(easing::quinticOut, vs...);
            case easing::enumerated::quinticInOut: return via(easing::quinticInOut, vs...);
            case easing::enumerated::sinusoidalIn: return via(easing::sinusoidalIn, vs...);
            case easing::enumerated::sinusoidalOut: return via(easing::sinusoidalOut, vs...);
            case easing::enumerated::sinusoidalInOut: return via(easing::sinusoidalInOut, vs...);
            case easing::enumerated::exponentialIn: return via(easing::exponentialIn, vs...);
            case easing::enumerated::exponentialOut: return via(easing::exponentialOut, vs...);
            case easing::enumerated::exponentialInOut: return via(easing::exponentialInOut, vs...);
            case easing::enumerated::circularIn: return via(easing::circularIn, vs...);
            case easing::enumerated::circularOut: return via(easing::circularOut, vs...);
            case easing::enumerated::circularInOut: return via(easing::circularInOut, vs...);
            case easing::enumerated::bounceIn: return via(easing::bounceIn, vs...);
            case easing::enumerated::bounceOut: return via(easing::bounceOut, vs...);
            case easing::enumerated::bounceInOut: return via(easing::bounceInOut, vs...);
            case easing::enumerated::elasticIn: return via(easing::elasticIn, vs...);
            case easing::enumerated::elasticOut: return via(easing::elasticOut, vs...);
            case easing::enumerated::elasticInOut: return via(easing::elasticInOut, vs...);
            case easing::enumerated::backIn: return via(easing::backIn, vs...);
            case easing::enumerated::backOut: return via(easing::backOut, vs...);
            case easing::enumerated::backInOut: return via(easing::backInOut, vs...);
            default: return via(easing::def, vs...);
        }
    }

    template<typename T>
    template<typename... Fs>
    tween <T> & tween<T>::via(const std::string & easing, Fs... vs) {
        if (easing == "stepped") return via(easing::stepped, vs...);
        if (easing == "linear") return via(easing::linear, vs...);
        if (easing == "quadraticIn") return via(easing::quadraticIn, vs...);
        if (easing == "quadraticOut") return via(easing::quadraticOut, vs...);
        if (easing == "quadraticInOut") return via(easing::quadraticInOut, vs...);
        if (easing == "cubicIn") return via(easing::cubicIn, vs...);
        if (easing == "cubicOut") return via(easing::cubicOut, vs...);
        if (easing == "cubicInOut") return via(easing::cubicInOut, vs...);
        if (easing == "quarticIn") return via(easing::quarticIn, vs...);
        if (easing == "quarticOut") return via(easing::quarticOut, vs...);
        if (easing == "quarticInOut") return via(easing::quarticInOut, vs...);
        if (easing == "quinticIn") return via(easing::quinticIn, vs...);
        if (easing == "quinticOut") return via(easing::quinticOut, vs...);
        if (easing == "quinticInOut") return via(easing::quinticInOut, vs...);
        if (easing == "sinusoidalIn") return via(easing::sinusoidalIn, vs...);
        if (easing == "sinusoidalOut") return via(easing::sinusoidalOut, vs...);
        if (easing == "sinusoidalInOut") return via(easing::sinusoidalInOut, vs...);
        if (easing == "exponentialIn") return via(easing::exponentialIn, vs...);
        if (easing == "exponentialOut") return via(easing::exponentialOut, vs...);
        if (easing == "exponentialInOut") return via(easing::exponentialInOut, vs...);
        if (easing == "circularIn") return via(easing::circularIn, vs...);
        if (easing == "circularOut") return via(easing::circularOut, vs...);
        if (easing == "circularInOut") return via(easing::circularInOut, vs...);
        if (easing == "bounceIn") return via(easing::bounceIn, vs...);
        if (easing == "bounceOut") return via(easing::bounceOut, vs...);
        if (easing == "bounceInOut") return via(easing::bounceInOut, vs...);
        if (easing == "elasticIn") return via(easing::elasticIn, vs...);
        if (easing == "elasticOut") return via(easing::elasticOut, vs...);
        if (easing == "elasticInOut") return via(easing::elasticInOut, vs...);
        if (easing == "backIn") return via(easing::backIn, vs...);
        if (easing == "backOut") return via(easing::backOut, vs...);
        if (easing == "backInOut") return via(easing::backInOut, vs...);
        return via(easing::def, vs...);
    }

    template<typename T>
    template<typename... Fs>
    tween <T> & tween<T>::via(const char * easing, Fs... vs) {
        return via(std::string(easing));
    }

    template<typename T>
    template<typename... Ds>
    inline tween<T> & tween<T>::during(Ds... ds) {
        total = 0;
        points.at(points.size() - 2).during(ds...);
        for (detail::tweenpoint<T> & p : points) {
            total += p.duration();
            p.stacked = total;
        }
        return *this;
    }

    template<typename T>
    inline const T & tween<T>::step(int32_t dt, bool suppress) {
        return step(static_cast<float>(dt)/static_cast<float>(total), suppress);
    }

    template<typename T>
    inline const T & tween<T>::step(uint32_t dt, bool suppress) {
        return step(static_cast<int32_t>(dt), suppress);
    }

    template<typename T>
    inline const T & tween<T>::step(float dp, bool suppress) {
        dp *= currentDirection;
        seek(currentProgress + dp, true);
        if (!suppress) dispatch(onStepCallbacks);
        return current;
    }

    template<typename T>
    inline const T & tween<T>::seek(float p, bool suppress) {
        p = detail::clip(p, 0.0f, 1.0f);
        currentProgress = p;
        render(p);
        if (!suppress) dispatch(onSeekCallbacks);
        return current;
    }

    template<typename T>
    inline const T & tween<T>::seek(int32_t t, bool suppress) {
        return seek(static_cast<float>(t) / static_cast<float>(total), suppress);
    }

    template<typename T>
    inline const T & tween<T>::seek(uint32_t t, bool suppress) {
        return seek(static_cast<float>(t) / static_cast<float>(total), suppress);
    }

    template<typename T>
    inline uint32_t tween<T>::duration() const {
        return total;
    }

    template<typename T>
    inline void tween<T>::interpolate(float prog, unsigned point, T & value) const {
        auto & p = points.at(point);
        auto pointDuration = uint32_t(p.duration() - (p.stacked - (prog * static_cast<float>(total))));
        float pointTotal = static_cast<float>(pointDuration) / static_cast<float>(p.duration());
        if (pointTotal > 1.0f) pointTotal = 1.0f;
        auto easing = std::get<0>(p.easings);
        value = easing(pointTotal, std::get<0>(p.values), std::get<0>(points.at(point+1).values));
    }

    template<typename T>
    inline void tween<T>::render(float p) {
        currentPoint = pointAt(p);
        interpolate(p, currentPoint, current);
    }

    template<typename T>
    tween<T> & tween<T>::onStep(typename detail::tweentraits<T>::callbackType callback) {
        onStepCallbacks.push_back(callback);
        return *this;
    }

    template<typename T>
    tween<T> & tween<T>::onStep(typename detail::tweentraits<T>::noValuesCallbackType callback) {
        onStepCallbacks.push_back([callback](tween<T> & tween, T) { return callback(tween); });
        return *this;
    }

    template<typename T>
    tween<T> & tween<T>::onStep(typename detail::tweentraits<T>::noTweenCallbackType callback) {
        onStepCallbacks.push_back([callback](tween<T> &, T v) { return callback(v); });
        return *this;
    }

    template<typename T>
    tween<T> & tween<T>::onSeek(typename detail::tweentraits<T>::callbackType callback) {
        onSeekCallbacks.push_back(callback);
        return *this;
    }

    template<typename T>
    tween<T> & tween<T>::onSeek(typename detail::tweentraits<T>::noValuesCallbackType callback) {
        onSeekCallbacks.push_back([callback](tween<T> & t, T) { return callback(t); });
        return *this;
    }

    template<typename T>
    tween<T> & tween<T>::onSeek(typename detail::tweentraits<T>::noTweenCallbackType callback) {
        onSeekCallbacks.push_back([callback](tween<T> &, T v) { return callback(v); });
        return *this;
    }

    template<typename T>
    void tween<T>::dispatch(std::vector<typename traits::callbackType> & cbVector) {
        std::vector<size_t> dismissed;
        for (size_t i = 0; i < cbVector.size(); ++i) {
            auto && cb = cbVector[i];
            bool dismiss = cb(*this, current);
            if (dismiss) dismissed.push_back(i);
        }

        if (dismissed.size() > 0) {
            for (size_t i = 0; i < dismissed.size(); ++i) {
                size_t index = dismissed[i];
                cbVector[index] = cbVector.at(cbVector.size() - 1 - i);
            }
            cbVector.resize(cbVector.size() - dismissed.size());
        }
    }

    template<typename T>
    const T & tween<T>::peek() const {
        return current;
    }

    template<typename T>
    T tween<T>::peek(float progress) const {
        T value;
        interpolate(progress, pointAt(progress), value);
        return value;
    }

    template<typename T>
    T tween<T>::peek(uint32_t time) const {
        T value;
        float progress = static_cast<float>(time) / static_cast<float>(total);
        interpolate(progress, pointAt(progress), value);
        return value;
    }

  template<typename T>
    float tween<T>::progress() const {
        return currentProgress;
    }

    template<typename T>
    tween<T> & tween<T>::forward() {
        currentDirection = 1;
        return *this;
    }

    template<typename T>
    tween<T> & tween<T>::backward() {
        currentDirection = -1;
        return *this;
    }

    template<typename T>
    int tween<T>::direction() const {
        return currentDirection;
    }

    template<typename T>
    inline const T & tween<T>::jump(size_t p, bool suppress) {
        p = detail::clip(p, static_cast<size_t>(0), points.size() -1);
        return seek(points.at(p).stacked, suppress);
    }

    template<typename T> inline uint16_t tween<T>::point() const {
        return currentPoint;
    }

    template<typename T> inline uint16_t tween<T>::pointAt(float progress) const {
        auto t = static_cast<uint32_t>(progress * total);
        uint16_t point = 0;
        while (t > points.at(point).stacked) point++;
        if (point > 0 && t <= points.at(point - 1u).stacked) point--;
        return point;
    }
}
#endif //TWEENY_TWEENONE_TCC

#endif //TWEENY_TWEEN_H

/**
 * @brief The tweeny namespace contains all symbols and names for the Tweeny library.
 */
namespace tweeny {
  /**
   * @brief Creates a tween starting from the values defined in the arguments.
   *
   * Starting values can have heterogeneous types, even user-defined types, provided they implement the
   * four arithmetic operators (+, -, * and /). The types used will also define the type of each next step, the type
   * of the callback and the type of arguments the passed easing functions must have.
   *
   * @sa tweeny::tween
   */
    template<typename... Ts> tween<Ts...> from(Ts... vs);
}

/*
 This file is part of the Tweeny library.

 Copyright (c) 2016-2021 Leonardo Guilherme Lucena de Freitas
 Copyright (c) 2016 Guilherme R. Costa

 Permission is hereby granted, free of charge, to any person obtaining a copy of
 this software and associated documentation files (the "Software"), to deal in
 the Software without restriction, including without limitation the rights to
 use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of
 the Software, and to permit persons to whom the Software is furnished to do so,
 subject to the following conditions:

 The above copyright notice and this permission notice shall be included in all
 copies or substantial portions of the Software.

 THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS
 FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR
 COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER
 IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
 CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
*/

/*
 * This file provides the implementation for tweeny.h
 */

#ifndef TWEENY_TWEENY_TCC
#define TWEENY_TWEENY_TCC

namespace tweeny {
  template<typename... Ts> inline tween<Ts...> from(Ts... vs) {
    return tween<Ts...>::from(vs...);
  }
}

#endif //TWEENY_TWEENY_TCC

#endif //TWEENY_TWEENY_H
