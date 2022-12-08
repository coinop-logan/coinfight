#include <vector>
#include <boost/shared_ptr.hpp>
#include "engine.h"
#include "input.h"

#ifndef TUTORIAL_H
#define TUTORIAL_H

using namespace std;

const string TUTORIAL_PLAYER_ADDRESS_STR = "0x0f0f00f000f00f00f000f00f00f000f00f00f000";
const string TUTORIAL_OTHER_ADDRESS_STR = "0xf00f00f000f00f00f000f00f00f000f00f00f000";

void setupTutorialScenario(Game* game);

class TutorialStep
{
public:
    string idName;
    bool waitForEnter;
    TutorialStep(string idName, bool waitForEnter, Game* game, GameUI* ui);
    virtual void start(Game* game, GameUI* ui);
    virtual void update(Game* game, GameUI* ui);
    virtual void ping(int num);
    virtual bool isReadyToFinish(Game* game, GameUI* ui);
    virtual optional<float> getProgress(Game* game, GameUI* ui);
    virtual tuple<vector<string>, vector<string>> getText(Game* game, GameUI* ui);
};

class Tutorial
{
    vector<boost::shared_ptr<TutorialStep>> steps;
    unsigned stepIter;
    Game* game;
    GameUI* ui;
public:
    Tutorial(Game* game, GameUI* ui);
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