#include "input.h"
#include "config.h"

extern Game game;
extern UI ui;
extern vector<boost::shared_ptr<Cmd>> cmdsToSend;

bool isShiftPressed()
{
    return sf::Keyboard::isKeyPressed(sf::Keyboard::LShift) || sf::Keyboard::isKeyPressed(sf::Keyboard::RShift);
}

UI::UI()
{
    camera.gamePos = vector2f(0, 0);
    debugInt = 0;
    cmdState = Default;
    interfaceCmdsWithState = vector<UnitInterfaceCmdWithState>
    {
        boost::shared_ptr<UnitInterfaceCmd>(new DepositInterfaceCmd()),
        boost::shared_ptr<UnitInterfaceCmd>(new GatewayBuildPrimeInterfaceCmd()),
        boost::shared_ptr<UnitInterfaceCmd>(new GatewayBuildFighterInterfaceCmd()),
        boost::shared_ptr<UnitInterfaceCmd>(new PrimeBuildGatewayInterfaceCmd())
    };
}

void UI::updateAvailableUnitInterfaceCmds()
{
    // set all to false, and create a list of pointers to track those we've yet to enable
    vector<UnitInterfaceCmdWithState*> notYetEnabled;
    for (uint i=0; i<interfaceCmdsWithState.size(); i++)
    {
        interfaceCmdsWithState[i].eligible = false;
        notYetEnabled.push_back(&interfaceCmdsWithState[i]);
    }
    
    for (uint i=0; i<selectedUnits.size(); i++)
    {
        // if we've enabled all cmds, no need to keep looking
        if (notYetEnabled.size() == 0)
            break;
        
        for (uint j=0; j<notYetEnabled.size(); j++)
        {
            if (notYetEnabled[j]->interfaceCmd->isUnitEligible(selectedUnits[i]))
            {
                // enable it and remove this pointer from the list
                notYetEnabled[j]->eligible = true;
                notYetEnabled.erase(notYetEnabled.begin() + j);
                j--;
            }
        }
    }
}

vector<boost::shared_ptr<Cmd>> UI::handlePossibleUnitInterfaceCmd(sf::Keyboard::Key key)
{
    for (uint i=0; i<interfaceCmdsWithState.size(); i++)
    {
        if (interfaceCmdsWithState[i].eligible && interfaceCmdsWithState[i].interfaceCmd->getKey() == key)
        {
            return {interfaceCmdsWithState[i].interfaceCmd->execute(this)};
        }
    }

    return {};
}

vector2f screenPosToGamePos(CameraState cameraState, vector2i screenPos)
{
    vector2i screenPosFromCenter = screenPos - HALF_SCREENDIM;
    vector2f result = cameraState.gamePos + vector2f(screenPosFromCenter.x, -screenPosFromCenter.y);

    return result;
}

vector2i gamePosToScreenPos(CameraState cameraState, vector2i gamePos)
{
    vector2f subtractedFromCamera = gamePos - cameraState.gamePos;
    subtractedFromCamera.y *= -1;
    vector2i result = subtractedFromCamera + HALF_SCREENDIM;
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
        for (uint i=0; i<selectedGateways.size(); i++)
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
        for (uint i=0; i<selectedPrimes.size(); i++)
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

vector<boost::shared_ptr<Cmd>> pollWindowEventsAndUpdateUI(Game *game, UI *ui, int playerId, sf::RenderWindow *window)
{
    ui->updateAvailableUnitInterfaceCmds();

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
        case sf::Event::MouseButtonPressed:
            if (event.mouseButton.button == sf::Mouse::Left)
            {
                switch (ui->cmdState)
                {
                    case UI::Default:
                    {
                        if (boost::shared_ptr<Entity> clickedEntity = getTargetAtScreenPos(*game, ui->camera, mouseButtonToVec(event.mouseButton)).castToEntityPtr(*game))
                        {
                            if (auto clickedUnit = boost::dynamic_pointer_cast<Unit, Entity>(clickedEntity))
                            {
                                ui->selectedUnits.clear();
                                ui->selectedUnits.push_back(clickedUnit);
                            }
                        }
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
                }
            }
            else if (event.mouseButton.button == sf::Mouse::Right)
            {
                if (ui->cmdState == UI::Default)
                {
                    if (ui->selectedUnits.size() > 0)
                    {
                        cmdsToSend.push_back(makeRightclickCmd(*game, *ui, playerId, getTargetAtScreenPos(*game, ui->camera, mouseButtonToVec(event.mouseButton))));
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
                case sf::Keyboard::Tab:
                    ui->debugInt ++;
                    break;
                case sf::Keyboard::Escape:
                    if (ui->cmdState != UI::Default)
                    {
                        ui->cmdState = UI::Default;
                    }
                    else
                    {
                        ui->selectedUnits.clear();
                    }
                default:
                    vector<boost::shared_ptr<Cmd>> cmds = ui->handlePossibleUnitInterfaceCmd(event.key.code);
                    cmdsToSend.insert(cmdsToSend.begin(), cmds.begin(), cmds.end());
                    break;
            }
        default:
            break;
        }
    }
    return cmdsToSend;
}