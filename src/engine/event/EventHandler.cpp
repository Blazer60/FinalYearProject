/**
 * @file Keymap.cpp
 * @author Ryan Purse
 * @date 23/08/2023
 */


#include "EventHandler.h"
#include "Editor.h"
#include "glfw3.h"
#include "EngineState.h"

namespace engine
{
    namespace event
    {
        FirstPersonCamera::FirstPersonCamera(ImGuiKey key)
            : key(key)
        {
        
        }
        
        void FirstPersonCamera::update()
        {
            bool temp = isActive;
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
            bool temp = isActive;
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
                    onZoom.doAction();
                }
            }
        }
        
        ThirdPersonCamera::ThirdPersonCamera(ImGuiKey key)
            : key(key)
        {
        
        }
        
        void ThirdPersonCamera::update()
        {
            bool temp = isActive;
            isActive = ImGui::IsKeyDown(key);
            if (isActive != temp)
                onStateChanged.broadcast(isActive);
            
            if (isActive)
            {
                onOrbitCamera.doAction();
                onZoomCamera.doAction();
            }
        }
        
    }
    
    void RootEventHandler::update()
    {
        viewport.update();
    }
    
    void RootEventHandler::updateMouseDelta(double xPos, double yPos)
    {
        glm::dvec2 mousePosition = { xPos, yPos };
        mMousePositionDelta = mousePosition - mLastMousePosition;
        mLastMousePosition = mousePosition;
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
