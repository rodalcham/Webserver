#include "../include/Webserv.hpp"
#include "../include/log.hpp"


uint16_t	ft_htons(uint16_t port)
{
	uint16_t test = 1;

	bool isLittleEndian = *(reinterpret_cast<uint8_t*>(&test)) == 1;

	if (isLittleEndian) {
		return (port >> 8) | (port << 8);
	} else {
		return port;
	}
}

void	debug(string message)
{
	if	(DEBUG)
	{
		static	Log	log;

		log.event(message);
		cout << "\n" << message << "\n";
	}
}