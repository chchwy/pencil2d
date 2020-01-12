#ifndef MATHUTILS_H
#define MATHUTILS_H

#include <QtMath>
#include <QPoint>

namespace MathUtils
{
    /** Convert angles from radians to degrees.
     *
     * \param radians Angle in radians.
     * \return Angle in degrees.
     */
    inline qreal radToDeg(const qreal radians)
    {
        return radians * 180.0 / M_PI;
    }

    /** Convert angles from degrees to radians.
     *
     * \param degrees Angle in degrees.
     * \return Angle in radians.
     */
    inline qreal degToRad(const qreal degrees)
    {
        return degrees * M_PI / 180.0;
    }

    /** Get the angle from the difference vector a->b to the x-axis.
     *
     * \param a Start point of vector
     * \param b End point of vector
     * \return Angle in radians from [-pi,+pi]
     */
    inline qreal getDifferenceAngle(const QPointF a, const QPointF b)
    {
        return qAtan2(b.y() - a.y(), b.x() - a.x());
    }


    /**
     * @brief linearMap
     * affine transformation
     * formula: y = (x-a)*(d-c)/(b-a)+c
     * \param value The value that needs to be mapped
     * \param fromA The old min value
     * \param fromB The old max value
     * \param toC The new min value
     * \param toD The new max value
     * \return The value in the new range
     */
    inline qreal linearMap(qreal value, qreal fromA, qreal fromB, qreal toC, qreal toD)
    {
        return (value-fromA)*((toD-toC)/(fromB-fromA))+toC;
    }

    /**
     * @brief mapFromNormalized
     * maps value between 0-1 to any given range
     * \param value The value that needs to be mapped
     * \param newMin The new min value
     * \param newMax The new max value
     * \return The value in the new range
     */
    inline qreal mapFromNormalized(qreal value, qreal newMin, qreal newMax)
    {
        return linearMap(value, 0, 1, newMin, newMax);
    }

    inline qreal normalize(qreal value, qreal fromMin, qreal fromMax)
    {
        return (value-fromMin)/(fromMax-fromMin);
    }
}

#endif // MATHUTILS_H
