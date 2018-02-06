#include "websocket_request.h"
#include <WinSock2.h>

Websocket_Request::Websocket_Request():
		fin_(),
		opcode_(),
		mask_(),
		masking_key_(),
		payload_length_(),
		payload_()
{
}

Websocket_Request::~Websocket_Request(){

}

int Websocket_Request::fetch_websocket_info(std::vector<char>& data)
{
	int pos = 0;
	int size = data.size();
	char* msg = &data[0];

	for (;pos<size;)
	{
		fetch_fin(msg, pos);
		fetch_opcode(msg, pos);
		fetch_mask(msg, pos);
		fetch_payload_length(msg, pos);
		fetch_masking_key(msg, pos);

		if (pos + payload_length_ > size)
		{
			break;
		}

		fetch_payload(msg, pos);
	}

	if (pos != 0)
	{
		data.erase(data.begin(), data.begin() + pos);
	}

	return 0;
}

uint8_t Websocket_Request::getOpenCode() {
	return opcode_;
}

void Websocket_Request::reset(){
	fin_ = 0;
	opcode_ = 0;
	mask_ = 0;
	memset(masking_key_, 0, sizeof(masking_key_));
	payload_length_ = 0;
	memset(payload_, 0, sizeof(payload_));
}

int Websocket_Request::fetch_fin(char *msg, int &pos){
	fin_ = (unsigned char)msg[pos] >> 7;
	return 0;
}

int Websocket_Request::fetch_opcode(char *msg, int &pos){
	opcode_ = msg[pos] & 0x0f;
	pos++;
	return 0;
}

int Websocket_Request::fetch_mask(char *msg, int &pos){
	mask_ = (unsigned char)msg[pos] >> 7;
	return 0;
}

int Websocket_Request::fetch_masking_key(char *msg, int &pos){
	if(mask_ != 1)
		return 0;
	for(int i = 0; i < 4; i++)
		masking_key_[i] = msg[pos + i];
	pos += 4;
	return 0;
}

int Websocket_Request::fetch_payload_length(char *msg, int &pos){
	payload_length_ = msg[pos] & 0x7f;
	pos++;
	if(payload_length_ == 126){
		uint16_t length = 0;
		memcpy(&length, msg + pos, 2);
		pos += 2;
		payload_length_ = ntohs(length);
	}
	else if(payload_length_ == 127){
		uint32_t length = 0;
		memcpy(&length, msg + pos, 4);
		pos += 4;
		payload_length_ = ntohl(length);
	}
	return 0;
}

int Websocket_Request::fetch_payload(char *msg, int &pos){
	memset(payload_, 0, sizeof(payload_));
	if(mask_ != 1){
		memcpy(payload_, msg + pos, payload_length_);
	}
	else {
		for(int i = 0; i < payload_length_; i++){
			int j = i % 4;
			payload_[i] = msg[pos + i] ^ masking_key_[j];
		}
	}

	dataStr += payload_;

	pos += payload_length_;
	return 0;
}
