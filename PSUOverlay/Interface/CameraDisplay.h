#pragma once

#include "IFWinCtrl.h"

class CameraDisplay : public IFWinCtrl
{
private:

public:
	CameraDisplay( const CameraDisplay & ) = delete;
	CameraDisplay &operator=( const CameraDisplay & ) = delete;

	CameraDisplay();
	~CameraDisplay();

	static std::shared_ptr< CameraDisplay > Create()
	{
		return std::make_shared< CameraDisplay >();
	}

	void SetInitialWindowPosition() override;
	void Render() override;
};