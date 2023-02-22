#include <sstream>
#include "tutorial.h"
#include "myvectors.h"

using namespace std;

void setupTutorialScenario(Game* game)
{
    game->players.push_back(Player(Address(TUTORIAL_PLAYER_ADDRESS_STR)));
    game->players.push_back(Player(Address(TUTORIAL_OTHER_ADDRESS_STR)));

    game->players[0].credit.createMoreByFiat(dollarsToCoinsIntND(4.5));
}

TutorialStep::TutorialStep(string idName, bool waitForEnter, Game* game, GameUI* ui):
    idName(idName), waitForEnter(waitForEnter) {}

void TutorialStep::start(Game* game, GameUI* ui)
{
    throw runtime_error("start() has not been defined for tutorial step '" + idName + "'.\n");
}
void TutorialStep::update(Game* game, GameUI* ui)
{
    throw runtime_error("update() has not been defined for tutorial step '" + idName + "'.\n");
}
void TutorialStep::ping(int num)
{
    throw runtime_error("ping() has not been defined for tutorial step '" + idName + "'.\n");
}
bool TutorialStep::isReadyToFinish(Game* game, GameUI* ui)
{
    throw runtime_error("isReadyToFinish() has not been defined for tutorial step '" + idName + "'.\n");
}
optional<float> TutorialStep::getProgress(Game* game, GameUI* ui)
{
    return {};
}
tuple<vector<string>, vector<string>> TutorialStep::getText(Game* game, GameUI* ui)
{
    throw runtime_error("getText() has not been defined for tutorial step '" + idName + "'.\n");
}

class CameraStep : public TutorialStep
{
public:
    float distanceMoved;
    vector2i lastCameraPos;

    CameraStep(Game* game, GameUI* ui):
        TutorialStep("camera", false, game, ui)
    {
        distanceMoved = 0;
        lastCameraPos = fromSFVec(ui->cameraView.getCenter());
    }

    tuple<vector<string>, vector<string>> getText(Game* game, GameUI* ui)
    {
        return
        {
            {
                "Hey there! This tutorial will explain the basics of Coinfight. You can hide this tutorial (or show it again) anytime by hitting F1.",
                "First, move the camera by dragging with the middle mouse button, or by moving your mouse to the edges of the screen.",
                "Go ahead, wiggle 'er around a bit!"
            },
            {}
        };
    }

    void start(Game* game, GameUI* ui)
    {}

    void update(Game* game, GameUI* ui)
    {
        distanceMoved += (lastCameraPos - fromSFVec(ui->cameraView.getCenter())).getMagnitude();
        lastCameraPos = fromSFVec(ui->cameraView.getCenter());
    }

    void ping(int num)
    {}

    bool isReadyToFinish(Game* game, GameUI* ui)
    {
        return distanceMoved >= 2000;
    }

    optional<float> getProgress(Game* game, GameUI* ui)
    {
        return distanceMoved / 2000.0;
    }
};

class SpawnBeaconStep : public TutorialStep
{
public:
    SpawnBeaconStep(Game* game, GameUI* ui)
        : TutorialStep("beacon", false, game, ui)
    {}

    tuple<vector<string>, vector<string>> getText(Game* game, GameUI* ui)
    {
        return
        {
            {
                "A real Coinfight game will require crypto, but for now we'll pretend you deposited $4.50 into your Coinfight wallet - see your balance on the upper left.",
                "The first step after joining a game will be to spawn in your first Gateway. See the button for this near the lower-left of your screen.",
            },
            {}
        };
    }
    
    void start(Game* game, GameUI* ui)
    {}

    void update(Game* game, GameUI* ui)
    {}

    bool isReadyToFinish(Game* game, GameUI* ui)
    {
        return (game->entities.size() > 0);
    }
};

class SpawnFinishStep : public TutorialStep
{
public:
    SpawnFinishStep(Game* game, GameUI* ui):
        TutorialStep("spawnfinish", true, game, ui)
    {}
    
    tuple<vector<string>, vector<string>> getText(Game* game, GameUI* ui)
    {
        return
        {
            {
                "Note that this is spending money from your Coinfight wallet. All told, the Gateway takes a $4 investment."
            },
            {
                "In a real game, you can only warp-in a Gateway like this by spending a ticket to a Coinfight round.",
                "Any additional Gateways will have to be built with units and gold on location."
            }
        };
    }

    void start(Game* game, GameUI* ui)
    {}

    void update(Game* game, GameUI* ui)
    {}

    void ping(int num)
    {}

    bool isReadyToFinish(Game* game, GameUI* ui)
    {
        auto gateways = filterForType<Gateway, Entity>(game->entities);

        return (gateways.size() > 0);
    }

    optional<float> getProgress(Game* game, GameUI* ui)
    {
        auto gateways = filterForType<Gateway, Entity>(game->entities);
        if (gateways.size() > 0)
            return 1;
        
        auto beacons = filterForType<Beacon, Entity>(game->entities);

        if (beacons.size() > 0)
        {
            return float(beacons[0]->getBuiltRatio());
        }
        else
        {
            return 0;
        }
    }
};

class BuildFirstPrimeStep : public TutorialStep
{
public:
    BuildFirstPrimeStep(Game* game, GameUI* ui)
        : TutorialStep("firstprime", true, game, ui)
        {}
    
    tuple<vector<string>, vector<string>> getText(Game* game, GameUI* ui)
    {
        return
        {
            {
                "Now that your Gateway is finished, you can build your first unit.",
                "Select your Gateway and click the Build Prime button (or hit Q)."
            },
            {
                "This money being invested--$4 in the Gateway, $0.50 for the Prime--will be dropped onto the battlefield if they die, for you or someone else to pick up."
            }
        };
    }
    
    void start(Game* game, GameUI* ui)
    {}

    void update(Game* game, GameUI* ui)
    {}

    void ping(int num)
    {}

    bool isReadyToFinish(Game* game, GameUI* ui)
    {
        auto primes = filterForType<Prime, Entity>(game->entities);

        return (primes.size() > 0 && primes[0]->isFullyBuilt());
    }

    optional<float> getProgress(Game* game, GameUI* ui)
    {
        auto primes = filterForType<Prime, Entity>(game->entities);

        if (primes.size() > 0)
        {
            return float(primes[0]->getBuiltRatio());
        }
        else return 0;
    }
};

class PickupGoldStep : public TutorialStep
{
public:
    PickupGoldStep(Game* game, GameUI* ui)
        : TutorialStep("pickupgold", true, game, ui)
        {}

    tuple<vector<string>, vector<string>> getText(Game* game, GameUI* ui)
    {
        return
        {
            {
                "Bad news: you're broke! Good news: you have a Prime, and he can go pick up the gold nearby. You might need to move your camera to find the gold.",
                "Select your Prime and use its Collect command on the gold pile. Your Prime has a max capacity of $0.50.",
            },
            {
                "Gold is the only resource in Coinfight, and is backed by Godwoken USDC. In a real game, your ultimate goal is simply to exit the game with as much USDC as possible."
            }
        };
    }
    
    void start(Game* game, GameUI* ui)
    {
        auto gateways = filterForType<Gateway, Entity>(game->entities);

        vector2fp gpPos = gateways[0]->getPos() + vector2fp(randomVectorWithMagnitude(600));

        boost::shared_ptr<GoldPile> gp1 = boost::shared_ptr<GoldPile>(new GoldPile(gpPos));
        gp1->gold.createMoreByFiat(dollarsToCoinsIntND(0.3));
        game->registerNewEntityIgnoringConstraints(gp1);
    }

    void update(Game* game, GameUI* ui)
    {}

    void ping(int num)
    {}

    coinsInt getTotalGoldGathered(Game *game)
    {
        auto primes = filterForType<Prime, Entity>(game->entities);

        coinsInt total = 0;
        for (unsigned int i=0; i<primes.size(); i++)
        {
            total += primes[i]->heldGold.getInt();
        }

        total += game->players[0].credit.getInt();

        return total;
    }
    bool isReadyToFinish(Game* game, GameUI* ui)
    {
        return (getTotalGoldGathered(game) >= dollarsToCoinsIntND(0.2));
    }

    optional<float> getProgress(Game* game, GameUI* ui)
    {
        return (float(getTotalGoldGathered(game)) / dollarsToCoinsIntND(0.2));
    }
};

class ReturnGoldStep : public TutorialStep
{
public:
    ReturnGoldStep(Game* game, GameUI* ui)
        : TutorialStep("returngold", true, game, ui)
        {}

    tuple<vector<string>, vector<string>> getText(Game* game, GameUI* ui)
    {
        return
        {
            {
                "Great! To finish capturing that gold, it has to be brought to the Gateway. Use your Prime's Deposit command on the Gateway.",
                "Capture at least $0.20 of gold to continue."
            },
            {
                "You're now funneling the credit out of the game and into your wallet, via your Gateway.",
                "Only Gateways can bring gold in and out of the game, from and to your wallet. So keep them safe!"
            }
        };
    }
    
    void start(Game* game, GameUI* ui)
    {}

    void update(Game* game, GameUI* ui)
    {}

    void ping(int num)
    {}

    bool isReadyToFinish(Game* game, GameUI* ui)
    {
        return (game->players[0].credit.getInt() >= dollarsToCoinsIntND(0.2));
    }

    optional<float> getProgress(Game* game, GameUI* ui)
    {
        return (float(game->players[0].credit.getInt()) / dollarsToCoinsIntND(0.2));
    }
};

class MoreJobsStep : public TutorialStep
{
public:
    int numGPsCreated;

    MoreJobsStep(Game* game, GameUI* ui)
        : TutorialStep("morejobs", true, game, ui)
    {
        numGPsCreated = 0;
    }
    
    tuple<vector<string>, vector<string>> getText(Game* game, GameUI* ui)
    {
        return
        {
            {
                "Here's some more fake tutorial money!",
                "Right clicking on additional gold piles will add them to the Prime's job queue. Holding shift will change where in the queue the new job goes.",
                "You can also cast the Prime's Collect command on a location, to have the Prime approach the location and pick up any gold it finds on the way.",
                "Continue gathering gold and make another Prime.",
            },
            {
                "Primes will continue working until they run out of sources of gold or ways to store/invest it."
                "Moving the Prime will only briefly interrupt this work--to stop it completely and clear its job queue, use the Stop command.",
                "Generally, right-clicking with Primes does what you want it to, but the Deposit, Fetch, and Scuttle commands are more precise."
            }
        };
    }
    
    void start(Game* game, GameUI* ui)
    {
    }

    void update(Game* game, GameUI* ui)
    {
        if (game->frame % 10 == 0 && numGPsCreated < 10)
        {
            vector2fp gatewayPos = filterForType<Gateway, Entity>(game->entities)[0]->getPos();

            bool gpCreated = false;
            while (!gpCreated)
            {
                vector2fp pos = gatewayPos + vector2fp(randomVectorWithMagnitudeRange(200, 400));
                float gold = ((((double)rand() / RAND_MAX) * 7.5) + 0.5);
                boost::shared_ptr<GoldPile> gp = boost::shared_ptr<GoldPile>(new GoldPile(pos));
                gp->gold.createMoreByFiat(dollarsToCoinsIntND(gold));

                if (game->registerNewEntityIfInMapAndNoCollision(gp))
                {
                    gpCreated = true;
                }
            }
            numGPsCreated ++;
        }
    }

    float primeBuildProgress(Game* game)
    {
        float count = 0;
        for (unsigned int i = 0; i<game->entities.size(); i++)
        {
            if (auto entity = game->entities[i])
            {
                if (auto prime = boost::dynamic_pointer_cast<Prime, Entity>(entity))
                {
                    count += float(prime->getBuiltRatio());
                }
            }
        }
        return count;
    }

    void ping(int num)
    {}

    bool isReadyToFinish(Game* game, GameUI* ui)
    {
        return primeBuildProgress(game) >= 2;
    }

    optional<float> getProgress(Game* game, GameUI* ui)
    {
        return (primeBuildProgress(game) - 1);
    }
};

class ScuttleStep : public TutorialStep
{
public:
    int startingNumPrimes;
    ScuttleStep(Game* game, GameUI* ui)
        : TutorialStep("scuttle", true, game, ui)
        {}
        
    tuple<vector<string>, vector<string>> getText(Game* game, GameUI* ui)
    {
        return
        {
            {
                "When you're ready to get out of the game (hopefully at a profit!), you'll want to recover the money you invested in your army.",
                "Both Gateways and Primes can \"scuttle\" friendly units, deconstructing them to recover their investment cost.",
                "Scuttle one Prime with another by using its Scuttle command.",
                "Once again, holding shift will delay the job; otherwise it will be done immediately."
            },
            {
                "Gateways can also scuttle units in this way, and will send the money directly to your wallet."
            }
        };
    }

    int getNumPrimes(Game* game)
    {
        int count = 0;
        for (unsigned int i=0; i<game->entities.size(); i++)
        {
            if (auto prime = boost::dynamic_pointer_cast<Prime, Entity>(game->entities[i]))
            {
                count ++;
            }
        }
        return count;
    }
    
    void start(Game* game, GameUI* ui)
    {
        startingNumPrimes = getNumPrimes(game);
    }

    void update(Game* game, GameUI* ui)
    {}

    void ping(int num)
    {}

    bool isReadyToFinish(Game* game, GameUI* ui)
    {
        return getNumPrimes(game) < startingNumPrimes;
    }

    optional<float> getProgress(Game* game, GameUI* ui)
    {
        if (isReadyToFinish(game, ui))
            return 1;
        
        float highestProgress = 0;
        for (unsigned int i=0; i<game->entities.size(); i++)
        {
            if (auto prime = boost::dynamic_pointer_cast<Prime, Entity>(game->entities[i]))
            {
                for (unsigned int j=0; j<prime->scavengeTargetQueue.size(); j++)
                {
                    if (auto unit = boost::dynamic_pointer_cast<Unit, Entity>(prime->scavengeTargetQueue[j].castToEntityPtr(*game)))
                    {
                        float progress = 1 - float(unit->getBuiltRatio());
                        if (progress > highestProgress)
                            highestProgress = progress;
                    }
                }
            }
        }
        return highestProgress;
    }
};

class ConcludeTutorialStep : public TutorialStep
{
public:
    ConcludeTutorialStep(Game* game, GameUI* ui)
        : TutorialStep("buildstuff", false, game, ui)
        {}
        
    tuple<vector<string>, vector<string>> getText(Game* game, GameUI* ui)
    {
        return
        {
            {
                "We're just about at the end of the tutorial! Just a few more points, then a fun minigame:",
                "Note that Gateways can build Fighters as well as Primes, and Primes can build both Gateways and Turrets.",
                "When Primes are constructing buildings, they'll need more gold than they can carry, so make sure they have some source of gold (like a Gateway or some gold piles) queued up so they can finish the job.",
                "",
                "To start the ~Tutorial Graduation Minigame~, build two combat units (Fighers and/or Turrets).",
            },
            {}
        };
    }
    
    void start(Game* game, GameUI* ui)
    {}

    void update(Game* game, GameUI* ui)
    {}

    void ping(int num)
    {}

    bool isReadyToFinish(Game* game, GameUI* ui)
    {
        auto combatUnits = filterForType<CombatUnit, Entity>(game->entities);

        int finished = 0;
        for (unsigned int i=0; i<combatUnits.size(); i++)
        {
            if (combatUnits[i]->isFullyBuilt())
            {
                finished ++;
            }
        }

        return finished >= 2;
    }

    optional<float> getProgress(Game* game, GameUI* ui)
    {
        auto combatUnits = filterForType<CombatUnit, Entity>(game->entities);

        float builtRatioTotal = 0;
        for (unsigned int i=0; i<combatUnits.size(); i++)
        {
            builtRatioTotal += float(combatUnits[i]->getBuiltRatio());
        }

        return builtRatioTotal / 2;
    }
};

float randomFloat()
{
    return (float)rand() / RAND_MAX;
}
float randomFloatRange(float min, float max)
{
    return randomFloat() * (max - min) + min;
}
float randomFloatUnder1()
{
    return (float)rand() / (float(RAND_MAX) + 1);
}
template<class T> T randomChoice(vector<T> vec)
{
    int iter = int(randomFloatUnder1() * vec.size());
    return vec[iter];
}

class EndMinigameStep : public TutorialStep
{
public:
    const int FRAMES_BETWEEN_WAVES = 6000;
    int minigameStartFrame;
    int difficulty;

    EndMinigameStep(Game* game, GameUI* ui)
        : TutorialStep("minigame", false, game, ui)
        {}
         
    tuple<vector<string>, vector<string>> getText(Game* game, GameUI* ui)
    {
        int numFighersComing = 0;
        for (unsigned int i=0; i<game->entities.size(); i++)
        {
            if (auto fighter = boost::dynamic_pointer_cast<Fighter, Entity>(game->entities[i]))
            {
                if (fighter->ownerId == 1)
                {
                    numFighersComing ++;
                }
            }
        }
        stringstream numFightersSS;
        if (numFighersComing > 0)
        {
            numFightersSS << numFighersComing << " fighters are coming!";
        }
        else
        {
            numFightersSS << " ";
        }
        return {
            {
                "In a real game, your wallet balance (upper left) is withdrawable USDC, even after your army is lost. How high can you get it before you die?",
                numFightersSS.str(),
                "Next wave countdown:"
            },
            {
                // "Tab will toggle a (really minimal) view of the map!"
            }
        };
    }

    void spawnInFighters(Game* game)
    {
        int numFightersToCreate = int(pow(float(difficulty), 1.7) * randomFloatRange(0.7, 1.3)) + 1;

        vector2fp groupPos;
        Target groupTarget = Target(vector2fp());
        int numCreatedThisGroup = 0;
        while (numFightersToCreate > 0)
        {
            if (groupPos == vector2fp() || randomFloat() > 0.8)
            {
                groupPos = vector2fp(randomVectorWithMagnitudeRange(500, 4000));

                // get a target
                vector<vector2fp> playerEntityPositions;
                for (unsigned int i=0; i<game->entities.size(); i++)
                {
                    if (auto unit = boost::dynamic_pointer_cast<Unit, Entity>(game->entities[i]))
                    {
                        if (unit->ownerId == 0)
                        {
                            playerEntityPositions.push_back(unit->getPos());
                        }
                    }
                }
                if (playerEntityPositions.size() > 0)
                    groupTarget = Target(randomChoice(playerEntityPositions));
                else
                    groupTarget = Target(vector2fp());

                numCreatedThisGroup = 0;
            }

            vector2fp fighterPos = groupPos + vector2fp(randomVectorWithMagnitude(numCreatedThisGroup*2)); // for now a hacky way to avoid collision.cpp failing on too-close units
            auto fighter = boost::shared_ptr<Fighter>(new Fighter(1, fighterPos));
            game->registerNewEntityIfInMapIgnoringCollision(fighter);

            game->players[1].credit.createMoreByFiat(FIGHTER_COST);
            fighter->completeBuildingInstantly(&game->players[1].credit);

            fighter->cmdAttack(groupTarget);

            numFightersToCreate --;
            numCreatedThisGroup ++;
        }

        difficulty ++;
    } 
    
    void start(Game* game, GameUI* ui)
    {
        minigameStartFrame = game->frame;
        spawnInFighters(game);
    }

    void update(Game* game, GameUI* ui)
    {
        if ((game->frame - minigameStartFrame) % FRAMES_BETWEEN_WAVES == 0)
        {
            spawnInFighters(game);
        }
    }

    void ping(int num)
    {}

    bool isReadyToFinish(Game* game, GameUI* ui)
    {
        return false;
    }

    optional<float> getProgress(Game* game, GameUI* ui)
    {
        // hackily being used as a next wave counter
        return {
            float((game->frame - minigameStartFrame) % FRAMES_BETWEEN_WAVES) / FRAMES_BETWEEN_WAVES
        };
    }
};

// just here for copy/pasting convenience
class TutorialStepTemplate : public TutorialStep
{
public:
    TutorialStepTemplate(Game* game, GameUI* ui)
        : TutorialStep("name", false, game, ui)
        {}
        
    tuple<vector<string>, vector<string>> getText(Game* game, GameUI* ui)
    {
        return
        {
            {

            },
            {}
        };
    }
    
    void start(Game* game, GameUI* ui)
    {}

    void update(Game* game, GameUI* ui)
    {}

    void ping(int num)
    {}

    bool isReadyToFinish(Game* game, GameUI* ui)
    {
        return false;
    }

    optional<float> getProgress(Game* game, GameUI* ui)
    {
        return {};
    }
};

Tutorial::Tutorial(Game* game, GameUI* ui)
    : game(game), ui(ui)
{
    steps.push_back(boost::shared_ptr<TutorialStep>(new CameraStep(game, ui)));
    steps.push_back(boost::shared_ptr<TutorialStep>(new SpawnBeaconStep(game, ui)));
    steps.push_back(boost::shared_ptr<TutorialStep>(new SpawnFinishStep(game, ui)));
    steps.push_back(boost::shared_ptr<TutorialStep>(new BuildFirstPrimeStep(game, ui)));
    steps.push_back(boost::shared_ptr<TutorialStep>(new PickupGoldStep(game, ui)));
    steps.push_back(boost::shared_ptr<TutorialStep>(new ReturnGoldStep(game, ui)));
    steps.push_back(boost::shared_ptr<TutorialStep>(new MoreJobsStep(game, ui)));
    steps.push_back(boost::shared_ptr<TutorialStep>(new ScuttleStep(game, ui)));
    steps.push_back(boost::shared_ptr<TutorialStep>(new ConcludeTutorialStep(game, ui)));
    steps.push_back(boost::shared_ptr<TutorialStep>(new EndMinigameStep(game, ui)));

    stepIter = 0;
}
void Tutorial::start()
{
    srand(time(0));
    stepIter = 0;
    steps[stepIter]->start(game, ui);
}
void Tutorial::stepForward()
{
    stepIter ++;
    if (stepIter < steps.size())
    {
        currentStep()->start(game, ui);
    }
}
void Tutorial::update()
{
    currentStep()->update(game, ui);
    if (currentStep()->isReadyToFinish(game, ui) && !currentStep()->waitForEnter)
    {
        stepForward();
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
void Tutorial::enterPressed()
{
    if (currentStep()->isReadyToFinish(game, ui) && currentStep()->waitForEnter)
    {
        stepForward();
    }
}