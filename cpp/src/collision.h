#include <boost/shared_ptr.hpp>

#include "entities.h"
#include "myvectors.h"

/*
 * This code is largely adapted from the RVO2 codebase found at https://github.com/snape/RVO2,
 * which contains the following license information
 * 
 *
 * RVO.h
 * RVO2 Library
 *
 * Copyright 2008 University of North Carolina at Chapel Hill
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 *
 * Please send all bug reports to <geom@cs.unc.edu>.
 *
 * The authors may be contacted via:
 *
 * Jur van den Berg, Stephen J. Guy, Jamie Snape, Ming C. Lin, Dinesh Manocha
 * Dept. of Computer Science
 * 201 S. Columbia St.
 * Frederick P. Brooks, Jr. Computer Science Bldg.
 * Chapel Hill, N.C. 27599-3175
 * United States of America
 *
 * <http://gamma.cs.unc.edu/RVO2/>
 */

#ifndef COLLISION_H
#define COLLISION_H

using namespace std;

const fixed32 RVO_EPSILON(0.0001);

struct Line {
    vector2fp point;
    vector2fp direction;
};

vector2fp calcNewVelocityToAvoidCollisions(boost::shared_ptr<MobileUnit> unit, vector<boost::shared_ptr<Unit>> nearbyUnits, fixed32 timeHorizon, fixed32 timeStep);

bool linearProgram1(const std::vector<Line> &lines, size_t lineNo, fixed32 radius, const vector2fp &optVelocity, bool directionOpt, vector2fp &result);
size_t linearProgram2(const std::vector<Line> &lines, fixed32 radius, const vector2fp &optVelocity, bool directionOpt, vector2fp &result);
void linearProgram3(const std::vector<Line> &lines, size_t numObstLines, size_t beginLine, fixed32 radius, vector2fp &result);

#endif