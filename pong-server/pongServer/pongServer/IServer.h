#pragma once

class IServer
{
public:
	virtual bool initServer() = 0;
	virtual bool upServer() = 0;
	virtual void downServer() = 0;
};