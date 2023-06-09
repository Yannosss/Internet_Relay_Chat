#include "client.hpp"

bool	Client::cmdPASS(std::vector<std::string> &cmd)
{
	if (this->status == COMING)
	{
		if (cmd.empty() or cmd.size() == 1)
		{
			sendMessage(ERR_NEEDMOREPARAMS("PASS"));
			this->online = false;
			return (false);
		}
		if (cmd.size() == 2)
		{
			if (this->status == REGISTERED)
			{
				sendMessage(ERR_ALREADYREGISTERED);
				return (true);
			}
			if (this->server.get_password().compare(cmd[1]) == 0)
			{
				this->status = REGISTERED;
				return (true);
			}
			else
			{
				sendMessage(this->getPrefix() + " 464 " + this->userInfos.nickName + ERR_PASSWDMISMATCH);
				this->status = COMING;
				this->online = false;
				return(false);
			}
		}		
	}
	return (false);
}