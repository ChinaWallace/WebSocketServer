#ifndef _DATAFRAME_H__
#define _DATAFRAME_H__

#define byte char
#define sbyte unsigned char

class DataFrameHeader
{
public:
        bool _fin;//描述消息是否结束,如果为1则该消息为消息尾部,如果为零则还有后续数据包
        bool _rsv1;//以下3位是用于扩展定义的,如果没有扩展约定的情况则必须为0
        bool _rsv2;
        bool _rsv3;
        sbyte _opcode;
        bool _maskcode;
        sbyte _payloadlength;

		DataFrameHeader(){}
		DataFrameHeader(byte buffer[], int nBufLen);
		DataFrameHeader(bool fin,bool rsv1,bool rsv2,bool rsv3,sbyte opcode,bool hasmask,int length);

		byte* GetBytes();
};


class DataFrame
{
public:
    DataFrameHeader _header;
    byte* _extend;
    byte* _mask;
    const byte* _content;
	int nExtendLen;
	int nMaskLen;
	int nContentLen;

	//DataFrame(byte* buffer, int nBufLen);
	DataFrame(const char* content, int nContentLen);

	byte* GetBytes(int& nTotalSize);
	byte* Mask(byte* data, int nDataLen, byte* mask);
};
#endif