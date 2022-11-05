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
optional<float> TutorialStep::getProgress(Game* game, UI* ui)
{
    return {};
}
string TutorialStep::getText(Game* game, UI* ui)
{
    throw runtime_error("getText() has not been defined for tutorial step '" + idName + "'.\n");
}

class SpawnBeaconStep : public TutorialStep
{
public:
    SpawnBeaconStep(Game* game, UI* ui)
        : TutorialStep("beacon", game, ui)
    {}
    
    void start(Game* game, UI* ui)
    {}

    void update(Game* game, UI* ui)
    {}

    bool isFinished(Game* game, UI* ui)
    {
        return (game->entities.size() > 1);
    }

    string getText(Game* game, UI* ui)
    {
        stringstream ss;
        ss << "Hey there! This tutorial will explain the basics of Coinfight." << endl;
        ss << "You can hide this tutorial (or show it again) anytime by hitting F1." << endl;
        ss << endl;
        ss << "Other than this playground/tutorial, Coinfight is always played in an arena against others, and is always played with real money." << endl;
        ss << "For now, pretend you've just deposited $4.50 into your account, and joined a game. This is what you'll see!" << endl;
        ss << "The first step after joining a game will be to spawn in your first Gateway, with a one-time-use \"Beacon\"." << endl;
        ss << "Do this now by hitting \"B\" and selecting a location. For now, choose a location outside of the fourth circle." << endl;

        return ss.str();
    }
};

class CameraStep : public TutorialStep
{
public:
    vector2i lastCameraPos;
    float totalDistanceMoved;
    CameraStep(Game* game, UI* ui):
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

    optional<float> getProgress(Game* game, UI* ui)
    {
        return totalDistanceMoved / REQUIRED_CAMERA_MOVE;
    }

    string getText(Game* game, UI* ui)
    {
        stringstream ss;
        ss << "Nice! While that's spawning, you can move the camera around by dragging with the middle mouse button." << endl;
        ss << "Go ahead, wiggle 'er around a bit!";

        return ss.str();
    }
};

class SpawnFinishStep : public TutorialStep
{
public:
    SpawnFinishStep(Game* game, UI* ui):
        TutorialStep("spawnfinish", game, ui)
    {}

    void start(Game* game, UI* ui)
    {}

    void update(Game* game, UI* ui)
    {}

    void ping(int num)
    {}

    bool isFinished(Game* game, UI* ui)
    {
        if (game->entities.size() < 3)
        {
            return false;
        }
        else
        {
            if (auto gateway = boost::dynamic_pointer_cast<Gateway, Entity>(game->entities[2]))
            {
                return true;
            }
            else
            {
                return false;
            }
        }
    }

    optional<float> getProgress(Game* game, UI* ui)
    {
        if (game->entities.size() < 2)
        {
            return 0;
        }
        else if (auto beacon = boost::dynamic_pointer_cast<Beacon, Entity>(game->entities[1]))
        {
            return float(beacon->getBuiltRatio());
        }
        else if (game->entities.size() > 2)
        {
            if (auto gateway = boost::dynamic_pointer_cast<Gateway, Entity>(game->entities[1]))
            {
                return 1;
            }
        }
        return 0;
    }

    string getText(Game* game, UI* ui)
    {
        stringstream ss;
        ss << "Just waiting for that Gateway to spawn in..." << endl;
        ss << "Note that this is spending money from your Coinfight wallet. All told it will cost $4." << endl;
        ss << endl;
        ss << "In a real game, everyone only one Beacon--one chance to \"teleport in\" to anywhere on the map." << endl;
        ss << "Any additional Gateways will have to be built with resources on location." << endl;

        return ss.str();
    }
};

class BuildFirstPrimeStep : public TutorialStep
{
public:
    string getText(Game* game, UI* ui)
    {
        stringstream ss;
        
        ss << "Now that your Gateway is finished, you can build your first unit." << endl;
        ss << "Select your Gateway and hit the 'Q' key." << endl;
        ss << "This will build a Prime, the main worker/builder in Coinfight, for $0.50." << endl;

        return ss.str();
    }

    BuildFirstPrimeStep(Game* game, UI* ui)
        : TutorialStep("firstprime", game, ui)
        {}
    
    void start(Game* game, UI* ui)
    {}

    void update(Game* game, UI* ui)
    {}

    void ping(int num)
    {}

    bool isFinished(Game* game, UI* ui)
    {
        for (unsigned int i=0; i<game->entities.size(); i++)
        {
            if (auto entity = game->entities[i])
            {
                if (auto unit = boost::dynamic_pointer_cast<Unit, Entity>(entity))
                {
                    if (unit->ownerId == 0 && unit->typechar() == PRIME_TYPECHAR && unit->getBuiltRatio() == fixed32(1))
                    {
                        return true;
                    }
                }
            }
        }
        return false;
    }

    optional<float> getProgress(Game* game, UI* ui)
    {
        for (unsigned int i=0; i<game->entities.size(); i++)
        {
            if (auto entity = game->entities[i])
            {
                if (auto unit = boost::dynamic_pointer_cast<Unit, Entity>(entity))
                {
                    if (unit->ownerId == 0 && unit->typechar() == PRIME_TYPECHAR)
                    {
                        return float(unit->getBuiltRatio());
                    }
                }
            }
        }
        return 0;
    }
};

class PickupGoldStep : public TutorialStep
{
public:
    string getText(Game* game, UI* ui)
    {
        stringstream ss;
        
        ss << "Bad news: you're broke! Good news: you have a Prime, and he can go get that gold nearby." << endl;
        ss << "Select your Prime and right-click on that gold pile to start picking it up." << endl;
        ss << "Pick up at least $0.50 to continue." << endl;

        return ss.str();
    }

    PickupGoldStep(Game* game, UI* ui)
        : TutorialStep("pickupgold", game, ui)
        {}
    
    void start(Game* game, UI* ui)
    {}

    void update(Game* game, UI* ui)
    {}

    void ping(int num)
    {}

    coinsInt getTotalGoldGathered(Game *game)
    {
        coinsInt total = 0;

        for (unsigned int i=0; i<game->entities.size(); i++)
        {
            if (auto entity = game->entities[i])
            {
                if (auto prime = boost::dynamic_pointer_cast<Prime, Entity>(entity))
                {
                    total += prime->heldGold.getInt();
                }
            }
        }

        return total;
    }
    bool isFinished(Game* game, UI* ui)
    {
        return (getTotalGoldGathered(game) >= dollarsToCoinsIntND(0.5));
    }

    optional<float> getProgress(Game* game, UI* ui)
    {
        return (float(getTotalGoldGathered(game)) / dollarsToCoinsIntND(0.5));
    }
};

// just here for copy/pasting convenience
class TutorialStepTemplate : public TutorialStep
{
public:
    string getText(Game* game, UI* ui)
    {
        stringstream ss;
        
        ss << "some text here!" << endl;

        return ss.str();
    }

    TutorialStepTemplate(Game* game, UI* ui)
        : TutorialStep("name", game, ui)
        {}
    
    void start(Game* game, UI* ui)
    {}

    void update(Game* game, UI* ui)
    {}

    void ping(int num)
    {}

    bool isFinished(Game* game, UI* ui)
    {
        return false;
    }

    optional<float> getProgress(Game* game, UI* ui)
    {
        return {};
    }
};

Tutorial::Tutorial(Game* game, UI* ui)
{
    steps.push_back(boost::shared_ptr<TutorialStep>(new SpawnBeaconStep(game, ui)));
    steps.push_back(boost::shared_ptr<TutorialStep>(new CameraStep(game, ui)));
    steps.push_back(boost::shared_ptr<TutorialStep>(new SpawnFinishStep(game, ui)));
    steps.push_back(boost::shared_ptr<TutorialStep>(new BuildFirstPrimeStep(game, ui)));
    steps.push_back(boost::shared_ptr<TutorialStep>(new PickupGoldStep(game, ui)));


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