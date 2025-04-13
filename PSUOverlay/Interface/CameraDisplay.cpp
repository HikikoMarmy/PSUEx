#include "CameraDisplay.h"
#include "../Misc/Globals.h"
#include "../Misc/Constants.h"
#include "../PSUExConfig.h"

CameraDisplay::CameraDisplay() : IFWinCtrl( "CameraDisplay" )
{
	auto &config = Config::Get().camera;
	this->m_bEnabled = config.m_windowInfo.m_enabled;
}

CameraDisplay::~CameraDisplay()
{
}

void CameraDisplay::SetInitialWindowPosition()
{
	auto &config = Config::Get().camera;

	auto x = config.m_windowInfo.m_position.x;
	auto y = config.m_windowInfo.m_position.y;

	if( x < 0 || x > Global::screenWidth )
	{
		x = 0;
	}

	if( y < 0 || y > Global::screenHeight )
	{
		y = 0;
	}

	ImGui::SetNextWindowPos( ImVec2( x, y ), ImGuiCond_FirstUseEver );
	ImGui::SetNextWindowSize( ImVec2( config.m_windowInfo.m_size.x, config.m_windowInfo.m_size.y ), ImGuiCond_FirstUseEver );
}

void CameraDisplay::Render()
{
    auto &config = Config::Get().camera;
	config.m_windowInfo.m_enabled = this->m_bEnabled;

	if( !this->m_bEnabled )
	{
		return;
	}

    SetInitialWindowPosition();

    ImGui::Begin( "Camera Settings", &m_bEnabled, ImGuiWindowFlags_AlwaysAutoResize );
    {
		// Capture current position
		config.m_windowInfo.m_position = ImGui::GetWindowPos();
		config.m_windowInfo.m_size = ImGui::GetWindowSize();

        // Header
        ImGui::Text( "Camera Display Settings" );
       // ImGui::Separator();
        ImGui::SameLine();
        ImGui::Spacing();

        if( ImGui::Button( "Reset Default" ) )
        {
			config.m_followDistance = 1.0f;
			config.m_rotationSpeedH = 1.0f;
			config.m_rotationSpeedV = 1.0f;

            config.m_followHeight = Constant::Camera::FOLLOW_HEIGHT;
            config.m_smoothing = Constant::Camera::FOLLOW_SMOOTHING;
            config.m_refocusSpeed = Constant::Camera::REFOCUS_SPEED;
			config.m_turnAcceleration = Constant::Camera::TURN_ACCELERATION;
        }

        ImGui::Separator();
        ImGui::Spacing();

        // Follow Settings
        ImGui::Text( "Follow Settings" );
        ImGui::Indent();
        ImGui::SliderFloat( "Follow Distance", &config.m_followDistance, 0.5f, 2.0f, "%.2f" );
        ImGui::SliderFloat( "Follow Height", &config.m_followHeight, 5.0f, 30.0f, "%.2f" );
        ImGui::SliderFloat( "Damping (Firmness)", &config.m_smoothing, 0.15f, 1.0f, "%.2f" );
        ImGui::Unindent();

        ImGui::Spacing();
        ImGui::Separator();
        ImGui::Spacing();

        // Refocus Settings
        ImGui::Text( "Refocus Settings" );
        ImGui::Indent();
        ImGui::SliderFloat( "Refocus Speed", &config.m_refocusSpeed, 0.1f, 1.0f, "%.2f" );
        ImGui::SliderFloat( "Horizontal Speed", &config.m_rotationSpeedH, 0.2f, 5.0f, "%.2f" );
        ImGui::SliderFloat( "Vertical Speed", &config.m_rotationSpeedV, 0.2f, 5.0f, "%.2f" );
        ImGui::SliderFloat( "Turn Acceleration", &config.m_turnAcceleration, 0.1f, 1.0f, "%.2f" );
        ImGui::Unindent();

        ImGui::Spacing();
        ImGui::Separator();
    }
    ImGui::End();
}