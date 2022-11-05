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
tuple<vector<string>, vector<string>> TutorialStep::getText(Game* game, UI* ui)
{
    throw runtime_error("getText() has not been defined for tutorial step '" + idName + "'.\n");
}

class SpawnBeaconStep : public TutorialStep
{
public:
    tuple<vector<string>, vector<string>> getText(Game* game, UI* ui)
    {
        return
        {
            {"Hey there! This tutorial will explain the basics of Coinfight.",
             "You can hide this tutorial (or show it again) anytime by hitting F1.",
             "Other than this playground/tutorial, Coinfight is always played in an arena against others, and is always played with real money. For now, pretend you've just deposited $4.50 into your account, and joined a game. This is what you'll see!",
             "The first step after joining a game will be to spawn in your first Gateway, with a one-time-use \"Beacon\". Do this now by hitting \"B\" and selecting a location. For now, choose a location outside of the fourth circle.",
            },
            {}
        };
    }

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
};

class CameraStep : public TutorialStep
{
public:
    tuple<vector<string>, vector<string>> getText(Game* game, UI* ui)
    {
        return
        {
            {
                "Nice! While that's spawning, you can move the camera around by dragging with the middle mouse button.",
                "Go ahead, wiggle 'er around a bit!"
            },
            {}
        };
    }

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
};

class SpawnFinishStep : public TutorialStep
{
public:
    tuple<vector<string>, vector<string>> getText(Game* game, UI* ui)
    {
        return
        {
            {
                "Just waiting for that Gateway to spawn in...",
                "Note that this is spending money from your Coinfight wallet. All told it will cost $4."
            },
            {
                "In a real game, everyone has only one Beacon--one chance to \"teleport in\" a Gateway like this anywhere on the map.",
                "Any additional Gateways will have to be built with units and resources on location."
            }
        };
    }

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
};

class BuildFirstPrimeStep : public TutorialStep
{
public:
    tuple<vector<string>, vector<string>> getText(Game* game, UI* ui)
    {
        return
        {
            {
                "Now that your Gateway is finished, you can build your first unit.",
                "Select your Gateway and hit the 'Q' key.",
                "This will build a Prime, the main worker/builder in Coinfight, for $0.50."
            },
            {}
        };
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
    tuple<vector<string>, vector<string>> getText(Game* game, UI* ui)
    {
        return
        {
            {
                "Bad news: you're broke! Good news: you have a Prime, and he can go get that gold nearby.",
                "Select your Prime and right-click on that gold pile to start picking it up.",
                "Pick up at least $0.50 to continue."
            },
            {
                "Gold is the only resource in Coinfight, and is backed by DAI. In a real game, your main focus will be on finding and securing gold, to either spend on units or withdraw as DAI."
            }
        };
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

class ReturnGoldStep : public TutorialStep
{
public:
    tuple<vector<string>, vector<string>> getText(Game* game, UI* ui)
    {
        return
        {
            {
                "Great! To finish capturing that gold, it has to be brought to the Gateway. With your Prime selected, right click on the Gateway."
            },
            {
                "The Prime is putting down gold within range of the Gateway, and the Gateway is pulling this gold out of the game, into your Coinfight wallet.",
                "Only Gateways can mediate between your Coinfight wallet and gold/units on the map. This means that if all your Gateways are lost, all the money you invested in your army will be stranded! So protect your Gateways at all costs."
            }
        };
    }

    ReturnGoldStep(Game* game, UI* ui)
        : TutorialStep("returngold", game, ui)
        {}
    
    void start(Game* game, UI* ui)
    {}

    void update(Game* game, UI* ui)
    {}

    void ping(int num)
    {}

    bool isFinished(Game* game, UI* ui)
    {
        return (game->players[0].credit.getInt() > dollarsToCoinsIntND(0.5));
    }

    optional<float> getProgress(Game* game, UI* ui)
    {
        return (float(game->players[0].credit.getInt()) / dollarsToCoinsIntND(0.5));
    }
};

// just here for copy/pasting convenience
class TutorialStepTemplate : public TutorialStep
{
public:
    tuple<vector<string>, vector<string>> getText(Game* game, UI* ui)
    {
        return
        {
            {

            },
            {}
        };
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
    steps.push_back(boost::shared_ptr<TutorialStep>(new ReturnGoldStep(game, ui)));


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