#include "movieclip2.h"
#include <algorithm>
#include "completeevent.h"
#include "platformutil.h"

/*
Disclaimer for Robert Penner's Easing Equations license:

TERMS OF USE - EASING EQUATIONS

Open source under the BSD License.

Copyright (c) 2001 Robert Penner
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
    * Neither the name of the author nor the names of contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#include <math.h>
#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif
#ifndef M_PI_2
#define M_PI_2 (M_PI / 2)
#endif

//QT_USE_NAMESPACE
typedef double qreal;

inline qreal qCos(qreal a)
{
	return ::cos(a);
}
inline qreal qSin(qreal a)
{
	return ::sin(a);
}
inline qreal qPow(qreal a, qreal b)
{
	return ::pow(a, b);
}
inline qreal qFabs(qreal a)
{
	return ::fabs(a);
}
inline qreal qAsin(qreal a)
{
	return ::asin(a);
}
inline qreal qMin(qreal a, qreal b)
{
	return std::min(a, b);
}
inline qreal qMax(qreal a, qreal b)
{
	return std::max(a, b);
}


/**
 * Easing equation function for a simple linear tweening, with no easing.
 *
 * @param t		Current time (in frames or seconds).
 * @return		The correct value.
 */
static qreal easeNone(qreal progress)
{
    return progress;
}

/**
 * Easing equation function for a quadratic (t^2) easing in: accelerating from zero velocity.
 *
 * @param t		Current time (in frames or seconds).
 * @return		The correct value.
 */
static qreal easeInQuad(qreal t)
{
    return t*t;
}

/**
* Easing equation function for a quadratic (t^2) easing out: decelerating to zero velocity.
*
* @param t		Current time (in frames or seconds).
* @return		The correct value.
*/
static qreal easeOutQuad(qreal t)
{
    return -t*(t-2);
}

/**
 * Easing equation function for a quadratic (t^2) easing in/out: acceleration until halfway, then deceleration.
 *
 * @param t		Current time (in frames or seconds).
 * @return		The correct value.
 */
static qreal easeInOutQuad(qreal t)
{
    t*=2.0;
    if (t < 1) {
        return t*t/qreal(2);
    } else {
        --t;
        return -0.5 * (t*(t-2) - 1);
    }
}

/**
 * Easing equation function for a quadratic (t^2) easing out/in: deceleration until halfway, then acceleration.
 *
 * @param t		Current time (in frames or seconds).
 * @return		The correct value.
 */
static qreal easeOutInQuad(qreal t)
{
    if (t < 0.5) return easeOutQuad (t*2)/2;
    return easeInQuad((2*t)-1)/2 + 0.5;
}

/**
 * Easing equation function for a cubic (t^3) easing in: accelerating from zero velocity.
 *
 * @param t		Current time (in frames or seconds).
 * @return		The correct value.
 */
static qreal easeInCubic(qreal t)
{
    return t*t*t;
}

/**
 * Easing equation function for a cubic (t^3) easing out: decelerating from zero velocity.
 *
 * @param t		Current time (in frames or seconds).
 * @return		The correct value.
 */
static qreal easeOutCubic(qreal t)
{
    t-=1.0;
    return t*t*t + 1;
}

/**
 * Easing equation function for a cubic (t^3) easing in/out: acceleration until halfway, then deceleration.
 *
 * @param t		Current time (in frames or seconds).
 * @return		The correct value.
 */
static qreal easeInOutCubic(qreal t)
{
    t*=2.0;
    if(t < 1) {
        return 0.5*t*t*t;
    } else {
        t -= qreal(2.0);
        return 0.5*(t*t*t + 2);
    }
}

/**
 * Easing equation function for a cubic (t^3) easing out/in: deceleration until halfway, then acceleration.
 *
 * @param t		Current time (in frames or seconds).
 * @return		The correct value.
 */
static qreal easeOutInCubic(qreal t)
{
    if (t < 0.5) return easeOutCubic (2*t)/2;
    return easeInCubic(2*t - 1)/2 + 0.5;
}

/**
 * Easing equation function for a quartic (t^4) easing in: accelerating from zero velocity.
 *
 * @param t		Current time (in frames or seconds).
 * @return		The correct value.
 */
static qreal easeInQuart(qreal t)
{
    return t*t*t*t;
}

/**
 * Easing equation function for a quartic (t^4) easing out: decelerating from zero velocity.
 *
 * @param t		Current time (in frames or seconds).
 * @return		The correct value.
 */
static qreal easeOutQuart(qreal t)
{
    t-= qreal(1.0);
    return - (t*t*t*t- 1);
}

/**
 * Easing equation function for a quartic (t^4) easing in/out: acceleration until halfway, then deceleration.
 *
 * @param t		Current time (in frames or seconds).
 * @return		The correct value.
 */
static qreal easeInOutQuart(qreal t)
{
    t*=2;
    if (t < 1) return 0.5*t*t*t*t;
    else {
        t -= 2.0f;
        return -0.5 * (t*t*t*t- 2);
    }
}

/**
 * Easing equation function for a quartic (t^4) easing out/in: deceleration until halfway, then acceleration.
 *
 * @param t		Current time (in frames or seconds).
 * @return		The correct value.
 */
static qreal easeOutInQuart(qreal t)
{
    if (t < 0.5) return easeOutQuart (2*t)/2;
    return easeInQuart(2*t-1)/2 + 0.5;
}

/**
 * Easing equation function for a quintic (t^5) easing in: accelerating from zero velocity.
 *
 * @param t		Current time (in frames or seconds).
 * @return		The correct value.
 */
static qreal easeInQuint(qreal t)
{
    return t*t*t*t*t;
}

/**
 * Easing equation function for a quintic (t^5) easing out: decelerating from zero velocity.
 *
 * @param t		Current time (in frames or seconds).
 * @return		The correct value.
 */
static qreal easeOutQuint(qreal t)
{
    t-=1.0;
    return t*t*t*t*t + 1;
}

/**
 * Easing equation function for a quintic (t^5) easing in/out: acceleration until halfway, then deceleration.
 *
 * @param t		Current time (in frames or seconds).
 * @return		The correct value.
 */
static qreal easeInOutQuint(qreal t)
{
    t*=2.0;
    if (t < 1) return 0.5*t*t*t*t*t;
    else {
        t -= 2.0;
        return 0.5*(t*t*t*t*t + 2);
    }
}

/**
 * Easing equation function for a quintic (t^5) easing out/in: deceleration until halfway, then acceleration.
 *
 * @param t		Current time (in frames or seconds).
 * @return		The correct value.
 */
static qreal easeOutInQuint(qreal t)
{
    if (t < 0.5) return easeOutQuint (2*t)/2;
    return easeInQuint(2*t - 1)/2 + 0.5;
}

/**
 * Easing equation function for a sinusoidal (sin(t)) easing in: accelerating from zero velocity.
 *
 * @param t		Current time (in frames or seconds).
 * @return		The correct value.
 */
static qreal easeInSine(qreal t)
{
    return (t == 1.0) ? 1.0 : -::qCos(t * M_PI_2) + 1.0;
}

/**
 * Easing equation function for a sinusoidal (sin(t)) easing out: decelerating from zero velocity.
 *
 * @param t		Current time (in frames or seconds).
 * @return		The correct value.
 */
static qreal easeOutSine(qreal t)
{
    return ::qSin(t* M_PI_2);
}

/**
 * Easing equation function for a sinusoidal (sin(t)) easing in/out: acceleration until halfway, then deceleration.
 *
 * @param t		Current time (in frames or seconds).
 * @return		The correct value.
 */
static qreal easeInOutSine(qreal t)
{
    return -0.5 * (::qCos(M_PI*t) - 1);
}

/**
 * Easing equation function for a sinusoidal (sin(t)) easing out/in: deceleration until halfway, then acceleration.
 *
 * @param t		Current time (in frames or seconds).
 * @return		The correct value.
 */
static qreal easeOutInSine(qreal t)
{
    if (t < 0.5) return easeOutSine (2*t)/2;
    return easeInSine(2*t - 1)/2 + 0.5;
}

/**
 * Easing equation function for an exponential (2^t) easing in: accelerating from zero velocity.
 *
 * @param t		Current time (in frames or seconds).
 * @return		The correct value.
 */
static qreal easeInExpo(qreal t)
{
    return (t==0 || t == 1.0) ? t : ::qPow(2.0, 10 * (t - 1)) - qreal(0.001);
}

/**
 * Easing equation function for an exponential (2^t) easing out: decelerating from zero velocity.
 *
 * @param t		Current time (in frames or seconds).
 * @return		The correct value.
 */
static qreal easeOutExpo(qreal t)
{
    return (t==1.0) ? 1.0 : 1.001 * (-::qPow(2.0f, -10 * t) + 1);
}

/**
 * Easing equation function for an exponential (2^t) easing in/out: acceleration until halfway, then deceleration.
 *
 * @param t		Current time (in frames or seconds).
 * @return		The correct value.
 */
static qreal easeInOutExpo(qreal t)
{
    if (t==0.0) return qreal(0.0);
    if (t==1.0) return qreal(1.0);
    t*=2.0;
    if (t < 1) return 0.5 * ::qPow(qreal(2.0), 10 * (t - 1)) - 0.0005;
    return 0.5 * 1.0005 * (-::qPow(qreal(2.0), -10 * (t - 1)) + 2);
}

/**
 * Easing equation function for an exponential (2^t) easing out/in: deceleration until halfway, then acceleration.
 *
 * @param t		Current time (in frames or seconds).
 * @return		The correct value.
 */
static qreal easeOutInExpo(qreal t)
{
    if (t < 0.5) return easeOutExpo (2*t)/2;
    return easeInExpo(2*t - 1)/2 + 0.5;
}

/**
 * Easing equation function for a circular (sqrt(1-t^2)) easing in: accelerating from zero velocity.
 *
 * @param t		Current time (in frames or seconds).
 * @return		The correct value.
 */
static qreal easeInCirc(qreal t)
{
    return -(::sqrt(1 - t*t) - 1);
}

/**
 * Easing equation function for a circular (sqrt(1-t^2)) easing out: decelerating from zero velocity.
 *
 * @param t		Current time (in frames or seconds).
 * @return		The correct value.
 */
static qreal easeOutCirc(qreal t)
{
    t-= qreal(1.0);
    return ::sqrt(1 - t* t);
}

/**
 * Easing equation function for a circular (sqrt(1-t^2)) easing in/out: acceleration until halfway, then deceleration.
 *
 * @param t		Current time (in frames or seconds).
 * @return		The correct value.
 */
static qreal easeInOutCirc(qreal t)
{
    t*=qreal(2.0);
    if (t < 1) {
        return -0.5 * (::sqrt(1 - t*t) - 1);
    } else {
        t -= qreal(2.0);
        return 0.5 * (::sqrt(1 - t*t) + 1);
    }
}

/**
 * Easing equation function for a circular (sqrt(1-t^2)) easing out/in: deceleration until halfway, then acceleration.
 *
 * @param t		Current time (in frames or seconds).
 * @return		The correct value.
 */
static qreal easeOutInCirc(qreal t)
{
    if (t < 0.5) return easeOutCirc (2*t)/2;
    return easeInCirc(2*t - 1)/2 + 0.5;
}

static qreal easeInElastic_helper(qreal t, qreal b, qreal c, qreal d, qreal a, qreal p)
{
    if (t==0) return b;
    qreal t_adj = (qreal)t / (qreal)d;
    if (t_adj==1) return b+c;

    qreal s;
    if(a < ::qFabs(c)) {
        a = c;
        s = p / 4.0f;
    } else {
        s = p / (2 * M_PI) * ::qAsin(c / a);
    }

    t_adj -= 1.0f;
    return -(a*::qPow(2.0f,10*t_adj) * ::qSin( (t_adj*d-s)*(2*M_PI)/p )) + b;
}

/**
 * Easing equation function for an elastic (exponentially decaying sine wave) easing in: accelerating from zero velocity.
 *
 * @param t		Current time (in frames or seconds).
 * @param a		Amplitude.
 * @param p		Period.
 * @return		The correct value.
 */
static qreal easeInElastic(qreal t, qreal a, qreal p)
{
    return easeInElastic_helper(t, 0, 1, 1, a, p);
}

static qreal easeOutElastic_helper(qreal t, qreal /*b*/, qreal c, qreal /*d*/, qreal a, qreal p)
{
    if (t==0) return 0;
    if (t==1) return c;

    qreal s;
    if(a < c) {
        a = c;
        s = p / 4.0f;
    } else {
        s = p / (2 * M_PI) * ::qAsin(c / a);
    }

    return (a*::qPow(2.0f,-10*t) * ::qSin( (t-s)*(2*M_PI)/p ) + c);
}

/**
 * Easing equation function for an elastic (exponentially decaying sine wave) easing out: decelerating from zero velocity.
 *
 * @param t		Current time (in frames or seconds).
 * @param a		Amplitude.
 * @param p		Period.
 * @return		The correct value.
 */
static qreal easeOutElastic(qreal t, qreal a, qreal p)
{
    return easeOutElastic_helper(t, 0, 1, 1, a, p);
}

/**
 * Easing equation function for an elastic (exponentially decaying sine wave) easing in/out: acceleration until halfway, then deceleration.
 *
 * @param t		Current time (in frames or seconds).
 * @param a		Amplitude.
 * @param p		Period.
 * @return		The correct value.
 */
static qreal easeInOutElastic(qreal t, qreal a, qreal p)
{
    if (t==0) return 0.0;
    t*=2.0;
    if (t==2) return 1.0;

    qreal s;
    if(a < 1.0) {
        a = 1.0;
        s = p / 4.0f;
    } else {
        s = p / (2 * M_PI) * ::qAsin(1.0 / a);
    }

    if (t < 1) return -.5*(a*::qPow(2.0f,10*(t-1)) * ::qSin( (t-1-s)*(2*M_PI)/p ));
    return a*::qPow(2.0f,-10*(t-1)) * ::qSin( (t-1-s)*(2*M_PI)/p )*.5 + 1.0;
}

/**
 * Easing equation function for an elastic (exponentially decaying sine wave) easing out/in: deceleration until halfway, then acceleration.
 *
 * @param t		Current time (in frames or seconds).
 * @param a		Amplitude.
 * @param p		Period.
 * @return		The correct value.
 */
static qreal easeOutInElastic(qreal t, qreal a, qreal p)
{
    if (t < 0.5) return easeOutElastic_helper(t*2, 0, 0.5, 1.0, a, p);
    return easeInElastic_helper(2*t - 1.0, 0.5, 0.5, 1.0, a, p);
}

/**
 * Easing equation function for a back (overshooting cubic easing: (s+1)*t^3 - s*t^2) easing in: accelerating from zero velocity.
 *
 * @param t		Current time (in frames or seconds).
 * @param s		Overshoot ammount: higher s means greater overshoot (0 produces cubic easing with no overshoot, and the default value of 1.70158 produces an overshoot of 10 percent).
 * @return		The correct value.
 */
static qreal easeInBack(qreal t, qreal s)
{
    return t*t*((s+1)*t - s);
}

/**
 * Easing equation function for a back (overshooting cubic easing: (s+1)*t^3 - s*t^2) easing out: decelerating from zero velocity.
 *
 * @param t		Current time (in frames or seconds).
 * @param s		Overshoot ammount: higher s means greater overshoot (0 produces cubic easing with no overshoot, and the default value of 1.70158 produces an overshoot of 10 percent).
 * @return		The correct value.
 */
static qreal easeOutBack(qreal t, qreal s)
{
    t-= qreal(1.0);
    return t*t*((s+1)*t+ s) + 1;
}

/**
 * Easing equation function for a back (overshooting cubic easing: (s+1)*t^3 - s*t^2) easing in/out: acceleration until halfway, then deceleration.
 *
 * @param t		Current time (in frames or seconds).
 * @param s		Overshoot ammount: higher s means greater overshoot (0 produces cubic easing with no overshoot, and the default value of 1.70158 produces an overshoot of 10 percent).
 * @return		The correct value.
 */
static qreal easeInOutBack(qreal t, qreal s)
{
    t *= 2.0;
    if (t < 1) {
        s *= 1.525f;
        return 0.5*(t*t*((s+1)*t - s));
    } else {
        t -= 2;
        s *= 1.525f;
        return 0.5*(t*t*((s+1)*t+ s) + 2);
    }
}

/**
 * Easing equation function for a back (overshooting cubic easing: (s+1)*t^3 - s*t^2) easing out/in: deceleration until halfway, then acceleration.
 *
 * @param t		Current time (in frames or seconds).
 * @param s		Overshoot ammount: higher s means greater overshoot (0 produces cubic easing with no overshoot, and the default value of 1.70158 produces an overshoot of 10 percent).
 * @return		The correct value.
 */
static qreal easeOutInBack(qreal t, qreal s)
{
    if (t < 0.5) return easeOutBack (2*t, s)/2;
    return easeInBack(2*t - 1, s)/2 + 0.5;
}

static qreal easeOutBounce_helper(qreal t, qreal c, qreal a)
{
    if (t == 1.0) return c;
    if (t < (4/11.0)) {
        return c*(7.5625*t*t);
    } else if (t < (8/11.0)) {
        t -= (6/11.0);
        return -a * (1. - (7.5625*t*t + .75)) + c;
    } else if (t < (10/11.0)) {
        t -= (9/11.0);
        return -a * (1. - (7.5625*t*t + .9375)) + c;
    } else {
        t -= (21/22.0);
        return -a * (1. - (7.5625*t*t + .984375)) + c;
    }
}

/**
 * Easing equation function for a bounce (exponentially decaying parabolic bounce) easing out: decelerating from zero velocity.
 *
 * @param t		Current time (in frames or seconds).
 * @param a		Amplitude.
 * @return		The correct value.
 */
static qreal easeOutBounce(qreal t, qreal a)
{
    return easeOutBounce_helper(t, 1, a);
}

/**
 * Easing equation function for a bounce (exponentially decaying parabolic bounce) easing in: accelerating from zero velocity.
 *
 * @param t		Current time (in frames or seconds).
 * @param a		Amplitude.
 * @return		The correct value.
 */
static qreal easeInBounce(qreal t, qreal a)
{
    return 1.0 - easeOutBounce_helper(1.0-t, 1.0, a);
}


/**
 * Easing equation function for a bounce (exponentially decaying parabolic bounce) easing in/out: acceleration until halfway, then deceleration.
 *
 * @param t		Current time (in frames or seconds).
 * @param a		Amplitude.
 * @return		The correct value.
 */
static qreal easeInOutBounce(qreal t, qreal a)
{
    if (t < 0.5) return easeInBounce (2*t, a)/2;
    else return (t == 1.0) ? 1.0 : easeOutBounce (2*t - 1, a)/2 + 0.5;
}

/**
 * Easing equation function for a bounce (exponentially decaying parabolic bounce) easing out/in: deceleration until halfway, then acceleration.
 *
 * @param t		Current time (in frames or seconds).
 * @param a		Amplitude.
 * @return		The correct value.
 */
static qreal easeOutInBounce(qreal t, qreal a)
{
    if (t < 0.5) return easeOutBounce_helper(t*2, 0.5, a);
    return 1.0 - easeOutBounce_helper (2.0-2*t, 0.5, a);
}

static inline qreal qt_sinProgress(qreal value)
{
    return qSin((value * M_PI) - M_PI_2) / 2 + qreal(0.5);
}

static inline qreal qt_smoothBeginEndMixFactor(qreal value)
{
    return qMin(qMax(1 - value * 2 + qreal(0.3), qreal(0.0)), qreal(1.0));
}

// SmoothBegin blends Smooth and Linear Interpolation.
// Progress 0 - 0.3      : Smooth only
// Progress 0.3 - ~ 0.5  : Mix of Smooth and Linear
// Progress ~ 0.5  - 1   : Linear only

/**
 * Easing function that starts growing slowly, then increases in speed. At the end of the curve the speed will be constant.
 */
static qreal easeInCurve(qreal t)
{
    const qreal sinProgress = qt_sinProgress(t);
    const qreal mix = qt_smoothBeginEndMixFactor(t);
    return sinProgress * mix + t * (1 - mix);
}

/**
 * Easing function that starts growing steadily, then ends slowly. The speed will be constant at the beginning of the curve.
 */
static qreal easeOutCurve(qreal t)
{
    const qreal sinProgress = qt_sinProgress(t);
    const qreal mix = qt_smoothBeginEndMixFactor(1 - t);
    return sinProgress * mix + t * (1 - mix);
}

/**
 * Easing function where the value grows sinusoidally. Note that the calculated  end value will be 0 rather than 1.
 */
static qreal easeSineCurve(qreal t)
{
    return (qSin(((t * M_PI * 2)) - M_PI_2) + 1) / 2;
}

/**
 * Easing function where the value grows cosinusoidally. Note that the calculated start value will be 0.5 and the end value will be 0.5
 * contrary to the usual 0 to 1 easing curve.
 */
static qreal easeCosineCurve(qreal t)
{
    return (qCos(((t * M_PI * 2)) - M_PI_2) + 1) / 2;
}

//////////////////////////////////////////////////////////////////////////

static qreal easeInElastic(qreal t)
{
	const qreal a = 0;
	const qreal p = 0.3;
	return easeInElastic(t, a, p);
}
static qreal easeOutElastic(qreal t)
{
	const qreal a = 0;
	const qreal p = 0.3;
	return easeOutElastic(t, a, p);
}
static qreal easeInOutElastic(qreal t)
{
	const qreal a = 0;
	const qreal p = 0.3;
	return easeInOutElastic(t, a, p);
}
static qreal easeOutInElastic(qreal t)
{
	const qreal a = 0;
	const qreal p = 0.3;
	return easeOutInElastic(t, a, p);
}

static qreal easeInBack(qreal t)
{
	const qreal s = 1.70158;
	return easeInBack(t, s);
}
static qreal easeOutBack(qreal t)
{
	const qreal s = 1.70158;
	return easeOutBack(t, s);
}
static qreal easeInOutBack(qreal t)
{
	const qreal s = 1.70158;
	return easeInOutBack(t, s);
}
static qreal easeOutInBack(qreal t)
{
	const qreal s = 1.70158;
	return easeOutInBack(t, s);
}


static qreal easeOutBounce(qreal t)
{
	const qreal a = 1;
	return easeOutBounce(t, a);
}
static qreal easeInBounce(qreal t)
{
	const qreal a = 1;
	return easeInBounce(t, a);
}
static qreal easeInOutBounce(qreal t)
{
	static qreal a = 1;
	return easeInOutBounce(t, a);
}
static qreal easeOutInBounce(qreal t)
{
	const qreal a = 1;
	return easeOutInBounce(t, a);
}

static double (*getTweenFunction(TweenType tweenType))(double)
{
	double (*tweenFunction)(double) = easeNone;

	switch (tweenType)
	{
		case eEaseLinear:
			tweenFunction = easeNone;
			break;
		case eEaseInQuad:
			tweenFunction = easeInQuad;
			break;
		case eEaseOutQuad:
			tweenFunction = easeOutQuad;
			break;
		case eEaseInOutQuad:
			tweenFunction = easeInOutQuad;
			break;
		case eEaseOutInQuad:
			tweenFunction = easeOutInQuad;
			break;
		case eEaseInCubic:
			tweenFunction = easeInCubic;
			break;
		case eEaseOutCubic:
			tweenFunction = easeOutCubic;
			break;
		case eEaseInOutCubic:
			tweenFunction = easeInOutCubic;
			break;
		case eEaseOutInCubic:
			tweenFunction = easeOutInCubic;
			break;
		case eEaseInQuart:
			tweenFunction = easeInQuart;
			break;
		case eEaseOutQuart:
			tweenFunction = easeOutQuart;
			break;
		case eEaseInOutQuart:
			tweenFunction = easeInOutQuart;
			break;
		case eEaseOutInQuart:
			tweenFunction = easeOutInQuart;
			break;
		case eEaseInQuint:
			tweenFunction = easeInQuint;
			break;
		case eEaseOutQuint:
			tweenFunction = easeOutQuint;
			break;
		case eEaseInOutQuint:
			tweenFunction = easeInOutQuint;
			break;
		case eEaseOutInQuint:
			tweenFunction = easeOutInQuint;
			break;
		case eEaseInSine:
			tweenFunction = easeInSine;
			break;
		case eEaseOutSine:
			tweenFunction = easeOutSine;
			break;
		case eEaseInOutSine:
			tweenFunction = easeInOutSine;
			break;
		case eEaseOutInSine:
			tweenFunction = easeOutInSine;
			break;
		case eEaseInExpo:
			tweenFunction = easeInExpo;
			break;
		case eEaseOutExpo:
			tweenFunction = easeOutExpo;
			break;
		case eEaseInOutExpo:
			tweenFunction = easeInOutExpo;
			break;
		case eEaseOutInExpo:
			tweenFunction = easeOutInExpo;
			break;
		case eEaseInCirc:
			tweenFunction = easeInCirc;
			break;
		case eEaseOutCirc:
			tweenFunction = easeOutCirc;
			break;
		case eEaseInOutCirc:
			tweenFunction = easeInOutCirc;
			break;
		case eEaseOutInCirc:
			tweenFunction = easeOutInCirc;
			break;
		case eEaseInElastic:
			tweenFunction = easeInElastic;
			break;
		case eEaseOutElastic:
			tweenFunction = easeOutElastic;
			break;
		case eEaseInOutElastic:
			tweenFunction = easeInOutElastic;
			break;
		case eEaseOutInElastic:
			tweenFunction = easeOutInElastic;
			break;
		case eEaseInBack:
			tweenFunction = easeInBack;
			break;
		case eEaseOutBack:
			tweenFunction = easeOutBack;
			break;
		case eEaseInOutBack:
			tweenFunction = easeInOutBack;
			break;
		case eEaseOutInBack:
			tweenFunction = easeOutInBack;
			break;
		case eEaseOutBounce:
			tweenFunction = easeOutBounce;
			break;
		case eEaseInBounce:
			tweenFunction = easeInBounce;
			break;
		case eEaseInOutBounce:
			tweenFunction = easeInOutBounce;
			break;
		case eEaseOutInBounce:
			tweenFunction = easeOutInBounce;
			break;
		case eEaseInCurve:
			tweenFunction = easeInCurve;
			break;
		case eEaseOutCurve:
			tweenFunction = easeOutCurve;
			break;
		case eEaseSineCurve:
			tweenFunction = easeSineCurve;
			break;
		case eEaseCosineCurve:
			tweenFunction = easeCosineCurve;
			break;
	}

	return tweenFunction;
}

Parameter::Parameter(int param, float start, float end, TweenType tweenType) : 
	param(param),
	start(start),
	end(end)
{
	tweenFunction = getTweenFunction(tweenType);
}

Parameter::Parameter(const char* strparam, float start, float end, TweenType tweenType) : 
	param(StringId::instance().id(strparam)),
	start(start),
	end(end)
{
	strParam=strparam;
	tweenFunction = getTweenFunction(tweenType);
}

Parameter::Parameter(int param, float start, float end, const char* tweenType) :
	param(param),
	start(start),
	end(end)
{
	tweenFunction = getTweenFunction((TweenType)StringId::instance().id(tweenType));
}
Parameter::Parameter(const char* strparam, float start, float end, const char* tweenType) :
	param(StringId::instance().id(strparam)),
	start(start),
	end(end)
{
	strParam=strparam;
	tweenFunction = getTweenFunction((TweenType)StringId::instance().id(tweenType));
}

MovieClip::MovieClip(Type type, Application *application, bool holdWhilePlaying) : Sprite(application)
{
    type_ = type;
    playing_ = false;
    holdWhilePlaying_ = holdWhilePlaying;
}

MovieClip::~MovieClip()
{
	for (std::size_t i = 0; i < frames_.size(); ++i)
		frames_[i].sprite->unref();
}

void MovieClip::addFrame(int start, int end, Sprite* sprite, const std::vector<Parameter>& parameters, GStatus* status/* = NULL*/)
{
    switch (type_)
    {
    case eFrame:
        if (start < 1 || end < 1)
        {
            if (status)
                *status = GStatus(2100);		// Error #2100: Start and end frames must be greater than or equal to 1.
            return;
        }
        break;
    case eTime:
        if (start < 0 || end < 0)
        {
            if (status)
                *status = GStatus(2104);		// Error #2100: Start and end times must be greater than or equal to 0.
            return;
        }
        break;
    }

	if (start > end)
	{
		if (status)
            *status = GStatus(2101);		// Error #2101: End frame/time must be greater than or equal to start frame/time.
		return;
	}

	Frame frame;

	frame.start = start;
	frame.end = end;
	frame.parameters = parameters;
	frame.internalIndex=frames_.size();

#if 0
	frame.sprite = new Sprite;
	frame.sprite->addChild(sprite);
#else
	frame.sprite = sprite;
	frame.sprite->ref();
#endif

	frames_.push_back(frame);
}

void MovieClip::finalize()
{
    switch (type_)
    {
    case eFrame:
        maxframe_ = 1;
        break;
    case eTime:
        maxframe_ = 0;
        break;
    }

    for (std::size_t i = 0; i < frames_.size(); ++i)
	{
		allFrames_[frames_[i].start].push_back(&frames_[i]);
		maxframe_ = std::max(maxframe_, frames_[i].end);
	}

    switch (type_)
    {
    case eFrame:
        gotoAndPlay(1);
        break;
    case eTime:
        gotoAndPlay(0);
        break;
    }
}

bool MovieClip::oneFrame()
{
    if (!playing_)
    {
        return false;
    }

	if (passoneframe_ == true)
	{
		passoneframe_ = false;
		return false;
	}

	std::map<int, int>::iterator iter2 = actions_.find(frame_);
	if (iter2 != actions_.end())
	{
		bool unref=false;
        if (iter2->second == -1)
        {
			unref=stop(false);
            CompleteEvent event(CompleteEvent::COMPLETE);
            dispatchEvent(&event);
        }
		else
			gotoFrame(iter2->second);

		return unref;
	}

	if (frame_ == maxframe_)
	{
		bool unref=stop(false);
        CompleteEvent event(CompleteEvent::COMPLETE);
        dispatchEvent(&event);
        return unref;
	}

	std::map<int, std::vector<Frame*> >::iterator iter;

	iter = activeFrames_.find(frame_);
	if (iter != activeFrames_.end())
	{
		const std::vector<Frame*>& frames = iter->second;
		for (std::size_t i = 0; i < frames.size(); ++i)
			removeChild2(frames[i]->sprite);
		activeFrames_.erase(iter);
	}

	frame_ = frame_ + 1;

	iter = allFrames_.find(frame_);
	if (iter != allFrames_.end())
	{
		const std::vector<Frame*>& frames = iter->second;
		for (std::size_t i = 0; i < frames.size(); ++i)
		{
			activeFrames_[frames[i]->end].push_back(frames[i]);
			addChild2(frames[i]->sprite);
		}
	}
	
	interpolateParameters();

	return false;
}

void MovieClip::nextFrame(EnterFrameEvent *)
{
    switch (type_)
    {
    case eFrame:
        if (oneFrame()&&holdWhilePlaying_)
        	unref();
        break;
    case eTime:
    {
        double curr = iclock();
        int delta = (curr - prevClock_) * 1000;
        prevClock_ = curr;

        delta = std::min(std::max(delta, 0), 1000);

        for (int i = 0; i < delta; ++i)
            if (oneFrame()&&holdWhilePlaying_)
            {
            	unref();
            	return;
            }
        break;
    }
    }
}

void MovieClip::gotoFrame(int frame)
{
	std::map<int, std::vector<Frame*> >::iterator iter;

	for (iter = activeFrames_.begin(); iter != activeFrames_.end(); ++iter)
	{
		const std::vector<Frame*>& frames = iter->second;
		for (std::size_t i = 0; i < frames.size(); ++i)
			removeChild2(frames[i]->sprite);
	}

	activeFrames_.clear();

	frame_ = frame;

	for (std::size_t i = 0; i < frames_.size(); ++i)
	{
		int start = frames_[i].start;
		int end = frames_[i].end;

		if (start <= frame_ && frame_ <= end)
		{
			activeFrames_[end].push_back(&frames_[i]);
			addChild2(frames_[i].sprite);
		}
	}

	interpolateParameters();
}

void MovieClip::interpolateParameters()
{
	std::map<int, std::vector<Frame*> >::iterator iter;

	for (iter = activeFrames_.begin(); iter != activeFrames_.end(); ++iter)
	{
		const std::vector<Frame*>& frames = iter->second;
		for (std::size_t i = 0; i < frames.size(); ++i)
		{
			Sprite* sprite = frames[i]->sprite;

			int s = frames[i]->start;
			int e = frames[i]->end;

			float t = s == e ? 0 : ((float)(frame_ - s) / (float)(e - s));

			for (std::size_t j = 0; j < frames[i]->parameters.size(); ++j)
			{
				const Parameter& parameter = frames[i]->parameters[j];

				float s = parameter.start;
				float e = parameter.end;
				float tw = parameter.tweenFunction(t);
				float value = s * (1 - tw) + e * tw;

				setField(frames[i]->internalIndex,parameter, value);
			}
		}
	}
}


void MovieClip::setField(int frmIndex,Parameter param, float value)
{
	frames_[frmIndex].sprite->set(param.param,value);
}

float MovieClip::getField(int frmIndex,Parameter param)
{
	return frames_[frmIndex].sprite->get(param.param);
}

void MovieClip::gotoAndPlay(int frame)
{
	gotoFrame(frame);
	play();
}

void MovieClip::gotoAndStop(int frame)
{
	gotoFrame(frame);
	stop();
}

void MovieClip::play()
{
	passoneframe_ = true;
    prevClock_ = iclock();
    if (!playing_)
    {
    	playing_ = true;
    	addEventListener(EnterFrameEvent::ENTER_FRAME, &MovieClip::nextFrame);
    	if (holdWhilePlaying_)
    		ref();
    }
}

bool MovieClip::stop(bool unrefNow)
{
	passoneframe_ = false;
	if (playing_)
	{
		playing_ = false;
		removeEventListener(EnterFrameEvent::ENTER_FRAME, &MovieClip::nextFrame);
		if (unrefNow&&holdWhilePlaying_)
		{
			unref();
			return false;
		}
		return true;
	}
	return false;
}

void MovieClip::setStopAction(int frame)
{
    actions_[frame] = -1;
}

void MovieClip::setGotoAction(int frame, int destframe)
{
	actions_[frame] = destframe;
}

void MovieClip::clearAction(int frame)
{
	actions_.erase(frame);
}

void MovieClip::addChild2(Sprite* sprite)
{
	std::map<Sprite*, int>::iterator iter = counts_.find(sprite);

	if (iter == counts_.end() || iter->second == 0)
	{
#if 0
		addChild(sprite);
#else
		currentSprites_.push_back(sprite);
#endif
		counts_[sprite] = 1;
	}
	else
	{
		counts_[sprite]++;
	}
}

void MovieClip::removeChild2(Sprite* sprite)
{
	if (--counts_[sprite] == 0)
	{
#if 0
		removeChild(sprite);
#else
		currentSprites_.erase(std::find(currentSprites_.begin(), currentSprites_.end(), sprite));
#endif
	}
}

void MovieClip::doDraw(const CurrentTransform& transform, float sx, float sy, float ex, float ey)
{
	for (size_t i = 0; i < currentSprites_.size(); ++i)
        currentSprites_[i]->draw(transform, sx, sy, ex, ey);
}

void MovieClip::extraBounds(float* pminx, float* pminy, float* pmaxx, float* pmaxy) const
{
	float minx = 1e30;
	float miny = 1e30;
	float maxx = -1e30;
	float maxy = -1e30;

	for (size_t i = 0; i < currentSprites_.size(); ++i)
	{
		Sprite* sprite = currentSprites_[i];

		// get object bounds
		float ominx, ominy, omaxx, omaxy;
		sprite->objectBounds(&ominx, &ominy, &omaxx, &omaxy);

		// if empty bounds, continue
		if (ominx > omaxx || ominy > omaxy)
			continue;

		// transform 4 corners
		float x[4], y[4];
		sprite->matrix().transformPoint(ominx, ominy, &x[0], &y[0]);
		sprite->matrix().transformPoint(omaxx, ominy, &x[1], &y[1]);
		sprite->matrix().transformPoint(omaxx, omaxy, &x[2], &y[2]);
		sprite->matrix().transformPoint(ominx, omaxy, &x[3], &y[3]);

		// expand bounds
		for (int i = 0; i < 4; ++i)
		{
			minx = std::min(minx, x[i]);
			miny = std::min(miny, y[i]);
			maxx = std::max(maxx, x[i]);
			maxy = std::max(maxy, y[i]);
		}
	}

	if (pminx)
		*pminx = minx;
	if (pminy)
		*pminy = miny;
	if (pmaxx)
		*pmaxx = maxx;
	if (pmaxy)
		*pmaxy = maxy;
}

