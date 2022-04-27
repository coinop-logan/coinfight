#include <vector>
#include <boost/shared_ptr.hpp>

#include "entities.h"
#include "collision.h"
#include "myvectors.h"

float sqr(float x)
{
    return x*x;
}

vector2f calcNewVelocityToAvoidCollisions(boost::shared_ptr<MobileUnit> unit, vector<boost::shared_ptr<Unit>> nearbyUnits, float timeHorizon, float timeStep)
{
    vector<Line> orcaLines;

    const float invTimeHorizon = 1.0f / timeHorizon;

    /* Create agent ORCA lines. */
    for (size_t i = 0; i < nearbyUnits.size(); ++i) {
        boost::shared_ptr<Unit> other = nearbyUnits[i];

        vector2f relativePosition = other->getPos() - unit->getPos();
        vector2f otherVelocity;
        if (auto mobileUnit = boost::dynamic_pointer_cast<MobileUnit, Unit>(other))
        {
            otherVelocity = mobileUnit->getLastVelocity();
        }
        else
        {
            otherVelocity = vector2f(0,0);
        }
        vector2f relativeVelocity = unit->getLastVelocity() - otherVelocity;
        float distSq = relativePosition.getMagnitudeSquared();
        float combinedRadius = unit->getRadius() + other->getRadius();
        const float combinedRadiusSq = sqr(combinedRadius);

        Line line;
        vector2f u;

        if (distSq > combinedRadiusSq) {
            /* No collision. */
            vector2f w = relativeVelocity - invTimeHorizon * relativePosition;
            /* Vector from cutoff center to relative velocity. */
            const float wLengthSq = w.getMagnitudeSquared();

            const float dotProduct1 = w * relativePosition;

            if (dotProduct1 < 0.0f && sqr(dotProduct1) > combinedRadiusSq * wLengthSq) {
                /* Project on cut-off circle. */
                const float wLength = std::sqrt(wLengthSq);
                const vector2f unitW = w / wLength;

                line.direction = vector2f(unitW.y, -unitW.x);
                u = (combinedRadius * invTimeHorizon - wLength) * unitW;
            }
            else {
                /* Project on legs. */
                const float leg = std::sqrt(distSq - combinedRadiusSq);

                if (det(relativePosition, w) > 0.0f) {
                    /* Project on left leg. */
                    line.direction = vector2f(relativePosition.x * leg - relativePosition.y * combinedRadius, relativePosition.x * combinedRadius + relativePosition.y * leg) / distSq;
                }
                else {
                    /* Project on right leg. */
                    line.direction = -vector2f(relativePosition.x * leg + relativePosition.y * combinedRadius, -relativePosition.x * combinedRadius + relativePosition.y * leg) / distSq;
                }

                const float dotProduct2 = relativeVelocity * line.direction;

                u = dotProduct2 * line.direction - relativeVelocity;
            }
        }
        else {
            /* Collision. Project on cut-off circle of time timeStep. */
            const float invTimeStep = 1.0f / timeStep;

            /* Vector from cutoff center to relative velocity. */
            vector2f w = relativeVelocity - invTimeStep * relativePosition;

            const float wLength = w.getMagnitude();
            const vector2f unitW = w / wLength;

            line.direction = vector2f(unitW.y, -unitW.x);
            u = (combinedRadius * invTimeStep - wLength) * unitW;
        }

        line.point = unit->getLastVelocity() + 0.5f * u;
        orcaLines.push_back(line);
    }

    vector2f newVelocity;

    size_t lineFail = linearProgram2(orcaLines, unit->getMaxSpeed(), unit->getDesiredVelocity(), false, newVelocity);

    if (lineFail < orcaLines.size()) {
        linearProgram3(orcaLines, 0, lineFail, unit->getMaxSpeed(), newVelocity);
    }

    return newVelocity;
}

bool linearProgram1(const std::vector<Line> &lines, size_t lineNo, float radius, const vector2f &optVelocity, bool directionOpt, vector2f &result)
{
    const float dotProduct = lines[lineNo].point * lines[lineNo].direction;
    const float discriminant = sqr(dotProduct) + sqr(radius) - lines[lineNo].point.getMagnitudeSquared();

    if (discriminant < 0.0f) {
        /* Max speed circle fully invalidates line lineNo. */
        return false;
    }

    const float sqrtDiscriminant = std::sqrt(discriminant);
    float tLeft = -dotProduct - sqrtDiscriminant;
    float tRight = -dotProduct + sqrtDiscriminant;

    for (size_t i = 0; i < lineNo; ++i) {
        const float denominator = det(lines[lineNo].direction, lines[i].direction);
        const float numerator = det(lines[i].direction, lines[lineNo].point - lines[i].point);

        if (std::fabs(denominator) <= RVO_EPSILON) {
            /* Lines lineNo and i are (almost) parallel. */
            if (numerator < 0.0f) {
                return false;
            }
            else {
                continue;
            }
        }

        const float t = numerator / denominator;

        if (denominator >= 0.0f) {
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
        if (optVelocity * lines[lineNo].direction > 0.0f) {
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
        const float t = lines[lineNo].direction * (optVelocity - lines[lineNo].point);

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

size_t linearProgram2(const std::vector<Line> &lines, float radius, const vector2f &optVelocity, bool directionOpt, vector2f &result)
{
    if (directionOpt) {
        /*
            * Optimize direction. Note that the optimization velocity is of unit
            * length in this case.
            */
        result = optVelocity * radius;
    }
    else if (optVelocity.getMagnitudeSquared() > sqr(radius)) {
        /* Optimize closest point and outside circle. */
        result = optVelocity.normalized() * radius;
    }
    else {
        /* Optimize closest point and inside circle. */
        result = optVelocity;
    }

    for (size_t i = 0; i < lines.size(); ++i) {
        if (det(lines[i].direction, lines[i].point - result) > 0.0f) {
            /* Result does not satisfy constraint i. Compute new optimal result. */
            const vector2f tempResult = result;

            if (!linearProgram1(lines, i, radius, optVelocity, directionOpt, result)) {
                result = tempResult;
                return i;
            }
        }
    }

    return lines.size();
}

void linearProgram3(const std::vector<Line> &lines, size_t numObstLines, size_t beginLine, float radius, vector2f &result)
{
    float distance = 0.0f;

    for (size_t i = beginLine; i < lines.size(); ++i) {
        if (det(lines[i].direction, lines[i].point - result) > distance) {
            /* Result does not satisfy constraint of line i. */
            std::vector<Line> projLines(lines.begin(), lines.begin() + static_cast<ptrdiff_t>(numObstLines));

            for (size_t j = numObstLines; j < i; ++j) {
                Line line;

                float determinant = det(lines[i].direction, lines[j].direction);

                if (std::fabs(determinant) <= RVO_EPSILON) {
                    /* Line i and line j are parallel. */
                    if (lines[i].direction * lines[j].direction > 0.0f) {
                        /* Line i and line j point in the same direction. */
                        continue;
                    }
                    else {
                        /* Line i and line j point in opposite direction. */
                        line.point = 0.5f * (lines[i].point + lines[j].point);
                    }
                }
                else {
                    line.point = lines[i].point + (det(lines[j].direction, lines[i].point - lines[j].point) / determinant) * lines[i].direction;
                }

                line.direction = (lines[j].direction - lines[i].direction).normalized();
                projLines.push_back(line);
            }

            const vector2f tempResult = result;

            if (linearProgram2(projLines, radius, vector2f(-lines[i].direction.y, lines[i].direction.x), true, result) < projLines.size()) {
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