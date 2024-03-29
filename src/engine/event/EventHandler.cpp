/**
 * @file EventHandler.cpp
 * @author Ryan Purse
 * @date 23/08/2023
 */


#include "EventHandler.h"

#include "Core.h"
#include "Editor.h"
#include "glfw3.h"
#include "EngineState.h"

namespace engine
{
    namespace event
    {
        void hideMouseCursor()
        {
            glfwSetInputMode(editor->getViewportContext(), GLFW_CURSOR, GLFW_CURSOR_DISABLED);
        }

        void showMouseCursor()
        {
            glfwSetInputMode(editor->getViewportContext(), GLFW_CURSOR, GLFW_CURSOR_NORMAL);
        }
        FirstPersonCamera::FirstPersonCamera(ImGuiKey key)
            : key(key)
        {
        
        }
        
        void FirstPersonCamera::update()
        {
            const bool temp = isActive;
            isActive = ImGui::IsKeyDown(key);
            if (isActive != temp)
                onStateChanged.broadcast(isActive);
            
            if (isActive)
            {
                onMoveForward.doAction();
                onMoveRight.doAction();
                onMoveUp.doAction();
            }
        }
        
        void Viewport::update()
        {
            const bool temp = isActive;
            isActive = engine::editor->isViewportHovered() || glfwGetInputMode(engine::editor->getViewportContext(), GLFW_CURSOR) == GLFW_CURSOR_DISABLED;
            if (isActive != temp)
                onStateChanged.broadcast(isActive);
            
            if (isActive)
            {
                thirdPerson.update();
                if (!thirdPerson.isActive)
                    firstPerson.update();
                if (!firstPerson.isActive && !thirdPerson.isActive)
                {
                    onGizmoTranslate.doAction();
                    onGizmoRotate.doAction();
                    onGizmoScale.doAction();
                    onFocusActor.doAction();
                    onTransformActor.doAction();
                    onZoom.doAction();
                    onGotoActor.doAction();
                }
            }
        }
        
        ThirdPersonCamera::ThirdPersonCamera(const ImGuiKey key)
            : key(key)
        {
        
        }
        
        void ThirdPersonCamera::update()
        {
            const bool temp = isActive;
            isActive = ImGui::IsKeyDown(key);
            if (isActive != temp)
                onStateChanged.broadcast(isActive);
            
            if (isActive)
            {
                onOrbitCamera.doAction();
                onZoomCamera.doAction();
            }
        }
        
        void Editor::update()
        {
            const bool temp = isActive;
            isActive = !editor->isUsingPlayModeCamera();
            if (isActive != temp)
                onStateChanged.broadcast(isActive);

            if (isActive)
            {
                viewport.update();
                if (editor->isViewportHovered())
                    onDeleteActor.doAction();
            }
        }
    }
    
    void RootEventHandler::update()
    {
        editor.update();
        if (!editor.isActive && mUserEvents && updateUserEvents)
            mUserEvents->update();

        if (ImGui::IsKeyPressed(mEscapeUserEventKey, false))
        {
            event::showMouseCursor();
            updateUserEvents = false;
        }
    }
    
    void RootEventHandler::updateMouseDelta(double xPos, double yPos)
    {
        const glm::dvec2 mousePosition = { xPos, yPos };
        mMousePositionDelta = mousePosition - mLastMousePosition;
        mLastMousePosition = mousePosition;
    }

    void RootEventHandler::linkUserEvents(std::shared_ptr<EventHandler> eventHandler)
    {
        mUserEvents = std::move(eventHandler);
    }


    glm::vec2 RootEventHandler::getMouseDelta() const
    {
        return mMousePositionDelta;
    }


    void RootEventHandler::beginFrame()
    {
        mMousePositionDelta = glm::vec2(0.f);
    }
}
