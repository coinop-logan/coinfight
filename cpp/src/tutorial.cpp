#include <sstream>
#include "tutorial.h"
#include "myvectors.h"

using namespace std;

const float REQUIRED_CAMERA_MOVE = 1000;

TutorialStep::TutorialStep(string idName, Game* game, UI* ui):
    idName(idName) {}

void TutorialStep::start(Game* game, UI* ui)
{
    throw runtime_error("start() has not been defined for tutorial step '" + idName + "'.\n");
}
void TutorialStep::update(Game* game, UI* ui)
{
    throw runtime_error("update() has not been defined for tutorial step '" + idName + "'.\n");
}
void TutorialStep::ping(int num)
{
    throw runtime_error("ping() has not been defined for tutorial step '" + idName + "'.\n");
}
bool TutorialStep::isFinished(Game* game, UI* ui)
{
    throw runtime_error("isFinished() has not been defined for tutorial step '" + idName + "'.\n");
}
float TutorialStep::getProgress(Game* game, UI* ui)
{
    throw runtime_error("progress() has not been defined for tutorial step '" + idName + "'.\n");
}
string TutorialStep::getText(Game* game, UI* ui)
{
    throw runtime_error("getText() has not been defined for tutorial step '" + idName + "'.\n");
}

class Step1 : public TutorialStep
{
public:
    vector2i lastCameraPos;
    float totalDistanceMoved;
    Step1(Game* game, UI* ui):
        TutorialStep("camera", game, ui),
        lastCameraPos(ui->camera.gamePos),
        totalDistanceMoved(0)
    {}

    void start(Game* game, UI* ui)
    {}

    void update(Game* game, UI* ui)
    {
        vector2i newCameraPos = ui->camera.gamePos;
        totalDistanceMoved += (lastCameraPos - newCameraPos).getMagnitude();
        lastCameraPos = newCameraPos;
    }

    void ping(int num)
    {}

    bool isFinished(Game* game, UI* ui)
    {
        return totalDistanceMoved >= REQUIRED_CAMERA_MOVE;
    }

    float getProgress(Game* game, UI* ui)
    {
        return totalDistanceMoved / REQUIRED_CAMERA_MOVE;
    }

    string getText(Game* game, UI* ui)
    {
        return "You can move the camera around by dragging with the middle mouse button.\nGo ahead, wiggle 'er around a bit!";
    }
};

Tutorial::Tutorial(Game* game, UI* ui)
{
    steps.push_back(boost::shared_ptr<TutorialStep>(new Step1(game, ui)));

    stepIter = 0;
}
void Tutorial::start(Game* game, UI* ui)
{
    stepIter = 0;
    steps[stepIter]->start(game, ui);
}
void Tutorial::update(Game* game, UI* ui)
{
    currentStep()->update(game, ui);
    if (currentStep()->isFinished(game, ui))
    {
        stepIter ++;
        if (stepIter < steps.size())
        {
            currentStep()->start(game, ui);
        }
    }
}
void Tutorial::pingStep(string name, int num)
{
    if (currentStep()->idName == name)
    {
        currentStep()->ping(num);
    }
}
bool Tutorial::isFinished()
{
    return (stepIter >= steps.size());
}