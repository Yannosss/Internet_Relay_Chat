#include "server.hpp"
// TBD passer class dans fichiers .cpp et .hpp a part


Server::Server()
//bufferSize(BUFFER_SIZE)
{

}

std::vector<Client>::iterator Server::getClient(int fd)
{
	std::vector<Client>::iterator it;
	for (it = this->clientList.begin(); it != this->clientList.end(); it++)
		{
			if (it->socketFd == fd)
				return (it);
		}
	std::cout << BOLD_RED << "fd non trouve dans getclient -> /!\\ pour suite execution (return getClient non valid)" << RESET << std::endl;
	return (it); // return 1er client si fd non trouvé
}
std::vector<Client>::iterator Server::getClient(std::string user)
{
	std::vector<Client>::iterator it;
	for (it = this->clientList.begin(); it != this->clientList.end(); it++)
		{
			if (it->userInfos.userName == user)
				return (it);
		}
	std::cout << BOLD_RED << "user non trouve dans getclient -> /!\\ pour suite execution (return getClient non valid)" << RESET << std::endl;
	return (it); // return 1er client si fd non trouvé
}

void Server::set_port(int port)
{
	this->port = port; 
}

void Server::set_password(std::string password)
{
	this->password = password;
}

std::string Server::get_password(void)
{
	return (this->password);
}

void Server::init()
{
	//memset(this->buffer, 0, bufferSize); // TBD voir si il faut pas remplir de bufferS0ze * sizeof(int)
	this->serverFd = socket(AF_INET, SOCK_STREAM, 0);
	if (this->serverFd < 0)
	{
		std::cout << BOLD_RED << "Error while creating socket" << RESET << std::endl;
		exit(EXIT_FAILURE);
	}
	std::cout << BOLD_GREEN << "Socket server created successfully" << RESET << std::endl;
	this->serverAddr.sin_family = AF_INET;
	this->serverAddr.sin_port = htons(this->port);
	this->serverAddr.sin_addr.s_addr = htonl(INADDR_ANY);
	if (bind(this->serverFd, (struct sockaddr *)&(this->serverAddr), sizeof(this->serverAddr)) < 0)
	{
		std::cout << BOLD_RED << "Error while bindind socket to Port " << this->port << std::endl;
		perror("bind");
		std::cout << RESET << std::endl;
		exit(EXIT_FAILURE);
	}
	std::cout << BOLD_GREEN << "Socket bind successfully to port " << this->port << RESET << std::endl;
	// ecoute sur Fd du server
	if (listen(this->serverFd, 5) < 0) // TBD pourquoi 5 ?
	{
		std::cout << BOLD_RED << "Error while listening" << std::endl;
		perror("listen");
		std::cout << RESET << std::endl;
		exit(EXIT_FAILURE);
	}
	// ajout du fd server dans fdListened
	this->fdListened.push_back(pollfd());
	this->fdListened[0].fd = this->serverFd;
	this->fdListened[0].events = POLLIN; // event attendu = POLLIN

	// TBD fonction pour print les attributs du server
}

void Server::run()
{
	this->fdListened.reserve(100);

	int i = 0; // limite pour eviter de kill process en debug, TBD mettre loop infinie
	// TBD redefinir ctrl+C pour close les fd (evite de bloquer les ports en debug)
	
	int pollreturn = -1;
	while (i < 30)
	{
		std::cout << BOLD_BLUE << "loop step " << i << BLUE << " (1 loop = " << LISTENING_TIMEOUT/1000 << "s)  Waiting for event ..." << RESET << std::endl;
		pollreturn = poll(&(this->fdListened[0]), this->fdListened.size(), LISTENING_TIMEOUT);
		for (std::vector<pollfd>::iterator it = this->fdListened.begin(); it != this->fdListened.end(); it++)
		{
			std::cout << "event catch on fd " << it->fd << " : " << it->revents << std::endl;

		}
		if(pollreturn < 0)
		{
			std::cout << BOLD_RED << "Error with poll()" << std::endl;
			perror("poll");
			std::cout << RESET << std::endl;
			exit(EXIT_FAILURE);
		}
		if(pollreturn > 0)
		{
			std::cout << BOLD_BLUE << "Event detected by poll \n" << RESET;
			this->manage_poll_event();
			//this->printState();
		}
		if(pollreturn == 0) // TBD bloc pour debug, mais a supprimer car inutile a la fin
		{
			std::cout << BOLD_BLUE << "Poll delay expired \n\n" << RESET;

		}

		
		// temporisation pour debug (TBD a enlever a la fin)
		sleep(1); // sleep 1s
		i++;
	}
}

void Server::manage_poll_event()
{
	this->fdListened.reserve(this->fdListened.size() + 1);
	for (std::vector<pollfd>::iterator it = this->fdListened.begin(); it != this->fdListened.end(); it++)
	{
		if (it->revents == POLLIN)
		{
			if (it == this->fdListened.begin())
			{
				this->addNewClient();
			}
			else
			{
				this->listen_client(*(this->getClient(it->fd)));
			}
		}
	}
	
	/*std::vector<pollfd>::iterator ite = this->fdListened.begin();
	for (size_t i = 0; i < fdListened.size(); i++, ite++)
	{
		if(ite->revents == -1)
		{
			close(ite->fd);
			this->fdListened.erase(ite);

			std::vector<Client>::iterator it  = std::find(this->clientList.begin(), this->clientList.end(), *(this->getClient(ite->fd)) );
			if (it != clientList.end())
    		{	
				this->clientList.erase(it);
			}
		}
	}*/
}

void Server::addNewClient()
{
	Client newClient(*this);


	//-- Tentative d'acceptation du nouveau client
	newClient.clientSize = sizeof(newClient.clientAddr);
	std::cout << "check fd" << this->fdListened[0].fd << std::endl;
	if ((newClient.socketFd = accept(this->fdListened[0].fd, (struct sockaddr *)&newClient.clientAddr, &newClient.clientSize)) < 0)
	{
		perror("accept");
		std::cout << BOLD_RED << "Error while accepting connection" << RESET << std::endl;
		exit(EXIT_FAILURE);
	}
	this->clientList.push_back(newClient);

	this->fdListened.push_back(pollfd());
	//this->fdListened[1].fd = newClient.socketFd;
	(--this->fdListened.end())->fd = newClient.socketFd;
	(--this->fdListened.end())->events = POLLIN; // event attendu = POLLIN
	//this->authentication(*(--this->clientList.end()));
}

void Server::listen_client(Client &client)
{
		clear_str(client.buffer, client.bufferSize);
		recv(client.socketFd, client.buffer, client.bufferSize - 1, 0);
		std::cout << BOLD_YELLOW << "buffer read: -->" << YELLOW << client.buffer << BOLD_YELLOW << "<--" << RESET << std::endl;
		client.cmd += client.buffer;
		replace_rn_by_n(client.cmd);
		if (client.cmd.find("\n") != std::string::npos)
		{
			std::string cc = pop_command(client.cmd);
			std::cout << BOLD_YELLOW << "launched command: -->" << YELLOW << "|" << cc << "|" << BOLD_YELLOW << "<--" << RESET << std::endl;
			client.launchCommand(cc);
		}
}

void Server::terminate()
{
	while(this->fdListened.size() > 0)
	{
		std::cout << BOLD_YELLOW << "Close fd " << (*(--(this->fdListened.end()))).fd << RESET << std::endl;
		close((*(--(this->fdListened.end()))).fd); // close last fd
		this->fdListened.pop_back(); // remove last fd
	}
}

bool	Server::isChannelExisting(std::string name)
{
	if (this->channelList.count(name) >= 1)
		return (true);
	return false;
}

void	Server::addChannel(std::string name)
{
	if (this->isChannelExisting(name))
		return;
	Channel chan = Channel(*this); // TBD initialiser les attributs du channel si on en ajoute
	this->channelList.insert(std::make_pair(name, chan));
}

Channel& Server::getChannel(std::string name)
{
	return((this->channelList.find(name))->second);
}



void Server::printState()
{
	std::cout << BOLD_BLUE << "Server state after command" << RESET << std::endl;
	// server parameters
	std::cout << BLUE_PIPE << BOLD_PURPLE << "Server parameters" << RESET << std::endl;
	std::cout << BLUE_PIPE << PURPLE << "  socketFd " << socketFd << " | serverFd " << serverFd << std::endl;
	std::cout << BLUE_PIPE << PURPLE << "  fdListened (fd,events,revents): "<< PURPLE << "[";
	for (std::vector<pollfd>::iterator it=this->fdListened.begin(); it != this->fdListened.end(); it++)
	{
		std::cout << "(" << it->fd << "," << it->events << "," << it->revents << ")";
		if(it + 1 != this->fdListened.end())
			std::cout << ", ";
		else
			std::cout << "]";
	}
	std::cout << RESET << std::endl;

	// Clients parameters
	std::cout << BLUE_PIPE << BOLD_GREEN << "Clients" << RESET << std::endl;
	std::cout << BLUE_PIPE << GREEN << "  clientList (by fd):" << "[";
	if (this->clientList.begin() == this->clientList.end())
	{
		std::cout << "empty";
	}
	else
	{
		for (std::vector<Client>::iterator it=this->clientList.begin(); it != this->clientList.end(); it++)
		{
			std::cout << it->socketFd;
			if(it + 1 != this->clientList.end())
				std::cout << ", ";
		}
	}
	std::cout << "]";
	std::cout << RESET << std::endl;

	// Channels parameters
	std::cout << BLUE_PIPE << BOLD_YELLOW << "Channels" << RESET << std::endl;
	std::cout << BLUE_PIPE << YELLOW << "  channelList (by name):" << "[";
	if (this->channelList.begin() == this->channelList.end())
	{
		std::cout << "empty";
	}
	else
	{
		for (std::map<std::string, Channel>::iterator it=this->channelList.begin(); it != this->channelList.end(); it++)
		{
			std::cout << it->first;
			if(++it != this->channelList.end())
				std::cout << ", ";
			--it;
		}
	}
	std::cout << "]";

	std::cout << RESET << std::endl << std::endl;


}
