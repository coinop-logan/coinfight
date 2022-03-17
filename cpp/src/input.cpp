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
    if (ui.selectedEntities.size() == 0)
    {
        return boost::shared_ptr<Cmd>();
    }
    if (optional<vector2f> point = target.castToPoint())
    {
        return boost::shared_ptr<Cmd>(new MoveCmd(entityPtrsToRefs(ui.selectedEntities), *point));
    }
    else if (optional<boost::shared_ptr<Entity>> entityPtrPtr = target.castToEntityPtr(game))
    {
        boost::shared_ptr<Entity> entity = *entityPtrPtr;

        if (getAllianceType(playerID, entity) == Enemy)
        {
            vector<boost::shared_ptr<Entity>> fighters = filterForTypeKeepContainer<Fighter, Entity>(ui.selectedEntities);
            return boost::shared_ptr<Cmd>(new AttackCmd(entityPtrsToRefs(fighters), entity->ref));
        }
        else
        {
            if (entity->typechar() == GOLDPILE_TYPECHAR || entity->typechar() == GATEWAY_TYPECHAR)
            {
                vector<boost::shared_ptr<Entity>> primesInSelection = filterForTypeKeepContainer<Prime, Entity>(ui.selectedEntities);
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

boost::shared_ptr<Cmd> makeGatewayBuildCmd(vector<boost::shared_ptr<Entity>> selectedEntities, unsigned char buildUnitTypechar)
{
    auto selectedGateways = filterForType<Gateway, Entity>(selectedEntities);
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

boost::shared_ptr<Cmd> makePrimeBuildCmd(vector<boost::shared_ptr<Entity>> selectedEntities, unsigned char buildUnitTypechar, vector2f buildPos)
{
    auto selectedPrimes = filterForType<Prime, Entity>(selectedEntities);
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
                            ui->selectedEntities.clear();
                            ui->selectedEntities.push_back(clickedEntity);
                        }
                    }
                    break;
                    case UI::Deposit:
                    {
                        Target target = getTargetAtScreenPos(*game, ui->camera, mouseButtonToVec(event.mouseButton));
                        if (auto clickedEntity = target.castToEntityPtr(*game))
                        {
                            if (clickedEntity->typechar() == GATEWAY_TYPECHAR || clickedEntity->typechar() == GOLDPILE_TYPECHAR)
                            {
                                vector<boost::shared_ptr<Entity>> primesInSelection = filterForTypeKeepContainer<Prime, Entity>(ui->selectedEntities);
                                if (primesInSelection.size() > 0)
                                {
                                    cmdsToSend.push_back(boost::shared_ptr<Cmd>(new PutdownCmd(entityPtrsToRefs(primesInSelection), clickedEntity->ref)));
                                }
                                ui->cmdState = UI::Default;
                            }
                        }
                        else
                        {
                            vector<boost::shared_ptr<Entity>> primesInSelection = filterForTypeKeepContainer<Prime, Entity>(ui->selectedEntities);
                            if (primesInSelection.size() > 0)
                            {
                                cmdsToSend.push_back(boost::shared_ptr<Cmd>(new PutdownCmd(entityPtrsToRefs(primesInSelection), target)));
                            }
                            ui->cmdState = UI::Default;
                        }
                    }
                    break;
                    case UI::Build:
                    {
                        vector<boost::shared_ptr<Entity>> primesInSelection = filterForTypeKeepContainer<Prime, Entity>(ui->selectedEntities);
                        vector2f buildPos = screenPosToGamePos(ui->camera, mouseButtonToVec(event.mouseButton));
                        cmdsToSend.push_back(makePrimeBuildCmd(ui->selectedEntities, ui->ghostBuilding->typechar(), buildPos));
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
                    if (ui->selectedEntities.size() > 0)
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
                case sf::Keyboard::D:
                    if (filterForType<Prime, Entity>(ui->selectedEntities).size() > 0)
                    {
                        ui->cmdState = UI::Deposit;
                    }
                    break;
                case sf::Keyboard::Q:
                    if (auto cmd = makeGatewayBuildCmd(ui->selectedEntities, PRIME_TYPECHAR))
                        cmdsToSend.push_back(cmd);
                    break;
                case sf::Keyboard::W:
                    if (auto cmd = makeGatewayBuildCmd(ui->selectedEntities, FIGHTER_TYPECHAR))
                        cmdsToSend.push_back(cmd);
                    break;
                case sf::Keyboard::E:
                    if (filterForType<Prime, Entity>(ui->selectedEntities).size() > 0)
                    {
                        ui->cmdState = UI::Build;
                        ui->ghostBuilding = boost::shared_ptr<Building>(new Gateway(game, 0, playerId, vector2f(0,0)));
                    }
                    break;
                case sf::Keyboard::Escape:
                    if (ui->cmdState != UI::Default)
                    {
                        ui->cmdState = UI::Default;
                    }
                    else
                    {
                        ui->selectedEntities.clear();
                    }
                default:
                    break;
            }
        default:
            break;
        }
    }
    return cmdsToSend;
}