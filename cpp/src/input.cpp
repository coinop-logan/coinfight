#include "input.h"
#include "config.h"

extern Game game;
extern UI ui;
extern vector<boost::shared_ptr<Cmd>> cmdsToSend;

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

boost::shared_ptr<Cmd> makeRightclickCmd(const Game &game, vector<boost::shared_ptr<Entity>> selectedEntities, Target target)
{
    if (selectedEntities.size() == 0)
    {
        return boost::shared_ptr<Cmd>();
    }
    if (optional<vector2f> point = target.castToPoint())
    {
        return boost::shared_ptr<Cmd>(new MoveCmd(entityPtrsToRefs(selectedEntities), *point));
    }
    else if (optional<boost::shared_ptr<Entity>> entityPtrPtr = target.castToEntityPtr(game))
    {
        boost::shared_ptr<Entity> entity = *entityPtrPtr;

        if (entity->typechar() == GOLDPILE_TYPECHAR || entity->typechar() == GATEWAY_TYPECHAR)
        {
            vector<boost::shared_ptr<Entity>> primesInSelection = filterForTypeKeepContainer<Prime, Entity>(selectedEntities);
            if (entity->typechar() == GOLDPILE_TYPECHAR || entity->typechar() == GATEWAY_TYPECHAR)
            {
                return boost::shared_ptr<Cmd>(new PickupCmd(entityPtrsToRefs(primesInSelection), entity->ref));
            }
        }
        else
        {
            // maybe in future can do a "follow" type action. Or attack if all enemies.
            return boost::shared_ptr<Cmd>();
        }
    }
    
    // couldn't cast target to a point or an entity...
    cout << "issue casting target to a point or entity in makeRightclickCmd" << endl;
    return boost::shared_ptr<Cmd>(); // return null cmd
}

vector<boost::shared_ptr<Cmd>> pollWindowEventsAndUpdateUI(const Game &game, UI *ui, sf::RenderWindow *window)
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
                Target target = getTargetAtScreenPos(game, ui->camera, mouseMoveToVec(event.mouseMove));
                ui->mouseoverEntity = target.castToEntityPtr(game);
            }
            break;
        case sf::Event::MouseButtonPressed:
            if (event.mouseButton.button == sf::Mouse::Left)
            {
                if (ui->cmdState == UI::Default)
                {
                    if (boost::shared_ptr<Entity> clickedEntity = getTargetAtScreenPos(game, ui->camera, mouseButtonToVec(event.mouseButton)).castToEntityPtr(game))
                    {
                        ui->selectedEntities.clear();
                        ui->selectedEntities.push_back(clickedEntity);
                    }
                }
                else if (ui->cmdState == UI::Deposit)
                {
                    Target target = getTargetAtScreenPos(game, ui->camera, mouseButtonToVec(event.mouseButton));
                    if (auto clickedEntity = target.castToEntityPtr(game))
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
            }
            else if (event.mouseButton.button == sf::Mouse::Right)
            {
                if (ui->cmdState == UI::Default)
                {
                    if (ui->selectedEntities.size() > 0)
                    {
                        cmdsToSend.push_back(makeRightclickCmd(game, ui->selectedEntities, getTargetAtScreenPos(game, ui->camera, mouseButtonToVec(event.mouseButton))));
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
                    if (ui->selectedEntities.size() > 0)
                    {
                        ui->cmdState = UI::Deposit;
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