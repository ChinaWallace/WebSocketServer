#ifndef __WEBSOCKET_REQUEST__
#define __WEBSOCKET_REQUEST__

#include <vector>

class Websocket_Request {
public:
	Websocket_Request();
	~Websocket_Request();
	int fetch_websocket_info(std::vector<char>& data);
	void reset(); // ÖØÖÃ [2/21/2017 cai]

public:
	std::string getDataStr()
	{
		return dataStr;
	}

private:
	int fetch_fin(char *msg, int &pos);
	int fetch_opcode(char *msg, int &pos);
	int fetch_mask(char *msg, int &pos);
	int fetch_masking_key(char *msg, int &pos);
	int fetch_payload_length(char *msg, int &pos);
	int fetch_payload(char *msg, int &pos);
private:
	uint8_t fin_;
	uint8_t opcode_;
	uint8_t mask_;
	uint8_t masking_key_[4];
	uint64_t payload_length_;
	char payload_[2048];
	std::string dataStr;

public:
	uint8_t getOpenCode();
};

#endif
