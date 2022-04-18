#include "input.h"
#include "config.h"
#include "interface.h"

extern Game game;
extern UI ui;
extern vector<boost::shared_ptr<Cmd>> cmdsToSend;

bool isShiftPressed()
{
    return sf::Keyboard::isKeyPressed(sf::Keyboard::LShift) || sf::Keyboard::isKeyPressed(sf::Keyboard::RShift);
}

UI::UI()
    : spawnBeaconInterfaceCmdWithState(boost::shared_ptr<InterfaceCmd>(new SpawnBeaconInterfaceCmd))
{
    camera.gamePos = vector2f(0, 0);
    debugInt = 0;
    cmdState = Default;
    minimapEnabled = false;
    maybeSelectionBoxStart = {};
    unitInterfaceCmdsWithState = vector<InterfaceCmdWithState>
    {
        boost::shared_ptr<InterfaceCmd>(new DepositInterfaceCmd()),
        boost::shared_ptr<InterfaceCmd>(new GatewayBuildPrimeInterfaceCmd()),
        boost::shared_ptr<InterfaceCmd>(new GatewayBuildFighterInterfaceCmd()),
        boost::shared_ptr<InterfaceCmd>(new PrimeBuildGatewayInterfaceCmd()),
        boost::shared_ptr<InterfaceCmd>(new ScuttleInterfaceCmd())
    };
    quitNow = false;
    countdownToQuitOrNeg1 = -1;
    escapeTextCountdownOrNeg1 = -1;
    cleanDrawEnabled = false;
}

void UI::updateAvailableUnitInterfaceCmds(bool spawnBeaconAvailable)
{
    spawnBeaconInterfaceCmdWithState.eligible = spawnBeaconAvailable;

    // set all to false, and create a list of pointers to track those we've yet to enable
    vector<InterfaceCmdWithState*> notYetEnabledUnitInterfaceCmds;
    for (unsigned int i=0; i<unitInterfaceCmdsWithState.size(); i++)
    {
        unitInterfaceCmdsWithState[i].eligible = false;
        notYetEnabledUnitInterfaceCmds.push_back(&unitInterfaceCmdsWithState[i]);
    }

    for (unsigned int i=0; i<selectedUnits.size(); i++)
    {
        // if we've enabled all cmds, no need to keep looking
        if (notYetEnabledUnitInterfaceCmds.size() == 0)
            break;

        for (unsigned int j=0; j<notYetEnabledUnitInterfaceCmds.size(); j++)
        {
            if (auto unitInterfaceCmd = boost::dynamic_pointer_cast<UnitInterfaceCmd, InterfaceCmd>(notYetEnabledUnitInterfaceCmds[j]->interfaceCmd))
            {
                if (unitInterfaceCmd->isUnitEligible(selectedUnits[i]))
                {
                    // enable it and remove this pointer from the list
                    notYetEnabledUnitInterfaceCmds[j]->eligible = true;
                    notYetEnabledUnitInterfaceCmds.erase(notYetEnabledUnitInterfaceCmds.begin() + j);
                    j--;
                }
            }
        }
    }
}

void UI::startEscapeToQuit()
{
    countdownToQuitOrNeg1 = ESCAPE_TO_QUIT_TICKS;
    escapeTextCountdownOrNeg1 = ESCAPE_TO_QUIT_TEXT_LIFE;
}
void UI::cancelEscapeToQuit()
{
    countdownToQuitOrNeg1 = -1;
}
void UI::iterate()
{
    // quit countdown
    if (countdownToQuitOrNeg1 < 0)
    {
        escapeTextCountdownOrNeg1 --;
    }
    else
    {
        // if still counting down, reset display countdown
        escapeTextCountdownOrNeg1 = ESCAPE_TO_QUIT_TEXT_LIFE;
        if (countdownToQuitOrNeg1 == 0)
        {
            quitNow = true;
        }
        else
        {
            countdownToQuitOrNeg1 --;
        }
    }

    // remove dead or null units from selectedUnits
    for (unsigned int i=0; i<selectedUnits.size(); i++)
    {
        if (!selectedUnits[i] || selectedUnits[i]->dead)
        {
            selectedUnits.erase(selectedUnits.begin() + i);
            i --;
        }
    }

    // auto-select first gateway when done (also workaround related error when beacon transforms into gateway)
    if (selectedUnits.size() != 0)
    {
        if (boost::shared_ptr<Beacon> beacon = boost::dynamic_pointer_cast<Beacon, Unit>(selectedUnits[0]))
        {
            // when the beacon finishes and transforms into a Gateway,
            // this pointer will point to the "dead" beacon while one in game->entities will have changed
            boost::shared_ptr<Entity> entityWhereBeaconWas = entityRefToPtrOrNull(game, beacon->ref);
            if (entityWhereBeaconWas->typechar() == GATEWAY_TYPECHAR)
            {
                selectedUnits[0] = boost::dynamic_pointer_cast<Unit, Entity>(entityWhereBeaconWas);
            }
        }
    }
}

vector<boost::shared_ptr<Cmd>> UI::handlePossibleUnitInterfaceCmd(sf::Keyboard::Key key)
{
    if (spawnBeaconInterfaceCmdWithState.eligible && spawnBeaconInterfaceCmdWithState.interfaceCmd->getKey() == key)
        return {spawnBeaconInterfaceCmdWithState.interfaceCmd->execute(this)};

    for (unsigned int i=0; i<unitInterfaceCmdsWithState.size(); i++)
    {
        if (unitInterfaceCmdsWithState[i].eligible && unitInterfaceCmdsWithState[i].interfaceCmd->getKey() == key)
        {
            return {unitInterfaceCmdsWithState[i].interfaceCmd->execute(this)};
        }
    }

    return {};
}

vector2f screenPosToGamePos(CameraState cameraState, vector2i screenPos)
{
    vector2i screenPosFromCenter = screenPos - screenCenter;
    vector2f result = cameraState.gamePos + vector2f(screenPosFromCenter.x, -screenPosFromCenter.y);

    return result;
}

vector2i gamePosToScreenPos(CameraState cameraState, vector2i gamePos)
{
    vector2f subtractedFromCamera = gamePos - cameraState.gamePos;
    subtractedFromCamera.y *= -1;
    vector2i result = subtractedFromCamera + screenCenter;
    return result;
}

vector2i mouseButtonToVec(sf::Event::MouseButtonEvent mEvent)
{
    return vector2i(mEvent.x, mEvent.y);
}
vector2i mouseMoveToVec(sf::Event::MouseMoveEvent mEvent)
{
    return vector2i(mEvent.x, mEvent.y);
}

Target getTargetAtScreenPos(const Game &game, const CameraState &cameraState, vector2i screenPos)
{
    vector2f gamePos = screenPosToGamePos(cameraState, screenPos);

    boost::shared_ptr<Entity> closestValidEntity;
    float closestValidEntityDistance;
    for (unsigned int i = 0; i < game.entities.size(); i++)
    {
        boost::shared_ptr<Entity> e = game.entities[i];
        if (e)
        {
            if (e->collidesWithPoint(gamePos))
            {
                float distance = (gamePos - e->pos).getMagnitude();
                if (!closestValidEntity || distance < closestValidEntityDistance)
                {
                    closestValidEntity = e;
                    closestValidEntityDistance = distance;
                }
            }
        }
    }
    if (closestValidEntity)
        return Target(closestValidEntity->ref);
    else
        return Target(gamePos);
}

boost::shared_ptr<Cmd> makeRightclickCmd(const Game &game, UI ui, int playerID, Target target)
{
    if (ui.selectedUnits.size() == 0)
    {
        return boost::shared_ptr<Cmd>();
    }
    if (optional<vector2f> point = target.castToPoint())
    {
        return boost::shared_ptr<Cmd>(new MoveCmd(entityPtrsToRefs(ui.selectedUnits), *point));
    }
    else if (optional<boost::shared_ptr<Entity>> entityPtrPtr = target.castToEntityPtr(game))
    {
        boost::shared_ptr<Entity> entity = *entityPtrPtr;

        if (getAllianceType(playerID, entity) == Enemy)
        {
            vector<boost::shared_ptr<Unit>> fighters = filterForTypeKeepContainer<Fighter, Unit>(ui.selectedUnits);
            return boost::shared_ptr<Cmd>(new AttackCmd(entityPtrsToRefs(fighters), entity->ref));
        }
        else
        {
            if (auto unit = boost::dynamic_pointer_cast<Unit, Entity>(entity))
            {
                if (unit->getBuiltRatio() < 1)
                {
                    vector<boost::shared_ptr<Unit>> primesInSelection = filterForTypeKeepContainer<Prime, Unit>(ui.selectedUnits);
                    return boost::shared_ptr<Cmd>(new ResumeBuildingCmd(entityPtrsToRefs(primesInSelection), unit->ref));
                }
            }
            if (entity->typechar() == GOLDPILE_TYPECHAR || entity->typechar() == GATEWAY_TYPECHAR)
            {
                vector<boost::shared_ptr<Unit>> primesInSelection = filterForTypeKeepContainer<Prime, Unit>(ui.selectedUnits);
                return boost::shared_ptr<Cmd>(new PickupCmd(entityPtrsToRefs(primesInSelection), entity->ref));
            }
            else
            {
                // maybe in future can do a "follow" type action. Or attack if all enemies.
                return boost::shared_ptr<Cmd>();
            }
        }
    }

    // couldn't cast target to a point or an entity...
    cout << "issue casting target to a point or entity in makeRightclickCmd" << endl;
    return boost::shared_ptr<Cmd>(); // return null cmd
}

boost::shared_ptr<Cmd> makeGatewayBuildCmd(vector<boost::shared_ptr<Unit>> selectedUnits, unsigned char buildUnitTypechar)
{
    auto selectedGateways = filterForType<Gateway, Unit>(selectedUnits);
    if (selectedGateways.size() > 0)
    {
        boost::shared_ptr<Gateway> bestChoice;
        for (unsigned int i=0; i<selectedGateways.size(); i++)
        {
            if (!bestChoice)
            {
                bestChoice = selectedGateways[i];
                continue;
            }

            if (selectedGateways[i]->buildQueueWeight() < bestChoice->buildQueueWeight())
            {
                bestChoice = selectedGateways[i];
            }
        }

        vector<EntityRef> onlyOneGateway;
        onlyOneGateway.push_back(bestChoice->ref);
        return boost::shared_ptr<GatewayBuildCmd>(new GatewayBuildCmd(onlyOneGateway, buildUnitTypechar));
    }
    return boost::shared_ptr<GatewayBuildCmd>();
}

boost::shared_ptr<Cmd> makePrimeBuildCmd(vector<boost::shared_ptr<Unit>> selectedUnits, unsigned char buildUnitTypechar, vector2f buildPos)
{
    auto selectedPrimes = filterForType<Prime, Unit>(selectedUnits);
    if (selectedPrimes.size() > 0)
    {
        boost::shared_ptr<Prime> bestChoice;
        float bestDistanceSquared;
        for (unsigned int i=0; i<selectedPrimes.size(); i++)
        {
            if (!bestChoice)
            {
                bestChoice = selectedPrimes[i];
                bestDistanceSquared = (selectedPrimes[i]->pos - buildPos).getMagnitudeSquared();
                continue;
            }

            // if prime is already building something, not a great choice
            if (selectedPrimes[i]->state == Prime::Build)
                continue;

            float distanceSquared = (selectedPrimes[i]->pos - buildPos).getMagnitudeSquared();
            if (distanceSquared < bestDistanceSquared)
            {
                bestChoice = selectedPrimes[i];
                bestDistanceSquared = distanceSquared;
            }
        }

        vector<EntityRef> onlyOnePrime;
        onlyOnePrime.push_back(bestChoice->ref);
        return boost::shared_ptr<PrimeBuildCmd>(new PrimeBuildCmd(onlyOnePrime, buildUnitTypechar, buildPos));
    }
    return boost::shared_ptr<PrimeBuildCmd>();
}

vector<boost::shared_ptr<Cmd>> pollWindowEventsAndUpdateUI(Game *game, UI *ui, int playerIdOrNeg1, sf::RenderWindow *window)
{
    bool spawnBeaconAvailable = playerIdOrNeg1 < 0 ?
        false
        : ((game->getPlayerBeaconAvailable(playerIdOrNeg1)) && game->players[playerIdOrNeg1].credit.getInt() >= BEACON_COST);

    ui->updateAvailableUnitInterfaceCmds(spawnBeaconAvailable);

    vector<boost::shared_ptr<Cmd>> cmdsToSend;
    sf::Event event;
    while (window->pollEvent(event))
    {
        switch (event.type)
        {
        case sf::Event::Closed:
            window->close();
            break;
        case sf::Event::MouseMoved:
            {
                if (sf::Mouse::isButtonPressed(sf::Mouse::Button::Middle))
                {
                    vector2i moveVector = mouseMoveToVec(event.mouseMove) - ui->lastMousePos;
                    moveVector.y *= -1;
                    ui->camera.gamePos -= moveVector;
                }
                Target target = getTargetAtScreenPos(*game, ui->camera, mouseMoveToVec(event.mouseMove));
                ui->mouseoverEntity = target.castToEntityPtr(*game);

                ui->lastMousePos = mouseMoveToVec(event.mouseMove);
            }
            break;
        case sf::Event::MouseButtonReleased:
            if (event.mouseButton.button == sf::Mouse::Left)
            {
                if (ui->maybeSelectionBoxStart)
                {
                    vector2f mousePos = mouseButtonToVec(event.mouseButton);
                    if ((*ui->maybeSelectionBoxStart - mousePos).getMagnitudeSquared() <= 25)
                    {
                        vector2i averagedClick = (*ui->maybeSelectionBoxStart + mousePos) / 2;
                        if (boost::shared_ptr<Entity> clickedEntity = getTargetAtScreenPos(*game, ui->camera, averagedClick).castToEntityPtr(*game))
                        {
                            if (auto clickedUnit = boost::dynamic_pointer_cast<Unit, Entity>(clickedEntity))
                            {
                                if (clickedUnit->ownerId == playerIdOrNeg1)
                                {
                                    if (!isShiftPressed())
                                    {
                                        ui->selectedUnits.clear();
                                    }
                                    ui->selectedUnits.push_back(clickedUnit);
                                }
                            }
                        }
                    }
                    else
                    {
                        tuple<vector2i, vector2i> corners =
                        {
                            screenPosToGamePos(ui->camera, *ui->maybeSelectionBoxStart),
                            screenPosToGamePos(ui->camera, mousePos)
                        };
                        int rectLeft = min(get<0>(corners).x, get<1>(corners).x);
                        int rectRight = max(get<0>(corners).x, get<1>(corners).x);
                        int rectBottom = min(get<0>(corners).y, get<1>(corners).y);
                        int rectTop = max(get<0>(corners).y, get<1>(corners).y);

                        // For arg 2 here, we use rectBottom instead of rectTop, since we're using +Y=up here, while SFML assumes +Y=down
                        sf::Rect selectionRectGameCoords(rectLeft, rectBottom, (rectRight - rectLeft), (rectTop - rectBottom));

                        if (!isShiftPressed())
                        {
                            ui->selectedUnits.clear();
                        }
                        for (unsigned int i=0; i<game->entities.size(); i++)
                        {
                            if (auto unit = boost::dynamic_pointer_cast<Unit, Entity>(game->entities[i]))
                            {
                                if (unit->ownerId == playerIdOrNeg1)
                                {
                                    if (selectionRectGameCoords.contains(sf::Vector2i(unit->pos.x, unit->pos.y)))
                                    {
                                        ui->selectedUnits.push_back(unit);
                                    }
                                }
                            }
                        }
                    }

                    ui->maybeSelectionBoxStart = {};
                }
            }
            break;
        case sf::Event::MouseButtonPressed:
            if (event.mouseButton.button == sf::Mouse::Left)
            {
                switch (ui->cmdState)
                {
                    case UI::Default:
                    {
                        ui->maybeSelectionBoxStart = {vector2i(mouseButtonToVec(event.mouseButton))};
                    }
                    break;
                    case UI::SpawnBeacon:
                    {
                        vector2f spawnPos = screenPosToGamePos(ui->camera, mouseButtonToVec(event.mouseButton));

                        cmdsToSend.push_back(boost::shared_ptr<Cmd>(new SpawnBeaconCmd(spawnPos)));
                        ui->cmdState = UI::Default;
                    }
                    break;
                    case UI::Deposit:
                    {
                        Target target = getTargetAtScreenPos(*game, ui->camera, mouseButtonToVec(event.mouseButton));

                        vector<boost::shared_ptr<Unit>> primesInSelection = filterForTypeKeepContainer<Prime, Unit>(ui->selectedUnits);
                        vector<boost::shared_ptr<Unit>> gatewaysInSelection = filterForTypeKeepContainer<Gateway, Unit>(ui->selectedUnits);
                        auto primesAndGateways = primesInSelection;
                        primesAndGateways.insert(primesAndGateways.begin(), gatewaysInSelection.begin(), gatewaysInSelection.end());

                        if (primesAndGateways.size() > 0)
                        {
                            cmdsToSend.push_back(boost::shared_ptr<Cmd>(new PutdownCmd(entityPtrsToRefs(primesAndGateways), target)));
                            ui->cmdState = UI::Default;
                        }
                    }
                    break;
                    case UI::Build:
                    {
                        vector<boost::shared_ptr<Unit>> primesInSelection = filterForTypeKeepContainer<Prime, Unit>(ui->selectedUnits);
                        vector2f buildPos = screenPosToGamePos(ui->camera, mouseButtonToVec(event.mouseButton));
                        cmdsToSend.push_back(makePrimeBuildCmd(ui->selectedUnits, ui->ghostBuilding->typechar(), buildPos));
                        if (!isShiftPressed())
                        {
                            ui->cmdState = UI::Default;
                        }
                    }
                    break;
                    case UI::Scuttle:
                    {
                        if (auto targetEntity = getTargetAtScreenPos(*game, ui->camera, mouseButtonToVec(event.mouseButton)).castToEntityPtr(*game))
                        {
                        if (getAllianceType(playerIdOrNeg1, targetEntity) == Owned || targetEntity->typechar() == GOLDPILE_TYPECHAR)
                        {
                        if (boost::dynamic_pointer_cast<Unit, Entity>(targetEntity) || targetEntity->typechar() == GOLDPILE_TYPECHAR)
                        {
                            vector<boost::shared_ptr<Prime>> primesInSelection = filterForType<Prime, Unit>(ui->selectedUnits);
                            vector<boost::shared_ptr<Gateway>> gatewaysInSelection = filterForType<Gateway, Unit>(ui->selectedUnits);
                            // auto gatewaysAndPrimes = gatewaysInSelection;
                            // gatewaysAndPrimes.insert(gatewaysAndPrimes.begin(), primesInSelection.begin(), primesInSelection.end());

                            if (gatewaysInSelection.size() + primesInSelection.size() > 0)
                            {
                                boost::shared_ptr<Unit> bestChoice;

                                float bestGatewayDistanceSquared;
                                for (unsigned int i=0; i<gatewaysInSelection.size(); i++)
                                {
                                    if (!bestChoice)
                                    {
                                        bestChoice = gatewaysInSelection[i];
                                        bestGatewayDistanceSquared = (gatewaysInSelection[i]->pos - targetEntity->pos).getMagnitudeSquared();
                                    }
                                    else
                                    {
                                        float distanceSquared = (gatewaysInSelection[i]->pos - targetEntity->pos).getMagnitudeSquared();
                                        if (distanceSquared < bestGatewayDistanceSquared)
                                        {
                                            bestChoice = gatewaysInSelection[i];
                                            bestGatewayDistanceSquared = distanceSquared;
                                        }
                                    }
                                }
                                // if we still don't have a best choice, look through Primes
                                if (!bestChoice)
                                {
                                    float bestPrimeDistanceSquared;
                                    for (unsigned int i=0; i<primesInSelection.size(); i++)
                                    {
                                        if (!bestChoice)
                                        {
                                            bestChoice = primesInSelection[i];
                                            bestPrimeDistanceSquared = (primesInSelection[i]->pos - targetEntity->pos).getMagnitudeSquared();
                                        }
                                        else
                                        {
                                            float distanceSquared = (primesInSelection[i]->pos - targetEntity->pos).getMagnitudeSquared();
                                            if (distanceSquared < bestPrimeDistanceSquared)
                                            {
                                                bestChoice = primesInSelection[i];
                                                bestPrimeDistanceSquared = distanceSquared;
                                            }
                                        }
                                    }
                                }

                                if (!bestChoice)
                                {
                                    // we should have had a best choice by now...
                                    cout << "Can't find a bestChoice for the ScuttleCmd" << endl;
                                }
                                else
                                {
                                    if (auto gateway = boost::dynamic_pointer_cast<Gateway, Unit>(bestChoice))
                                    {
                                        cmdsToSend.push_back(boost::shared_ptr<Cmd>(new ScuttleCmd({gateway->ref}, targetEntity->ref)));
                                        ui->cmdState = UI::Default;
                                    }
                                    else if (auto prime = boost::dynamic_pointer_cast<Prime, Unit>(bestChoice))
                                    {
                                        cmdsToSend.push_back(boost::shared_ptr<Cmd>(new ScuttleCmd({prime->ref}, targetEntity->ref)));
                                        ui->cmdState = UI::Default;
                                    }
                                }
                            }
                        }}}
                    }
                    break;
                }
            }
            else if (event.mouseButton.button == sf::Mouse::Right)
            {
                if (ui->cmdState == UI::Default)
                {
                    if (ui->selectedUnits.size() > 0)
                    {
                        if (playerIdOrNeg1 >= 0)
                            cmdsToSend.push_back(makeRightclickCmd(*game, *ui, playerIdOrNeg1, getTargetAtScreenPos(*game, ui->camera, mouseButtonToVec(event.mouseButton))));
                    }
                }
                else
                {
                    ui->cmdState = UI::Default;
                }
            }
            break;
        case sf::Event::KeyPressed:
            switch (event.key.code)
            {
                case sf::Keyboard::Tilde:
                    ui->debugInt ++;
                    break;
                case sf::Keyboard::Tab:
                    ui->minimapEnabled = !(ui->minimapEnabled);
                    break;
                case sf::Keyboard::Escape:
                    if (ui->cmdState != UI::Default)
                    {
                        ui->cmdState = UI::Default;
                    }
                    else if (ui->selectedUnits.size() > 0)
                    {
                        ui->selectedUnits.clear();
                    }
                    else
                    {
                        ui->startEscapeToQuit();
                    }
                    break;
                case sf::Keyboard::H:
                    ui->cleanDrawEnabled = ! ui->cleanDrawEnabled;
                    break;
                default:
                    vector<boost::shared_ptr<Cmd>> cmds = ui->handlePossibleUnitInterfaceCmd(event.key.code);
                    cmdsToSend.insert(cmdsToSend.begin(), cmds.begin(), cmds.end());
                    break;
            }
            break;
        case sf::Event::KeyReleased:
            switch (event.key.code)
            {
                case sf::Keyboard::Escape:
                    ui->cancelEscapeToQuit();
                    break;
                default:
                    break;
            }
            break;
        default:
            break;
        }
    }
    return cmdsToSend;
}
