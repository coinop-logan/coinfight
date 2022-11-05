#include <vector>
#include <boost/shared_ptr.hpp>
#include "engine.h"
#include "input.h"

#ifndef TUTORIAL_H
#define TUTORIAL_H

using namespace std;

class TutorialStep
{
public:
    string idName;
    TutorialStep(string idName, Game* game, UI* ui);
    virtual void start(Game* game, UI* ui);
    virtual void update(Game* game, UI* ui);
    virtual void ping(int num);
    virtual bool isFinished(Game* game, UI* ui);
    virtual float getProgress(Game* game, UI* ui);
    virtual string getText(Game* game, UI* ui);
};

class Tutorial
{
    vector<boost::shared_ptr<TutorialStep>> steps;
    unsigned stepIter;
public:
    Tutorial(Game* game, UI* ui);
    void start(Game* game, UI* ui);
    void update(Game* game, UI* ui);
    void pingStep(string name, int num);
    bool isFinished();
    boost::shared_ptr<TutorialStep> currentStep()
    {
        if (stepIter < steps.size())
            return steps[stepIter];
        else
            throw "Trying get a step from the tutorial, but the tutorial is finished!";
    }
};

#endif