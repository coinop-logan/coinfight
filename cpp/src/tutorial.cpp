#include <sstream>
#include "tutorial.h"
#include "myvectors.h"

using namespace std;

void setupTutorialScenario(Game* game)
{
    game->players.push_back(Player(Address("0x0f0f00f000f00f00f000f00f00f000f00f00f000")));
    game->players.push_back(Player(Address("0xf00f00f000f00f00f000f00f00f000f00f00f000")));

    game->players[0].credit.createMoreByFiat(dollarsToCoinsIntND(4.5));

    // game->players[1].credit.createMoreByFiat(FIGHTER_COST);

    // vector2fp fighterPos(randomVectorWithMagnitude(1500));
    // boost::shared_ptr<Fighter> fighter = boost::shared_ptr<Fighter>(new Fighter(1, fighterPos));
    // fighter->completeBuildingInstantly(&game->players[1].credit);
    // fighter->takeHit(FIGHTER_HEALTH / 2);
    // game->registerNewEntityIgnoringCollision(fighter);
}

TutorialStep::TutorialStep(string idName, bool waitForEnter, Game* game, UI* ui):
    idName(idName), waitForEnter(waitForEnter) {}

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
bool TutorialStep::isReadyToFinish(Game* game, UI* ui)
{
    throw runtime_error("isReadyToFinish() has not been defined for tutorial step '" + idName + "'.\n");
}
optional<float> TutorialStep::getProgress(Game* game, UI* ui)
{
    return {};
}
tuple<vector<string>, vector<string>> TutorialStep::getText(Game* game, UI* ui)
{
    throw runtime_error("getText() has not been defined for tutorial step '" + idName + "'.\n");
}

class CameraStep : public TutorialStep
{
public:
    float distanceMoved;
    vector2i lastCameraPos;

    CameraStep(Game* game, UI* ui):
        TutorialStep("camera", false, game, ui)
    {
        distanceMoved = 0;
        lastCameraPos = ui->camera.gamePos;
    }

    tuple<vector<string>, vector<string>> getText(Game* game, UI* ui)
    {
        return
        {
            {
                "Hey there! This tutorial will explain the basics of Coinfight. You can hide this tutorial (or show it again) anytime by hitting F1.",
                "First things first: you can move the camera by dragging with the middle mouse button, or by moving your mouse to the edges of the screen.",
                "Go ahead, wiggle 'er around a bit!"
            },
            {}
        };
    }

    void start(Game* game, UI* ui)
    {}

    void update(Game* game, UI* ui)
    {
        distanceMoved += (lastCameraPos - ui->camera.gamePos).getMagnitude();
        lastCameraPos = ui->camera.gamePos;
    }

    void ping(int num)
    {}

    bool isReadyToFinish(Game* game, UI* ui)
    {
        return distanceMoved >= 2000;
    }

    optional<float> getProgress(Game* game, UI* ui)
    {
        return distanceMoved / 2000.0;
    }
};

class SpawnBeaconStep : public TutorialStep
{
public:
    SpawnBeaconStep(Game* game, UI* ui)
        : TutorialStep("beacon", false, game, ui)
    {}

    tuple<vector<string>, vector<string>> getText(Game* game, UI* ui)
    {
        return
        {
            {
                "Other than this playground/tutorial, Coinfight is always played in an arena against others, and is always played with real money. For now, this is what you'd see if you just deposited 4.5 DAI into your Coinfight wallet, and joined a game.",
                "The first step after joining a game will be to spawn in your first Gateway, with a one-time-use \"Beacon\".",
                "Do this now by hitting \"B\" and clicking on the map somewhere.",
            },
            {}
        };
    }
    
    void start(Game* game, UI* ui)
    {}

    void update(Game* game, UI* ui)
    {}

    bool isReadyToFinish(Game* game, UI* ui)
    {
        return (game->entities.size() > 0);
    }
};

class SpawnFinishStep : public TutorialStep
{
public:
    SpawnFinishStep(Game* game, UI* ui):
        TutorialStep("spawnfinish", true, game, ui)
    {}
    
    tuple<vector<string>, vector<string>> getText(Game* game, UI* ui)
    {
        return
        {
            {
                "Note that this is spending money from your Coinfight wallet. All told, the Gateway takes a $4 investment."
            },
            {
                "In a real game, everyone has only one Beacon--one chance to \"teleport in\" a Gateway like this anywhere on the map.",
                "Any additional Gateways will have to be built with units and gold on location."
            }
        };
    }

    void start(Game* game, UI* ui)
    {}

    void update(Game* game, UI* ui)
    {}

    void ping(int num)
    {}

    bool isReadyToFinish(Game* game, UI* ui)
    {
        auto gateways = filterForType<Gateway, Entity>(game->entities);

        return (gateways.size() > 0);
    }

    optional<float> getProgress(Game* game, UI* ui)
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
    BuildFirstPrimeStep(Game* game, UI* ui)
        : TutorialStep("firstprime", true, game, ui)
        {}
    
    tuple<vector<string>, vector<string>> getText(Game* game, UI* ui)
    {
        return
        {
            {
                "Now that your Gateway is finished, you can build your first unit.",
                "Select your Gateway and hit Q.",
                "This will build a Prime, the main worker/builder in Coinfight, for $0.50."
            },
            {
                "This money being invested--$4 in the Gateway, $0.50 for the Prime--will be dropped onto the battlefield if they die, for you or someone else to pick up."
            }
        };
    }
    
    void start(Game* game, UI* ui)
    {}

    void update(Game* game, UI* ui)
    {}

    void ping(int num)
    {}

    bool isReadyToFinish(Game* game, UI* ui)
    {
        auto primes = filterForType<Prime, Entity>(game->entities);

        return (primes.size() > 0 && primes[0]->getBuiltRatio() == fixed32(1));
    }

    optional<float> getProgress(Game* game, UI* ui)
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
    PickupGoldStep(Game* game, UI* ui)
        : TutorialStep("pickupgold", true, game, ui)
        {}

    tuple<vector<string>, vector<string>> getText(Game* game, UI* ui)
    {
        return
        {
            {
                "Bad news: you're broke! Good news: you have a Prime, and he can go pick up the gold nearby. You might need to move your camera to find the gold.",
                "Select your Prime and right-click on that gold pile to start picking it up. Your Prime has a max capacity of $0.50.",
            },
            {
                "Gold is the only resource in Coinfight, and is backed by DAI. In a real game, your main focus will be on finding and securing gold to either spend on your army or take out of the game as DAI."
            }
        };
    }
    
    void start(Game* game, UI* ui)
    {
        auto gateways = filterForType<Gateway, Entity>(game->entities);

        vector2fp gpPos = gateways[0]->getPos() + vector2fp(randomVectorWithMagnitude(600));

        boost::shared_ptr<GoldPile> gp1 = boost::shared_ptr<GoldPile>(new GoldPile(gpPos));
        gp1->gold.createMoreByFiat(dollarsToCoinsIntND(0.3));
        game->registerNewEntityIgnoringCollision(gp1);
    }

    void update(Game* game, UI* ui)
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

        return total;
    }
    bool isReadyToFinish(Game* game, UI* ui)
    {
        return (getTotalGoldGathered(game) >= dollarsToCoinsIntND(0.2));
    }

    optional<float> getProgress(Game* game, UI* ui)
    {
        return (float(getTotalGoldGathered(game)) / dollarsToCoinsIntND(0.2));
    }
};

class ReturnGoldStep : public TutorialStep
{
public:
    ReturnGoldStep(Game* game, UI* ui)
        : TutorialStep("returngold", true, game, ui)
        {}

    tuple<vector<string>, vector<string>> getText(Game* game, UI* ui)
    {
        return
        {
            {
                "Great! To finish capturing that gold, it has to be brought to the Gateway. With your Prime selected, right click on the Gateway.",
                "Capture at least $0.20 of gold to continue."
            },
            {
                "You're now funneling the credit out of the game and into your wallet, via your Gateway.",
                "Only Gateways can bring gold in and out of the game, from and to your wallet. So keep them safe!"
            }
        };
    }
    
    void start(Game* game, UI* ui)
    {}

    void update(Game* game, UI* ui)
    {}

    void ping(int num)
    {}

    bool isReadyToFinish(Game* game, UI* ui)
    {
        return (game->players[0].credit.getInt() >= dollarsToCoinsIntND(0.2));
    }

    optional<float> getProgress(Game* game, UI* ui)
    {
        return (float(game->players[0].credit.getInt()) / dollarsToCoinsIntND(0.2));
    }
};

class MoreJobsStep : public TutorialStep
{
public:
    int numGPsCreated;

    MoreJobsStep(Game* game, UI* ui)
        : TutorialStep("morejobs", true, game, ui)
    {
        numGPsCreated = 0;
    }
    
    tuple<vector<string>, vector<string>> getText(Game* game, UI* ui)
    {
        return
        {
            {
                "Here's some more fake tutorial money!",
                "Right clicking on additional gold piles will add them to the Prime's job queue. Without shift pressed, the new job will be done immediately; with shift pressed, it'll be added to the end of the queue.",
                "You can also hit F and click a location, to add a \"fetch to\" job: the Prime will approach the location and pick up any gold it finds on the way.",
                "Continue gathering gold and make another Prime.",
            },
            {
                "Primes will continue working until they run out of sources of gold (loot and Gateways) or ways to store/invest it (build jobs, Gateways, or gold piles).",
                "Moving the Prime will only briefly interrupt this work--to stop it completely and clear its job queue, hit S.",
                "(right-clicking usually does the right thing, but the keys D (Deposit) and F (Fetch) can be more specific)"
            }
        };
    }
    
    void start(Game* game, UI* ui)
    {
    }

    void update(Game* game, UI* ui)
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

                if (game->registerNewEntityIfNoCollision(gp))
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

    bool isReadyToFinish(Game* game, UI* ui)
    {
        return primeBuildProgress(game) >= 2;
    }

    optional<float> getProgress(Game* game, UI* ui)
    {
        return (primeBuildProgress(game) - 1);
    }
};

class ScuttleStep : public TutorialStep
{
public:
    int startingNumPrimes;
    ScuttleStep(Game* game, UI* ui)
        : TutorialStep("scuttle", true, game, ui)
        {}
        
    tuple<vector<string>, vector<string>> getText(Game* game, UI* ui)
    {
        return
        {
            {
                "When you're ready to get out of the game (hopefully at a profit!), you'll want to recover the money you invested in your army.",
                "Both Gateways and Primes can \"absorb\" friendly units, deconstructing them to recover their investment cost.",
                "Absorb one Prime with another by selecting one Prime, hitting A, and clicking on the other.",
                "Once again, holding shift will delay the job; otherwise it will be done immediately."
            },
            {
                "Gateways can also absorb units in this way, and will send the money directly to your wallet. But they can only take in one source of gold at a time, and will prioritize bringing in raw gold over absorbing functional units.",
                "If you want, you can try this out by selecting the Gateway and right-clicking on one of your Primes."
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
    
    void start(Game* game, UI* ui)
    {
        startingNumPrimes = getNumPrimes(game);
    }

    void update(Game* game, UI* ui)
    {}

    void ping(int num)
    {}

    bool isReadyToFinish(Game* game, UI* ui)
    {
        return getNumPrimes(game) < startingNumPrimes;
    }

    optional<float> getProgress(Game* game, UI* ui)
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
    ConcludeTutorialStep(Game* game, UI* ui)
        : TutorialStep("buildstuff", false, game, ui)
        {}
        
    tuple<vector<string>, vector<string>> getText(Game* game, UI* ui)
    {
        return
        {
            {
                "Take a look at the key hints in the lower left.",
                "The top row (QWER keys) is for building units. The first two are mobile units that can be built with Gateways; the last two are buildings that Primes can build.",
                "When Primes are constructing buildings, they'll need more gold than they can carry, so make sure they have some source of gold (like a Gateway or some gold piles) queued up so they can finish the job.",
                "",
                "This pretty much concludes the tutorial, but there's a bit of a minigame for you to try out some of the combat.",
                "Build two combat units to continue (Fighers and/or Turrets). Multiple Primes can help build any unit or building, by right clicking or hitting D.",
            },
            {}
        };
    }
    
    void start(Game* game, UI* ui)
    {}

    void update(Game* game, UI* ui)
    {}

    void ping(int num)
    {}

    bool isReadyToFinish(Game* game, UI* ui)
    {
        auto combatUnits = filterForType<CombatUnit, Entity>(game->entities);

        int finished = 0;
        for (unsigned int i=0; i<combatUnits.size(); i++)
        {
            if (combatUnits[i]->getBuiltRatio() == fixed32(1))
            {
                finished ++;
            }
        }

        return finished >= 2;
    }

    optional<float> getProgress(Game* game, UI* ui)
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

    EndMinigameStep(Game* game, UI* ui)
        : TutorialStep("minigame", false, game, ui)
        {}
         
    tuple<vector<string>, vector<string>> getText(Game* game, UI* ui)
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
                numFightersSS.str(),
                "Next wave countdown:"
            },
            {
                "Tab will toggle a (really minimal) view of the map!"
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
                groupTarget = Target(randomChoice(playerEntityPositions));

                numCreatedThisGroup = 0;
            }

            vector2fp fighterPos = groupPos + vector2fp(randomVectorWithMagnitude(numCreatedThisGroup*2)); // for now a hacky way to avoid collision.cpp failing on too-close units
            auto fighter = boost::shared_ptr<Fighter>(new Fighter(1, fighterPos));
            game->registerNewEntityIgnoringCollision(fighter);

            game->players[1].credit.createMoreByFiat(FIGHTER_COST);
            fighter->completeBuildingInstantly(&game->players[1].credit);

            fighter->cmdAttack(groupTarget);

            numFightersToCreate --;
            numCreatedThisGroup ++;
        }

        difficulty ++;
    } 
    
    void start(Game* game, UI* ui)
    {
        minigameStartFrame = game->frame;
        spawnInFighters(game);
    }

    void update(Game* game, UI* ui)
    {
        if ((game->frame - minigameStartFrame) % FRAMES_BETWEEN_WAVES == 0)
        {
            spawnInFighters(game);
        }
    }

    void ping(int num)
    {}

    bool isReadyToFinish(Game* game, UI* ui)
    {
        return false;
    }

    optional<float> getProgress(Game* game, UI* ui)
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
    TutorialStepTemplate(Game* game, UI* ui)
        : TutorialStep("name", false, game, ui)
        {}
        
    tuple<vector<string>, vector<string>> getText(Game* game, UI* ui)
    {
        return
        {
            {

            },
            {}
        };
    }
    
    void start(Game* game, UI* ui)
    {}

    void update(Game* game, UI* ui)
    {}

    void ping(int num)
    {}

    bool isReadyToFinish(Game* game, UI* ui)
    {
        return false;
    }

    optional<float> getProgress(Game* game, UI* ui)
    {
        return {};
    }
};

Tutorial::Tutorial(Game* game, UI* ui)
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