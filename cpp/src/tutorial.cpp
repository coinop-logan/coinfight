#include <sstream>
#include "tutorial.h"
#include "myvectors.h"

using namespace std;

void setupTutorialScenario(Game* game)
{
    game->players.push_back(Player(Address("0x0f0f00f000f00f00f000f00f00f000f00f00f000")));
    game->players.push_back(Player(Address("0xf00f00f000f00f00f000f00f00f000f00f00f000")));

    game->players[0].credit.createMoreByFiat(dollarsToCoinsIntND(4.5));

    boost::shared_ptr<GoldPile> gp = boost::shared_ptr<GoldPile>(new GoldPile(vector2fp()));
    gp->gold.createMoreByFiat(dollarsToCoinsIntND(3));
    game->registerNewEntityIgnoringCollision(gp);

    game->players[1].credit.createMoreByFiat(FIGHTER_COST);

    vector2fp fighterPos(randomVectorWithMagnitude(1500));
    boost::shared_ptr<Fighter> fighter = boost::shared_ptr<Fighter>(new Fighter(1, fighterPos));
    fighter->completeBuildingInstantly(&game->players[1].credit);
    fighter->takeHit(FIGHTER_HEALTH / 2);
    game->registerNewEntityIgnoringCollision(fighter);
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
            {"Hey there! This tutorial will explain the basics of Coinfight.",
             "You can hide this tutorial (or show it again) anytime by hitting F1.",
             "Other than this playground/tutorial, Coinfight is always played in an arena against others, and is always played with real money. For now, pretend you've just deposited $4.50 into your account, and joined a game. This is what you'll see!",
             "The first step after joining a game will be to spawn in your first Gateway, with a one-time-use \"Beacon\".",
             "Do this now by hitting \"B\" and selecting a location. For now, choose a location outside of the fourth circle.",
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
        return (game->entities.size() > 2);
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
                "Note that this is spending money from your Coinfight wallet. All told it will cost $4."
            },
            {
                "In a real game, everyone has only one Beacon--one chance to \"teleport in\" a Gateway like this anywhere on the map.",
                "Any additional Gateways will have to be built with units and resources on location."
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
        if (game->entities.size() < 4)
        {
            return false;
        }
        else
        {
            if (auto gateway = boost::dynamic_pointer_cast<Gateway, Entity>(game->entities[3]))
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
        else if (auto beacon = boost::dynamic_pointer_cast<Beacon, Entity>(game->entities[2]))
        {
            return float(beacon->getBuiltRatio());
        }
        else if (game->entities.size() > 2)
        {
            if (auto gateway = boost::dynamic_pointer_cast<Gateway, Entity>(game->entities[3]))
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
    BuildFirstPrimeStep(Game* game, UI* ui)
        : TutorialStep("firstprime", true, game, ui)
        {}
    
    tuple<vector<string>, vector<string>> getText(Game* game, UI* ui)
    {
        return
        {
            {
                "Now that your Gateway is finished, you can build your first unit.",
                "Select your Gateway and hit the 'Q' key.",
                "This will build a Prime, the main worker/builder in Coinfight, for $0.50."
            },
            {
                "This money being invested--$4 in the Gateway, $0.50 for the Prime--will be dropped onto the battlefield upon death.",
                "Near the end of this tutorial, you'll learn how to recover the full cost of any units that have survived."
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
    PickupGoldStep(Game* game, UI* ui)
        : TutorialStep("pickupgold", false, game, ui)
        {}

    tuple<vector<string>, vector<string>> getText(Game* game, UI* ui)
    {
        return
        {
            {
                "Bad news: you're broke! Good news: you have a Prime, and he can go get that gold nearby.",
                "Select your Prime and right-click on that gold pile to start picking it up. Your Prime has a max capacity of $0.50.",
            },
            {
                "Gold is the only resource in Coinfight, and is backed by DAI. In a real game, your main focus will be on finding and securing gold to spend on units, or eventually, withdraw as winnings in DAI."
            }
        };
    }
    
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
    bool isReadyToFinish(Game* game, UI* ui)
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
    ReturnGoldStep(Game* game, UI* ui)
        : TutorialStep("returngold", true, game, ui)
        {}

    tuple<vector<string>, vector<string>> getText(Game* game, UI* ui)
    {
        return
        {
            {
                "Great! To finish capturing that gold, it has to be brought to the Gateway. With your Prime selected, right click on the Gateway."
            },
            {
                "The Prime is putting down gold within range of the Gateway, and the Gateway is pulling this gold out of the game, into your Coinfight wallet.",
                "Only Gateways can bring gold out of the game and into your Coinfight wallet (or vise versa). This means that if all your Gateways are lost, all the money you invested in your army will be stranded! So protect your Gateways at all costs."
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
        return (game->players[0].credit.getInt() >= dollarsToCoinsIntND(0.5));
    }

    optional<float> getProgress(Game* game, UI* ui)
    {
        return (float(game->players[0].credit.getInt()) / dollarsToCoinsIntND(0.5));
    }
};

class MorePrimesStep : public TutorialStep
{
public:
    int entitiesListLengthAtStart;

    MorePrimesStep(Game* game, UI* ui)
        : TutorialStep("moreprimes", false, game, ui)
    {
    }
    
    tuple<vector<string>, vector<string>> getText(Game* game, UI* ui)
    {
        return
        {
            {
                "Now that we have a bit more money again, queue up 3 more Primes (select Gateway, hit Q)."
            },
            {}
        };
    }
    
    void start(Game* game, UI* ui)
    {
        entitiesListLengthAtStart = game->entities.size();
    }

    void update(Game* game, UI* ui)
    {}

    void ping(int num)
    {}

    int numAdditionalPrimes(Game* game)
    {
        int count = 0;
        for (unsigned int i = entitiesListLengthAtStart; i<game->entities.size(); i++)
        {
            if (auto entity = game->entities[i])
            {
                if (auto prime = boost::dynamic_pointer_cast<Prime, Entity>(entity))
                {
                    count ++;
                }
            }
        }
        return count;
    }

    bool isReadyToFinish(Game* game, UI* ui)
    {
        return numAdditionalPrimes(game) >= 3;
    }

    optional<float> getProgress(Game* game, UI* ui)
    {
        return numAdditionalPrimes(game) / 3.0;
    }
};

class GatherStep : public TutorialStep
{
public:
    GatherStep(Game* game, UI* ui)
        : TutorialStep("gather", false, game, ui)
        {}
        
    tuple<vector<string>, vector<string>> getText(Game* game, UI* ui)
    {
        return
        {
            {
                "Now, you probably don't have enough money to build all those Primes, so you'll run out of money soon, and your Gateway will stop building.",
                "Let's have the Primes automatically gather gold and bring it to the Gateway, by way of a \"gather\" command.",
                "Left-click and drag to select ALL of your primes (even the ones that aren't done building). Then hit the 'A' key and click near the gold pile.",
                "The unbuilt ones will execute the order once they're fully built.",
                "Bring all the gold to your Gateway to continue."
            },
            {}
        };
    }

    coinsInt uncapturedGoldAtStart;
    
    coinsInt countUncapturedGold(Game* game)
    {
        auto gateway = boost::dynamic_pointer_cast<Gateway, Entity>(game->entities[3]);

        coinsInt count = 0;
        for (unsigned int i=0; i<game->entities.size(); i++)
        {
            if (auto entity = game->entities[i])
            {
                if (entity->getRefOrThrow() == gateway->getRefOrThrow())
                    continue;

                if (auto goldpile = boost::dynamic_pointer_cast<GoldPile, Entity>(entity))
                {
                    // ignore if it's in the gateway's scuttle queue
                    bool inQueue = false;
                    for (unsigned int j=0; j<gateway->scuttleTargetQueue.size(); j++)
                    {
                        if (gateway->scuttleTargetQueue[j] == goldpile->getRefOrThrow())
                        {
                            inQueue = true;
                            break;
                        }
                    }

                    if (!inQueue)
                    {
                        count += goldpile->gold.getInt();
                    }
                }
                else if (auto prime = boost::dynamic_pointer_cast<Prime, Entity>(entity))
                {
                    count += prime->heldGold.getInt();
                }
            }
        }

        return count;
    }

    void start(Game* game, UI* ui)
    {
        uncapturedGoldAtStart = countUncapturedGold(game);
    }

    void update(Game* game, UI* ui)
    {}

    void ping(int num)
    {}

    bool isReadyToFinish(Game* game, UI* ui)
    {
        return countUncapturedGold(game) == 0;
    }

    optional<float> getProgress(Game* game, UI* ui)
    {
        return 1 - (float(countUncapturedGold(game)) / uncapturedGoldAtStart);
    }
};

class BuildFighterStep : public TutorialStep
{
public:
    BuildFighterStep(Game* game, UI* ui)
        : TutorialStep("name", false, game, ui)
        {}
        
    tuple<vector<string>, vector<string>> getText(Game* game, UI* ui)
    {
        return
        {
            {
                "Let's get into some combat! Queue up a Fighter by selecting your Gateway and hitting 'W'."
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
        for (unsigned int i=2; i<game->entities.size(); i++)
        {
            if (auto entity = game->entities[i])
            {
                if (auto fighter = boost::dynamic_pointer_cast<Fighter, Entity>(entity))
                {
                    return true;
                }
            }
        }
        
        return false;
    }

    optional<float> getProgress(Game* game, UI* ui)
    {
        return {};
    }
};

boost::shared_ptr<Fighter> getWoundedFighter(Game* game)
{
    if (auto entity = game->entities[1])
    {
        if (auto fighter = boost::dynamic_pointer_cast<Fighter, Entity>(entity))
        {
            return fighter;
        }
    }

    return {};
}

class CameraStep : public TutorialStep
{
public:
    CameraStep(Game* game, UI* ui):
        TutorialStep("camera", false, game, ui)
    {}

    tuple<vector<string>, vector<string>> getText(Game* game, UI* ui)
    {
        return
        {
            {
                "While we're waiting on that Fighter to build, let's look around.",
                "You can move the camera by dragging with the middle mouse button, or by moving your mouse to the edges of the screen.",
                "To continue, find the wounded foreign Fighter."
            },
            {}
        };
    }

    float getDistanceToFighterWithMargin(Game *game, UI* ui)
    {
        float distance = (ui->camera.gamePos - getWoundedFighter(game)->getPos()).getMagnitude();
        return max(0.f, distance - 200);
    }

    float startDistanceToFighter;

    void start(Game* game, UI* ui)
    {
        startDistanceToFighter = getDistanceToFighterWithMargin(game, ui);
    }

    void update(Game* game, UI* ui)
    {}

    void ping(int num)
    {}

    bool isReadyToFinish(Game* game, UI* ui)
    {
        return (getDistanceToFighterWithMargin(game, ui) == 0);
    }

    optional<float> getProgress(Game* game, UI* ui)
    {
        return 1 - (getDistanceToFighterWithMargin(game, ui) / startDistanceToFighter);
    }
};

class AttackStep : public TutorialStep
{
public:
    AttackStep(Game* game, UI* ui)
        : TutorialStep("attack", false, game, ui)
        {}
        
    tuple<vector<string>, vector<string>> getText(Game* game, UI* ui)
    {
        return
        {
            {
                "There it is! Once your Fighter is done building, select your fighter and go kill it!",
                "(You might not have enough for a Fighter if you built a lot of Primes. If so, you can scuttle your Primes and recover their cost by selecting the Gateway and right-clicking on a Prime.)",
                "With any unit, right-clicking moves the unit, and right clicking on an enemy unit attacks it.",
                "You can also hit 'A' and click on a location to issue an \"attack-to\" command. This will cause Fighters to move toward the target location and fight anything they encounter.",
                "Now, go kill!"
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
        if (auto fighter = getWoundedFighter(game))
        {
            return false;
        }
        else
        {
            return true;
        }
    }

    optional<float> getProgress(Game* game, UI* ui)
    {
        return {};  
    }
};

class DropGoldExplainer : public TutorialStep
{
public:
    DropGoldExplainer(Game* game, UI* ui)
        : TutorialStep("dropgoldexplainer", true, game, ui)
        {}
        
    tuple<vector<string>, vector<string>> getText(Game* game, UI* ui)
    {
        return
        {
            {
                "As you can see, when the Fighter died, it dropped $1.50. You can come pick it up with your Primes if you want.",
                "Upon death, units drop their investment cost as gold onto the map in the same way. As battles are waged, the battlefield will become littered with gold piles where units died. Opportunistic players might be able to make a profit simply by picking up the pieces of a larger battle between other players.",
                "At the end of this tutorial, you'll learn how to recoup your investment in your army. Just a few more things to go over first!"
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
        return true;
    }

    optional<float> getProgress(Game* game, UI* ui)
    {
        return {};
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
    steps.push_back(boost::shared_ptr<TutorialStep>(new SpawnBeaconStep(game, ui)));
    steps.push_back(boost::shared_ptr<TutorialStep>(new SpawnFinishStep(game, ui)));
    steps.push_back(boost::shared_ptr<TutorialStep>(new BuildFirstPrimeStep(game, ui)));
    steps.push_back(boost::shared_ptr<TutorialStep>(new PickupGoldStep(game, ui)));
    steps.push_back(boost::shared_ptr<TutorialStep>(new ReturnGoldStep(game, ui)));
    steps.push_back(boost::shared_ptr<TutorialStep>(new MorePrimesStep(game, ui)));
    steps.push_back(boost::shared_ptr<TutorialStep>(new GatherStep(game, ui)));
    steps.push_back(boost::shared_ptr<TutorialStep>(new BuildFighterStep(game, ui)));
    steps.push_back(boost::shared_ptr<TutorialStep>(new CameraStep(game, ui)));
    steps.push_back(boost::shared_ptr<TutorialStep>(new AttackStep(game, ui)));
    steps.push_back(boost::shared_ptr<TutorialStep>(new DropGoldExplainer(game, ui)));

    stepIter = 0;
}
void Tutorial::start()
{
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