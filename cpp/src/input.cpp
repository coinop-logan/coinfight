#include "input.h"
#include "config.h"
#include "interface.h"
#include "tutorial.h"
#include "cmds.h"

extern Game game;
extern GameUI ui;
extern vector<boost::shared_ptr<Cmd>> cmdsToSend;

vector<boost::shared_ptr<Cmd>> noCmds;

bool isShiftPressed()
{
    return sf::Keyboard::isKeyPressed(sf::Keyboard::LShift) || sf::Keyboard::isKeyPressed(sf::Keyboard::RShift);
}
bool isCtrlPressed()
{
    return sf::Keyboard::isKeyPressed(sf::Keyboard::LControl) || sf::Keyboard::isKeyPressed(sf::Keyboard::RControl);
}

void printHi()
{
    cout << "hi" << endl;
}

GameUI::GameUI(sf::Font* font, bool online)
    : font(font), online(online)
{
    camera.gamePos = vector2fl(0, 0);
    lastMousePos = screenCenter;
    debugInt = 0;
    cmdState = Default;
    minimapEnabled = false;
    maybeSelectionBoxStart = {};
    unitInterfaceCmds = vector<boost::shared_ptr<UnitInterfaceCmd>>
    {
        boost::shared_ptr<UnitInterfaceCmd>(new GatewayBuildPrimeInterfaceCmd()),
        boost::shared_ptr<UnitInterfaceCmd>(new GatewayBuildFighterInterfaceCmd()),
        boost::shared_ptr<UnitInterfaceCmd>(new PrimeBuildGatewayInterfaceCmd()),
        boost::shared_ptr<UnitInterfaceCmd>(new PrimeBuildTurretInterfaceCmd()),
        boost::shared_ptr<UnitInterfaceCmd>(new AttackAbsorbInterfaceCmd()),
        boost::shared_ptr<UnitInterfaceCmd>(new StopInterfaceCmd()),
        boost::shared_ptr<UnitInterfaceCmd>(new DepositInterfaceCmd()),
        boost::shared_ptr<UnitInterfaceCmd>(new FetchInterfaceCmd())
    };
    quitNow = false;
    cleanDrawEnabled = false;
    showTutorial = true; // only matters for coinfight_local right now
    hideUX = false;
}

void GameUI::updateAvailableUnitInterfaceCmds(bool spawnBeaconAvailable)
{
    spawnBeaconInterfaceCmd.eligible = spawnBeaconAvailable;

    // set all to false, and create a list of pointers to track those we've yet to enable
    vector<boost::shared_ptr<InterfaceCmd>> notYetEnabledUnitInterfaceCmds;
    for (unsigned int i=0; i<unitInterfaceCmds.size(); i++)
    {
        unitInterfaceCmds[i]->eligible = false;
        notYetEnabledUnitInterfaceCmds.push_back(unitInterfaceCmds[i]);
    }

    for (unsigned int i=0; i<selectedUnits.size(); i++)
    {
        // if we've enabled all cmds, no need to keep looking
        if (notYetEnabledUnitInterfaceCmds.size() == 0)
            break;

        for (unsigned int j=0; j<notYetEnabledUnitInterfaceCmds.size(); j++)
        {
            if (auto unitInterfaceCmd = boost::dynamic_pointer_cast<UnitInterfaceCmd, InterfaceCmd>(notYetEnabledUnitInterfaceCmds[j]))
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

void GameUI::selectAllUnitsOfSimilarTypeOnScreen(Game* game, boost::shared_ptr<Unit> targetUnit)
{
    vector2i halfScreenDimensions = screenDimensions / 2;
    vector2fp corner1 = this->camera.gamePos - vector2fp(halfScreenDimensions);
    vector2fp corner2 = this->camera.gamePos + vector2fp(halfScreenDimensions);

    auto visibleEntities = game->entitiesWithinRect(corner1, corner2);
    auto visibleUnits = filterForType<Unit, Entity>(visibleEntities);

    vector<boost::shared_ptr<Unit>> ownedVisibleUnitsOfType;
    // filter for units that have same owner and same type
    copy_if(visibleUnits.begin(), visibleUnits.end(),
        back_inserter(ownedVisibleUnitsOfType),
        [targetUnit](boost::shared_ptr<Unit> unit)
            {
                return unit->ownerId == targetUnit->ownerId && unit->typechar() == targetUnit->typechar();
            }
    );

    // we want to avoid duplicate entries - we must then copy the original, sort both lists, and merge.
    // copy the selection
    auto selectedUnitsCopy = this->selectedUnits;

    // clear and reserve space for this->selectedUnits
    this->selectedUnits.clear();
    this->selectedUnits.reserve(selectedUnitsCopy.size() + visibleUnits.size());

    // sort both collections
    sort(selectedUnitsCopy.begin(), selectedUnitsCopy.end());
    sort(ownedVisibleUnitsOfType.begin(), ownedVisibleUnitsOfType.end());

    // merge via set_union
    set_union(selectedUnitsCopy.begin(), selectedUnitsCopy.end(), ownedVisibleUnitsOfType.begin(), ownedVisibleUnitsOfType.end(), back_inserter(this->selectedUnits));
}

void GameUI::openInGameMenu()
{
    vector<tuple<string, InGameMenuMsg>> menuOptions;
    
    menuOptions.push_back({"Resume", Resume});
    if (selectedUnits.size() > 0)
    {
        stringstream ss;
        ss << "Gift Selected Units (" << selectedUnits.size() << ")";
        menuOptions.push_back({ss.str(), GiftUnits});
    }
    if (online) menuOptions.push_back({"Withdraw Wallet Balance", Withdraw});
    menuOptions.push_back({"Disconnect", Disconnect});
    if (online) menuOptions.push_back({"Disconnect and Withdraw Wallet Balance", WithdrawAndDisconnect});

    inGameMenu = MainMenu(menuOptions, font);
}
void GameUI::iterate()
{
    // remove dead or null units from selectedUnits
    for (unsigned int i=0; i<selectedUnits.size(); i++)
    {
        if (!selectedUnits[i] || selectedUnits[i]->dead)
        {
            selectedUnits.erase(selectedUnits.begin() + i);
            i --;
        }
    }
}

void GameUI::removeDuplicatesFromSelection()
{
    for (unsigned int i=0; i<selectedUnits.size(); i++)
    {
        for (unsigned int j = i+1; j<selectedUnits.size(); j++)
        {
            if (selectedUnits[i]->getRefOrThrow() == selectedUnits[j]->getRefOrThrow())
            {
                selectedUnits.erase(selectedUnits.begin() + j);
                j --;
            }
        }
    }
}

vector<boost::shared_ptr<Cmd>> executeUnitInterfaceCmd(boost::shared_ptr<UnitInterfaceCmd> unitInterfaceCmd, GameUI* ui)
{
    if (boost::dynamic_pointer_cast<AttackAbsorbInterfaceCmd, UnitInterfaceCmd>(unitInterfaceCmd))
    {
        ui->returnToDefaultState();
        ui->cmdState = GameUI::AttackAbsorb;
        unitInterfaceCmd->active = true;

        return noCmds;
    }
    else if (boost::dynamic_pointer_cast<StopInterfaceCmd, UnitInterfaceCmd>(unitInterfaceCmd))
    {
        if (ui->selectedUnits.size() > 0)
        {
            unitInterfaceCmd->visualFlashClock.restart();
            return {boost::shared_ptr<StopCmd>(new StopCmd(entityPtrsToRefsOrThrow(ui->selectedUnits)))};
        }
        else
        {
            return noCmds;
        }
    }
    else if (boost::dynamic_pointer_cast<DepositInterfaceCmd, UnitInterfaceCmd>(unitInterfaceCmd))
    {
        ui->returnToDefaultState();
        ui->cmdState = GameUI::Deposit;
        unitInterfaceCmd->active = true;

        return noCmds;
    }
    else if (boost::dynamic_pointer_cast<FetchInterfaceCmd, UnitInterfaceCmd>(unitInterfaceCmd))
    {
        ui->returnToDefaultState();
        ui->cmdState = GameUI::Fetch;
        unitInterfaceCmd->active = true;

        return noCmds;
    }
    else if (boost::dynamic_pointer_cast<GatewayBuildPrimeInterfaceCmd, UnitInterfaceCmd>(unitInterfaceCmd))
    {
        if (auto cmd = makeGatewayBuildCmd(ui->selectedUnits, PRIME_TYPECHAR))
        {
            unitInterfaceCmd->visualFlashClock.restart();
            return {cmd};
        }
        else
            return noCmds;
    }
    else if (boost::dynamic_pointer_cast<GatewayBuildFighterInterfaceCmd, UnitInterfaceCmd>(unitInterfaceCmd))
    {
        if (auto cmd = makeGatewayBuildCmd(ui->selectedUnits, FIGHTER_TYPECHAR))
        {
            unitInterfaceCmd->visualFlashClock.restart();
            return {cmd};
        }
        else
            return noCmds;
    }
    else if (boost::dynamic_pointer_cast<PrimeBuildGatewayInterfaceCmd, UnitInterfaceCmd>(unitInterfaceCmd))
    {
        ui->returnToDefaultState();
        ui->cmdState = GameUI::Build;
        ui->ghostBuilding = boost::shared_ptr<Building>(new Gateway(-1, vector2fp::zero));
        unitInterfaceCmd->active = true;

        return noCmds;
    }
    else if (boost::dynamic_pointer_cast<PrimeBuildTurretInterfaceCmd, UnitInterfaceCmd>(unitInterfaceCmd))
    {
        ui->returnToDefaultState();
        ui->cmdState = GameUI::Build;
        ui->ghostBuilding = boost::shared_ptr<Building>(new Turret(-1, vector2fp::zero));
        unitInterfaceCmd->active = true;

        return noCmds;
    }
    else
    {
        cout << "Logic error! I can't figure out which UnitInterfaceCmd that is!" << endl;
        return noCmds;
    }
}

vector<boost::shared_ptr<Cmd>> GameUI::handlePossibleUnitInterfaceCmd(sf::Keyboard::Key key)
{
    if (spawnBeaconInterfaceCmd.eligible && spawnBeaconInterfaceCmd.getKey() == key)
    {
        returnToDefaultState();
        cmdState = GameUI::SpawnBeacon;
        spawnBeaconInterfaceCmd.active = true;

        return noCmds;
    }

    for (unsigned int i=0; i<unitInterfaceCmds.size(); i++)
    {
        if (unitInterfaceCmds[i]->eligible && unitInterfaceCmds[i]->getKey() == key)
        {
            return executeUnitInterfaceCmd(unitInterfaceCmds[i], this);
        }
    }

    return {};
}

bool GameUI::selectionHasGateways()
{
    for (unsigned int i=0; i<selectedUnits.size(); i++)
    {
        if (auto unit = selectedUnits[i])
        {
            if (unit->typechar() == GATEWAY_TYPECHAR)
            {
                return true;
            }
        }
    }
    return false;
}

bool GameUI::selectionWouldStaySegregated(uint8_t typechar)
{
    if (selectedUnits.size() == 0) return true;

    // we assume that we have not yet violated segregation (i.e. mixing gateways/others)
    // so we can just check the first type

    // the first selected unit's gatewayness should equal the argument's gatewayness
    return ((selectedUnits[0]->typechar() == GATEWAY_TYPECHAR) == (typechar == GATEWAY_TYPECHAR));
}

vector2fp screenPosToGamePos(CameraState cameraState, vector2i screenPos)
{
    vector2i screenPosFromCenter = screenPos - screenCenter;
    vector2fp result = cameraState.gamePos + vector2fl(screenPosFromCenter.x, -screenPosFromCenter.y);

    return result;
}

vector2i gamePosToScreenPos(CameraState cameraState, vector2fp gamePos)
{
    vector2fl subtractedFromCamera = vector2fl(gamePos) - cameraState.gamePos;
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

Target getTargetAtScreenPos(Game *game, const CameraState &cameraState, vector2i screenPos)
{
    vector2fp gamePos = screenPosToGamePos(cameraState, screenPos);

    vector<EntityRef> nearbyEntities = game->searchGrid.nearbyEntitiesSloppyIncludingEmpty(gamePos, fixed32(100));

    boost::shared_ptr<Entity> closestValidEntity;
    float closestValidEntityDistance;
    for (unsigned int i = 0; i < nearbyEntities.size(); i++)
    {
        boost::shared_ptr<Entity> e = maybeEntityRefToPtrOrNull(*game, {nearbyEntities[i]});
        if (e)
        {
            if (e->collidesWithPoint(gamePos))
            {
                float distance = vector2fl((gamePos - e->getPos())).getMagnitude();
                if (!closestValidEntity || distance < closestValidEntityDistance)
                {
                    closestValidEntity = e;
                    closestValidEntityDistance = distance;
                }
            }
        }
    }
    if (closestValidEntity)
        return Target(closestValidEntity->getRefOrThrow());
    else
        return Target(gamePos);
}

boost::shared_ptr<Cmd> makeRightClickCmd(const Game &game, GameUI ui, int playerID, Target target)
{
    boost::shared_ptr<Cmd> noCmd;
    if (ui.selectedUnits.size() == 0)
    {
        return noCmd;
    }

    bool asap = !isShiftPressed();

    if (ui.selectionHasGateways())
    {
        if (auto point = target.castToPoint())
        {
            // in future, could set rally point
            return noCmd;
        }
        else if (auto entity = target.castToEntityPtr(game))
        {
            if (getAllianceType(playerID, entity) == Foreign)
            {
                return noCmd;
            }

            // We have to handle a right click on an entity, with possibly multiple GWs selected.

            // The first question is: is the entity already in any of the queues of the GWs?
            // If so, store these as pointers to the first such gateway
            boost::shared_ptr<Gateway> firstGatewayContainingEntityInBuildQueue;
            boost::shared_ptr<Gateway> firstGatewayContainingEntityInScuttleQueue;

            for (unsigned int i=0; i<ui.selectedUnits.size(); i++)
            {
                if (auto gateway = boost::dynamic_pointer_cast<Gateway, Unit>(ui.selectedUnits[i]))
                {
                    if (!firstGatewayContainingEntityInBuildQueue)
                    {
                        for (unsigned int j=0; j<gateway->buildTargetQueue.size(); j++)
                        {
                            if (entity->getRefOrThrow() == gateway->buildTargetQueue[j])
                            {
                                firstGatewayContainingEntityInBuildQueue = gateway;
                            }
                        }
                    }
                    if (!firstGatewayContainingEntityInScuttleQueue)
                    {
                        for (unsigned int j=0; j<gateway->scuttleTargetQueue.size(); j++)
                        {
                            if (entity->getRefOrThrow() == gateway->scuttleTargetQueue[j])
                            {
                                firstGatewayContainingEntityInScuttleQueue = gateway;
                            }
                        }
                    }
                }

                if (firstGatewayContainingEntityInBuildQueue && firstGatewayContainingEntityInScuttleQueue)
                    break;
            }

            bool isCompletedUnit;
            if (auto unit = boost::dynamic_pointer_cast<Unit, Entity>(entity))
            {
                isCompletedUnit = (unit->getBuiltRatio() == fixed32(1.0));
            }
            else
            {
                isCompletedUnit = false;
            }

            // We now have to determine a command based on the following info:
              // Whether the target already queued in scuttle and/or build queues; if so which one
              // Whether it's a gold pile or a unit
                // if it's a unit, whether it's built and active, or not less than fully built
            
            // We're also targeting the UX of: right clicking on a target with a GW will cycle between "relationships" to the unit:
            // no activity -> trying to build or deposit -> trying to capture or scuttle

            // A simple case to tackle first is that the entity is not in either queue.
            if (!firstGatewayContainingEntityInBuildQueue && !firstGatewayContainingEntityInScuttleQueue)
            {
                if (isCompletedUnit)
                {
                    // We want to scuttle.
                    // search through GWs to find the most appropriate one based on scuttle queue weight
                    coinsInt bestChoiceScore(0);
                    boost::shared_ptr<Gateway> bestChoice;
                    for (unsigned int i=0; i<ui.selectedUnits.size(); i++)
                    {
                        if (auto gateway = boost::dynamic_pointer_cast<Gateway, Entity>(ui.selectedUnits[i]))
                        {
                            coinsInt score = gateway->scuttleQueueWeight();
                            if (!bestChoice || score < bestChoiceScore)
                            {
                                bestChoice = gateway;
                                bestChoiceScore = score;
                            }
                        }
                    }
                    if (bestChoice)
                    {
                        return boost::shared_ptr<Cmd>(new GatewayScuttleCmd({bestChoice->getRefOrThrow()}, entity->getRefOrThrow()));
                    }
                    else
                    {
                        cout << "logic error: are there gateways in this selection or not?" << endl;

                    }
                }
                else
                {
                    return boost::shared_ptr<Cmd>(new DepositCmd(entityPtrsToRefsOrThrow(ui.selectedUnits), entity->getRefOrThrow(), asap));
                }
            }

            // In theory GWs in the selection could have different such relationships to the entity.
            // Not sure how to handle this case at the moment, so for now we just ignore it.
            else if (firstGatewayContainingEntityInBuildQueue && firstGatewayContainingEntityInScuttleQueue)
            {
                return noCmd;
            }

            else // logically, this means one but not both of the firstGateway(...) vars are set.
            {
                // We assume here that we aren't dealing with multiple gateways with this this relationship,
                // and only deal with the one we know about
                // Here we just "cycle" the relationship

                // if it's being built, switch to scuttle
                if (firstGatewayContainingEntityInBuildQueue) 
                {
                    return boost::shared_ptr<Cmd>(new FetchCmd({firstGatewayContainingEntityInBuildQueue->getRefOrThrow()}, entity->getRefOrThrow(), asap));
                }
                // if it's being scuttled, stop scuttling
                else if (firstGatewayContainingEntityInScuttleQueue)
                {
                    return boost::shared_ptr<Cmd>(new StopScuttleCmd({firstGatewayContainingEntityInScuttleQueue->getRefOrThrow()}, entity->getRefOrThrow()));
                }
            }
        }
        else
        {
            cout << "issue casting target to a point or entity in makeRightClickCmd" << endl;
            return noCmd;
        }
    }
    else // selectedUnits are not Gateways
    {
        if (auto point = target.castToPoint())
        {
            auto selectedMobileUnits = filterForTypeKeepContainer<MobileUnit, Unit>(ui.selectedUnits);
            return boost::shared_ptr<Cmd>(new MoveCmd(entityPtrsToRefsOrThrow(selectedMobileUnits), *point));
        }
        else if (boost::shared_ptr<Entity> entity = target.castToEntityPtr(game))
        {
            if (getAllianceType(playerID, entity) == Foreign)
            {
                auto selectedCombatUnits = filterForTypeKeepContainer<CombatUnit, Unit>(ui.selectedUnits);
                return boost::shared_ptr<Cmd>(new AttackAbsorbCmd(entityPtrsToRefsOrThrow(selectedCombatUnits), entity->getRefOrThrow(), asap));
            }
            else
            {
                if (auto goldPile = boost::dynamic_pointer_cast<GoldPile, Entity>(entity))
                {
                    auto primesInSelection = filterForTypeKeepContainer<Prime, Unit>(ui.selectedUnits);
                    return boost::shared_ptr<Cmd>(new FetchCmd(entityPtrsToRefsOrThrow(primesInSelection), entity->getRefOrThrow(), asap));
                }
                else if (auto unit = boost::dynamic_pointer_cast<Unit, Entity>(entity))
                {
                    if (unit->getBuiltRatio() < fixed32(1))
                    {
                        auto primesInSelection = filterForTypeKeepContainer<Prime, Unit>(ui.selectedUnits);
                        return boost::shared_ptr<Cmd>(new DepositCmd(entityPtrsToRefsOrThrow(primesInSelection), entity->getRefOrThrow(), asap));
                    }
                    else if (auto gateway = boost::dynamic_pointer_cast<Gateway, Unit>(unit))
                    {
                        auto primesInSelection = filterForType<Prime, Unit>(ui.selectedUnits);

                        bool foundPrimeWithGold = false;
                        vector<EntityRef> primeRefs;

                        for (unsigned int i=0; i<primesInSelection.size(); i++)
                        {
                            if (primesInSelection[i]->heldGold.getInt() > 0)
                            {
                                foundPrimeWithGold = true;
                            }
                            primeRefs.push_back(primesInSelection[i]->getRefOrThrow());
                        }

                        if (foundPrimeWithGold)
                        {
                            return boost::shared_ptr<Cmd>(new DepositCmd(primeRefs, Target(gateway), asap));
                        }
                        else
                        {
                            return boost::shared_ptr<Cmd>(new FetchCmd(primeRefs, Target(gateway), asap));
                        }
                    }
                }
                else
                {
                    cout << "Can't cast the entity to a unit or a GoldPile in makeRightClickCmd..." << endl;
                    return noCmd;
                }
            }
        }
        else
        {
            cout << "Can't cast the target to an entity or a point in makeRightClickCmd..." << endl;
            return noCmd;
        }
    }
    return noCmd;
}

boost::shared_ptr<Cmd> makeGatewayBuildCmd(vector<boost::shared_ptr<Unit>> selectedUnits, uint8_t buildUnitTypechar)
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
        onlyOneGateway.push_back(bestChoice->getRefOrThrow());
        return boost::shared_ptr<GatewayBuildCmd>(new GatewayBuildCmd(onlyOneGateway, buildUnitTypechar));
    }
    return boost::shared_ptr<GatewayBuildCmd>();
}

boost::shared_ptr<Cmd> makePrimeBuildCmd(vector<boost::shared_ptr<Unit>> selectedUnits, uint8_t buildUnitTypechar, vector2fl buildPos, bool asap)
{
    auto selectedPrimes = filterForTypeKeepContainer<Prime, Unit>(selectedUnits);
    if (selectedPrimes.size() > 0)
    {
        return boost::shared_ptr<PrimeBuildCmd>(new PrimeBuildCmd(entityPtrsToRefsOrThrow(selectedPrimes), buildUnitTypechar, vector2fp(buildPos), asap));
    }
    else
    {
        return boost::shared_ptr<Cmd>();
    }
}

vector<boost::shared_ptr<Cmd>> pollWindowEventsAndUpdateUI(Game *game, GameUI *ui, optional<uint8_t> maybePlayerId, sf::RenderWindow *window, Tutorial* tutorial)
{
    bool spawnBeaconAvailable = maybePlayerId ?
        ((game->getPlayerBeaconAvailable(*maybePlayerId)) && game->players[*maybePlayerId].credit.getInt() >= GATEWAY_COST)
        : false;

    ui->updateAvailableUnitInterfaceCmds(spawnBeaconAvailable);

    vector<boost::shared_ptr<Cmd>> cmdsToSend;
    sf::Event event;
    while (window->pollEvent(event))
    {
        if (ui->giftUnitsWindow)
        {
            optional<GiftUnitsWindow::Msg> maybeMsg = ui->giftUnitsWindow->processEvent(event);
            if (maybeMsg)
            {
                switch (*maybeMsg)
                {
                    case GiftUnitsWindow::Back:
                    {
                        ui->giftUnitsWindow = {};
                        break;
                    }
                    case GiftUnitsWindow::AddressSubmitted:
                    {
                        if (ui->giftUnitsWindow->submittedAddress)
                        {
                            auto newOnwerId = game->getPlayerId_createIfNone(*ui->giftUnitsWindow->submittedAddress);
                            
                            cmdsToSend.push_back(boost::shared_ptr<Cmd>(new GiftCmd(entityPtrsToRefsOrThrow(ui->selectedUnits), newOnwerId)));
                            ui->giftUnitsWindow = {};
                        }
                        break;
                    }
                }
            }
        }
        else if (ui->inGameMenu)
        {
            optional<InGameMenuMsg> maybeMsg = ui->inGameMenu->processEvent(event);
            if (maybeMsg) {
                switch (*maybeMsg)
                {
                    case Resume:
                    {
                        ui->inGameMenu = {};
                        break;
                    }
                    case GiftUnits:
                    {
                        ui->giftUnitsWindow = GiftUnitsWindow(screenCenter, ui->font);
                        break;
                    }
                    case Withdraw:
                    {
                        if (maybePlayerId)
                        {
                            coinsInt playerCredit = game->players[*maybePlayerId].credit.getInt();
                            if (playerCredit != 0)
                            {
                                cmdsToSend.push_back(boost::shared_ptr<Cmd>(new WithdrawCmd(playerCredit)));
                            }
                        }
                        break;
                    }
                    case Disconnect:
                    {
                        ui->quitNow = true;
                        break;
                    }
                    case WithdrawAndDisconnect:
                    {
                        coinsInt playerCredit = game->players[*maybePlayerId].credit.getInt();
                        if (playerCredit != 0)
                        {
                            cmdsToSend.push_back(boost::shared_ptr<Cmd>(new WithdrawCmd(playerCredit)));
                        }
                        ui->quitNow = true;
                        break;
                    }
                    default:
                        break;
                }
            }
        }
        else
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
                        vector2i mousePos = mouseMoveToVec(event.mouseMove);
                        vector2i moveVector = mousePos - ui->lastMousePos;
                        moveVector.y *= -1;
                        ui->camera.gamePos -= moveVector;
                    }

                    Target target = getTargetAtScreenPos(game, ui->camera, mouseMoveToVec(event.mouseMove));
                    ui->mouseoverEntity = target.castToEntityPtr(*game);

                    ui->lastMousePos = mouseMoveToVec(event.mouseMove);
                }
                break;
            case sf::Event::MouseButtonReleased:
                if (event.mouseButton.button == sf::Mouse::Left)
                {
                    sf::Time sinceLastLClick = ui->lClickClock.getElapsedTime();
                    ui->lClickClock.restart();
                    bool isDoubleClick = (sinceLastLClick < sf::milliseconds(300));

                    if (auto playerId = maybePlayerId)
                    {
                        if (ui->maybeSelectionBoxStart)
                        {
                            vector2fl mousePos = mouseButtonToVec(event.mouseButton);
                            if ((*ui->maybeSelectionBoxStart - mousePos).getMagnitudeSquared() <= 25)
                            {
                                vector2i averagedClick = (*ui->maybeSelectionBoxStart + mousePos) / 2;
                                if (boost::shared_ptr<Entity> clickedEntity = getTargetAtScreenPos(game, ui->camera, averagedClick).castToEntityPtr(*game))
                                {
                                    if (auto clickedUnit = boost::dynamic_pointer_cast<Unit, Entity>(clickedEntity))
                                    {
                                        if (clickedUnit->ownerId == *playerId)
                                        {
                                            if (!isShiftPressed())
                                            {
                                                ui->selectedUnits.clear();
                                            }

                                            if (ui->selectionWouldStaySegregated(clickedUnit->typechar()))
                                            {
                                                if (isCtrlPressed() || isDoubleClick)
                                                {
                                                    ui->selectAllUnitsOfSimilarTypeOnScreen(game, clickedUnit);
                                                }
                                                else
                                                {
                                                    bool removedFromSelection = false;
                                                    if (isShiftPressed())
                                                    {
                                                        // if it's in the selection, remove it
                                                        for (unsigned int i=0; i<ui->selectedUnits.size(); i++)
                                                        {
                                                            if (ui->selectedUnits[i]->getRefOrThrow() == clickedUnit->getRefOrThrow())
                                                            {
                                                                ui->selectedUnits.erase(ui->selectedUnits.begin() + i);
                                                                i --;
                                                                removedFromSelection = true;
                                                                // we don't break because there may be duplicates in the list at this point
                                                            }
                                                        }
                                                    }
                                                    if (!removedFromSelection)
                                                    {
                                                        ui->selectedUnits.push_back(clickedUnit);
                                                    }
                                                }
                                            }
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

                                auto entitiesInSelectionBox = game->entitiesWithinRect(get<0>(corners), get<1>(corners));

                                // we need to preserve selection segregation:
                                // if selection is non-empty, just preserve existing segregation
                                // if selection is empty, scan box for any non-GW units, and decide based on that
                                bool filterForGateways;
                                if (ui->selectedUnits.size() > 0)
                                {
                                    filterForGateways = (ui->selectedUnits[0]->typechar() == GATEWAY_TYPECHAR);
                                }
                                else
                                {
                                    // scan box for non-GW units
                                    bool foundNonGWUnit = false;
                                    for (unsigned int i=0; i<entitiesInSelectionBox.size(); i++)
                                    {
                                        if (getAllianceType(maybePlayerId, entitiesInSelectionBox[i]) == Owned)
                                        {
                                            if (entitiesInSelectionBox[i]->typechar() != GATEWAY_TYPECHAR)
                                            {
                                                foundNonGWUnit = true;
                                                break;
                                            }
                                        }
                                    }

                                    if (foundNonGWUnit)
                                    {
                                        filterForGateways = false;
                                    }
                                    else
                                    {
                                        filterForGateways = true;
                                    }
                                }

                                for (unsigned int i=0; i<entitiesInSelectionBox.size(); i++)
                                {
                                    if (auto unit = boost::dynamic_pointer_cast<Unit, Entity>(entitiesInSelectionBox[i]))
                                    {
                                        // finally, filter out units for segregation
                                        if ((filterForGateways && unit->typechar() == GATEWAY_TYPECHAR)
                                        || (!filterForGateways && unit->typechar() != GATEWAY_TYPECHAR))
                                        {
                                            if (unit->ownerId == *playerId)
                                            {
                                                if (selectionRectGameCoords.contains(toSFVec(vector2i(unit->getPos()))))
                                                {
                                                    ui->selectedUnits.push_back(unit);
                                                }
                                            }
                                        }
                                    }
                                }
                            }

                            ui->maybeSelectionBoxStart = {};
                        }
                    }

                    ui->removeDuplicatesFromSelection();
                }
                break;
            case sf::Event::MouseButtonPressed:
                if (auto playerId = maybePlayerId)
                {
                    bool asap = !isShiftPressed();

                    if (event.mouseButton.button == sf::Mouse::Left)
                    {
                        switch (ui->cmdState)
                        {
                            case GameUI::Default:
                            {
                                ui->maybeSelectionBoxStart = {vector2i(mouseButtonToVec(event.mouseButton))};
                            }
                            break;
                            case GameUI::SpawnBeacon:
                            {
                                vector2fp spawnPos = screenPosToGamePos(ui->camera, mouseButtonToVec(event.mouseButton));

                                cmdsToSend.push_back(boost::shared_ptr<Cmd>(new SpawnBeaconCmd(spawnPos)));
                                if (!isShiftPressed())
                                {
                                    ui->returnToDefaultState();
                                }
                            }
                            break;
                            case GameUI::AttackAbsorb:
                            {
                                Target target = getTargetAtScreenPos(game, ui->camera, mouseButtonToVec(event.mouseButton));

                                if (ui->selectionHasGateways())
                                {
                                    if (auto entity = target.castToEntityPtr(*game))
                                    {
                                        cmdsToSend.push_back(boost::shared_ptr<Cmd>(new GatewayScuttleCmd(entityPtrsToRefsOrThrow(ui->selectedUnits), entity->getRefOrThrow())));
                                        if (!isShiftPressed())
                                        {
                                            ui->returnToDefaultState();
                                        }
                                    }
                                }
                                else
                                {
                                    vector<boost::shared_ptr<Unit>> fightersInSelection = filterForTypeKeepContainer<Fighter, Unit>(ui->selectedUnits);
                                    vector<boost::shared_ptr<Unit>> primesInSelection = filterForTypeKeepContainer<Prime, Unit>(ui->selectedUnits);
                                    auto fightersAndPrimes = fightersInSelection;
                                    fightersAndPrimes.insert(fightersAndPrimes.begin(), primesInSelection.begin(), primesInSelection.end());

                                    if (fightersAndPrimes.size() > 0)
                                    {
                                        cmdsToSend.push_back(boost::shared_ptr<Cmd>(new AttackAbsorbCmd(entityPtrsToRefsOrThrow(fightersAndPrimes), target, asap)));
                                        if (!isShiftPressed())
                                        {
                                            ui->returnToDefaultState();
                                        }
                                    }
                                }
                            }
                            break;
                            case GameUI::Deposit:
                            {
                                Target target = getTargetAtScreenPos(game, ui->camera, mouseButtonToVec(event.mouseButton));

                                vector<boost::shared_ptr<Unit>> primesInSelection = filterForTypeKeepContainer<Prime, Unit>(ui->selectedUnits);
                                vector<boost::shared_ptr<Unit>> gatewaysInSelection = filterForTypeKeepContainer<Gateway, Unit>(ui->selectedUnits);
                                auto primesAndGateways = primesInSelection;
                                primesAndGateways.insert(primesAndGateways.begin(), gatewaysInSelection.begin(), gatewaysInSelection.end());

                                if (primesAndGateways.size() > 0)
                                {
                                    cmdsToSend.push_back(boost::shared_ptr<Cmd>(new DepositCmd(entityPtrsToRefsOrThrow(primesAndGateways), target, asap)));
                                    if (!isShiftPressed())
                                    {
                                        ui->returnToDefaultState();
                                    }
                                }
                            }
                            break;
                            case GameUI::Fetch:
                            {
                                Target clickTarget = getTargetAtScreenPos(game, ui->camera, mouseButtonToVec(event.mouseButton));
                                if (auto targetEntity = clickTarget.castToEntityPtr(*game))
                                {
                                    if (getAllianceType(*playerId, targetEntity) == Owned || targetEntity->typechar() == GOLDPILE_TYPECHAR)
                                    {
                                        if (ui->selectionHasGateways())
                                        {
                                            vector<boost::shared_ptr<Gateway>> gatewaysInSelection = filterForType<Gateway, Unit>(ui->selectedUnits);

                                            boost::shared_ptr<Gateway> bestChoice;

                                            float bestGatewayDistanceSquared;
                                            for (unsigned int i=0; i<gatewaysInSelection.size(); i++)
                                            {
                                                vector2fl gatewayPos(gatewaysInSelection[i]->getPos());

                                                if (!bestChoice)
                                                {
                                                    bestChoice = gatewaysInSelection[i];
                                                    bestGatewayDistanceSquared = (gatewayPos - vector2fl(targetEntity->getPos())).getMagnitudeSquared();
                                                }
                                                else
                                                {
                                                    float distanceSquared = (gatewayPos - vector2fl(targetEntity->getPos())).getMagnitudeSquared();
                                                    if (distanceSquared < bestGatewayDistanceSquared)
                                                    {
                                                        bestChoice = gatewaysInSelection[i];
                                                        bestGatewayDistanceSquared = distanceSquared;
                                                    }
                                                }
                                            }

                                            if (!bestChoice)
                                            {
                                                // we should have had a best choice by now...
                                                cout << "Can't find a bestChoice for the GatewayScuttleCmd" << endl;
                                            }
                                            else
                                            {
                                                cmdsToSend.push_back(boost::shared_ptr<Cmd>(new GatewayScuttleCmd({bestChoice->getRefOrThrow()}, targetEntity->getRefOrThrow())));
                                                if (!isShiftPressed())
                                                {
                                                    ui->returnToDefaultState();
                                                }
                                            }
                                        }
                                        else
                                        {
                                            // selection does not have Gateways. Issue cmd to all Primes.
                                            auto primesInSelection = filterForTypeKeepContainer<Prime, Unit>(ui->selectedUnits);
                                            if (primesInSelection.size() > 0)
                                            {
                                                cmdsToSend.push_back(boost::shared_ptr<Cmd>(new FetchCmd(entityPtrsToRefsOrThrow(primesInSelection), targetEntity->getRefOrThrow(), asap)));
                                                ui->returnToDefaultState();
                                            }
                                        }
                                    }
                                }
                                else if (auto targetPoint = clickTarget.castToPoint())
                                {
                                    // selection does not have Gateways. Issue cmd to all Primes.
                                    auto primesInSelection = filterForTypeKeepContainer<Prime, Unit>(ui->selectedUnits);
                                    if (primesInSelection.size() > 0)
                                    {
                                        cmdsToSend.push_back(boost::shared_ptr<Cmd>(new FetchCmd(entityPtrsToRefsOrThrow(primesInSelection), clickTarget, asap)));
                                        ui->returnToDefaultState();
                                    }
                                }
                            }
                            break;
                            case GameUI::Build:
                            {
                                vector<boost::shared_ptr<Unit>> primesInSelection = filterForTypeKeepContainer<Prime, Unit>(ui->selectedUnits);
                                vector2fl buildPos(screenPosToGamePos(ui->camera, mouseButtonToVec(event.mouseButton)));
                                cmdsToSend.push_back(makePrimeBuildCmd(ui->selectedUnits, ui->ghostBuilding->typechar(), buildPos, asap));
                                if (!isShiftPressed())
                                {
                                    ui->returnToDefaultState();
                                }
                            }
                            break;
                        }
                    }
                    else if (event.mouseButton.button == sf::Mouse::Right)
                    {
                        if (ui->cmdState == GameUI::Default)
                        {
                            if (ui->selectedUnits.size() > 0)
                            {
                                if (playerId)
                                    cmdsToSend.push_back(makeRightClickCmd(*game, *ui, *playerId, getTargetAtScreenPos(game, ui->camera, mouseButtonToVec(event.mouseButton))));
                            }
                        }
                        else
                        {
                            ui->returnToDefaultState();
                        }
                    }
                }
                break;
            case sf::Event::KeyPressed:
                switch (event.key.code)
                {
                    case sf::Keyboard::F1:
                        ui->showTutorial = !ui->showTutorial;
                        break;
                    case sf::Keyboard::Enter:
                        if (tutorial)
                            tutorial->enterPressed();
                        break;
                    case sf::Keyboard::Tilde:
                        ui->debugInt ++;
                        break;
                    case sf::Keyboard::Tab:
                        ui->minimapEnabled = !(ui->minimapEnabled);
                        break;
                    case sf::Keyboard::F10:
                        if (!ui->inGameMenu)
                        {
                            ui->openInGameMenu();
                        }
                        break;
                    case sf::Keyboard::Escape:
                        if (ui->cmdState != GameUI::Default)
                        {
                            ui->returnToDefaultState();
                        }
                        else if (ui->selectedUnits.size() > 0)
                        {
                            ui->selectedUnits.clear();
                        }
                        else
                        {
                            ui->openInGameMenu();
                        }
                        break;
                    case sf::Keyboard::H:
                        ui->cleanDrawEnabled = ! ui->cleanDrawEnabled;
                        break;
                    case sf::Keyboard::J:
                        ui->hideUX = ! ui->hideUX;
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
                    default:
                        break;
                }
                break;
            default:
                break;
            }
        }

        if (! sf::Mouse::isButtonPressed(sf::Mouse::Button::Middle))
        {
            // mouse-edge camera move
            vector2i screenEdgeCameraMove;
            screenEdgeCameraMove.x =
                (ui->lastMousePos.x == 0)                       ?  - SCREEN_EDGE_SCROLL_AMOUNT :
                (screenDimensions.x - ui->lastMousePos.x == 1)  ?    SCREEN_EDGE_SCROLL_AMOUNT :
                0;
            screenEdgeCameraMove.y =
                (ui->lastMousePos.y == 0)                       ?    SCREEN_EDGE_SCROLL_AMOUNT :
                (screenDimensions.y - ui->lastMousePos.y == 1)  ?  - SCREEN_EDGE_SCROLL_AMOUNT :
                0;

            ui->camera.gamePos += screenEdgeCameraMove;
        }

        // constrain camera
        if (ui->camera.gamePos.getMagnitude() > 4500)
        {
            ui->camera.gamePos = composeVector2i(ui->camera.gamePos.getAngle(), 4500);
        }
    }

    return cmdsToSend;
}

void GameUI::returnToDefaultState()
{
    cmdState = Default;
    spawnBeaconInterfaceCmd.active = false;
    for (unsigned int i=0; i<unitInterfaceCmds.size(); i++)
    {
        unitInterfaceCmds[i]->active = false;
    }
}