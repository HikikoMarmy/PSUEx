#pragma once

#include "IFWinCtrl.h"
#include "..\ImageMng.h"

class ConfigDisplay : public IFWinCtrl
{
private:
	static inline const char *m_elementName[ 7 ] =
	{ "Neutral", "Fire", "Ice", "Lightning", "Ground", "Light", "Dark" };

	bool m_bSaveConfig;

public:
	ConfigDisplay( const ConfigDisplay & ) = delete;
	ConfigDisplay &operator=( const ConfigDisplay & ) = delete;

	ConfigDisplay();
	~ConfigDisplay();

	static std::shared_ptr< ConfigDisplay > Create()
	{
		return std::make_shared< ConfigDisplay >();
	}

	void SetInitialWindowPosition() override;
	void Render() override;

	void RenderLootReaderConfig();
	void RenderDisplayTab();
	void RenderItemFilterTab();
	void RenderRarityFilterTab();
	void RenderElementFilterTab();

	void RenderHoverItemConfig();
};