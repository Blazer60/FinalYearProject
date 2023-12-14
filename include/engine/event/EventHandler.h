/**
 * @file Keymap.h
 * @author Ryan Purse
 * @date 23/08/2023
 */


#pragma once

#include "Pch.h"
#include "Input.h"

namespace engine
{
    namespace event
    {
        void hideMouseCursor();
        void showMouseCursor();

        class EventHandler
        {
        public:
            virtual ~EventHandler() = default;
            virtual void update() = 0;
            bool isActive { false };
            
            Callback<bool> onStateChanged;
        };
        
        class ThirdPersonCamera
            : public EventHandler
        {
        public:
            explicit ThirdPersonCamera(ImGuiKey key);
            void update() override;
            
            Value onOrbitCamera { { ImGuiKey_MouseLeft, 1.f } };
            Value onZoomCamera { { ImGuiKey_MouseRight, 1.f } };
            
            ImGuiKey key;
        };
        
        class FirstPersonCamera
            : public EventHandler
        {
        public:
            explicit FirstPersonCamera(ImGuiKey key);
            void update() override;
            
            Value onMoveForward { { { ImGuiKey_W, -1.f }, { ImGuiKey_S,  1.f } } };
            Value onMoveRight   { { { ImGuiKey_A, -1.f }, { ImGuiKey_D,  1.f } } };
            Value onMoveUp      { { { ImGuiKey_Q, -1.f }, { ImGuiKey_E,  1.f } } };
            
            ImGuiKey key;
        };
        
        class Viewport
            : public EventHandler
        {
        public:
            void update() override;

            Button onGizmoTranslate { ImGuiKey_W };
            Button onGizmoRotate    { ImGuiKey_E };
            Button onGizmoScale     { ImGuiKey_R };
            Button onFocusActor     { ImGuiKey_F };
            ScrollWheel onZoom;
            
            FirstPersonCamera firstPerson { ImGuiKey_MouseRight };
            ThirdPersonCamera thirdPerson { ImGuiKey_LeftAlt };
        };

        class Editor
            : public EventHandler
        {
        public:
            void update() override;

            Button onDeleteActor { ImGuiKey_Delete };

            Viewport viewport;
        };
    }
    
    class RootEventHandler
        : public event::EventHandler
    {
    public:
        /**
         * @brief Resets inputs back to their default values. Must be called before ImGui::BeginFrame().
         */
        void beginFrame();
        
        /**
         * @brief Muse be called after ImGui::BeginFrame().
         */
        void update() override;
        
        void updateMouseDelta(double xPos, double yPos);

        void linkUserEvents(std::shared_ptr<EventHandler> eventHandler);

        /**
         * @returns The delta from the last frame. This will be valid even if the cursor is disabled.
         */
        [[nodiscard]] glm::vec2 getMouseDelta() const;

        event::Editor editor;
        bool updateUserEvents { false };

    protected:
        glm::dvec2 mLastMousePosition { 0.f };
        glm::vec2  mMousePositionDelta { 0.f };
        std::shared_ptr<EventHandler> mUserEvents { nullptr };

        ImGuiKey mEscapeUserEventKey { ImGuiKey_Escape };
    };
}

