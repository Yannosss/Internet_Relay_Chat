#include "client.hpp"

#define USER_MODE 0
#define CHANNEL_MODE 1

int		Client::parseModes(std::string modes, int modeType, std::string user, std::string chan )
{
	size_t j = 0;
	for (j = 0; j < modes.size(); j++)
	{
		if (modes[j] == '+' or modes[j] == '-')
		{
			if (modeType == USER_MODE)
				updateUserModes(modes[j], modes[j + 1]);
			else
			{
				if (user.empty())
					handleInviteOnlyMode(modes[j], modes[j + 1], chan);
				else
					handleOperatorChannelMode(modes[j], modes[j + 1], user, chan);
			}
		}
	}
	return(0);
}

int		Client::handleOperatorChannelMode(char sign, char mode, std::string user, std::string chan )
{
	if (sign == '+')
	{
		if (mode == 'o')
		{
			this->server.getChannel(chan)->second.clientOperators.insert(user);
			this->server.getChannel(chan)->second.sendMessageToClients(this->getPrefix() + RPL_CHANNELMODEIS(this->userInfos.nickName, chan, "+o"), "");
			return (0);
		}
	}
	else if (sign == '-')
	{
		if (mode == 'o')
		{
			this->server.getChannel(chan)->second.clientOperators.erase(user);
			this->server.getChannel(chan)->second.sendMessageToClients(this->getPrefix() + RPL_CHANNELMODEIS(this->userInfos.nickName, chan, "-o"), "");
			return(0);
		}
	}
	else
		sendMessage(getPrefix() + " 472 " + mode + " :is unknown mode char to me\r\n");
	return (0);
}

int		Client::handleInviteOnlyMode(char sign, char mode, std::string chan )
{
	if (sign == '+')
	{
		if (mode == 'i')
		{
			this->server.getChannel(chan)->second.isInviteOnly = true;
			this->server.getChannel(chan)->second.sendMessageToClients(this->getPrefix() + RPL_CHANNELMODEIS(this->userInfos.nickName, chan, "+i"), "");
			return (0);
		}
	}
	else if (sign == '-')
	{
		if (mode == 'i')
		{
			this->server.getChannel(chan)->second.isInviteOnly = false;
			this->server.getChannel(chan)->second.sendMessageToClients(this->getPrefix() + RPL_CHANNELMODEIS(this->userInfos.nickName, chan, "-1"), "");
			return(0);
		}
	}
	else
		sendMessage(getPrefix() + " 472 " + mode + " :is unknown mode char to me\r\n");
	return (0);
}

int		Client::updateUserModes(char sign, char mode)
{
	std::string character(1, mode);
	if (sign == '+')
	{
		if (mode == 'i')
		{
			this->userInfos.invisibleMode = true;
			sendMessage(getPrefix() + RPL_UMODEIS(this->userInfos.nickName, "+", character));
		}
		else
		{
			if (mode !='o')
				sendMessage(getPrefix() + " 472 " + character + " :is unknown mode char to me\r\n");
		}
	}
	else
	{
		if (mode == 'i')
		{
			this->userInfos.invisibleMode = false;
			sendMessage(getPrefix() + RPL_UMODEIS(this->userInfos.nickName, "-", character));
		}
		if (mode == 'o' and this->userInfos.operatorMode == true)
		{
			this->userInfos.operatorMode = false;
			sendMessage(getPrefix() + RPL_UMODEIS(this->userInfos.nickName, "-", character));
		}
		else if (mode != 'i' and mode != 'o')
		{
			sendMessage(getPrefix() + " 472 " + character + " :is unknown mode char to me\r\n");
		}
	}
	return (1);
}


// CASE 1) Not enough parameter.
// CASE 2) Channel does not exist.
// CASE 3) Client asking is not on the channel.
// CASE 4) Client asking is not channel operator.
// CASE 5) If +o/-o, User we want to change mode is not on the channel.
bool	Client::parsingErrorChannel(std::vector<std::string> cmd)
{
	if (cmd.size() == 2)
	{
		if (this->server.isChannelExisting(cmd[1]) == true)
		{
			if (this->server.getChannel(cmd[1])->second.isInviteOnly == true)
				sendMessage(this->getPrefix() + RPL_CHANNELMODEIS(this->userInfos.nickName, cmd[1], "+i"));
			else
				sendMessage(this->getPrefix() + RPL_CHANNELMODEIS(this->userInfos.nickName, cmd[1], "-i"));
			return (false);
		}
	}
	std::string  channelName = cmd[1];
	toLowerStr(channelName);
	if (!this->server.isChannelExisting(channelName))
	{
		sendMessage(ERR_NOSUCHCHANNEL(channelName));
		return (false);
	}
	if (cmd.size() < 3)
	{
		sendMessage(ERR_NEEDMOREPARAMS("MODE"));
		return (false);
	}
	if (!this->server.getChannel(channelName)->second.isclientConnected(this->userInfos.nickName))
	{
		sendMessage(ERR_NOTONCHANNEL(channelName));
		return (false);
	}
	if (!this->server.getChannel(channelName)->second.isClientOperatorChannel(this->userInfos.nickName))
	{
		sendMessage(getPrefix() + ERR_CHANOPRIVSNEEDED(channelName, this->userInfos.nickName));
		return(false);
	}
	if (cmd.size() == 4)
	{
		if (!this->server.getChannel(channelName)->second.isclientConnected(cmd[3]))
		{
			sendMessage(ERR_NOSUCHNICK(cmd[3]));
			return (false);
		}
	}
	return (true);
}

bool	Client::addUserMode(std::vector<std::string> cmd)
{
	if (cmd.size() < 3)
	{
		sendMessage(ERR_NEEDMOREPARAMS("MODE"));
		return (false);
	}
	if (cmd[1].compare(this->userInfos.nickName) != 0)
	{
		sendMessage(ERR_USERSDONTMATCH(this->userInfos.nickName));
		return (false);
	}
	if (cmd.size() == 3)
	{
		parseModes(cmd[2], USER_MODE, "", "");
		return(true);
	}
	else if (cmd.size() > 3)
	{
		for (std::vector<std::string>::iterator it = cmd.begin() ; it != cmd.end(); it++)
			parseModes(*it, USER_MODE, "", "");
		return (true);
	}
	return (true);
}

bool	Client::addChannelMode(std::vector<std::string> cmd)
{
	if (!parsingErrorChannel(cmd))
		return(false);
	std::string targetName = "";
	if (cmd.size() == 4)
		targetName = cmd[3];
	parseModes(cmd[2], CHANNEL_MODE, targetName, cmd[1]);
	return(true);
}

// cmd : MODE <target> <mode changes>
bool	Client::cmdMODE(std::vector<std::string> &cmd)
{
	if (this->status != CONNECTED)
		return(false);
	if (cmd.size() < 2)
	{
		sendMessage(ERR_NEEDMOREPARAMS("MODE"));
		return (false);
	}
	if (cmd[1].find("#") != std::string::npos)
		this->addChannelMode(cmd);
	else
		this->addUserMode(cmd);
	return (true);
}
