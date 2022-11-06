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
    bool waitForEnter;
    TutorialStep(string idName, bool waitForEnter, Game* game, UI* ui);
    virtual void start(Game* game, UI* ui);
    virtual void update(Game* game, UI* ui);
    virtual void ping(int num);
    virtual bool isReadyToFinish(Game* game, UI* ui);
    virtual optional<float> getProgress(Game* game, UI* ui);
    virtual tuple<vector<string>, vector<string>> getText(Game* game, UI* ui);
};

class Tutorial
{
    vector<boost::shared_ptr<TutorialStep>> steps;
    unsigned stepIter;
    Game* game;
    UI* ui;
public:
    Tutorial(Game* game, UI* ui);
    void start();
    void stepForward();
    void update();
    void pingStep(string name, int num);
    bool isFinished();
    void enterPressed();
    boost::shared_ptr<TutorialStep> currentStep()
    {
        if (stepIter < steps.size())
            return steps[stepIter];
        else
            throw "Trying get a step from the tutorial, but the tutorial is finished!";
    }
};

#endif