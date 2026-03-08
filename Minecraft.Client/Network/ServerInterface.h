#pragma once


class ServerInterface
{
	virtual int getConfigInt(const std::wstring &name, int defaultValue) = 0;
	virtual std::wstring getConfigString(const std::wstring &name, const std::wstring &defaultValue) = 0;
	virtual bool getConfigBoolean(const std::wstring &name, bool defaultValue) = 0;
	virtual void setProperty(std::wstring &propertyName, void *value) = 0;
	virtual void configSave() = 0;
	virtual std::wstring getConfigPath() = 0;
	virtual std::wstring getServerIp() = 0;
	virtual int getServerPort() = 0;
	virtual std::wstring getServerName() = 0;
	virtual std::wstring getServerVersion() = 0;
	virtual int getPlayerCount() = 0;
	virtual int getMaxPlayers() = 0;
	virtual std::wstring[] getPlayerNames() = 0;
	virtual std::wstring getWorldName() = 0;
	virtual std::wstring getPluginNames() = 0;
	virtual void disablePlugin() = 0;
	virtual std::wstring runCommand(const std::wstring &command) = 0;
	virtual bool isDebugging() = 0;
	// Logging
	virtual void info(const std::wstring &string) = 0;
	virtual void warn(const std::wstring &string) = 0;
	virtual void error(const std::wstring &string) = 0;
	virtual void debug(const std::wstring &string) = 0;
};