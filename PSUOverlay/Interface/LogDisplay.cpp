#include "LogDisplay.h"
#include "../PSUExConfig.h"

std::string LogDisplay::WCharToUTF8( const std::wstring &wstr )
{
    if( wstr.empty() ) return std::string();

    int sizeNeeded = WideCharToMultiByte( CP_UTF8, 0, wstr.c_str(), -1, nullptr, 0, nullptr, nullptr );
    if( sizeNeeded <= 0 ) return std::string();

    std::string utf8Str( sizeNeeded, 0 );
    WideCharToMultiByte( CP_UTF8, 0, wstr.c_str(), -1, &utf8Str[ 0 ], sizeNeeded, nullptr, nullptr );

    return utf8Str;
}

LogDisplay::LogDisplay() : IFWinCtrl( "LogDisplay" )
{
	this->m_bEnabled = true;
}

LogDisplay::~LogDisplay()
{
}

void LogDisplay::Render()
{
	if( !this->m_bEnabled )
	{
		return;
	}

	ImGui::Begin( "LogDisplay", &m_bEnabled );
    {
        ImGui::BeginChild( "ChatScroll", ImVec2( 0, 0 ), true, ImGuiWindowFlags_AlwaysVerticalScrollbar );

        float lineHeight = ImGui::GetTextLineHeightWithSpacing();
        float scrollY = ImGui::GetScrollY();
        float windowHeight = ImGui::GetWindowHeight();

        int totalItems = m_messages.size();
        int visibleStart = ( int )( scrollY / lineHeight );
        int visibleEnd = ( int )( ( scrollY + windowHeight ) / lineHeight );

        // Clamp values to avoid out-of-bounds errors
        visibleStart = ( visibleStart < 0 ) ? 0 : visibleStart;
        visibleEnd = ( visibleEnd > totalItems ) ? totalItems : visibleEnd;

        // Dummy spacing to keep scroll bar accurate
        ImGui::Dummy( ImVec2( 0, visibleStart * lineHeight ) );

        for( int i = visibleStart; i < visibleEnd; i++ )
        {
            const auto &entry = m_messages[ i ];

            switch( entry.type )
            {
				case MESSAGE_TYPE::PUBLIC_CHAT:
				{
					ImGui::TextColored( ImVec4( 1.0f, 1.0f, 1.0f, 1.0f ), "[Public] " );
				} break;

				case MESSAGE_TYPE::PARTY_CHAT:
				{
					ImGui::TextColored( ImVec4( 0.0f, 1.0f, 0.0f, 1.0f ), "[Party] " );
				} break;
			}

            if( !entry.name.empty() )
            {
                ImGui::SameLine();
				ImGui::TextColored( ImVec4( 1.0f, 1.0f, 0.0f, 1.0f ), "%s: ", entry.name.c_str() );
            }
            ImGui::SameLine();
            ImGui::Text( "%S", entry.msg.c_str() );
        }

        ImGui::Dummy( ImVec2( 0, ( totalItems - visibleEnd ) * lineHeight ) );

        ImGui::EndChild();
	}
	ImGui::End();
}

void LogDisplay::AddChatMessage( const wchar_t *name, const wchar_t *msg, bool party )
{
	std::string utf8Name = WCharToUTF8( name );
	std::string utf8Msg = WCharToUTF8( msg );

    std::lock_guard< std::mutex > lock( m_mtx );

	m_messages.push_back( { party ? MESSAGE_TYPE::PARTY_CHAT : MESSAGE_TYPE::PUBLIC_CHAT, utf8Name, utf8Msg } );
}
