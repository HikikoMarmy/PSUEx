#pragma once

#include <mutex>

#include "IFWinCtrl.h"
#include "..\ImageMng.h"

class LogDisplay : public IFWinCtrl
{
private:
	enum class MESSAGE_TYPE {
		PUBLIC_CHAT,
		PARTY_CHAT,
	};

	struct s_log_message {
		MESSAGE_TYPE type;
		std::string name;
		std::string msg;
	};

	std::vector< s_log_message > m_messages;
	std::mutex m_mtx;

	std::string WCharToUTF8( const std::wstring &wstr );

public:
	LogDisplay( const LogDisplay & ) = delete;
	LogDisplay &operator=( const LogDisplay & ) = delete;

	LogDisplay();
	~LogDisplay();

	static std::shared_ptr< LogDisplay > Create()
	{
		return std::make_shared< LogDisplay >();
	}

	void Render() override;

public:
	void AddChatMessage( const wchar_t *name, const wchar_t *msg, bool party );
};