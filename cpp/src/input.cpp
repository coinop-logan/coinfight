#include "input.h"
#include "config.h"
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
bool isAltPressed()
{
    return sf::Keyboard::isKeyPressed(sf::Keyboard::LAlt) || sf::Keyboard::isKeyPressed(sf::Keyboard::RAlt);
}

void printHi()
{
    cout << "hi" << endl;
}

GameUI::GameUI(sf::RenderWindow* window, sf::Font* font, sf::Sprite* (*getSpriteForMsg)(KeyButtonMsg), sf::View uxView, bool online)
  : font(font),
    selectedUnits(),
    unitInfoBox(
        vector2i(
            0,
            getViewSize(window, uxView).y - UNITINFOBOX_SIZE.y
        ),
        font,
        &selectedUnits
    ),
    keyButtonBox(
        vector2i(
            UNITINFOBOX_SIZE.x,
            getViewSize(window, uxView).y - KEYBUTTONBOX_SIZE.y
        ),
        font,
        getSpriteForMsg
    ),
    online(online)
{
    cameraView = window->getDefaultView();
    cameraView.setCenter(sf::Vector2f(0, 0));
    lastMousePos = getScreenSize(window) / 2;
    debugInt = 0;
    cmdState = Default;
    minimapEnabled = false;
    maybeSelectionBoxStart = {};
    displayAllRadii = false;
    quitNow = false;
    cleanDrawEnabled = false;
    showTutorial = true; // only matters for coinfight_local right now
    hideUX = false;
}

void GameUI::updateUnitCmds(bool spawnBeaconAvailable)
{
    KeyButtonHintInfo tempDefault("Temp", dollarsToCoinsIntND(1), 't', "Temp description!", {"bullet 1", "bullet 2"});
    
    keyButtonBox.clearActionInfos();

    if (selectedUnits.size() == 0)
    {
        if (spawnBeaconAvailable)
        {
            keyButtonBox.setUnitCmdOrThrow(
                sf::Keyboard::W,
                KeyButtonHintInfo(
                    "Warp In Gateway",
                    GATEWAY_COST,
                    'W',
                    "Warp in a Gateway, investing $4. This can only be done once per game.",
                    {
                        "Gateways allow you to invest in an army, and can pull in-game gold into your Coinfight wallet for withdrawal.",
                        "The warp-in takes about 20 seconds. During this time you are vulnerable to attack.",
                        "The warp can be reversed before completion, refunding the $4 and allowing you to warp in somewhere else."
                    }
                ),
                WarpIn
            );
        }
    }
    else // selectedUnits.size > 0
    {
        // We iterate through all selectedEntities to determine a few facts about the unit makeup
        bool foundNonBeaconUnits = false;
        bool foundBeaconSpawning = false;
        bool selectionHasGateways = false;
        bool foundCombatUnits = false;
        bool foundPrimes = false;
        for (unsigned int i=0; i<selectedUnits.size(); i++)
        {
            auto unit = selectedUnits[i];
            if (auto beacon = boost::dynamic_pointer_cast<Beacon, Unit>(unit))
            {
                if (beacon->state == Beacon::Spawning)
                {
                    foundBeaconSpawning = true;
                }
            }
            else
            {
                foundNonBeaconUnits = true;
            }

            if (auto gateway = boost::dynamic_pointer_cast<Gateway, Unit>(unit))
            {
                selectionHasGateways = true;
            }
            if (auto combatUnit = boost::dynamic_pointer_cast<CombatUnit, Unit>(unit))
            {
                foundCombatUnits = true;
            }
            if (auto prime = boost::dynamic_pointer_cast<Prime, Unit>(unit))
            {
                foundPrimes = true;
            }
        }

        if (!foundNonBeaconUnits)
        {
            if (foundBeaconSpawning)
            {
                keyButtonBox.setUnitCmdOrThrow(
                    sf::Keyboard::Q,
                    // KeyButtonHintInfo("Warp Out"),
                    tempDefault,
                    WarpOut
                );
            }
        }
        else
        {
            if (selectionHasGateways)
            {
                // Selection is kept GW-segregated, so we know the selection ONLY has gateways
                keyButtonBox.setUnitCmdOrThrow(
                    sf::Keyboard::Q,
                    // KeyButtonHintInfo("Build Prime"),
                    tempDefault,
                    KeyButtonMsg::BuildPrime
                );
                keyButtonBox.setUnitCmdOrThrow(
                    sf::Keyboard::W,
                    // KeyButtonHintInfo("Build Fighter"),
                    tempDefault,
                    KeyButtonMsg::BuildFighter
                );
                keyButtonBox.setUnitCmdOrThrow(
                    sf::Keyboard::A,
                    // KeyButtonHintInfo("Attack/Scuttle"),
                    tempDefault,
                    KeyButtonMsg::AttackScuttle
                );
                keyButtonBox.setUnitCmdOrThrow(
                    sf::Keyboard::S,
                    // KeyButtonHintInfo("Stop"),
                    tempDefault,
                    KeyButtonMsg::Stop
                );
                keyButtonBox.setUnitCmdOrThrow(
                    sf::Keyboard::D,
                    // KeyButtonHintInfo("Spend/Construct"),
                    tempDefault,
                    KeyButtonMsg::Invest
                );
                keyButtonBox.setUnitCmdOrThrow(
                    sf::Keyboard::F,
                    // KeyButtonHintInfo("Collect"),
                    tempDefault,
                    KeyButtonMsg::Fetch
                );
            }
            else // selection has non-GW units and at least one non-beacon unit
            {
                keyButtonBox.setUnitCmdOrThrow(
                    sf::Keyboard::S,
                    // KeyButtonHintInfo("Stop"),
                    tempDefault,
                    KeyButtonMsg::Stop
                );

                if (foundCombatUnits)
                {
                    keyButtonBox.setUnitCmdOrThrow(
                        sf::Keyboard::A,
                        // KeyButtonHintInfo("Attack"),
                        tempDefault,
                        KeyButtonMsg::AttackScuttle
                    );
                }
                else if (foundPrimes)
                {
                    keyButtonBox.setUnitCmdOrThrow(
                        sf::Keyboard::A,
                        // KeyButtonHintInfo("Scuttle/Collect"),
                        tempDefault,
                        KeyButtonMsg::AttackScuttle
                    );
                }

                if (foundPrimes)
                {
                    keyButtonBox.setUnitCmdOrThrow(
                        sf::Keyboard::E,
                        // KeyButtonHintInfo("Build Gateway"),
                        tempDefault,
                        KeyButtonMsg::BuildGateway
                    );
                    keyButtonBox.setUnitCmdOrThrow(
                        sf::Keyboard::R,
                        // KeyButtonHintInfo("Build Turret"),
                        tempDefault,
                        KeyButtonMsg::BuildTurret
                    );
                    keyButtonBox.setUnitCmdOrThrow(
                        sf::Keyboard::F,
                        // KeyButtonHintInfo("Fetch/Collect"),
                        tempDefault,
                        KeyButtonMsg::Fetch
                    );
                    keyButtonBox.setUnitCmdOrThrow(
                        sf::Keyboard::D,
                        // KeyButtonHintInfo("Deposit/Construct"),
                        tempDefault,
                        KeyButtonMsg::Invest
                    );
                }
            }
        }
    }
}

void GameUI::selectAllUnitsOfSimilarTypeOnScreen(sf::RenderWindow* window, Game* game, boost::shared_ptr<Unit> targetUnit)
{
    vector2fp corner1 = screenPosToGamePos(window, vector2i(0,0));
    vector2fp corner2 = screenPosToGamePos(window, getScreenSize(window));

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

void GameUI::openInGameMenu(sf::RenderWindow* window)
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

    inGameMenu = MainMenu(window, menuOptions, font);
}
void GameUI::iterate()
{
    displayAllRadii = isAltPressed();
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

vector2fp screenPosToGamePos(sf::RenderWindow* window, vector2i screenPos)
{
    return vector2fp(
        fromSFVec(
            window->mapPixelToCoords(
                toSFVecI(screenPos)
            )
        )
    );
}

vector2i gamePosToScreenPos(sf::RenderWindow* window, vector2fp gamePos)
{
    return fromSFVec(
        window->mapCoordsToPixel(
            toSFVecF(gamePos)
        )
    );
}

vector2i mouseEventVec(sf::Event::MouseButtonEvent mEvent)
{
    return vector2i(mEvent.x, mEvent.y);
}
vector2i mouseEventVec(sf::Event::MouseMoveEvent mEvent)
{
    return vector2i(mEvent.x, mEvent.y);
}

Target getTargetAtScreenPos(sf::RenderWindow* window, Game *game, vector2i screenPos)
{
    vector2fp gamePos = screenPosToGamePos(window, screenPos);

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

boost::shared_ptr<Cmd> makeRightClickCmd(const Game &game, GameUI* ui, int playerID, Target target)
{
    boost::shared_ptr<Cmd> noCmd;
    if (ui->selectedUnits.size() == 0)
    {
        return noCmd;
    }

    bool asap = !isShiftPressed();

    if (ui->selectionHasGateways())
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
                return boost::shared_ptr<Cmd>(new AttackScuttleCmd(entityPtrsToRefsOrThrow(ui->selectedUnits), target, true));
            }

            // We have to handle a right click on an entity, with possibly multiple GWs selected.

            // The first question is: is the entity already in any of the queues of the GWs?
            // If so, store these as pointers to the first such gateway
            boost::shared_ptr<Gateway> firstGatewayContainingEntityInBuildQueue;
            boost::shared_ptr<Gateway> firstGatewayContainingEntityInScuttleQueue;

            for (unsigned int i=0; i<ui->selectedUnits.size(); i++)
            {
                if (auto gateway = boost::dynamic_pointer_cast<Gateway, Unit>(ui->selectedUnits[i]))
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
                    // ... unless it's a Gateway - scuttling a GW is too serious a command to leave to right clicking!
                    // if it's a GW, for now just do nothing
                    if (auto gateway = boost::dynamic_pointer_cast<Gateway, Entity>(entity))
                    {
                        return noCmd;
                    }

                    // search through GWs to find the most appropriate one based on scuttle queue weight
                    coinsInt bestChoiceScore(0);
                    boost::shared_ptr<Gateway> bestChoice;
                    for (unsigned int i=0; i<ui->selectedUnits.size(); i++)
                    {
                        if (auto gateway = boost::dynamic_pointer_cast<Gateway, Entity>(ui->selectedUnits[i]))
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
                    return boost::shared_ptr<Cmd>(new DepositCmd(entityPtrsToRefsOrThrow(ui->selectedUnits), entity->getRefOrThrow(), asap));
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
            auto selectedMobileUnits = filterForTypeKeepContainer<MobileUnit, Unit>(ui->selectedUnits);
            return boost::shared_ptr<Cmd>(new MoveCmd(entityPtrsToRefsOrThrow(selectedMobileUnits), *point));
        }
        else if (boost::shared_ptr<Entity> entity = target.castToEntityPtr(game))
        {
            if (getAllianceType(playerID, entity) == Foreign)
            {
                auto selectedCombatUnits = filterForTypeKeepContainer<CombatUnit, Unit>(ui->selectedUnits);
                return boost::shared_ptr<Cmd>(new AttackScuttleCmd(entityPtrsToRefsOrThrow(selectedCombatUnits), entity->getRefOrThrow(), asap));
            }
            else
            {
                if (auto goldPile = boost::dynamic_pointer_cast<GoldPile, Entity>(entity))
                {
                    auto primesInSelection = filterForTypeKeepContainer<Prime, Unit>(ui->selectedUnits);
                    return boost::shared_ptr<Cmd>(new FetchCmd(entityPtrsToRefsOrThrow(primesInSelection), entity->getRefOrThrow(), asap));
                }
                else if (auto unit = boost::dynamic_pointer_cast<Unit, Entity>(entity))
                {
                    if (unit->getBuiltRatio() < fixed32(1))
                    {
                        auto primesInSelection = filterForTypeKeepContainer<Prime, Unit>(ui->selectedUnits);
                        return boost::shared_ptr<Cmd>(new DepositCmd(entityPtrsToRefsOrThrow(primesInSelection), entity->getRefOrThrow(), asap));
                    }
                    else if (auto gateway = boost::dynamic_pointer_cast<Gateway, Unit>(unit))
                    {
                        auto primesInSelection = filterForType<Prime, Unit>(ui->selectedUnits);

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

vector<boost::shared_ptr<Cmd>> pollWindowEventsAndUpdateUI(Game *game, GameUI *ui, optional<uint8_t> maybePlayerId, sf::RenderWindow *window, Tutorial* tutorial, sf::View uxView)
{
    bool spawnBeaconAvailable = maybePlayerId ?
        ((game->getPlayerBeaconAvailable(*maybePlayerId)) && game->players[*maybePlayerId].credit.getInt() >= GATEWAY_COST)
        : false;

    ui->updateUnitCmds(spawnBeaconAvailable);

    vector<boost::shared_ptr<Cmd>> cmdsToSend;
    sf::Event event;
    while (window->pollEvent(event))
    {
        if (event.type == sf::Event::Closed)
        {
            window->close();
            break;
        }

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
                        vector2i center = getScreenSize(window) / 2;

                        ui->giftUnitsWindow = GiftUnitsWindow(center, ui->font);
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
            bool overlayConsumedEvent = false; // until proven otherwise

            // If the user isn't middle-click-dragging or using a selectionBox, try interface with overlay
            if (!sf::Mouse::isButtonPressed(sf::Mouse::Button::Middle) && !ui->maybeSelectionBoxStart)
            {
                auto wasConsumedAndMaybeCmd = ui->processEventForOverlay(event, (getScreenSize(window).x / uxView.getSize().x));

                overlayConsumedEvent = get<0>(wasConsumedAndMaybeCmd);
                if (auto cmd = get<1>(wasConsumedAndMaybeCmd))
                {
                    cmdsToSend.push_back(*cmd);
                }
            }
            if (!overlayConsumedEvent)
            {
                switch (event.type)
                {
                case sf::Event::MouseMoved:
                    {
                        if (sf::Mouse::isButtonPressed(sf::Mouse::Button::Middle))
                        {
                            vector2i mousePos = mouseEventVec(event.mouseMove);

                            float zoomFactor = getCurrentViewSize(window).x / getScreenSize(window).x ;

                            vector2fl moveVector = vector2fl(mousePos - ui->lastMousePos) * zoomFactor;
                            ui->cameraView.move(toSFVecF(moveVector * -1));
                        }

                        Target target = getTargetAtScreenPos(window, game, mouseEventVec(event.mouseMove));
                        ui->mouseoverEntity = target.castToEntityPtr(*game);

                        ui->lastMousePos = mouseEventVec(event.mouseMove);
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
                                vector2fl mousePos = mouseEventVec(event.mouseButton);
                                if ((*ui->maybeSelectionBoxStart - mousePos).getMagnitudeSquared() <= 25)
                                {
                                    vector2i averagedClick = (*ui->maybeSelectionBoxStart + mousePos) / 2;
                                    if (boost::shared_ptr<Entity> clickedEntity = getTargetAtScreenPos(window, game, averagedClick).castToEntityPtr(*game))
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
                                                        ui->selectAllUnitsOfSimilarTypeOnScreen(window, game, clickedUnit);
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
                                        screenPosToGamePos(window, *ui->maybeSelectionBoxStart),
                                        screenPosToGamePos(window, mousePos)
                                    };
                                    int rectLeft = min(get<0>(corners).x, get<1>(corners).x);
                                    int rectRight = max(get<0>(corners).x, get<1>(corners).x);
                                    int rectBottom = min(get<0>(corners).y, get<1>(corners).y);
                                    int rectTop = max(get<0>(corners).y, get<1>(corners).y);

                                    // For arg1.x here, we use rectBottom instead of rectTop, since we're using +Y=up here, while SFML assumes +Y=down
                                    sf::Rect selectionRectGameCoords(sf::Vector2f(rectLeft, rectBottom), sf::Vector2f((rectRight - rectLeft), (rectTop - rectBottom)));

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
                                                    if (selectionRectGameCoords.contains(toSFVecF(vector2i(unit->getPos()))))
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
                                    ui->maybeSelectionBoxStart = {vector2i(mouseEventVec(event.mouseButton))};
                                }
                                break;
                                case GameUI::SpawnBeacon:
                                {
                                    vector2fp spawnPos = screenPosToGamePos(window, mouseEventVec(event.mouseButton));

                                    cmdsToSend.push_back(boost::shared_ptr<Cmd>(new SpawnBeaconCmd(spawnPos)));
                                    if (!isShiftPressed())
                                    {
                                        ui->returnToDefaultState();
                                    }
                                }
                                break;
                                case GameUI::AttackScuttle:
                                {
                                    Target target = getTargetAtScreenPos(window, game, mouseEventVec(event.mouseButton));

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
                                            cmdsToSend.push_back(boost::shared_ptr<Cmd>(new AttackScuttleCmd(entityPtrsToRefsOrThrow(fightersAndPrimes), target, asap)));
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
                                    Target target = getTargetAtScreenPos(window, game, mouseEventVec(event.mouseButton));

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
                                    Target clickTarget = getTargetAtScreenPos(window, game, mouseEventVec(event.mouseButton));
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
                                    vector2fl buildPos(screenPosToGamePos(window, mouseEventVec(event.mouseButton)));
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
                                        cmdsToSend.push_back(makeRightClickCmd(*game, ui, *playerId, getTargetAtScreenPos(window, game, mouseEventVec(event.mouseButton))));
                                }
                            }
                            else
                            {
                                ui->returnToDefaultState();
                            }
                        }
                    }
                    break;
                case sf::Event::MouseWheelMoved:
                {
                    bool zoomIn = event.mouseWheel.delta > 0;
                    float zoomFactor = zoomIn ? (1 / ZOOM_AMOUNT) : (ZOOM_AMOUNT) ;

                    unsigned int zoomAmount = abs(event.mouseWheel.delta);

                    for (unsigned int i=0; i<zoomAmount; i++)
                    {
                        ui->cameraView.zoom(zoomFactor);
                    }
                    break;
                }
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
                                ui->openInGameMenu(window);
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
                                ui->openInGameMenu(window);
                            }
                            break;
                        case sf::Keyboard::H:
                            ui->cleanDrawEnabled = ! ui->cleanDrawEnabled;
                            break;
                        case sf::Keyboard::J:
                            ui->hideUX = ! ui->hideUX;
                            break;
                        default:
                            break;
                    }
                    break;
                default:
                    break;
                }
            }
        }
    }

    if (! sf::Mouse::isButtonPressed(sf::Mouse::Button::Middle))
    {
        // mouse-edge camera move
        vector2i screenDimensions = getScreenSize(window);

        vector2i screenEdgeCameraMove;
        screenEdgeCameraMove.x =
            (ui->lastMousePos.x == 0)                       ?  - SCREEN_EDGE_SCROLL_AMOUNT :
            (screenDimensions.x - ui->lastMousePos.x == 1)  ?    SCREEN_EDGE_SCROLL_AMOUNT :
            0;
        screenEdgeCameraMove.y =
            (ui->lastMousePos.y == 0)                       ?  - SCREEN_EDGE_SCROLL_AMOUNT :
            (screenDimensions.y - ui->lastMousePos.y == 1)  ?    SCREEN_EDGE_SCROLL_AMOUNT :
            0;

        float zoomFactor = getCurrentViewSize(window).x / getScreenSize(window).x ;

        ui->cameraView.move(toSFVecF(screenEdgeCameraMove * zoomFactor));
    }

    // constrain camera
    if (fromSFVec(ui->cameraView.getCenter()).getMagnitude() > float(game->mapRadius))
    {
        float angle = fromSFVec(ui->cameraView.getCenter()).getAngle();
        vector2i newCameraCenter = composeVector2i(angle, float(game->mapRadius));
        ui->cameraView.setCenter(toSFVecF(newCameraCenter));
    }

    return cmdsToSend;
}

void GameUI::returnToDefaultState()
{
    cmdState = Default;
    keyButtonBox.returnToDefaultState();
}

tuple<bool, optional<boost::shared_ptr<Cmd>>> GameUI::processEventForOverlay(sf::Event event, float uxViewZoom)
{
    bool consumed = false;
    optional<tuple<KeyButton*, KeyButtonMsg>> maybeButtonAndMsg = {};

    switch (event.type)
    {
        case sf::Event::MouseButtonPressed:
        {
            if (event.mouseButton.button == sf::Mouse::Right && cmdState != CmdState::Default)
            {
                returnToDefaultState();
                cmdState = CmdState::Default;
                consumed = true;
            }
            else if (event.mouseButton.button == sf::Mouse::Left)
            {
                vector2fl point = vector2fl(mouseEventVec(event.mouseButton)) * (1/uxViewZoom);
                consumed = keyButtonBox.registerPress(point);
                
                if (!consumed)
                    consumed = unitInfoBox.pointCollides(point);
            }
            break;
        }
        case sf::Event::MouseButtonReleased:
        {
            if (event.mouseButton.button == sf::Mouse::Left)
            {
                vector2fl point = vector2fl(mouseEventVec(event.mouseButton)) * (1/uxViewZoom);
                auto consumedAndMaybeMsg = keyButtonBox.registerRelease(point);
                consumed = get<0>(consumedAndMaybeMsg);
                maybeButtonAndMsg = get<1>(consumedAndMaybeMsg);

                if (!consumed)
                    consumed = unitInfoBox.pointCollides(point);
            }
            break;
        }
        case sf::Event::MouseMoved:
        {
            vector2fl point = vector2fl(mouseEventVec(event.mouseMove)) * (1.0/uxViewZoom);
            consumed = keyButtonBox.registerMouseMove(point);

            if (!consumed)
                consumed = unitInfoBox.pointCollides(point);
            
            break;
        }
        case sf::Event::KeyPressed:
        {
            maybeButtonAndMsg = keyButtonBox.handleKey(event.key.code);
            if (maybeButtonAndMsg)
            {
                consumed = true;
            }
            break;
        }
        default: break;
    }

    optional<boost::shared_ptr<Cmd>> maybeCmd;

    if (maybeButtonAndMsg)
    {
        auto button = get<0>(*maybeButtonAndMsg);
        auto msg = get<1>(*maybeButtonAndMsg);
        switch (msg)
        {
            case KeyButtonMsg::WarpIn:
            {
                returnToDefaultState();
                cmdState = CmdState::SpawnBeacon;
                button->active = true;
                ghostBuilding = boost::shared_ptr<Building>(new Gateway(-1, vector2fp::zero));

                break;
            }
            case KeyButtonMsg::WarpOut:
            {
                returnToDefaultState();
                maybeCmd = boost::shared_ptr<Cmd>(new WarpOutCmd(entityPtrsToRefsOrThrow(selectedUnits)));
                button->visualFlashClock.restart();

                break;
            }
            case KeyButtonMsg::Stop:
            {
                returnToDefaultState();
                maybeCmd = boost::shared_ptr<Cmd>(new StopCmd(entityPtrsToRefsOrThrow(selectedUnits)));
                button->visualFlashClock.restart();

                break;
            }
            case KeyButtonMsg::Invest:
            {
                returnToDefaultState();
                cmdState = CmdState::Deposit;
                button->active = true;

                break;
            }
            case KeyButtonMsg::Fetch:
            {
                returnToDefaultState();
                cmdState = CmdState::Fetch;
                button->active = true;

                break;
            }
            case KeyButtonMsg::AttackScuttle:
            {
                returnToDefaultState();
                cmdState = CmdState::AttackScuttle;
                button->active = true;

                break;
            }
            case KeyButtonMsg::BuildPrime:
            {
                returnToDefaultState();
                if (auto cmd = makeGatewayBuildCmd(selectedUnits, PRIME_TYPECHAR))
                {
                    button->visualFlashClock.restart();
                    maybeCmd = {cmd};
                }

                break;
            }
            case KeyButtonMsg::BuildFighter:
            {
                returnToDefaultState();
                if (auto cmd = makeGatewayBuildCmd(selectedUnits, FIGHTER_TYPECHAR))
                {
                    button->visualFlashClock.restart();
                    maybeCmd = {cmd};
                }

                break;
            }
            case KeyButtonMsg::BuildGateway:
            {
                returnToDefaultState();
                cmdState = CmdState::Build;
                ghostBuilding = boost::shared_ptr<Building>(new Gateway(-1, vector2fp::zero));
                button->active = true;

                break;
            }
            case KeyButtonMsg::BuildTurret:
            {
                returnToDefaultState();
                cmdState = CmdState::Build;
                ghostBuilding = boost::shared_ptr<Building>(new Turret(-1, vector2fp::zero));
                button->active = true;

                break;
            }
        }
    }

    return {consumed, maybeCmd};
}