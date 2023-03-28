#include "client.hpp"

// INFO POUR LES CHANNELS 
// Servers MUST process the parameters of this command as lists on incoming commands from
// clients, with the first <key> being used for the first <channel>, the second <key> being
// used for the second <channel>, etc.
// While a client is joined to a channel, they receive all relevant information about that
// channel including the JOIN, PART, KICK, and MODE messages affecting the channel. They
// receive all PRIVMSG and NOTICE messages sent to the channel, and they also receive QUIT
// messages from other clients joined to the same channel (to let them know those users have
// left the channel and the network). This allows them to keep track of other channel members
// and channel modes.

// Syntaxe et fonctionnemen fonction:
// syntaxe <JOIN> <[&, #, +, !]channel1> <other> ....
// syntaxe <JOIN> <&channel1> <other> ....
// seul le 1er channel est ajouté, les other sont ignorés pour coller au comportement d'Irssi
// Erreur gerees:
// ERR_NEEDMOREPARAMS
// Erreurs non gerees
// ERR_TOOMANYCHANNELS
// ERR_BANNEDFROMCHAN : Returned to indicate that a JOIN command failed because the
// client has been banned from the channel and has not had a ban exception set for them.
// The text used in the last param of this message may vary.
// ERR_INVITEONLYCHAN : Returned to indicate that a JOIN command failed because the channel
// is set to [invite-only] mode and the client has not been invited to the channel or had
// an invite exception set for them. The text used in the last param of this message may vary.
// ERRFULL : TBD voir avec Arzu ce que c'est ?
// Note that this command also accepts the special argument of ("0", 0x30) instead of any of 
// the usual parameters, which requests that the sending client leave all channels they are
// currently connected to. The server will process this command as though the client had sent
// a PART command for each channel they are a member of.
 
// This message may be sent from a server to a client to notify the client that someone has
// joined a channel. In this case, the message <source> will be the client who is joining, and
// <channel> will be the channel which that client has joined
// Servers SHOULD NOT send multiple channels in this message to clients, and SHOULD distribute
// these multiple-channel JOIN messages as a series of messages with a single channel name on each.
 
// Des qu'il a rejoint on doit envoyer une message "<source> joined the channel" doit etre envoye
// ainsi que le topic du channel (avec RPL_TOPIC) et la liste des utilisateurs connectes (avc RPL_NAMREPLY
// suivi d'un RPL_ENDOFNAMES)

//comportement
// rejoind le channel dans cmd[1]. les channels suivants (cmd[2], cmd[3]...) ne sont pas rejoints
// pour coller au comportement d'irssi
bool	Client::cmdJOIN(std::vector<std::string> &cmd)
{
	if (cmd.size() <=1)
	{
		sendMessage(ERR_NEEDMOREPARAMS("JOIN"));
		return (false);
	}
	std::string channel_name = cmd[1];
	toLowerStr(channel_name);
	if (cmd[1].size() <= 1 || !isChannelName(channel_name))
	{
		this->sendMessage(ERR_BADCHANMASK(cmd[1]));
		return (false);
	}
	this->server.addChannel(channel_name); // check existence channel fait dans addChannel
	this->server.getChannel(channel_name)->second.addClient(*this);

	
	
	return (true);
}
