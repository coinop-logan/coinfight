#include <vector>
#include <boost/shared_ptr.hpp>

#include "entities.h"
#include "collision.h"
#include "myvectors.h"
#include "fpm/math.hpp"

fixed32 sqr(fixed32 x)
{
    return x*x;
}

vector2fp calcNewVelocityToAvoidCollisions(boost::shared_ptr<MobileUnit> unit, vector<boost::shared_ptr<Unit>> nearbyUnits, fixed32 timeHorizon, fixed32 timeStep)
{
    vector<Line> orcaLines;

    const fixed32 invTimeHorizon = fixed32(1) / timeHorizon;

    /* Create agent ORCA lines. */
    for (size_t i = 0; i < nearbyUnits.size(); ++i) {
        boost::shared_ptr<Unit> other = nearbyUnits[i];

        vector2fp relativePosition = other->getPos() - unit->getPos();
        vector2fp otherVelocity;
        if (auto mobileUnit = boost::dynamic_pointer_cast<MobileUnit, Unit>(other))
        {
            if (mobileUnit->isActive())
            {
                otherVelocity = mobileUnit->getLastVelocity();
            }
            else
            {
                otherVelocity = vector2fp::zero;
            }
        }
        else
        {
            otherVelocity = vector2fp::zero;
        }
        vector2fp relativeVelocity = unit->getLastVelocity() - otherVelocity;
        uint32_t distSq = relativePosition.getFloorMagnitudeSquared();
        fixed32 combinedRadius = unit->getRadius() + other->getRadius();
        uint32_t combinedRadiusSq = floorSquareFixed(combinedRadius);

        Line line;
        vector2fp u;

        if (distSq > combinedRadiusSq) {
            /* No collision. */
            vector2fp w = relativeVelocity - invTimeHorizon * relativePosition;
            /* Vector from cutoff center to relative velocity. */
            const uint32_t wLengthSq = w.getFloorMagnitudeSquared();

            const fixed32 dotProduct1 = w * relativePosition;

            if (dotProduct1 < fixed32(0) && floorSquareFixed(dotProduct1) > combinedRadiusSq * wLengthSq) {
                /* Project on cut-off circle. */
                const fixed32 wLength = w.getRoughMagnitude();
                const vector2fp unitW = w / wLength;

                line.direction = vector2fp(unitW.y, -unitW.x);
                u = (combinedRadius * invTimeHorizon - wLength) * unitW;
            }
            else {
                /* Project on legs. */
                const fixed32 leg(newtonSqrtFloor(distSq - combinedRadiusSq));

                if (det(relativePosition, w) > fixed32(0)) {
                    /* Project on left leg. */
                    line.direction = vector2fp(relativePosition.x * leg - relativePosition.y * combinedRadius, relativePosition.x * combinedRadius + relativePosition.y * leg) / distSq;
                }
                else {
                    /* Project on right leg. */
                    line.direction = -vector2fp(relativePosition.x * leg + relativePosition.y * combinedRadius, -relativePosition.x * combinedRadius + relativePosition.y * leg) / distSq;
                }

                const fixed32 dotProduct2 = relativeVelocity * line.direction;

                u = dotProduct2 * line.direction - relativeVelocity;
            }
        }
        else {
            /* Collision. Project on cut-off circle of time timeStep. */
            const fixed32 invTimeStep = fixed32(1) / timeStep;

            /* Vector from cutoff center to relative velocity. */
            vector2fp w = relativeVelocity - invTimeStep * relativePosition;

            fixed32 wLength = w.getRoughMagnitude();
            if (wLength == fixed32(0))
            {
                // hacky fix to avoid a divide by zero: pretend we're almost at a zero vector but not quite.
                w = vector2fp(fixed32::from_raw_value(0), fixed32::from_raw_value(1));
                wLength = w.getRoughMagnitude();
            }
            const vector2fp unitW = w / wLength;

            line.direction = vector2fp(unitW.y, -unitW.x);
            u = (combinedRadius * invTimeStep - wLength) * unitW;
        }

        line.point = unit->getLastVelocity() + (u / 2);
        orcaLines.push_back(line);
    }

    vector2fp newVelocity;

    size_t lineFail = linearProgram2(orcaLines, unit->getMaxSpeed(), unit->getDesiredVelocity(), false, newVelocity);

    if (lineFail < orcaLines.size()) {
        linearProgram3(orcaLines, 0, lineFail, unit->getMaxSpeed(), newVelocity);
    }

    return newVelocity;
}

bool linearProgram1(const std::vector<Line> &lines, size_t lineNo, fixed32 radius, const vector2fp &optVelocity, bool directionOpt, vector2fp &result)
{
    const fixed32 dotProduct = lines[lineNo].point * lines[lineNo].direction;
    const fixed32 discriminant = sqr(dotProduct) + sqr(radius) - lines[lineNo].point.getFloorMagnitudeSquared();

    if (discriminant < fixed32(0)) {
        /* Max speed circle fully invalidates line lineNo. */
        return false;
    }

    const fixed32 sqrtDiscriminant = sqrt(discriminant);
    fixed32 tLeft = -dotProduct - sqrtDiscriminant;
    fixed32 tRight = -dotProduct + sqrtDiscriminant;

    for (size_t i = 0; i < lineNo; ++i) {
        const fixed32 denominator = det(lines[lineNo].direction, lines[i].direction);
        const fixed32 numerator = det(lines[i].direction, lines[lineNo].point - lines[i].point);

        if (abs(denominator) <= RVO_EPSILON) {
            /* Lines lineNo and i are (almost) parallel. */
            if (numerator < fixed32(0)) {
                return false;
            }
            else {
                continue;
            }
        }

        const fixed32 t = numerator / denominator;

        if (denominator >= fixed32(0)) {
            /* Line i bounds line lineNo on the right. */
            tRight = std::min(tRight, t);
        }
        else {
            /* Line i bounds line lineNo on the left. */
            tLeft = std::max(tLeft, t);
        }

        if (tLeft > tRight) {
            return false;
        }
    }

    if (directionOpt) {
        /* Optimize direction. */
        if (optVelocity * lines[lineNo].direction > fixed32(0)) {
            /* Take right extreme. */
            result = lines[lineNo].point + tRight * lines[lineNo].direction;
        }
        else {
            /* Take left extreme. */
            result = lines[lineNo].point + tLeft * lines[lineNo].direction;
        }
    }
    else {
        /* Optimize closest point. */
        const fixed32 t = lines[lineNo].direction * (optVelocity - lines[lineNo].point);

        if (t < tLeft) {
            result = lines[lineNo].point + tLeft * lines[lineNo].direction;
        }
        else if (t > tRight) {
            result = lines[lineNo].point + tRight * lines[lineNo].direction;
        }
        else {
            result = lines[lineNo].point + t * lines[lineNo].direction;
        }
    }

    return true;
}

size_t linearProgram2(const std::vector<Line> &lines, fixed32 radius, const vector2fp &optVelocity, bool directionOpt, vector2fp &result)
{
    if (directionOpt) {
        /*
            * Optimize direction. Note that the optimization velocity is of unit
            * length in this case.
            */
        result = optVelocity * radius;
    }
    else if (optVelocity.getFloorMagnitudeSquared() > floorSquareFixed(radius)) {
        /* Optimize closest point and outside circle. */
        result = optVelocity.normalized() * radius;
    }
    else {
        /* Optimize closest point and inside circle. */
        result = optVelocity;
    }

    for (size_t i = 0; i < lines.size(); ++i) {
        if (det(lines[i].direction, lines[i].point - result) > fixed32(0)) {
            /* Result does not satisfy constraint i. Compute new optimal result. */
            const vector2fp tempResult = result;

            if (!linearProgram1(lines, i, radius, optVelocity, directionOpt, result)) {
                result = tempResult;
                return i;
            }
        }
    }

    return lines.size();
}

void linearProgram3(const std::vector<Line> &lines, size_t numObstLines, size_t beginLine, fixed32 radius, vector2fp &result)
{
    fixed32 distance(0);

    for (size_t i = beginLine; i < lines.size(); ++i) {
        if (det(lines[i].direction, lines[i].point - result) > distance) {
            /* Result does not satisfy constraint of line i. */
            std::vector<Line> projLines(lines.begin(), lines.begin() + static_cast<ptrdiff_t>(numObstLines));

            for (size_t j = numObstLines; j < i; ++j) {
                Line line;

                fixed32 determinant = det(lines[i].direction, lines[j].direction);

                if (abs(determinant) <= RVO_EPSILON) {
                    /* Line i and line j are parallel. */
                    if (lines[i].direction * lines[j].direction > fixed32(0)) {
                        /* Line i and line j point in the same direction. */
                        continue;
                    }
                    else {
                        /* Line i and line j point in opposite direction. */
                        line.point = (lines[i].point + lines[j].point) / 2;
                    }
                }
                else {
                    line.point = lines[i].point + (det(lines[j].direction, lines[i].point - lines[j].point) / determinant) * lines[i].direction;
                }

                line.direction = (lines[j].direction - lines[i].direction).normalized();
                projLines.push_back(line);
            }

            const vector2fp tempResult = result;

            if (linearProgram2(projLines, radius, vector2fp(-lines[i].direction.y, lines[i].direction.x), true, result) < projLines.size()) {
                /* This should in principle not happen.  The result is by definition
                    * already in the feasible region of this linear program. If it fails,
                    * it is due to small floating point error, and the current result is
                    * kept.
                    */
                result = tempResult;
            }

            distance = det(lines[i].direction, lines[i].point - result);
        }
    }
}